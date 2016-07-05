/*
 * (C) Copyright 2004-Present
 * Teddy Reed <reed@fb.com>, Facebook, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __FBYOSEMITE_CONFIG_H
#define __FBYOSEMITE_CONFIG_H

#define CONFIG_BOOTARGS		"debug console=ttyS0,57600n8 root=/dev/ram rw"
#define CONFIG_UPDATE		"tftp 40800000 ast2400.scr; so 40800000'"
#define CONFIG_BOOTCOMMAND 	"bootm 20080000"	/* Location of FIT */
#define CONFIG_BOOTFILE		"all.bin"

/*
 * Memory Configuration
 * Deprecating with v2016.03: CONFIG_ASPEED_WRITE_DEFAULT_ENV
 */
#define PHYS_SDRAM_1_SIZE	0x10000000 	/* 256 MB */

/*
 * Serial configuration
 */
#define CONFIG_CONS_INDEX	2
#define CONFIG_BAUDRATE		57600
#define CONFIG_ASPEED_COM	0x1e784000	/* COM2(UART5) */

/*
 * NIC configuration
 */
#define CONFIG_MAC1_ENABLE
#define CONFIG_MAC1_PHY_SETTING		2
#define CONFIG_MAC2_PHY_SETTING		0
#define CONFIG_ASPEED_MAC_NUMBER	1
#define CONFIG_ASPEED_MAC_CONFIG	1

#include "facebook_common.h"

#endif	/* __FBYOSEMITE_CONFIG_H */
