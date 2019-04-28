/* Metrowerks x86 Runtime Support Library 
 * Copyright © 1995-2003 Metrowerks Corporation.  All rights reserved.
 *
 * $Date: 2003/04/25 05:03:26 $
 * $Revision: 1.2 $
 */

//	More MSVC runtime compatibility routines

#include <stdio.h>
#include <setjmp.h>

#undef _vsnprintf
#undef vsnprintf

asm int __cdecl	_vsnprintf(void);
asm int __cdecl	_vsnprintf(void)
{
	__asm jmp	vsnprintf
}

asm unsigned long long __cdecl	_aullshr(void);
asm unsigned long long __cdecl	_aullshr(void)
{
	__asm {
		test cl, 0x20
		jne hi
		shld edx, eax, cl
		sal eax, cl
		ret
	hi:
		mov edx, eax
		sal edx, cl
		mov eax, 0
		ret
	}
}

// __except_list
// __EH_prolog

// this is just a symbolic constant
int __except_list : 0x00;

// forward to the MW routine
extern void __cdecl __SEHFrameHandler(void);
asm void __cdecl _except_handler3(void);
asm void __cdecl _except_handler3(void)
{
	__asm jmp __SEHFrameHandler
}

asm void __cdecl _EH_prolog(void);
asm void __cdecl _EH_prolog(void)
{
	__asm {
		push	ebp
		mov 	ebp, esp
		push 	-1
		push 	dword ptr [eax+16]
		mov		eax,[esp+12]			// retrieve return code
		push	__SEHFrameHandler
		push	dword ptr fs:[0]
		mov		dword ptr fs:[0],esp
		push	eax						// dummy value
		jmp		eax
	}
}

//	The real version links into the except chain,
//	but we just hack it to work with our current _Setjmp.
asm void __cdecl _setjmp3(void);
asm void __cdecl _setjmp3(void)
{
	__asm {
		POP		ECX					// return address to ECX
		POP		EAX					// env pointer to EAX
		ADD		ESP,4				// ??? extra param
		
		MOV		[EAX], EBX			// save EBX in first loc
		MOV		4[EAX], ESI			// save ESI in second loc
		MOV		8[EAX], EDI			// save EDI in third loc
		MOV		12[EAX], ESP		// save ESP in fourth loc
		MOV		16[EAX], EBP		// save EBP in fifth loc
		MOV		20[EAX], ECX		// save return address in sixth loc
		
		XOR		EAX, EAX			// value is 0, indicating return from setjmp instead of longjmp
		PUSH	EAX					// leave space for the passed param
		PUSH	ECX					// put back return addres
		RETN						// and return
	}
}

#undef _snprintf
#undef snprintf

asm int __cdecl _snprintf(void);
asm int __cdecl _snprintf(void)
{
 __asm jmp snprintf
}

/* Change History
 * ejs 030424	Created
 */
