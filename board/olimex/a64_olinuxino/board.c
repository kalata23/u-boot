/*
 * OLinuXino Board initialization
 *
 * Copyright (C) 2018 Olimex Ltd.
 *   Author: Stefan Mavrodiev <stefan@olimex.com>
 *
 * SPDX-License-Identifier: (GPL-2.0+ OR MIT)
 */
#include <common.h>
#include <dm.h>
#include <environment.h>
#include <axp_pmic.h>
#include <generic-phy.h>
#include <phy-sun4i-usb.h>
#include <netdev.h>
#include <miiphy.h>
#include <nand.h>
#include <mmc.h>
#include <spl.h>
#include <cli.h>
#include <asm/arch/display.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/gpio.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mmc.h>
#include <asm/arch/spl.h>
#include <asm/setup.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/ctype.h>

#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

#ifdef CONFIG_FDT_FIXUP_PARTITIONS
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#endif

#include "../common/board_detect.h"
#include "../common/boards.h"

DECLARE_GLOBAL_DATA_PTR;

void eth_init_board(void)
{

}

void i2c_init_board(void)
{
// TODO: fix this!
#ifdef CONFIG_I2C0_ENABLE
	sunxi_gpio_set_cfgpin(SUNXI_GPB(0), SUN4I_GPB_TWI0);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(1), SUN4I_GPB_TWI0);
	clock_twi_onoff(0, 1);
#endif

#ifdef CONFIG_I2C1_ENABLE
	sunxi_gpio_set_cfgpin(SUNXI_GPB(18), SUN4I_GPB_TWI1);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(19), SUN4I_GPB_TWI1);
	clock_twi_onoff(1, 1);
#endif

#if defined(CONFIG_I2C2_ENABLE) && !defined(CONFIG_SPL_BUILD)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(20), SUN4I_GPB_TWI2);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(21), SUN4I_GPB_TWI2);
	clock_twi_onoff(2, 1);
#endif
}

#if defined(CONFIG_ENV_IS_IN_SPI_FLASH) || defined(CONFIG_ENV_IS_IN_FAT) || defined(CONFIG_ENV_IS_IN_EXT4)
enum env_location env_get_location(enum env_operation op, int prio)
{
	uint32_t boot = sunxi_get_boot_device();

	switch (boot) {
		/* In case of FEL boot check board storage */
		case BOOT_DEVICE_BOARD:
			if (prio == 0)
				return ENVL_EXT4;
			else if (prio == 1)
				return ENVL_FAT;
			else
				return ENVL_UNKNOWN;

		case BOOT_DEVICE_SPI:
			return (prio == 0) ? ENVL_SPI_FLASH : ENVL_UNKNOWN;

		case BOOT_DEVICE_MMC1:
		case BOOT_DEVICE_MMC2:
			if (prio == 0)
				return ENVL_EXT4;
			else if (prio == 1)
				return ENVL_FAT;
			else
				return ENVL_UNKNOWN;

		default:
			return ENVL_UNKNOWN;
	}
}
#endif

#if defined(CONFIG_ENV_IS_IN_EXT4)
char *get_fat_device_and_part(void)
{
	uint32_t boot = sunxi_get_boot_device();

	switch (boot) {
		case BOOT_DEVICE_MMC1:
			return "0:auto";
		case BOOT_DEVICE_MMC2:
			return "1:auto";
		default:
			return CONFIG_ENV_EXT4_DEVICE_AND_PART;
	}
}
#endif

/* add board specific code here */
int board_init(void)
{
	__maybe_unused struct udevice *dev;
	int ret;

	gd->bd->bi_boot_params = (PHYS_SDRAM_0 + 0x100);

#ifdef CONFIG_DM_SPI_FLASH
	ret = uclass_first_device(UCLASS_SPI_FLASH, &dev);
	if (ret) {
		printf("Failed to find SPI flash device\n");
		return 0;
	}

	ret = device_probe(dev);
	if (ret) {
		printf("Failed to probe SPI flash device\n");
		return 0;
	}
#endif

#ifdef CONFIG_DM_I2C
	/*
	 * Temporary workaround for enabling I2C clocks until proper sunxi DM
	 * clk, reset and pinctrl drivers land.
	 */
	i2c_init_board();
#endif

	return 0;
}

