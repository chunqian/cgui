/*

   How to use this file:

   1. Include this file from the top of all header files.

   2. Use the macros listed below to declare any identifiers in this file.
      Type definitions and macros should be declared/defined as usually,
      except for function pointers in structs (see METHOD()).

   VAR(type, name)
      Declare a variable.
         Example:
            VAR(int, x);
         means
            extern int x;

   ARRAY(type, name)
      Declare an array.
         Example:
            ARRAY(int, list);
         means
            extern int list[];

   FUNC(return_type, name, (parameters))
      Declare a function.
         Example:
            FUNC(int, foo, (int x, int y));
         means
            int foo(int x, int y);

   FUNCPTR(return_type, name, (parameters))
      Declare a function pointer.
         Example:
            FUNCPTR(int, foo, (int x, int y));
         means
            int (*foo)(int x, int y);

   METHOD(return_type, name, (parameters))
      Declare a function pointer when it occurs as a function parameter or as
      a struct member.
         Example:
            FUNC(void, foo, (METHOD(int, bar, (int x))));
         means
            void foo(int (*bar)(int x));

         Example:
            typedef struct {
               METHOD(int, foo, (int x));
               METHOD(void, bar, (void));
            } my_struct;
         means
            typedef struct {
               int (*foo)(int x);
               void (*bar)(void);
            } my_struct;

   INLINE(return_type, name, (parameters), { code })
      Used to declare an inline function. This works smoothly even on
      platforms that don't support inline. Unfortunately the macro form puts
      some limits on the code: it must not contain commas that are not
      surrounded by parentheses. While this is usually not a problem, it can
      be hard to track the problem down when it happens. The two
      possibilities to break this rule is to use inline assembler on certain
      platforms, or to use the comma operator.
         Example:
            INLINE(int, square, (int x),
            {
               return x * x;
            })
         means
            extern inline int square(int x)
            {
               return x * x;
            }

   BEGIN_INLINE(return_type, name, (parameters))
   END_OF_INLINE()
      These provide an alternative way to declare an inline function. These
      are more flexible in that you can put commas anywhere in the code (see
      the documentation for INLINE()), but require more lines of code.
         Example:
            BEGIN_INLINE(int, square, (int x))
            #ifdef INLINE_PROVIDE_CODE
            {
               return x;
            }
            #endif
            END_OF_INLINE()
         declares an inline function that returns the square of its argument.

   LOCAL_INLINE(return_type, name, (parameters), code)
   BEGIN_LOCAL_INLINE(return_type, name, (parameters))
   END_OF_LOCAL_INLINE()
      Use these rather than the ones without LOCAL when you declare inline
      functions in source files, ie inline functions that will only be called
      from within the same file.

   PRINTF_FUNC(return_type, name, (parameters), string_index, vararg_index)
      Declare a function which takes a printf()-style format string.
      string_index specifies which argument is the format string, and
      vararg_index specifies which argument is the first that should be
      checked.
         Example:
            PRINTF_FUNC(int, fprintf, (FILE *f, char *format, ...), 2, 3);
         means that the second parameter to fprintf() is the format string
         and that the first parameter to check against the format string is
         parameter number 3.

   CONST_FUNC(return_type, name, (parameters))
      Declare a const function. A const function function is one that doesn't
      read or write any global memory, ie one whose return value is
      completely determined by its parameters without dereferencing them.

   CONST
      Qualifier to constant function arguments.
         Example:
            FUNC(int, strcmp, (CONST char *s1, CONST char *s2));
         means that strcmp won't write to s1 or s2.

   Other macros:
   HAS_INLINE
      You normally don't need to bother about this. It's defined if the
      current platform supports inline functions.

   CGUI_INLINE_PROVIDE_CODE
      This is defined if and only if you should provide a function body
      between BEGIN_INLINE() and END_OF_INLINE(). So you must conditionalize
      it out as in the example in the documentation for BEGIN_INLINE().

   CGUI_INSTANTIATE_INLINE
      You normally don't need to bother about this. It's defined by inline.c
      so that this file knows about the special care that needs to be taken.



   The problems that makes all this necessary are:

   1. Declarations need special attributes when the symbol is in a dll. One
   attribute is needed when compiling the dll and another when compiling a
   program using the dll. And of course, these attributes are invalid on
   other platforms or when compiling a static library.

   2. Inline functions are not ansi but are in practice supported by most
   modern compilers, hence it can be a good idea to use them when
   appropriate. However, different platforms want them declared in different
   ways. There is also a problem with instantiating them: On most platforms
   you need to do that separately. Hence the inline.c hack is needed.

   3. There are some other useful extensions that are available on some
   compilers but not on other, eg. gcc can check the format string given to
   printf() at compile time and give appropriate warnings.
*/



