#ifndef _COMPAT_H_
#define _COMPAT_H_

// Detect EDK2 environment
#ifdef __EDK2__
  #include <Uefi.h>
  #include <Library/BaseLib.h>
  #include <Library/BaseMemoryLib.h>
  #include <Library/MemoryAllocationLib.h>
#else
  // Native C environment
  #include <stdint.h>
  #include <stddef.h>
  #include <string.h>

  // Type aliases (like EDK2)
  typedef uint8_t   UINT8;
  typedef uint16_t  UINT16;
  typedef uint32_t  UINT32;
  typedef uint64_t  UINT64;
  typedef int8_t    INT8;
  typedef int16_t   INT16;
  typedef int32_t   INT32;
  typedef int64_t   INT64;
  typedef size_t    UINTN;
  typedef void      VOID;
  typedef int       BOOLEAN;

  #ifndef TRUE
  #define TRUE 1
  #define FALSE 0
  #endif

  // Macros to match EDK2
  #define EFIAPI
  #define IN
  #define OUT
  #define OPTIONAL
  #define CONST const

  // Memory functions
  #define CopyMem(dest, src, size) memmove((dest), (src), (size))
  #define SetMem(buf, size, val) memset((buf), (val), (size))
  #define ZeroMem(buf, size) memset((buf), 0, (size))
  #define CompareMem(a, b, len) memcmp((a), (b), (len))

  // String aliases
  #define StrLen(s) strlen((const char*)(s))
  #define StrCmp strcmp
  #define StrnCmp strncmp
  #define AsciiStrLen strlen
  #define AsciiStrCmp strcmp

  // Stubbed functions for non-libc use cases
  #ifndef __has_include
    #define __has_include(x) 0
  #endif

  #if !__has_include(<stdio.h>)
    static inline void printf(const char* fmt, ...) {}
  #endif

  #if !__has_include(<stdlib.h>)
    static inline void abort() { while (1) {} }
    static inline void exit(int code) { (void)code; while (1) {} }
  #endif

#endif // __EDK2__

#endif // _COMPAT_H_