/*
 * On older SoCs the SPL is actually at address zero, so using NULL as
 * an error value does not work.
 */
#define INVALID_SPL_HEADER ((void *)~0UL)

static struct boot_file_head * get_spl_header(uint8_t req_version)
{
	struct boot_file_head *spl = (void *)(ulong)SPL_ADDR;
	uint8_t spl_header_version = spl->spl_signature[3];

	/* Is there really the SPL header (still) there? */
	if (memcmp(spl->spl_signature, SPL_SIGNATURE, 3) != 0)
		return INVALID_SPL_HEADER;

	if (spl_header_version < req_version) {
		printf("sunxi SPL version mismatch: expected %u, got %u\n",
		       req_version, spl_header_version);
		return INVALID_SPL_HEADER;
	}

	return spl;
}

int dram_init(void)
{
	struct boot_file_head *spl = get_spl_header(SPL_DRAM_HEADER_VERSION);

	if (spl == INVALID_SPL_HEADER)
		gd->ram_size = get_ram_size((long *)PHYS_SDRAM_0,
					    PHYS_SDRAM_0_SIZE);
	else
		gd->ram_size = (phys_addr_t)spl->dram_size << 20;

	if (gd->ram_size > CONFIG_SUNXI_DRAM_MAX_SIZE)
		gd->ram_size = CONFIG_SUNXI_DRAM_MAX_SIZE;

	return 0;
}

#ifdef CONFIG_MMC
static void mmc_pinmux_setup(int sdc)
{
	unsigned int pin;

	switch (sdc) {
	case 0:
		/* SDC0: PF0-PF5 */
		for (pin = SUNXI_GPF(0); pin <= SUNXI_GPF(5); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUNXI_GPF_SDC0);
			sunxi_gpio_set_drv(pin, 2);
		}
		break;

	case 2:
		/* SDC2: PC5-PC6, PC8-PC16 */
		for (pin = SUNXI_GPC(5); pin <= SUNXI_GPC(6); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_SDC2);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}

		for (pin = SUNXI_GPC(8); pin <= SUNXI_GPC(16); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_SDC2);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}
		break;

	default:
		break;
	}
}

int board_mmc_init(bd_t *bis)
{
	struct mmc *mmc;

	/* Try to initialize MMC0 */
	mmc_pinmux_setup(0);
	mmc = sunxi_mmc_init(0);
	if (!mmc) {
		printf("Failed to init MMC0!\n");
		return -1;
	}

	if (eeprom->config.storage != 'e')
		return 0;

	/* Initialize MMC2 on boards with eMMC */
	mmc_pinmux_setup(2);
	mmc = sunxi_mmc_init(2);
	if (!mmc) {
		printf("Failed to init MMC2!\n");
		return -1;
	}

	return 0;
}
#ifndef CONFIG_SPL_BUILD
int mmc_get_env_dev(void)
{
	unsigned long bootdev = 0;
	char *bootdev_string;

	bootdev_string = env_get("mmc_bootdev");

	if (bootdev_string) {
		bootdev = simple_strtoul(bootdev_string, NULL, 10);
	}
	return bootdev;
}
#endif /* !CONFIG_SPL_BUILD */

#endif /* CONFIG_MMC */

static void sunxi_spl_store_dram_size(phys_addr_t dram_size)
{
	struct boot_file_head *spl = get_spl_header(SPL_DT_HEADER_VERSION);

	if (spl == INVALID_SPL_HEADER)
		return;

	/* Promote the header version for U-Boot proper, if needed. */
	if (spl->spl_signature[3] < SPL_DRAM_HEADER_VERSION)
		spl->spl_signature[3] = SPL_DRAM_HEADER_VERSION;

	spl->dram_size = dram_size >> 20;
}

