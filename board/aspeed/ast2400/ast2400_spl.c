/*
 * (C) Copyright 2016-Present, Facebook, Inc.
 * Teddy Reed <reed@fb.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <asm/spl.h>
#include <malloc.h>

#include <image.h>
#include <libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

static ulong fdt_getprop_u32(const void *fdt, int node, const char *prop)
{
	const u32 *cell;
	int len;

	cell = fdt_getprop(fdt, node, prop, &len);
	if (len != sizeof(*cell))
		return -1U;
	return fdt32_to_cpu(*cell);
}

void __noreturn jump(u32 to)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)(unsigned long)to;
	image_entry();
}

u32 spl_boot_device() {
	/* Include this NOP symbol to use the SPL_FRAMEWORK APIs. */
	return BOOT_DEVICE_NONE;
}

void spl_display_print() {
	/* Nothing */
}

void load_fit(u32 from) {
	struct image_header *header;

	header = (struct image_header*)(from);
	if (!IS_ENABLED(CONFIG_SPL_LOAD_FIT) ||
	    image_get_magic(header) != FDT_MAGIC) {
		/* FIT loading is not available or this U-Boot is not a FIT. */
		/* This will bypass signature checking */
		jump(from);
	}

	void *fit = header;
	u32 size = fdt_totalsize(fit);
	size = (size + 3) & ~3;
	u32 base_offset = (size + 3) & ~3;
	debug("size=%x base_offset=%x\n", size, base_offset);

	/* Node path to images */
	int images = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__, images);
		hang();
	}

	/* Be simple, select the first image. */
	int node = fdt_first_subnode(fit, images);
	if (node < 0) {
		debug("%s: Cannot find first image node: %d\n", __func__, node);
		hang();
	}

	/* Get its information and set up the spl_image structure */
	int data_offset = fdt_getprop_u32(fit, node, "data-offset");
	int data_size = fdt_getprop_u32(fit, node, "data-size");
	u32 load = fdt_getprop_u32(fit, node, "load");
	debug("data_offset=%x, data_size=%x\n", data_offset, data_size);
	debug("load=%x\n", load);

	/*
	 * The load and offset *should* be the same, we'll need to fix that
	 * in the FIT generation + external movement.
	 *
	 * For now we can set this to the data.
	 */
	const void *data = (void*)load;

	const void *signature_store = gd_fdt_blob();
	if (signature_store == 0x0) {
		/* It is possible the spl_init method did not find a ftb. */
		printf("No signature store was included in the SPL.\n");
		hang();
	}

	/* Node path to subordinate keys. */
	int keys = fdt_path_offset(fit, FIT_KEYS_PATH);
	if (keys < 0) {
		debug("%s: Cannot find /keys node: %d\n", __func__, keys);
		hang();
	}

	/* Be simple, select the first key. */
	int key_node = fdt_first_subnode(fit, keys);
	int subordinate_verified = 0;

	/* Now verify subordinates as images with inline "data". */
	const void* key_data;
	size_t key_data_size;
	if (fit_image_get_data(fit, key_node, &key_data, &key_data_size)) {
		printf("Unable to find subordinate key data.\n");
		hang();
	}

	if (fit_image_verify_required_sigs(fit, key_node, key_data,
					   key_data_size, signature_store,
					   &subordinate_verified)) {
		printf("Unable to verify required subordinate signature.\n");
		hang();
	}

	if (subordinate_verified != 0) {
		printf("No subordinate keys were verified.\n");
		hang();
	}

	/* Now change the signature store to use the verified subordinate. */
	signature_store = key_data;

	/*
	 * Check that at least 1 image was verified.
	 * This is an interesting error state communication, but it is the API
	 * given, so let's make it as clear as possible.
	 */
	int verified = 0;

	/* Verify all required signatures, keys must be marked required. */
	if (fit_image_verify_required_sigs(fit, node, data, data_size,
					   signature_store, &verified)) {
		printf("Unable to verify required signature.\n");
		hang();
	}

	if (verified != 0) {
		/* When verified is 0, then an image was verified. */
		printf("No images were verified.\n");
		hang();
	}

	printf("Images verified\n");
	//jump(load);
}

void board_init_f(ulong bootflag)
{
	/* Must set up console for printing/logging. */
	preloader_console_init();
	/* Must set up global data pointers for local device tree. */
	spl_init();

	/*
	 * We are not relocated, use the simple malloc with the relocated
	 * malloc start and size configuration options.
	 *
	 * Malloc will forever count forward, there is no free, as state
	 * tracking does not mean anything.
	 */
	gd->malloc_base = CONFIG_SYS_SPL_MALLOC_START;
	gd->malloc_limit = CONFIG_SYS_SPL_MALLOC_SIZE;

	/*
	 * This will never be relocated, so jump directly to the U-boot.
	 */
	 /* jump(CONFIG_SPL_UBOOT_START) */
	load_fit(CONFIG_SYS_SPL_FIT_BASE);
	hang();
}
