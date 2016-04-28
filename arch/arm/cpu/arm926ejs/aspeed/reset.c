/*
 * (C) Copyright 2016-present
 * ASPEED and OpenBMC development.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>

void reset_cpu(ulong addr)
{
	writel(0x10, CONFIG_AST_WDT_BASE + 0x04);
	writel(0x4755, CONFIG_AST_WDT_BASE + 0x08);
	/* reset the full chip */
	writel(0x23, CONFIG_AST_WDT_BASE + 0x0c);

	while (1)
	/*nothing*/;
}
