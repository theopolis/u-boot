/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 * Teddy Reed <reed@fb.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/spl.h>

DECLARE_GLOBAL_DATA_PTR;

void __noreturn jump(void)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)(unsigned long)CONFIG_SYS_UBOOT_START;
	image_entry();
}

void board_init_f(ulong bootflag)
{
	jump();
}