void sunxi_board_init(void)
{
	printf("DRAM:");
	gd->ram_size = sunxi_dram_init();
	printf(" %d MiB\n", (int)(gd->ram_size >> 20));
	if (!gd->ram_size)
		hang();

	sunxi_spl_store_dram_size(gd->ram_size);
}

#if defined(CONFIG_SPL_BOARD_INIT)
void spl_board_init(void)
{
	struct mmc *mmc = NULL;

	/* Make sure ram is empty */
	memset((void *)eeprom, 0xFF, 256);

	/**
	 * Try some detection:
	 *
	 * 1. If RAM > 1G, then A64-OLinuXino-2Ge8G-IND
	 * 2. If SPI is present, then A64-OLinuXino-1Gs16M
	 * 3. If eMMC is present, then A64-OLinuXino-1Ge4GW
	 * 4. Else -> A64-OLinuXino-1G
	 */
	if ((int)(gd->ram_size >> 20) > 1024) {
		eeprom->id = 8861;
		eeprom->config.storage = 'e';
		return;
	}

	if (sunxi_spi_is_present()) {
		eeprom->id = 9065;
		eeprom->config.storage = 's';
		return;
	}

	mmc_initialize(NULL);
	mmc = find_mmc_device(1);
	if (!mmc_init(mmc)) {
		eeprom->id = 8367;
		eeprom->config.storage = 'e';
		return;
	}

	eeprom->id = 8857;
}
#endif

#ifndef CONFIG_SPL_BUILD

#ifdef CONFIG_USB_GADGET
int g_dnl_board_usb_cable_connected(void)
{
	/* This is workaround. Must be fixed in the future */
	return 1;
}
#endif /* CONFIG_USB_GADGET */

#ifdef CONFIG_SERIAL_TAG
void get_board_serial(struct tag_serialnr *serialnr)
{
	char *serial_string;
	unsigned long long serial;

	serial_string = env_get("serial#");

	if (serial_string) {
		serial = simple_strtoull(serial_string, NULL, 16);

		serialnr->high = (unsigned int) (serial >> 32);
		serialnr->low = (unsigned int) (serial & 0xffffffff);
	} else {
		serialnr->high = 0;
		serialnr->low = 0;
	}
}
#endif /* CONFIG_SERIAL_TAG */

/*
 * Check the SPL header for the "sunxi" variant. If found: parse values
 * that might have been passed by the loader ("fel" utility), and update
 * the environment accordingly.
 */
static void parse_spl_header(const uint32_t spl_addr)
{
	struct boot_file_head *spl = get_spl_header(SPL_ENV_HEADER_VERSION);

	if (spl == INVALID_SPL_HEADER)
		return;

	if (!spl->fel_script_address)
		return;

	if (spl->fel_uEnv_length != 0) {
		/*
		 * data is expected in uEnv.txt compatible format, so "env
		 * import -t" the string(s) at fel_script_address right away.
		 */
		himport_r(&env_htab, (char *)(uintptr_t)spl->fel_script_address,
			  spl->fel_uEnv_length, '\n', H_NOCLEAR, 0, 0, NULL);
		return;
	}
	/* otherwise assume .scr format (mkimage-type script) */
	env_set_hex("fel_scriptaddr", spl->fel_script_address);
}
/*
 * Note this function gets called multiple times.
 * It must not make any changes to env variables which already exist.
 */
