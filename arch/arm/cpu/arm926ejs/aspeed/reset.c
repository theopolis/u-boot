/*
 * (C) Copyright 2016-present
 * ASPEED and OpenBMC development.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>

#define AST_WDT_BASE 0x1e785000
void reset_cpu(ulong addr)
{
	__raw_writel(0x10, AST_WDT_BASE + 0x04);
	__raw_writel(0x4755, AST_WDT_BASE + 0x08);
	__raw_writel(0x23, AST_WDT_BASE + 0x0c); /* reset the full chip */

	while (1)
	/*nothing*/;
}
