/* Copyright (c) 2018 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_KERNEL_SRC_DEV_KEYBOARD_C
#define GUARD_KERNEL_SRC_DEV_KEYBOARD_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/align.h>
#include <kos/types.h>
#include <kos/keymap.h>
#include <kos/keyboard.h>
#include <kos/keyboard-ioctl.h>
#include <kernel/malloc.h>
#include <dev/keymap.h>
#include <dev/keyboard.h>
#include <kernel/heap.h>
#include <kernel/user.h>
#include <except.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <hybrid/section.h>

/* Keyboard Abstract Kernel API Interface. */

DECL_BEGIN


PRIVATE void KCALL
keymap_copyregion(struct vm_region *__restrict region,
                  struct keyboard_keymap *__restrict map) {
 vm_vpage_t vpage;
 STATIC_ASSERT(CEILDIV(sizeof(struct vm_region),PAGESIZE) == 1);
 assert(region->vr_size == 1);
 /* Temporarily map the new keymap into the kernel address space. */
 vpage = task_temppage();
 vm_acquire(&vm_kernel);
 TRY {
  pagedir_mapone(vpage,
                 region->vr_parts->vp_phys.py_iscatter[0].ps_addr,
                 PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE);
 } FINALLY {
  vm_release(&vm_kernel);
 }
 pagedir_syncone(vpage);
 /* Copy data from the default keymap. */
 memcpyl((void *)VM_PAGE2ADDR(vpage),map,
          sizeof(struct keyboard_keymap)/4);
 if (sizeof(struct keyboard_keymap) < PAGESIZE) {
  memsetl((byte_t *)VM_PAGE2ADDR(vpage)+sizeof(struct keyboard_keymap),
          0,(PAGESIZE-sizeof(struct keyboard_keymap))/4);
 }
}

/* Allocate a new keymap.
 * NOTE: These functions allocate/free whole pages, meaning that the returned
 *       keyboard map is suitable for mapping in user-space as a USHARE segment.
 * NOTE: The data contained within the region is a `struct keyboard_keymap' */
PUBLIC PAGE_ALIGNED ATTR_RETNONNULL
REF struct vm_region *KCALL keymap_alloc(void) {
 REF struct vm_region *result;
 result = vm_region_alloc(1);
 TRY {
  result->vr_flags |= (VM_REGION_FCANTSHARE|VM_REGION_FDONTMERGE);
  result->vr_part0.vp_state                       = VM_PART_INCORE;
  result->vr_part0.vp_phys.py_num_scatter         = 1;
  result->vr_part0.vp_phys.py_iscatter[0].ps_addr = page_malloc(1,MZONE_ANY);
  result->vr_part0.vp_phys.py_iscatter[0].ps_size = 1;
  keymap_copyregion(result,&default_keymap);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  vm_region_decref(result);
  error_rethrow();
 }
 return result;
}


PUBLIC bool KCALL
keyboard_loadmap(struct keyboard *__restrict self,
                 USER CHECKED void *blob) {
 bool result;
 struct heapptr temp_map;
 /* Allocate a temporary buffer for the new keymap in the heap.
  * Since a keymap is as large as it is, better not put it on
  * the stack, just so we don't run into any troubles when it
  * comes to small amounts of remaining stack memory. */
 temp_map = heap_alloc_untraced(&kernel_heaps[GFP_SHARED],
                                 sizeof(struct keyboard_keymap),
                                 GFP_SHARED);
 TRY {
  /* Load the given blob. */
  result = keymap_load_blob((struct keyboard_keymap *)temp_map.hp_ptr,blob);
  /* If that worked, copy the new keymap into the keyboard. */
  if (result)
      keymap_copyregion(self->k_map,(struct keyboard_keymap *)temp_map.hp_ptr);
 } FINALLY {
  heap_free(&kernel_heaps[GFP_SHARED],
             temp_map.hp_ptr,
             temp_map.hp_siz,
             GFP_SHARED);
 }
 return result;
}



