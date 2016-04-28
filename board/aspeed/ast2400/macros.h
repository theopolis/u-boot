/*
 * AST2400 Platform initialization macros
 *
 * (C) Copyright 2004, ASPEED Technology Inc.
 * Gary Hsu, <gary_hsu@aspeedtech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/******************************************************************************
 Calibration Macro Start
 Usable registers:
  r0, r1, r2, r3, r5, r6, r7, r8, r9, r10, r11

 Unusable registers:
  r4
 ******************************************************************************/
PATTERN_TABLE:
	.word	0xff00ff00
	.word	0xcc33cc33
	.word	0xaa55aa55
	.word	0x88778877
	.word	0x92cc4d6e       @ 5
	.word	0x543d3cde
	.word	0xf1e843c7
	.word	0x7c61d253
	.word	0x00000000       @ 8

.macro init_delay_timer
	ldr	r0, =0x1e782024          @ Set Timer3 Reload
	str	r2, [r0]

	ldr	r0, =0x1e6c0038          @ Clear Timer3 ISR
	ldr	r1, =0x00040000
	str	r1, [r0]

	ldr	r0, =0x1e782030          @ Enable Timer3
	ldr	r1, [r0]
	mov	r2, #7
	orr	r1, r1, r2, lsl #8
	str	r1, [r0]

	ldr	r0, =0x1e6c0090          @ Check ISR for Timer3 timeout
.endm

.macro check_delay_timer
	ldr	r1, [r0]
	bic	r1, r1, #0xFFFBFFFF
	mov	r2, r1, lsr #18
	cmp	r2, #0x01
.endm

.macro clear_delay_timer
	ldr	r0, =0x1e782030          @ Disable Timer3
	ldr	r1, [r0]
	bic	r1, r1, #0x00000F00
	str	r1, [r0]

	ldr	r0, =0x1e6c0038          @ Clear Timer3 ISR
	ldr	r1, =0x00040000
	str	r1, [r0]
.endm

.macro record_dll2_pass_range
	ldr	r1, [r0]
	bic	r2, r1, #0xFFFFFF00
	cmp	r2, r3                   @ record min
	bicgt	r1, r1, #0x000000FF
	orrgt	r1, r1, r3
	bic	r2, r1, #0xFFFF00FF
	cmp	r3, r2, lsr #8           @ record max
	bicgt	r1, r1, #0x0000FF00
	orrgt	r1, r1, r3, lsl #8
	str	r1, [r0]
.endm

.macro record_dll2_pass_range_h
	ldr	r1, [r0]
	bic	r2, r1, #0xFF00FFFF
	mov	r2, r2, lsr #16
	cmp	r2, r3                   @ record min
	bicgt	r1, r1, #0x00FF0000
	orrgt	r1, r1, r3, lsl #16
	bic	r2, r1, #0x00FFFFFF
	cmp	r3, r2, lsr #24          @ record max
	bicgt	r1, r1, #0xFF000000
	orrgt	r1, r1, r3, lsl #24
	str	r1, [r0]
.endm

.macro init_spi_checksum
	ldr	r0, =0x1e620084
	ldr	r1, =0x20010000
	str	r1, [r0]
	ldr	r0, =0x1e62008C
	ldr	r1, =0x20000200
	str	r1, [r0]
	ldr	r0, =0x1e620080
	ldr	r1, =0x0000000D
	orr	r2, r2, r7
	orr	r1, r1, r2, lsl #8
	and	r2, r6, #0xF
	orr	r1, r1, r2, lsl #4
	str	r1, [r0]
	ldr	r0, =0x1e620008
	ldr	r2, =0x00000800
.endm
