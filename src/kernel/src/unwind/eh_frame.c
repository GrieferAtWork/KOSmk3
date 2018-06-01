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
#ifndef GUARD_KERNEL_SRC_KERNEL_UNWIND_EH_FRAME_C
#define GUARD_KERNEL_SRC_KERNEL_UNWIND_EH_FRAME_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <unwind/eh_frame.h>
#include <kernel/debug.h>
#include <string.h>
#include <assert.h>

DECL_BEGIN

INTERN intptr_t KCALL
dwarf_decode_sleb128(byte_t **__restrict ptext) {
 byte_t byte,*text = *ptext;
 intptr_t result = 0;
 unsigned int shift = 0;
 for (;;) {
  byte    = *text++;
  result |= ((byte & 0x7f) << shift);
  shift  += 7;
  if (!(byte & 0x80))
        break;
 }
 if ((byte & 0x40) &&
     (shift < sizeof(intptr_t)*8))
      result |= -((intptr_t)1 << shift);
 *ptext = text;
 return result;
}

INTERN uintptr_t KCALL
dwarf_decode_uleb128(byte_t **__restrict ptext) {
 byte_t byte,*text = *ptext;
 unsigned int shift = 0;
 uintptr_t result = 0;
 for (;;) {
  byte    = *text++;
  result |= ((byte & 0x7f) << shift);
  shift  += 7;
  if (!(byte & 0x80))
        break;
 }
 *ptext = text;
 return result;
}

/* Decode a pointer, as seen in `FDE' descriptors. */
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
INTERN uintptr_t KCALL
dwarf_decode_pointer3264(byte_t **__restrict ptext,
                         u8 encoding, bool compat_mode)
#else
INTERN uintptr_t KCALL
dwarf_decode_pointer(byte_t **__restrict ptext, u8 encoding)
#endif
{
 uintptr_t result;
 byte_t *text = *ptext;
 /* Relative encoding formats. */
 switch (encoding & 0x70) {
 case DW_EH_PE_pcrel:
  result = (uintptr_t)text; /* Relative to here. */
  break;
 case DW_EH_PE_textrel:
  /* ??? */
 case DW_EH_PE_datarel:
  /* ??? */
 case DW_EH_PE_funcrel:
  /* ??? */
 case DW_EH_PE_aligned:
  /* ??? */
 default:
 case DW_EH_PE_absptr:
  result = 0;
  break;
 }
 switch (encoding & 0xf) {
 case DW_EH_PE_absptr:
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
  if (compat_mode) {
   result += *(u32 *)text;
   text   += 4;
  } else {
   result += *(uintptr_t *)text;
   text   += sizeof(uintptr_t);
  }
#else
  result += *(uintptr_t *)text;
  text   += sizeof(uintptr_t);
#endif
  break;
 case DW_EH_PE_udata2:
  result += *(u16 *)text;
  text   += 2;
  break;
 case DW_EH_PE_udata4:
  result += *(u32 *)text;
  text   += 4;
  break;
 case DW_EH_PE_udata8:
  result += (uintptr_t)*(u64 *)text;
  text   += 8;
  break;
 case DW_EH_PE_sdata2:
  result += *(s16 *)text;
  text   += 2;
  break;
 case DW_EH_PE_sdata4:
  result += *(s32 *)text;
  text   += 4;
  break;
 case DW_EH_PE_sdata8:
  result += (intptr_t)*(s64 *)text;
  text   += 8;
  break;
 case DW_EH_PE_uleb128:
  result += dwarf_decode_uleb128(&text);
  break;
 case DW_EH_PE_sleb128:
  result += dwarf_decode_sleb128(&text);
  break;
 default: break;
 }
 *ptext = text;
 return result;
}



#ifdef CONFIG_ELF_SUPPORT_CLASS3264
#define DECODE_POINTER(ptext,encoding)  dwarf_decode_pointer3264(ptext,encoding,compat_mode)
#else
#define DECODE_POINTER(ptext,encoding)  dwarf_decode_pointer(ptext,encoding)
#endif

/* Find the FDE associated with a given `ip' by
 * searching the given `eh_frame' section. */
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
PUBLIC bool KCALL
eh_findfde3264(byte_t *__restrict eh_frame_start,
               size_t eh_frame_size, uintptr_t ip,
               struct fde_info *__restrict result,
               bool compat_mode)
#else
PUBLIC bool KCALL
eh_findfde(byte_t *__restrict eh_frame_start,
           size_t eh_frame_size, uintptr_t ip,
           struct fde_info *__restrict result)