PRIVATE void KCALL
keyboard_fini(struct character_device *__restrict self) {
 struct keyboard *me = (struct keyboard *)self;
 if (me->k_ops && me->k_ops->ko_fini)
   (*me->k_ops->ko_fini)(me);
 if (me->k_map)
     vm_region_decref(me->k_map);
}
PRIVATE ssize_t KCALL
keyboard_ioctl(struct character_device *__restrict self,
               unsigned long cmd,
               USER UNCHECKED void *arg,
               iomode_t flags) {
 struct keyboard *me = (struct keyboard *)self;
 switch (cmd) {

 case KEYBOARD_ENABLE_SCANNING:
  keyboard_enable_scanning(me,flags);
  break;
 case KEYBOARD_DISABLE_SCANNING:
  keyboard_disable_scanning(me,flags);
  break;

 case KEYBOARD_GET_LEDS:
  validate_writable(arg,sizeof(keyboard_ledset_t));
  *(keyboard_ledset_t *)arg = keyboard_getleds(me);
  break;
 case KEYBOARD_SET_LEDS:
  validate_readable(arg,2*sizeof(keyboard_ledset_t));
  keyboard_setleds(me,
                 ((keyboard_ledset_t *)arg)[0],
                 ((keyboard_ledset_t *)arg)[1],
                   flags);
  break;

 case KEYBOARD_GET_MODE:
  validate_writable(arg,sizeof(u16));
  *(u16 *)arg = keyboard_getmode(me);
  break;
 case KEYBOARD_SET_MODE:
  validate_readable(arg,2*sizeof(u16));
  keyboard_setmode(me,((u16 *)arg)[0],((u16 *)arg)[1],flags);
  break;

 case KEYBOARD_GET_DELAY:
  validate_writable(arg,sizeof(struct kbdelay));
  *(struct kbdelay *)arg = keyboard_getrepeat(me);
  break;
 case KEYBOARD_SET_DELAY:
  validate_readable(arg,sizeof(struct kbdelay));
  keyboard_setrepeat(me,*(struct kbdelay *)arg,flags);
  break;

 case KEYBOARD_LOAD_KEYMAP:
  validate_readable(arg,sizeof(Kmp_Header));
  if (!keyboard_loadmap(me,arg))
       error_throw(E_INVALID_ARGUMENT);
  break;

 default:
  error_throw(E_NOT_IMPLEMENTED);
  break;
 }
 return 0;
}
PRIVATE void KCALL
keyboard_stat(struct character_device *__restrict self,
              USER CHECKED struct stat64 *result) {
 struct keyboard *me = (struct keyboard *)self;
 result->st_size    = ATOMIC_READ(me->k_buf.wb_state.bs_rcnt);
 result->st_blksize = MAX_INPUT;
 result->st_blocks  = 1;
}

PRIVATE size_t KCALL
keyboard_read(struct character_device *__restrict self,
              USER CHECKED void *buf, size_t bufsize,
              iomode_t flags) {
 struct keyboard *me = (struct keyboard *)self;
 size_t result = 0;
 keyboard_key_t *dst = (keyboard_key_t *)buf;
 while (bufsize >= sizeof(keyboard_key_t)) {
  keyboard_key_t key;
  key = wordbuffer_trygetword(&me->k_buf);
  if (key == KEY_UNKNOWN) {
   if (flags & IO_NONBLOCK) break; /* Don't block. */
   if (result) break; /* Something was already read. */
   /* Do a blocking read. */
   key = wordbuffer_getword(&me->k_buf);
  }
  COMPILER_READ_BARRIER();
  *dst++ = key;
  COMPILER_WRITE_BARRIER();
  result  += sizeof(keyboard_key_t);
  bufsize -= sizeof(keyboard_key_t);
 }
 return result;
}

PRIVATE size_t KCALL
keyboard_write(struct character_device *__restrict self,
               USER CHECKED void const *buf, size_t bufsize,
               iomode_t flags) {
 struct keyboard *me = (struct keyboard *)self;
 size_t result = 0;
 keyboard_key_t *src = (keyboard_key_t *)buf;
 while (bufsize >= sizeof(keyboard_key_t)) {
  keyboard_key_t key = *src++;
  COMPILER_BARRIER();
  /* Make sure not to write `KEY_UNKNOWN', as it has special meaning. */
  if unlikely(key == KEY_UNKNOWN)
     error_throw(E_INVALID_ARGUMENT);
  /* Add the key to the buffer. */
  wordbuffer_putword(&me->k_buf,key);
  result  += sizeof(keyboard_key_t);
  bufsize -= sizeof(keyboard_key_t);
 }
 return result;
}

