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
#ifndef GUARD_APPS_WMS_WINDOWMAP_C
#define GUARD_APPS_WMS_WINDOWMAP_C 1
#define _KOS_SOURCE 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <kos/types.h>
#include <wm/api.h>
#include <malloc.h>
#include <except.h>

#include "window.h"
#include "display.h"

DECL_BEGIN

INTERN void WMCALL
WindowMap_Init(WindowMap *__restrict self) {
 self->wm_size     = 0;
 self->wm_mask     = 3;
 self->wm_nextid   = 1;
 self->wm_map      = (Window **)Xcalloc(4,sizeof(Window *));
 self->wm_overflow = false;
}

INTERN ATTR_NOTHROW void WMCALL
WindowMap_Fini(WindowMap *__restrict self_) {
 WindowMap *EXCEPT_VAR self = self_;
 wms_window_id_t EXCEPT_VAR i;
 for (i = 0; i <= self->wm_mask; ++i) {
  Window *EXCEPT_VAR iter = self->wm_map[i];
  while (iter) {
   Window *EXCEPT_VAR next;
   Display *EXCEPT_VAR disp = iter->w_display;
   next = iter->w_idchain.le_next;
   atomic_rwlock_write(&disp->d_lock);
   TRY {
    /* Destroy the window. */
    Window_DestroyUnlocked(iter);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    atomic_rwlock_endwrite(&disp->d_lock);
    error_printf("Failed to destroy window %p\n",iter);
    error_handled();
   }
   iter = next;
  }
 }
 free(self->wm_map);
}

INTERN ATTR_NOTHROW wms_window_id_t WMCALL
WindowMap_MakeID(WindowMap *__restrict self) {
 wms_window_id_t i,result;
 if (!self->wm_overflow) {
  result = self->wm_nextid++;
  if likely(result != 0)
     return result;
  self->wm_overflow = true;
 }
next_id:
 result = self->wm_nextid++;
 /* After the first overflow, we must search for IDs already in use. */
 for (i = 0; i <= self->wm_mask; ++i) {
  Window *iter;
  iter = self->wm_map[i];
  for (; iter; iter = iter->w_idchain.le_next) {
   if (iter->w_id == result)
       goto next_id;
  }
 }
 /* Id isn't being used. */
 return result;
}

INTERN void WMCALL
WindowMap_Insert(WindowMap *__restrict self,
                 Window *__restrict win) {
 /* TODO */
}

INTERN ATTR_NOTHROW void WMCALL
WindowMap_Remove(WindowMap *__restrict self,
                 Window *__restrict win) {
 /* TODO */
}

/* @return: NULL: Invalid window ID. */
INTERN ATTR_NOTHROW Window *WMCALL
WindowMap_Lookup(WindowMap *__restrict self,
                 wms_window_id_t id) {
 Window *result;
 result = self->wm_map[id & self->wm_mask];
 while (result && result->w_id != id)
        result = result->w_idchain.le_next;
 return result;
}


DECL_END

#endif /* !GUARD_APPS_WMS_WINDOWMAP_C */
