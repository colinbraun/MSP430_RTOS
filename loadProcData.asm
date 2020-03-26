	.cdecls C, list, "msp430.h"	; this allows us to use C headers

	.text						; locates code in 'text' section

	.global loadProcData					; declares as global

; updates the spot in memory pointed to by the argument (received in R12 by the MSP430 calling convention)
loadProcData:
	mov	#0x03, 0(R12) ; The 0 here is actually significant. It differentiates the addressing mode. This is just a test.
	reta ; return from call

	.end

