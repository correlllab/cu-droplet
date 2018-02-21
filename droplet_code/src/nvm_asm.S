/**
 * \file
 *
 * \brief Non Volatile Memory controller driver
 *
 * Copyright (c) 2010 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#include <avr/io.h>


	/* IAR doesn't accept dots in macro names */
	.macro  ld_addr, reg, sym
	lda.w   \reg, \sym
	.endm

	/* Define a function \a name that is either globally visible or only
	 * file-local.
	 */
	.macro gas_begin_func name, is_public
		.if \is_public
		.global \name
		.endif
		.section .text.\name, "ax", @progbits
		.type \name, @function
	\name :
	.endm

	/* Define a function \a name that is either globally visible or only
	 * file-local in a given segment.
	 */
	.macro gas_begin_func_segm name, is_public, segment
		.if \is_public
		.global \name
		.endif
		.section .\segment, "ax", @progbits
		.type \name, @function
	\name :
	.endm

#define FUNCTION(name) gas_begin_func name 0
#define PUBLIC_FUNCTION(name)   gas_begin_func name 1
#define PUBLIC_FUNCTION_SEGMENT(name, segment)          \
	gas_begin_func_segm name 1 segment
#define END_FUNC(name)                                  \
	.size   name, . - name

#  define CCP_SPM_gc 0x9D
//! NVM busy flag
#  define NVM_NVMBUSY_bp 7

//! NVM command for loading flash buffer
#  define NVM_CMD_LOAD_FLASH_BUFFER_gc 0x23
	PUBLIC_FUNCTION(nvm_read_byte)
	lds r20, NVM_CMD          ; Store NVM command register
	mov ZL, r22               ; Load byte index into low byte of Z.
	mov ZH, r23               ; Load high byte into Z.
	sts NVM_CMD, r24          ; Load prepared command into NVM Command register.
	lpm r24, Z                ; Perform an LPM to read out byte
	sts NVM_CMD, r20          ; Restore NVM command register

	ret

	END_FUNC(nvm_read_byte)

	/**
	 * \brief Perform SPM command
	 */
	PUBLIC_FUNCTION_SEGMENT(nvm_common_spm, BOOT)
	/**
	 * For GCC:
	 * \param address uint32_t r22:r25
	 * \param nvm_cmd uint8_t  r20
	 */
	in r25, RAMPZ         ; Store RAMPZ. Highest address byte is ignored, so using that
	out RAMPZ, r24        ; Load R24 into RAMPZ
	movw ZL, r22          ; Load R22:R23 into Z.
	lds r24, NVM_CMD      ; Store NVM command register (r24 is no longer needed)
	sts NVM_CMD, r20      ; Load prepared command into NVM Command register.
	ldi r23, CCP_SPM_gc   ; Prepare Protect SPM signature (r23 is no longer needed)
	sts CCP, r23          ; Enable SPM operation (this disables interrupts for 4 cycles).
	spm                   ; Self-program.
	sts NVM_CMD, r24      ; Restore NVM command register
	out RAMPZ, r25        ; Restore RAMPZ register.

	ret

	END_FUNC(nvm_common_spm)

	/**
	 * \brief Load byte to page buffer
	 *
	 */
	PUBLIC_FUNCTION_SEGMENT(nvm_flash_load_word_to_buffer, BOOT)

	/**
	 * For GCC:
	 * \param word_addr uint32_t r22:r25
	 * \param data      uint16_t r20:r21
	 */
wait_nvm:
	lds r18, NVM_STATUS
	sbrc r18, NVM_NVMBUSY_bp
	rjmp wait_nvm

	in r25, RAMPZ         ; Store RAMPZ. Highest address byte is ignored, so using that
	out RAMPZ, r24        ; Load R24 into RAMPZ
	movw ZL, r22          ; Load R22:R23 into Z.

	lds r24, NVM_CMD      ; Store NVM command register (r24 is no longer needed)
	ldi r18, NVM_CMD_LOAD_FLASH_BUFFER_gc
	sts NVM_CMD, r18      ; Load prepared command into NVM Command register.

	movw r0, r20          ; Load R20:R21 into R0:R1
	spm                   ; Self-program.

	clr r1                ; Clear R1 for GCC _zero_reg_ to function properly.
	sts NVM_CMD, r24      ; Restore NVM command register
	out RAMPZ, r25        ; Restore RAMPZ register.

	ret

	END_FUNC(nvm_flash_load_word_to_buffer)