static void setup_environment(const void *fdt)
{
	char serial_string[17] = { 0 };
	char fdtfile[64];
	unsigned int sid[4];
	uint8_t mac_addr[6];
	char ethaddr[16];
	int i, ret;

	ret = sunxi_get_sid(sid);
	if (ret == 0 && sid[0] != 0) {


		sid[3] = crc32(0, (unsigned char *)&sid[1], 12);

		/* Ensure the NIC specific bytes of the mac are not all 0 */
		if ((sid[3] & 0xffffff) == 0)
			sid[3] |= 0x800000;

		for (i = 0; i < 4; i++) {
			sprintf(ethaddr, "ethernet%d", i);
			if (!fdt_get_alias(fdt, ethaddr))
				continue;

			if (i == 0)
				strcpy(ethaddr, "ethaddr");
			else
				sprintf(ethaddr, "eth%daddr", i);

			/* Non OUI / registered MAC address */
			mac_addr[0] = (i << 4) | 0x02;
			mac_addr[1] = (sid[0] >>  0) & 0xff;
			mac_addr[2] = (sid[3] >> 24) & 0xff;
			mac_addr[3] = (sid[3] >> 16) & 0xff;
			mac_addr[4] = (sid[3] >>  8) & 0xff;
			mac_addr[5] = (sid[3] >>  0) & 0xff;

			if (!env_get(ethaddr))
				eth_env_set_enetaddr(ethaddr, mac_addr);
		}

		if (!env_get("serial#")) {
			snprintf(serial_string, sizeof(serial_string),
				"%08x%08x", sid[0], sid[3]);

			env_set("serial#", serial_string);
		}
	}

	sprintf(fdtfile, "allwinner/%s", olimex_get_board_fdt());
	env_set("fdtfile", fdtfile);

}

int misc_init_r(void)
{
	__maybe_unused struct udevice *dev;
	uint boot;
	int ret;

	env_set("fel_booted", NULL);
	env_set("fel_scriptaddr", NULL);
	env_set("mmc_bootdev", NULL);

	boot = sunxi_get_boot_device();
	/* determine if we are running in FEL mode */
	if (boot == BOOT_DEVICE_BOARD) {
		env_set("fel_booted", "1");
		parse_spl_header(SPL_ADDR);
	/* or if we booted from MMC, and which one */
	} else if (boot == BOOT_DEVICE_MMC1) {
		env_set("mmc_bootdev", "0");
	} else if (boot == BOOT_DEVICE_MMC2) {
		env_set("mmc_bootdev", "1");
	} else if (boot == BOOT_DEVICE_SPI) {
		env_set("spi_booted", "1");
	}

	/* Setup environment */
	setup_environment(gd->fdt_blob);

#ifdef CONFIG_USB_MUSB_GADGET
	ret = uclass_first_device(UCLASS_USB_GADGET_GENERIC, &dev);
	if (!dev || ret) {
		printf("No USB device found\n");
		return 0;
	}

	ret = device_probe(dev);
	if (ret) {
		printf("Failed to probe USB device\n");
		return 0;
	}
#endif
	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	int __maybe_unused r;

	/*
	 * Call setup_environment again in case the boot fdt has
	 * ethernet aliases the u-boot copy does not have.
	 */
	setup_environment(blob);

#ifdef CONFIG_VIDEO_DT_SIMPLEFB
	r = sunxi_simplefb_setup(blob);
	if (r)
		return r;
#endif
	return 0;
}


int show_board_info(void)
{
	printf("Model: %s\n", olimex_get_board_name());

	return 0;
}

#ifdef CONFIG_SET_DFU_ALT_INFO
void set_dfu_alt_info(char *interface, char *devstr)
{
	char *p = NULL;

	printf("interface: %s, devstr: %s\n", interface, devstr);

#ifdef CONFIG_DFU_RAM
	if (!strcmp(interface, "ram"))
		p = env_get("dfu_alt_info_ram");
#endif

#ifdef CONFIG_DFU_SF
	if (!strcmp(interface, "sf"))
		p = env_get("dfu_alt_info_sf");
#endif
	env_set("dfu_alt_info", p);
}

#endif /* CONFIG_SET_DFU_ALT_INFO */

#endif /* !CONFIG_SPL_BUILD */


#if defined(CONFIG_SPL_LOAD_FIT) || defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	const char *dtb;

	dtb = olimex_get_board_fdt();
	return (!strncmp(name, dtb, strlen(dtb) - 4)) ? 0 : -1;
}
#endif

#ifdef CONFIG_FDT_FIXUP_PARTITIONS
struct node_info nodes[] = {
	{ "jedec,spi-nor", MTD_DEV_TYPE_NOR, },
};
#endif

#if defined(CONFIG_OF_SYSTEM_SETUP)
int ft_system_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
	if (eeprom->config.storage == 's')
		fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif

	return 0;
}
#endif
