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
#ifndef GUARD_KERNEL_SRC_UNWIND_DEBUG_LINE_C
#define GUARD_KERNEL_SRC_UNWIND_DEBUG_LINE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <fs/linker.h>
#include <kernel/debug.h>
#include <kernel/paging.h>
#include <kernel/malloc.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <hybrid/align.h>
#include <unwind/debug_line.h>
#include <string.h>
#include <except.h>

DECL_BEGIN


/* Delete module debug information. */
PUBLIC ATTR_NOTHROW void KCALL
module_debug_delete(struct module_debug *__restrict self) {
 if (self->md_debug_line.ds_size)
     vm_unmap(VM_ADDR2PAGE((uintptr_t)self->md_data),
              CEIL_ALIGN(self->md_debug_line.ds_size,PAGESIZE),
              VM_UNMAP_NOEXCEPT|VM_UNMAP_SYNC,NULL);
 kfree(self);
}

/* Create a new debug information descriptor for `app'.
 * If `app' doesn't contain debug information, return `NULL' instead. */
FUNDEF /*inherit*/struct module_debug *KCALL
module_debug_alloc(struct application *__restrict app) {
 struct module_debug *EXCEPT_VAR result;
 struct dl_section debug_line;
 /* Lookup the .debug_line section */
 debug_line = application_dlsect(app,".debug_line");
 if (!debug_line.ds_size) return NULL; /* Nothing... */
 /* Allocate the module-debug descriptor. */
 result = (struct module_debug *)kmalloc(sizeof(struct module_debug),
                                         GFP_SHARED|GFP_CALLOC);
 memcpy(&result->md_debug_line,
        &debug_line,
        sizeof(struct dl_section));
 /* Map the debug_line section into memory. */
 TRY {
  REF struct vm_region *EXCEPT_VAR region;
  size_t num_pages,padding_size;
  padding_size = 64; /* Add some padding to guard against curruption. */
  num_pages = CEILDIV(debug_line.ds_size+padding_size,PAGESIZE);
  if (app->a_flags & APPLICATION_FTRUSTED)
      padding_size = 0; /* No need to add padding if the app can be trusted. */
  /* Construct a new VM region to represent a file mapping. */
  region = vm_region_alloc_file(num_pages,
                                false,
                               (struct inode *)app->a_module->m_fsloc,
                                0,
                                debug_line.ds_offset,
                                debug_line.ds_size,
                                0);
  TRY {
   /* Cannot share this region... */
   region->vr_flags |= VM_REGION_FCANTSHARE;
   /* Map the new region into memory.
    * NOTE: We add `PROT_WRITE' into the mix, so we're
    *       later able to do some fix-ups on the data. */
   result->md_data = (byte_t *)vm_map(VM_KERNELDEBUG_HINT,
                                      num_pages,
                                      1,
                                      0,
                                      VM_KERNELDEBUG_MODE,
                                      0,
                                      region,
                                      PROT_READ|PROT_WRITE|PROT_NOUSER,
                                      NULL,
                                      NULL);
  } FINALLY {
   vm_region_decref(region);
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(result);
  error_rethrow();
 }
 return result;
}


/* Similar to `module_debug_alloc()', but lazily check if debug
 * information had already been opened for the associated module,
 * before using `module_debug_alloc()' to create new information
 * and storing them in `app->a_module->m_debug', as well as
 * returning them. 
 * @return: * :   Pointer to debug debug information.
 * @return: NULL: No debug information present. */
FUNDEF struct module_debug *KCALL
module_debug_open(struct application *__restrict app) {
 struct module_debug *result,*new_result;
 struct module *mod = app->a_module;
 /* Quick check: has debug data already been opened? */
 if (mod->m_debug) return mod->m_debug;
 /* Quick check: could it be that there is no debug data? */
 if (mod->m_flags&MODULE_FNODEBUG) return NULL;
 /* All right. Open a new debug data descriptor. */
 result = module_debug_alloc(app);
 if (!result) {
  /* There is no debug data... */
  ATOMIC_FETCHOR(mod->m_flags,MODULE_FNODEBUG);
  return NULL;
 }
 /* Save the allocated debug data descriptor. */
 new_result = ATOMIC_CMPXCH_VAL(mod->m_debug,NULL,result);
 if likely(!new_result) return result;
 /* Some other thread was faster... (use their work) */
 module_debug_delete(result);
 return new_result;
}




/* Lookup an application at `ip' and invoke `module_debug_query()'.
 * @return: true:  Successfully found debug information.
 * @return: false: No application found, or application didn't contain debug information. */
FUNDEF uintptr_t KCALL
linker_debug_query(uintptr_t ip,
                   struct module_addr2line *__restrict result) {
 REF struct application *EXCEPT_VAR app = NULL; 
 uintptr_t return_value = (uintptr_t)-1;
 struct vm *EXCEPT_VAR effective_vm;
 struct vm_node *node;
 effective_vm = ip >= KERNEL_BASE ? &vm_kernel : THIS_VM;
 vm_acquire(effective_vm);
 TRY {
  node = vm_getnode(VM_ADDR2PAGE(ip));
  if (node && node->vn_notify == &application_notify) {
   app = (REF struct application *)node->vn_closure;
   application_incref(app);
  }
 } FINALLY {
  vm_release(effective_vm);
 }
 if (app) {
  TRY {
   /* Query debug information for the found application. */
   if (module_debug_query(app,ip-app->a_loadaddr,result))
       return_value = app->a_loadaddr;
  } FINALLY {
   application_decref(app);
  }
 }
 return return_value;
}






/* NOTE: The DWARF implementation here is based on information gathered
 *       from binutils, but mostly from the online specifications
 *       "http://www.dwarfstd.org/doc/DWARF4.pdf", section 6.2 */

#define DW_LNS_extended_op        0
#define DW_LNS_copy               1
#define DW_LNS_advance_pc         2
#define DW_LNS_advance_line       3
#define DW_LNS_set_file           4
#define DW_LNS_set_column         5
#define DW_LNS_negate_stmt        6
#define DW_LNS_set_basic_block    7
#define DW_LNS_const_add_pc       8
#define DW_LNS_fixed_advance_pc   9
#define DW_LNS_set_prologue_end   10
#define DW_LNS_set_epilogue_begin 11
#define DW_LNS_set_isa            12

#define DW_LNE_end_sequence                1
#define DW_LNE_set_address                 2
#define DW_LNE_define_file                 3
#define DW_LNE_set_discriminator           4
#define DW_LNE_HP_negate_is_UV_update      0x11
#define DW_LNE_HP_push_context             0x12
#define DW_LNE_HP_pop_context              0x13
#define DW_LNE_HP_set_file_line_column     0x14
#define DW_LNE_HP_set_routine_name         0x15
#define DW_LNE_HP_set_sequence             0x16
#define DW_LNE_HP_negate_post_semantics    0x17
#define DW_LNE_HP_negate_function_exit     0x18
#define DW_LNE_HP_negate_front_end_logical 0x19
#define DW_LNE_HP_define_proc              0x20

typedef struct {
    uintptr_t    address;
    size_t       file;
    int          line;
    unsigned int column;
    uintptr_t    flags;    /* Set of `MODULE_ADDR2LINE_F*' */
    u8           op_index;
    uintptr_t    discriminator;
} state_machine_t;

PRIVATE state_machine_t const default_state = {
    .file  = 1,
    .line  = 1,
    .flags = MODULE_ADDR2LINE_FINFUNC|MODULE_ADDR2LINE_FPROLOG,
};

INTDEF intptr_t  KCALL decode_sleb128(byte_t **__restrict ptext);
INTDEF uintptr_t KCALL decode_uleb128(byte_t **__restrict ptext);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

/* Query address2line debug information about the given application.
 * @return: true:  Successfully found debug information.
 * @return: false: Application doesn't contain debug information. */
FUNDEF bool KCALL
module_debug_query(struct application *__restrict app,
                   image_rva_t image_relative_ip,
                   struct module_addr2line *__restrict result) {
 struct module_debug *debug;
 byte_t *reader,*end;
#define GOTO_FAIL { debug_printf("%s(%d) : FAIL\n",__FILE__,__LINE__); goto fail; }
#define GOTO_FAILF(...) { debug_printf("%s(%d) : FAIL\n",__FILE__,__LINE__); debug_printf(__VA_ARGS__); debug_printf("\n"); goto fail; }
 unsigned int sourceno = 0;
 debug  = module_debug_open(app);
 if (!debug) return false;
 reader = debug->md_data;
 end    = reader + debug->md_debug_line.ds_size;
 while (reader < end) {
  uintptr_t length;
  byte_t *next_chunk; u16 version;
  byte_t *text_begin;
  bool islength64 = false;
  length = *(u32 *)reader;
  if (length <= 15) /* 15: Minimum size of the DWARF LineInfo header. */
      break; /* Sentinel */
  reader += 4;
  if unlikely(length == (u32)-1) {
   length = (uintptr_t)*(u64 *)reader;
   if (length <= 15)
       break; /* Sentinel */
   islength64 = true;
   reader += 8;
  }
  if unlikely(__builtin_add_overflow((uintptr_t)reader,length,
                                     (uintptr_t *)&next_chunk))
     GOTO_FAIL; /* Invalid overflow. */
  if unlikely(next_chunk > end)
              next_chunk = end; /* Fix overflow here. */
  /* Version number. */
  version = *(u16 *)reader,reader += 2;
  /* Offset to line number program. */
  if (islength64) {
   length = (uintptr_t)*(u64 *)reader;
   reader += 8;
  } else {
   length = (uintptr_t)*(u32 *)reader;
   reader += 4;
  }
  if unlikely(__builtin_add_overflow((uintptr_t)reader,length,
                                     (uintptr_t *)&text_begin))
     goto do_next_chunk; /* Invalid overflow. */
  if unlikely(text_begin >= next_chunk)
     goto do_next_chunk; /* Invalid overflow. */
  {
   state_machine_t old_state,state;
   u8 min_insn_length,max_ops_per_insn;
   u8 default_isstmt,line_range,opcode_base;
   u8 *opcode_lengths; s8 line_base;
   char *include_paths_table;

   min_insn_length  = *(u8 *)reader,reader += 1;
   max_ops_per_insn = 1;
   if (version >= 4) max_ops_per_insn = *(u8 *)reader,reader += 1;
   default_isstmt = *(u8 *)reader,reader += 1;
   line_base      = *(s8 *)reader,reader += 1;
   line_range     = *(u8 *)reader,reader += 1;
   opcode_base    = *(u8 *)reader,reader += 1;
   opcode_lengths = reader;
   if (opcode_base) reader += opcode_base-1;
   if unlikely(reader > text_begin)
      GOTO_FAIL; /* Illegal overflow */
   include_paths_table = (char *)reader;
   /* Start parsing the actual program text. */
   reader = text_begin;
   memcpy(&state,&default_state,sizeof(state_machine_t));
   if (default_isstmt)
       state.flags |= MODULE_ADDR2LINE_FISSTMT;
   old_state.address = (uintptr_t)-1;
   while (reader < next_chunk) {
    byte_t opcode;
    opcode = *reader++;
    if (opcode >= opcode_base) {
     /* Handle so-called special opcodes. */
     size_t temp;
     opcode -= opcode_base;
     temp = opcode/line_range;
     if (max_ops_per_insn == 1) {
      temp          *= min_insn_length;
      state.address += temp;
     } else {
      state.address += ((state.op_index+temp) / max_ops_per_insn) * min_insn_length;
      state.op_index = (state.op_index+temp) % max_ops_per_insn;
     }
     state.line += (ssize_t)(opcode % line_range)+line_base;
#define TEST_MATCH() \
     { /*debug_printf("state.address = %p\n",state.address);*/ \
       if (old_state.address <= image_relative_ip && \
           state.address > image_relative_ip) \
           goto found_state; \
     }
     if (old_state.address <= image_relative_ip &&
         state.address > image_relative_ip) {
      unsigned int dtab_count,fileno;
      char *file_table,*fileent_start;
found_state:
      /* Load the starting address of the file table. */
      file_table = include_paths_table;
      dtab_count = 0;
      for (;;) {
       if (!*file_table) { ++file_table; break; }
       if unlikely((byte_t *)file_table >= text_begin) break;
       file_table = strend(file_table)+1;
       ++dtab_count;
      }
      /* Save discriminator information. */
      result->d_name   = NULL; /* TODO */
      result->d_base   = NULL; /* TODO */
      result->d_begin  = old_state.address;
      result->d_end    = state.address;
      result->d_srcno  = sourceno;
      result->d_discr  = old_state.discriminator;
      result->d_line   = old_state.line;
      result->d_column = old_state.column;
      result->d_flags  = old_state.flags;

      fileno        = old_state.file;
      fileent_start = file_table;
      if (!fileno)
       result->d_file = NULL,
       result->d_path = NULL;
      else {
       while (--fileno && (byte_t *)fileent_start < text_begin) {
        if (!*fileent_start) break; /* Invalid file ID */
        fileent_start = strend(fileent_start)+1;
        decode_uleb128((byte_t **)&fileent_start);
        decode_uleb128((byte_t **)&fileent_start);
        decode_uleb128((byte_t **)&fileent_start);
       }
       result->d_file = fileent_start;
       /* Parse the directory number. */
       fileent_start = strend(fileent_start)+1;
       fileno = (uintptr_t)decode_uleb128((byte_t **)&fileent_start);
       if (!fileno || --fileno >= dtab_count)
        result->d_path = NULL;
       else {
        fileent_start = include_paths_table;
        while (fileno--) fileent_start = strend(fileent_start)+1;
        result->d_path = fileent_start;
       }
      }
      return true;
     }
     memcpy(&old_state,&state,sizeof(state_machine_t));
    } else {
     switch (opcode) {

     {
      byte_t *ext_data;
      uintptr_t temp;
     case DW_LNS_extended_op:
      temp = decode_uleb128(&reader);
      ext_data = reader;
      if unlikely(__builtin_add_overflow((uintptr_t)reader,temp,
                                         (uintptr_t *)&reader))
         goto do_next_chunk;
      if unlikely(reader > next_chunk)
         goto do_next_chunk;
      if (temp != 0) {
       opcode = *ext_data++;
       /* Extended opcodes. */
       switch (opcode) {

       case DW_LNE_end_sequence:
        /* Reset the state machine. */
        TEST_MATCH();
        memcpy(&state,&default_state,sizeof(state_machine_t));
        old_state.address = (uintptr_t)-1;
        if (default_isstmt)
            state.flags |= MODULE_ADDR2LINE_FISSTMT;
        break;

       {
        uintptr_t new_address;
       case DW_LNE_set_address:
             if ((size_t)temp-1 >= 8) new_address = (uintptr_t)*(__u64 *)ext_data;
        else if ((size_t)temp-1 >= 4) new_address = (uintptr_t)*(__u32 *)ext_data;
        else if ((size_t)temp-1 >= 2) new_address = (uintptr_t)*(__u16 *)ext_data;
        else                          new_address = (uintptr_t)*(__u8  *)ext_data;
        state.address  = new_address;
        state.op_index = 0;
       } break;

       case DW_LNE_define_file:
        /* TODO */
        break;

       case DW_LNE_set_discriminator:
        state.discriminator = decode_uleb128(&ext_data);
        break;

       default:
        break;
       }
      }
     } break;

     case DW_LNS_copy:
      TEST_MATCH();
      state.discriminator = 0;
      state.flags &= ~(MODULE_ADDR2LINE_FBBLOCK|MODULE_ADDR2LINE_FEPILOG);
      state.flags |=   MODULE_ADDR2LINE_FPROLOG;
      memcpy(&old_state,&state,sizeof(state_machine_t));
      break;

     {
      uintptr_t temp;
     case DW_LNS_advance_pc:
      temp = decode_uleb128(&reader);
      if (max_ops_per_insn == 1) {
       temp *= min_insn_length;
       state.address += temp;
      } else {
       state.address += ((state.op_index+temp)/max_ops_per_insn)*
                                             min_insn_length;
       state.op_index = ((state.op_index+temp)%max_ops_per_insn);
      }
     } break;

     case DW_LNS_advance_line:
      state.line += (int)decode_sleb128(&reader);
      break;

     case DW_LNS_set_file:
      state.file = (size_t)decode_uleb128(&reader);
      break;

     case DW_LNS_set_column:
      state.column = (unsigned int)decode_uleb128(&reader);
      break;

     case DW_LNS_negate_stmt:
      state.flags ^= MODULE_ADDR2LINE_FISSTMT;
      break;

     case DW_LNS_set_basic_block:
      state.flags |= MODULE_ADDR2LINE_FBBLOCK;
      break;

     {
      uintptr_t temp;
     case DW_LNS_const_add_pc:
      temp = (255 - opcode_base) / line_range;
      if (max_ops_per_insn == 1) {
       temp *= min_insn_length;
       state.address += temp;
      } else {
       state.address += ((state.op_index+temp) / max_ops_per_insn) * min_insn_length;
       state.op_index =  (state.op_index+temp) % max_ops_per_insn;
      }
     } break;

     case DW_LNS_fixed_advance_pc:
      state.address += *(u16 *)reader;
      state.op_index = 0;
      reader += 2;
      break;

     case DW_LNS_set_prologue_end:
      state.flags &= ~MODULE_ADDR2LINE_FPROLOG;
      break;

     case DW_LNS_set_epilogue_begin:
      state.flags |= MODULE_ADDR2LINE_FEPILOG;
      break;

     case DW_LNS_set_isa:
      decode_uleb128(&reader);
      break;

     default:
      if (opcode < opcode_base) {
       /* Custom opcode. */
       u8 n = opcode_lengths[opcode-1];
       while (n--) decode_uleb128(&reader);
      }
      break;
     }
    }
   }
do_next_chunk:
   ;
  }
  /* Move on to the next chunk. */
  reader = next_chunk;
  ++sourceno;
 }
fail:
 return false;
}

#pragma GCC diagnostic pop

DEFINE_SYSCALL3(xaddr2line,USER UNCHECKED uintptr_t,abs_pc,
                USER UNCHECKED struct dl_addr2line *,buf,
                size_t,bufsize) {
 struct module_addr2line info;
 uintptr_t load_addr; size_t result;
 char *strbuf; size_t strbuf_size;
 unsigned int i;
 validate_executable((void *)abs_pc);
 load_addr = linker_debug_query(abs_pc,&info);
 if (load_addr == (uintptr_t)-1) return 0; /* No data */
 result = sizeof(struct dl_addr2line);
 strbuf = (char *)((uintptr_t)buf+sizeof(struct dl_addr2line));
 strbuf_size = bufsize > sizeof(struct dl_addr2line) ? bufsize - sizeof(struct dl_addr2line) : 0;
 /* XXX: This assumes that all 4 strings immediately follow each other. */
 for (i = 0; i < 4; ++i) {
  char const *string = (&info.d_base)[i];
  size_t string_length;
  if (!string || ADDR_ISUSER(string))
       continue; /* String is already mapped to user-space. */
  string_length = (strlen(string)+1)*sizeof(char);
  /* Track the required buffer size. */
  result += string_length;
  /* Update the info pointer to be directed at user-space. */
  (&info.d_base)[i] = strbuf;
  /* Copy the string into user-space. */
  if (string_length > strbuf_size)
      string_length = strbuf_size;
  memcpy(strbuf,string,string_length);
  strbuf      += string_length;
  strbuf_size -= string_length;
 }
 /* Copy base descriptor data into user-space (string pointers have already been updated) */
 if (bufsize >= sizeof(struct dl_addr2line)) {
  buf->d_begin  = (void *)(load_addr + info.d_begin);
  buf->d_end    = (void *)(load_addr + info.d_end);
  buf->d_discr  = info.d_discr;
  buf->d_srcno  = info.d_srcno;
  buf->d_line   = info.d_line;
  buf->d_base   = info.d_base;
  buf->d_path   = info.d_path;
  buf->d_file   = info.d_file;
  buf->d_name   = info.d_name;
  buf->d_column = info.d_column;
  buf->d_flags  = info.d_flags;
 }
 return result;
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_UNWIND_DEBUG_LINE_C */