/* NOT_CGUI_STATICLINK forces shared library mode, and
   CGUI_STATICLINK forces static library mode.
   If none of them is defined, we do like Allegro.
   (This is really recommended: it wouldn't be nice having two dlls around of
   which one needs allegro to be statically linked and one needs it as a
   dll!)
*/
#if (defined NOT_CGUI_STATICLINK)
   #ifdef CGUI_STATICLINK
      #error Both CGUI_STATICLINK and NOT_CGUI_STATICLINK are defined.
   #endif
#elif (defined CGUI_STATICLINK)
   /* do nothing */
#elif (defined ALLEGRO_STATICLINK)
   #define CGUI_STATICLINK
#endif



/* define platform-specific declaration stuff */
#if defined CGUI_SCAN_EXPORT

   #define CGUI_VAR(type, name)                       _cgui_scanex_dll_data name##_dll
   #define CGUI_ARRAY(type, name)                     _cgui_scanex_dll_data name##_dll

   #define CGUI_FUNC(type, name, args)                _cgui_scanex_dll_code name##_dll
   #define CGUI_FUNCPTR(type, name, args)             _cgui_scanex_dll_data name##_dll

   #define CGUI_BEGIN_INLINE(type, name, args, code)  _cgui_scanex_dll_code name##_dll

/*
RSXNT is not (and will probably never be) supported because MingW32 is
superior.

#elif defined __RSXNT__
   #include "drsxnt.h"
*/

#elif defined __MINGW32__

   /* describe how function prototypes look to MINGW32 */
   #if (defined CGUI_STATICLINK)
      #define _CGUI_DLL
   #else
      #if (defined CGUI_SRC)
         #define _CGUI_DLL   __declspec(dllexport)
      #else
           #define _CGUI_DLL   __declspec(dllimport)
      #endif
   #endif

   #define CGUI_VAR(type, name)              extern _CGUI_DLL type name
   #define CGUI_ARRAY(type, name)            extern _CGUI_DLL type name[]
   #define CGUI_FUNC(type, name, args)       type __cdecl name args
   #define CGUI_METHOD(type, name, args)     type (*name) args
   #define CGUI_FUNCPTR(type, name, args)    extern _CGUI_DLL type (*name) args

#elif defined __BORLANDC__

   #ifdef CGUI_SRC
      #error Currently BCC32 should only use the DLL
   #endif

   #define CGUI_VAR(type, name)              extern _CGUI_DLL type name
   #define CGUI_ARRAY(type, name)            extern _CGUI_DLL type name[]
   #define CGUI_FUNC(type, name, args)       _CGUI_DLL type __cdecl name args
   #define CGUI_METHOD(type, name, args)     type (__cdecl *name) args
   #define CGUI_FUNCPTR(type, name, args)    extern _CGUI_DLL type (__cdecl *name) args

   #if (defined CGUI_STATICLINK) || (defined CGUI_SRC)
      #define _CGUI_DLL
   #else
      #define _CGUI_DLL   __declspec(dllimport)
   #endif

   #define CGUI_BEGIN_INLINE(type, name, args)        extern __inline type __cdecl name args
   #define CGUI_BEGIN_LOCAL_INLINE(type, name, args)   __inline type name args

#elif defined _MSC_VER

   #pragma warning (disable: 4200 4244 4305)

   /* describe how function prototypes look to MSVC */
   #if (defined CGUI_STATICLINK) || (defined CGUI_SRC)
      #define _CGUI_DLL
   #else
      #define _CGUI_DLL   __declspec(dllimport)
   #endif

   #define CGUI_VAR(type, name)              extern _CGUI_DLL type name
   #define CGUI_ARRAY(type, name)            extern _CGUI_DLL type name[]
   #define CGUI_FUNC(type, name, args)       _CGUI_DLL type __cdecl name args
   #define CGUI_METHOD(type, name, args)     type (__cdecl *name) args
   #define CGUI_FUNCPTR(type, name, args)    extern _CGUI_DLL type (__cdecl *name) args

   #define CGUI_BEGIN_INLINE(type, name, args, code) \
      __inline _CGUI_DLL type __cdecl name args
   #define CGUI_END_OF_INLINE(type, name, args, code) \
      void *_force_instantiate_##name = name;

   #define CGUI_BEGIN_LOCAL_INLINE(type, name, args) \
      __inline type name args

#elif defined __WATCOMC__

   #ifndef __SW_3S
      #error Cgui only supports stack based calling convention
   #endif

   #ifndef __SW_S
      #error Stack overflow checking must be disabled
   #endif

   #pragma disable_message (120 201)

   #define CGUI_va_list_p_DEFINED
   typedef va_list **cgui_va_list_p;
   #define cgui_va_arg_p(a, b)   va_arg(**a, b)

#elif defined DJGPP

   /* Nothing special needs to be done. */

#elif defined __unix__

   /* Include configuration generated by configure script.  */
   //??#include "alunixac.h"

#elif defined __BEOS__

   /* Nothing special needs to be done. */

#else

   #error unknown platform

#endif