#endif
{
 byte_t *end = eh_frame_start+eh_frame_size;
 byte_t *reader = eh_frame_start,*next;
 byte_t *cie_reader,*fde_reader;
#if 0
 debug_printf("eh_findfde(%p,%Iu,%p)\n",eh_frame_start,eh_frame_size,ip);
#endif

 while (reader < end) {
  size_t length; u32 cie_offset;
  char *cie_augstr; struct CIE *cie;
#if 0
  result->fi_fde = (struct FDE *)reader;
#endif
  length = (size_t)*(u32 *)reader;
  reader += 4;
  if unlikely((u32)length == (u32)-1) {
#if __SIZEOF_POINTER__ > 4
   length = (size_t)*(u64 *)reader;
   reader += 8;
#else
   break; /* Too large. Impossible to represent. */
#endif
  }
  if (!length) break;
  next = reader+length;
  cie_offset = *(u32 *)reader; /* f_cieptr */
  if (cie_offset == 0)
      goto next_chunk; /* This is a CIE, not an FDE */
  cie = (struct CIE *)(reader - cie_offset);
#if 0
  result->fi_cie = cie;
#endif
  fde_reader = reader+4;

#if 1
  if (!((byte_t *)cie >= eh_frame_start &&
        (byte_t *)cie < end))
      break;
#else
  assertf((byte_t *)cie >= eh_frame_start &&
          (byte_t *)cie < end,
          "Invalid CIE pointer (%p not in %p...%p)\n"
          "cie_offset = %I32u (%#I32x)\n",
          cie,eh_frame_start,end-1,
          cie_offset,cie_offset);
#endif

  /* Load the augmentation string of the associated CIE. */
  cie_reader  = (byte_t *)cie;
  cie_reader += 4;                        /* c_length */
  if (((u32 *)cie_reader)[-1] == (u32)-1) {
#if __SIZEOF_POINTER__ > 4
   cie_reader += 8;                       /* c_length64 */
#else
   goto next_chunk;
#endif
  }
  cie_reader += 4;                        /* c_cieid */
  cie_reader += 1;                        /* c_version */
  //debug_printf("CIE at %p, %Iu (%q)\n",result->fi_fde,length,cie_reader);
  cie_augstr = (char *)cie_reader;
  cie_reader = (byte_t *)strend(cie_augstr)+1;
  /* Read code and data alignments. */
  result->fi_codealign = dwarf_decode_uleb128(&cie_reader); /* c_codealignfac */
  result->fi_dataalign = dwarf_decode_sleb128(&cie_reader); /* c_dataalignfac */
  result->fi_retreg    = dwarf_decode_sleb128(&cie_reader); /* c_returnreg */
  /* Pointer encodings default to ZERO(0). */
  result->fi_encptr   = 0;
  result->fi_enclsda  = 0;
  result->fi_encperso = 0;
  result->fi_sigframe = 0;
  /* No personality function by default. */
  result->fi_persofun = 0;
  result->fi_lsdaaddr = 0;
  if (cie_augstr[0] == 'z') {
   char *aug_iter = cie_augstr;
   /* Interpret the augmentation string. */
   uintptr_t aug_length; byte_t *aug_end;
   aug_length = dwarf_decode_uleb128(&cie_reader); /* c_auglength */
   aug_end    = cie_reader+aug_length;
   if unlikely(aug_end < cie_reader || aug_end > end)
      break; /* Check for overflow/underflow. */
   while (*++aug_iter && cie_reader < aug_end) {
    if (*aug_iter == 'L') {
     result->fi_enclsda = *cie_reader++;
    } else if (*aug_iter == 'P') {
     result->fi_encperso = *cie_reader++;
     result->fi_persofun = DECODE_POINTER(&cie_reader,result->fi_encperso);
    } else if (*aug_iter == 'R') {
     result->fi_encptr = *cie_reader++;
    } else {
     /* XXX: What then? */
    }
   }
   /* `aug_end' now points at `c_initinstr' */
   cie_reader = aug_end;
  }
  result->fi_pcbegin = DECODE_POINTER(&fde_reader,result->fi_encptr);
  result->fi_pcend   = DECODE_POINTER(&fde_reader,result->fi_encptr & 0xf);
  if (__builtin_add_overflow(result->fi_pcbegin,
                             result->fi_pcend,
                            &result->fi_pcend))
      goto next_chunk;
  /* Check of the CIE points to the proper bounds. */
  if (result->fi_pcbegin > ip) goto next_chunk;
  if (result->fi_pcend <= ip) goto next_chunk;
  /* Found it! - Save the pointer to the initial instruction set. */
  result->fi_inittext = cie_reader;
  /* Figure out the max length of that instruction set. */
  cie_reader  = (byte_t *)cie;
  length      = *(u32 *)cie_reader;
  cie_reader += 4;
#if __SIZEOF_POINTER__ > 4
  /* Above code already asserted that the length fits into 32 bits of the CIE. */
  if unlikely((u32)length == (u32)-1) {
   length = (size_t)*(u64 *)reader;
   reader += 8;
  }
#endif
  cie_reader += length;
  result->fi_initsize = (size_t)(cie_reader-result->fi_inittext);
  if (cie_reader < result->fi_inittext)
      result->fi_initsize = 0; /* Shouldn't happen... */
#if 0
  result->fi_augstr = cie_augstr; /* XXX: Do we actually need this later? */
#endif
  /* Parse augmentation data of the FDE. */
  if (cie_augstr[0] == 'z') {
   uintptr_t aug_length; byte_t *aug_end;
   aug_length = dwarf_decode_uleb128(&fde_reader); /* c_auglength */
   aug_end    = fde_reader+aug_length;
   while (*++cie_augstr && fde_reader <= aug_end) {
    if (*cie_augstr == 'L') {
     if unlikely(fde_reader == aug_end) break;
     result->fi_lsdaaddr = DECODE_POINTER(&fde_reader,
                                           result->fi_enclsda);
    } else if (*cie_augstr == 'S') {
     result->fi_sigframe = 1;
    }
   }
   fde_reader = aug_end;
  }
  result->fi_evaltext = fde_reader;
  result->fi_evalsize = (size_t)(next - fde_reader);
  if unlikely(fde_reader > next)
     result->fi_evalsize = 0; /* Shouldn't happen... */

#if 0
  if (result->fi_lsdaaddr != 0) {
   debug_printf("result->fi_persofun = %p\n",result->fi_persofun);
   debug_printf("result->fi_lsdaaddr = %p\n",result->fi_lsdaaddr);
  }
#endif

  return true;
next_chunk:
  if unlikely(next < reader)
     break; /* Underflow */
  reader = next;
 }
 return false;
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_UNWIND_EH_FRAME_C */