PRIVATE REF struct vm_region *KCALL
keyboard_mmap(struct character_device *__restrict self,
              /*vm_raddr_t*/uintptr_t page_index,
              /*vm_raddr_t*/uintptr_t *__restrict pregion_start) {
 REF struct vm_region *result;
 struct keyboard *me = (struct keyboard *)self;
 /* Make sure that the caller actually requested the mapping of the keymap. */
 switch (page_index) {

 case VM_ADDR2PAGE(KEYBOARD_KEYMAP_OFFSET):
  /* Map the keymap. */
  *pregion_start = 0;
  vm_region_incref(me->k_map);
  result = me->k_map;
  break;

 default:
  error_throw(E_INVALID_ARGUMENT);
 }
 return result;
}



PRIVATE struct character_device_ops keyboard_character_ops = {
    .c_fini = &keyboard_fini,
    .c_file = {
        .f_ioctl = &keyboard_ioctl,
        .f_stat  = &keyboard_stat,
        .f_read  = &keyboard_read,
        .f_write = &keyboard_write,
        .f_mmap  = &keyboard_mmap,
        /* XXX: Polling for asynchronous signals? */
    }
};


/* Allocate and pre/null/zero-initialize a new keyboard device,
 * who's control structure has a size of `struct_size' bytes.
 * The caller must initialize the following members:
 *    - k_dev.c_device.d_namebuf      (Unless `d_name' is set directly)
 *    - k_dev.c_device.d_name         (Optionally; pre-initialized to `d_namebuf')
 *    - k_dev.c_device.d_devno        (See explanation below)
 * @throw: E_DRIVER_CLOSED: The given driver has been closed/is closing.
 * Following this, a device ID should be allocated for, or assigned
 * to the device, either by directly setting `return->d_device.d_devno',
 * or by calling `devno_alloc()', or `register_dynamic_device()' (see above.) */
PUBLIC ATTR_RETNONNULL REF struct keyboard *
(KCALL keyboard_alloc)(size_t struct_size,
                       struct keyboard_ops *__restrict ops,
                       struct driver *__restrict caller) {
 REF struct keyboard *result;
 assert(struct_size >= sizeof(struct keyboard));
 assert(ops);
 /* Assert mandatory callbacks. */
 assert(ops->ko_enable_scanning);
 assert(ops->ko_disable_scanning);
 /* Allocate a new keyboard. */
 result = (REF struct keyboard *)__character_device_alloc(struct_size,
                                                          caller);
 TRY {
  /* Allocate the keyboard map (must be done prior to keyboard ops being assigned) */
  result->k_map = keymap_alloc();
  COMPILER_WRITE_BARRIER();
  /* Initialize special members. */
  result->k_ops       = ops;
  result->k_dev.c_ops = &keyboard_character_ops; /* Use keyboard character device ops. */
  wordbuffer_cinit(&result->k_buf);
  mutex_cinit(&result->k_lock);
  result->k_mode = KEYBOARD_FINITIAL;
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  keyboard_decref(result);
  error_rethrow();
 }
 return result;
}

/* Enable/Disable scanning of keycodes. */
PUBLIC void KCALL
keyboard_enable_scanning(struct keyboard *__restrict self, iomode_t flags) {
 mutex_getf(&self->k_lock,flags);
 TRY {
  if (self->k_mode & KEYBOARD_FDISABLED) {
   if (flags & IO_NONBLOCK)
       error_throw(E_WOULDBLOCK);
   (*self->k_ops->ko_enable_scanning)(self);
   ATOMIC_FETCHAND(self->k_mode,~KEYBOARD_FDISABLED);
  }
 } FINALLY {
  mutex_put(&self->k_lock);
 }
}
PUBLIC void KCALL
keyboard_disable_scanning(struct keyboard *__restrict self, iomode_t flags) {
 mutex_getf(&self->k_lock,flags);
 TRY {
  if (!(self->k_mode & KEYBOARD_FDISABLED)) {
   if (flags & IO_NONBLOCK)
       error_throw(E_WOULDBLOCK);
   (*self->k_ops->ko_disable_scanning)(self);
   ATOMIC_FETCHOR(self->k_mode,KEYBOARD_FDISABLED);
  }
 } FINALLY {
  mutex_put(&self->k_lock);
 }
}

