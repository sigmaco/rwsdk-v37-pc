/* Metrowerks x86 Runtime Support Library 
 * Copyright © 1995-2003 Metrowerks Corporation.  All rights reserved.
 *
 * $Date: 2003/05/19 18:35:47 $
 * $Revision: 1.2 $
 */

//	MSVC runtime has math lib implemented in _CIxxx functions, 
//	which are called with the arguments pushed on the FPU
//	stack.  These are just stubs to forward to the real function.

#define _MSL_USE_INLINE	0

#include <math.h>

#define MATH_ARG1(func)	\
	asm double __cdecl _CI##func(void); \
	asm double __cdecl _CI##func(void) { \
		__asm sub esp, 8 				/* get room for arg */ \
		__asm fstp qword ptr [esp]	 	/* store arg #1 */ \
		__asm call func 				/* call */ \
		__asm add esp, 8				/* cleanup */ \
		__asm ret						/* home free */ \
	}

#define MATH_ARG2(func)	\
	asm double __cdecl _CI##func(void); \
	asm double __cdecl _CI##func(void) { \
		__asm sub esp, 16 				/* get room for args */ \
		__asm fstp qword ptr [esp+8] 	/* store arg #1 */ \
		__asm fstp qword ptr [esp]	 	/* store arg #2 */ \
		__asm call func 				/* call */ \
		__asm add esp, 16				/* cleanup */ \
		__asm ret						/* home free */ \
	}

MATH_ARG1(acos)
MATH_ARG1(asin)	
MATH_ARG1(atan)	
MATH_ARG2(atan2)
MATH_ARG1(cos)
MATH_ARG1(exp)
MATH_ARG2(fmod)
MATH_ARG1(log)
MATH_ARG1(log10)
MATH_ARG2(pow)
MATH_ARG1(sin)
MATH_ARG1(sinh)
MATH_ARG1(sqrt)
MATH_ARG1(tan)
MATH_ARG1(tanh)

int __cdecl _finite(double x);
int __cdecl _finite(double x)
{
	return isfinite(x);
}

long __cdecl _ftol(float x);
long __cdecl _ftol(float x)
{
	return (long)x;
}

/* Change History
 * ejs 030424	Created
 * ejs 030519	Some bug fixes
 */
 