/* special definitions for the GCC compiler, independent of machine/OS */
#ifdef __GNUC__
   #ifndef CGUI_BEGIN_INLINE
      #define CGUI_BEGIN_INLINE(type, name, args)  extern inline type name args
   #endif

   #define CGUI_PRINTF_FUNC(type, name, args, a, b) CGUI_FUNC(type, name, args) __attribute__ ((format (printf, a, b)))

   #ifndef CGUI_CONST
      #define CGUI_CONST   const
   #endif

   #define CGUI_CONST_FUNC(type, name, args)       CGUI_FUNC(type, name, args) __attribute__ ((const))
#endif



/* fill in some default definitions of language features and helper
   functions, which are conditionalised so they will only be included if none
   of the above headers defined custom versions.
*/
#ifndef CGUI_VAR
   #define CGUI_VAR(type, name)                    extern type name
#endif

#ifndef CGUI_ARRAY
   #define CGUI_ARRAY(type, name)                  extern type name[]
#endif

#ifndef CGUI_FUNC
   #define CGUI_FUNC(type, name, args)             type name args
#endif

#ifndef CGUI_PRINTF_FUNC
   #define CGUI_PRINTF_FUNC(type, name, args, a, b) CGUI_FUNC(type, name, args)
#endif

#ifndef CGUI_CONST_FUNC
   #define CGUI_CONST_FUNC(type, name, args)       CGUI_FUNC(type, name, args)
#endif

#ifndef CGUI_METHOD
   #define CGUI_METHOD(type, name, args)           type (*name) args
#endif

#ifndef CGUI_FUNCPTR
   #define CGUI_FUNCPTR(type, name, args)          extern type (*name) args
#endif



/* Inline declaration stuff. */

/* Does platform support inline? */
#ifndef CGUI_BEGIN_INLINE
   #define CGUI_BEGIN_INLINE(type, name, args)     type name args
   #undef CGUI_HAS_INLINE
#else
   #define CGUI_HAS_INLINE
#endif

/* Should inline code be included in headers? */
#ifdef CGUI_HAS_INLINE
   #define CGUI_INLINE_PROVIDE_CODE
#endif

/* Are we compiling inline.c? */
#ifdef CGUI_INSTANTIATE_INLINE
   #define CGUI_INLINE_PROVIDE_CODE
   #undef CGUI_BEGIN_INLINE
   #undef CGUI_END_OF_INLINE
   #define CGUI_BEGIN_INLINE(type, name, args)  CGUI_FUNC(type, name, args)
#endif

/* It's always safe to put an extra semicolon after inline functions. */
#ifndef CGUI_END_OF_INLINE
   #define CGUI_END_OF_INLINE()  ;
#endif

#ifndef CGUI_END_OF_LOCAL_INLINE
   #define CGUI_END_OF_LOCAL_INLINE()  ;
#endif

/* Define shorter ways to declare inline functions. */
#ifdef CGUI_INLINE_PROVIDE_CODE
   #define CGUI_INLINE(type, name, args, code) \
      CGUI_BEGIN_INLINE(type, name, args) code CGUI_END_OF_INLINE()
#else
   #define CGUI_INLINE(type, name, args, code) \
      CGUI_BEGIN_INLINE(type, name, args) CGUI_END_OF_INLINE()
#endif

#define CGUI_LOCAL_INLINE(type, name, args, code) \
   CGUI_BEGIN_LOCAL_INLINE(type, name, args) code \
   CGUI_END_OF_LOCAL_INLINE()



/* Some platforms (eg Watcom) define va_list in a weird way in order to make
   life harder for people who use it... */
#ifndef CGUI_va_list_p_DEFINED
// typedef va_list *cgui_va_list_p;
   #define cgui_va_arg_p(a, b) va_arg(*a, b)
#endif

#ifdef __GNUC__
/*   #define CGUI_PRINTFUNC(type, name, args, a, b)    CGUI_FUNC(type, name, args) __attribute__ ((format (printf, a, b)))*/
   #define CGUI_PRINTFUNC(type, name, args, a, b)    CGUI_FUNC(type, name, args)
#else
   #define CGUI_PRINTFUNC(type, name, args, a, b)    CGUI_FUNC(type, name, args)
#endif


#ifndef CGUI_FUNC_DEPRECATED
   #if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
      #define CGUI_FUNC_DEPRECATED(type, name, args)              CGUI_FUNC(__attribute__ ((deprecated)) type, name, args)
      #define CGUI_PRINTFUNC_DEPRECATED(type, name, args, a, b)   CGUI_PRINTFUNC(__attribute__ ((deprecated)) type, name, args, a, b)
      #define CGUI_INLINE_DEPRECATED(type, name, args, code)      CGUI_INLINE(__attribute__ ((deprecated)) type, name, args, code)
   #else
      #define CGUI_FUNC_DEPRECATED        CGUI_FUNC
      #define CGUI_PRINTFUNC_DEPRECATED   CGUI_PRINTFUNC
      #define CGUI_INLINE_DEPRECATED      CGUI_INLINE
   #endif
#endif