/* Set the mode of the given keyboard.
 * The new mode is set to `(old_mode & mask) | mode'
 * @param: mask: Mask of mode bits to keep.
 * @param: mode: One of `KEYBOARD_MODE_F*', or'd with any of `KEYBOARD_F*'
 * HINT: `keyboard_setmode(...,~0,KEYBOARD_FDISABLED)' is the same as calling `keyboard_disable_scanning()'
 * HINT: `keyboard_setmode(...,~KEYBOARD_FDISABLED,0)' is the same as calling `keyboard_enable_scanning()' */
PUBLIC void KCALL
keyboard_setmode(struct keyboard *__restrict self,
                 unsigned int mask, unsigned int mode,
                 iomode_t flags) {
 mutex_getf(&self->k_lock,flags);
 TRY {
  u16 old_mode = self->k_mode;
  u16 new_mode = (old_mode & mask) | mode;
  if ((new_mode & KEYBOARD_FDISABLED) &&
     !(old_mode & KEYBOARD_FDISABLED)) {
   if (flags & IO_NONBLOCK)
       error_throw(E_WOULDBLOCK);
   (*self->k_ops->ko_disable_scanning)(self);
   self->k_mode |= KEYBOARD_FDISABLED;
  } else if (!(new_mode & KEYBOARD_FDISABLED) &&
              (old_mode & KEYBOARD_FDISABLED)) {
   if (flags & IO_NONBLOCK)
       error_throw(E_WOULDBLOCK);
   (*self->k_ops->ko_enable_scanning)(self);
   self->k_mode &= ~KEYBOARD_FDISABLED;
  }
  if ((old_mode & (KEYBOARD_MODE_FUPDOWN|KEYBOARD_MODE_FDOWNONLY)) !=
      (new_mode & (KEYBOARD_MODE_FUPDOWN|KEYBOARD_MODE_FDOWNONLY))) {
   if (self->k_ops->ko_setmode) {
    if (flags & IO_NONBLOCK)
        error_throw(E_WOULDBLOCK);
    (*self->k_ops->ko_setmode)(self,mode&
                              (KEYBOARD_MODE_FUPDOWN|
                               KEYBOARD_MODE_FDOWNONLY));
   } else {
    /* Don't change the keyboard operations mode. */
    new_mode &= ~(KEYBOARD_MODE_FUPDOWN|KEYBOARD_MODE_FDOWNONLY);
    new_mode |= old_mode & (KEYBOARD_MODE_FUPDOWN|KEYBOARD_MODE_FDOWNONLY);
   }
  }
  self->k_mode = new_mode;
 } FINALLY {
  mutex_put(&self->k_lock);
 }
}

/* Set the state of keyboard LEDs.
 * The new led state is set to `(old_leds & mask) | leds' */
PUBLIC void KCALL
keyboard_setleds(struct keyboard *__restrict self,
                 keyboard_ledset_t mask,
                 keyboard_ledset_t leds,
                 iomode_t flags) {
 keyboard_ledset_t new_leds;
 if (!self->k_ops->ko_setled)
      return; /* No-op. */
 mutex_getf(&self->k_lock,flags);
 TRY {
  new_leds = (self->k_leds & mask) | leds;
  if (self->k_leds != new_leds) {
   /* Set the new LED state. */
   if (flags & IO_NONBLOCK)
       error_throw(E_WOULDBLOCK);
   (*self->k_ops->ko_setled)(self,new_leds);
   /* Save the new state. */
   self->k_leds = new_leds;
  }
 } FINALLY {
  mutex_put(&self->k_lock);
 }
}


/* Set the hardware-supported repeat capability of the keyboard. */
PUBLIC void KCALL
keyboard_setrepeat(struct keyboard *__restrict self,
                   struct kbdelay mode,
                   iomode_t flags) {
 if (!self->k_ops->ko_setdelay)
      return; /* No-op. */
 mutex_getf(&self->k_lock,flags);
 TRY {
  if (self->k_delay.kd_state != mode.kd_state) {
   /* Set the new repeat state. */
   if (flags & IO_NONBLOCK)
       error_throw(E_WOULDBLOCK);
   (*self->k_ops->ko_setdelay)(self,mode);
   /* Save the new state. */
   self->k_delay = mode;
  }
 } FINALLY {
  mutex_put(&self->k_lock);
 }
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_DEV_KEYBOARD_C */
