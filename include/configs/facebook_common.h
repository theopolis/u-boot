/*
 * (C) Copyright 2004-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FACEBOOK_CONFIG_H
#define __FACEBOOK_CONFIG_H

/*
 * Basic boot command configuration based on flash
 */
#define CONFIG_AUTOBOOT_PROMPT		"autoboot in %d seconds (stop with 'Delete' key)...\n"
#define CONFIG_AUTOBOOT_STOP_STR	"\x1b\x5b\x33\x7e"	/* 'Delete', ESC[3~ */
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_ZERO_BOOTDELAY_CHECK

/*
 * Environment configuration
 * This used to have:
 *   CONFIG_ENV_IS_IN_FLASH
 *   CONFIG_ENV_IS_IN_SPI_FLASH
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_OFFSET	0x60000		/* environment starts here  */
#define CONFIG_ENV_SIZE		0x20000		/* Total Size of Environment Sector */
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_OVERWRITE

/*
 * Flash configuration
 * It is possible to run using the SMC and not enable flash
 *   CONFIG_CMD_FLASH
 */
#define CONFIG_SYS_NO_FLASH

/*
 * Serial configuration
 */
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_REG_SIZE -4

/*
 * Watchdog timer configuration
 */
#define CONFIG_ASPEED_ENABLE_WATCHDOG
#define CONFIG_ASPEED_WATCHDOG_TIMEOUT	(5*60) /* 5 minutes */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory   */
#define CONFIG_SYS_TIMERBASE	0x1E782000 	/* use timer 1 */
#define CONFIG_SYS_HZ 		1000

/*
 * NIC configuration
 */
#define CONFIG_NET_RANDOM_ETHADDR
#define CONFIG_LIB_RAND

/*
 * SRAM configuration
 */
#define CONFIG_SYS_SRAM_BASE		0x1E720000
#define CONFIG_SYS_SRAM_TOP		0x1E727FFF

/*
 * Command configuration
 */
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_MEMINFO
#define CONFIG_CMD_MEMTEST
#define CONFIG_CMD_SDRAM

/*
 * Hash algorithm support
 */
#define CONFIG_SHA256

#ifdef CONFIG_SPL
/*
 * An SPL build expects U-Boot to be in another flash.
 * Notice the external flash (0x24000000) includes a 32kB offset (0x8000).
 * This allows mkimage to build a FIT, with a max size of 32kB to proceed the
 * firmware blob.
 */
#define CONFIG_SYS_UBOOT_START		0x24008000

/* Define the base address to search for a FIT within the SPL. */
#define CONFIG_SYS_SPL_FIT_BASE		0x24000000
/* This is an in-development change required for mkimage. */
#define CONFIG_SPL_LOAD_FIT_OFFSET	0x00008000
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_MAX_FOOTPRINT	0x100000
/* During an SPL build the base is 0x0. */
#define CONFIG_SYS_TEXT_BASE		0x00000000
#define CONFIG_SPL_TEXT_BASE		CONFIG_SYS_TEXT_BASE
/* Grow the stack down from 0x6000 to an expected max of 12kB. */
#define CONFIG_SPL_STACK		(CONFIG_SYS_SRAM_BASE + 0x6000)
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SPL_STACK
/* Establish an 'arena' for heap from 0x1000 - 0x3000, 8k */
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SYS_SRAM_BASE + 0x1000)
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x2000 /* 8kB */

/* General SPL build feature includes. */
#define CONFIG_SPL_DISPLAY_PRINT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT

/* Verified boot required features. */
#define CONFIG_SPL_CRYPTO_SUPPORT
#define CONFIG_SPL_HASH_SUPPORT
#define CONFIG_SPL_SHA256
/* This will increase binary size by +10kB */
#define CONFIG_FIT_SPL_PRINT
#else
/* During the U-Boot build the base address is the SPL FIT start address. */
#define CONFIG_SYS_TEXT_BASE		CONFIG_SYS_UBOOT_START
#endif
#endif

#include "ast2400_common.h"

#endif /* __FACEBOOK_CONFIG_H */
