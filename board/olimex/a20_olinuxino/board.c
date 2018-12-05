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
#include <crc.h>
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
#include <asm/armv7.h>
#include <asm/setup.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/ctype.h>

#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

#include "../common/board_detect.h"
#include "../common/boards.h"

#define GMAC_MODE_RGMII		0
#define GMAC_MODE_MII		1

DECLARE_GLOBAL_DATA_PTR;

void eth_init_board(void)
{
	static struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	uint8_t tx_delay = 0;
	uint8_t mode;
	int pin;

	if (!olimex_eeprom_is_valid())
		return;

 	mode = (olimex_board_is_lime() ||
		olimex_board_is_micro()) ?
 		GMAC_MODE_MII : GMAC_MODE_RGMII;

	if (olimex_board_is_lime2()) {
		if (eeprom->revision.major > 'E')
			/* RTL8211E */
			tx_delay = 2;
		else if (eeprom->revision.major > 'G')
			/* KSZ9031 */
			tx_delay = 4;
	} else if (olimex_board_is_som204_evb()) {
		tx_delay = 4;
	}

	/* Set up clock gating */
	setbits_le32(&ccm->ahb_gate1, 0x1 << AHB_GATE_OFFSET_GMAC);

	if (mode == GMAC_MODE_RGMII) {
		setbits_le32(&ccm->gmac_clk_cfg,
			     CCM_GMAC_CTRL_TX_CLK_DELAY(tx_delay));
		setbits_le32(&ccm->gmac_clk_cfg,
			     CCM_GMAC_CTRL_TX_CLK_SRC_INT_RGMII |
			     CCM_GMAC_CTRL_GPIT_RGMII);
	} else {
		setbits_le32(&ccm->gmac_clk_cfg,
			     CCM_GMAC_CTRL_TX_CLK_SRC_MII |
			     CCM_GMAC_CTRL_GPIT_MII);
	}

	/* Configure pins for GMAC */
	for (pin = SUNXI_GPA(0); pin <= SUNXI_GPA(16); pin++) {

		/* skip unused pins in RGMII mode */
		if (mode == GMAC_MODE_RGMII ) {
			if (pin == SUNXI_GPA(9) || pin == SUNXI_GPA(14))
				continue;
		}

		sunxi_gpio_set_cfgpin(pin, SUN7I_GPA_GMAC);
		sunxi_gpio_set_drv(pin, 3);
	}

	/* A20-OLinuXino-MICRO needs additional signal for TXERR */
	if (olimex_board_is_micro()) {
		sunxi_gpio_set_cfgpin(SUNXI_GPA(17),  SUN7I_GPA_GMAC);
	}
}

void i2c_init_board(void)
{
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
			if (olimex_eeprom_is_valid() &&
			    eeprom->config.storage == 's') {
				switch (prio) {
					case 0:
						return ENVL_SPI_FLASH;
					case 1:
						return ENVL_EXT4;
					case 2:
						return ENVL_FAT;
					default:
						return ENVL_UNKNOWN;
				}
			} else {
				if (prio == 0)
					return ENVL_EXT4;
				else if (prio == 1)
					return ENVL_FAT;
				else
					return ENVL_UNKNOWN;

			}

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
	__maybe_unused int id_pfr1, ret, satapwr_pin, macpwr_pin, btpwr_pin;
	__maybe_unused struct udevice *dev;

	gd->bd->bi_boot_params = (PHYS_SDRAM_0 + 0x100);

	asm volatile("mrc p15, 0, %0, c0, c1, 1" : "=r"(id_pfr1));
	debug("id_pfr1: 0x%08x\n", id_pfr1);
	/* Generic Timer Extension available? */
	if ((id_pfr1 >> CPUID_ARM_GENTIMER_SHIFT) & 0xf) {
		uint32_t freq;

		debug("Setting CNTFRQ\n");

		/*
		 * CNTFRQ is a secure register, so we will crash if we try to
		 * write this from the non-secure world (read is OK, though).
		 * In case some bootcode has already set the correct value,
		 * we avoid the risk of writing to it.
		 */
		asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r"(freq));
		if (freq != COUNTER_FREQUENCY) {
			debug("arch timer frequency is %d Hz, should be %d, fixing ...\n",
			      freq, COUNTER_FREQUENCY);
#ifdef CONFIG_NON_SECURE
			printf("arch timer frequency is wrong, but cannot adjust it\n");
#else
			asm volatile("mcr p15, 0, %0, c14, c0, 0"
				     : : "r"(COUNTER_FREQUENCY));
#endif
		}
	}

	axp_gpio_init();

	if (olimex_eeprom_is_valid()) {
		/*
		 * Setup SATAPWR
		 */
		if(olimex_board_is_micro())
			satapwr_pin = sunxi_name_to_gpio("PB8");
		else
			satapwr_pin = sunxi_name_to_gpio("PC3");

		if(satapwr_pin > 0) {
			gpio_request(satapwr_pin, "satapwr");
			gpio_direction_output(satapwr_pin, 1);
			/* Give attached sata device time to power-up to avoid link timeouts */
			mdelay(500);
		}

		/*
		 * A20-SOM204 needs manual reset for rt8723bs chip
		 */
		if (olimex_board_is_som204_evb()) {
			btpwr_pin = sunxi_name_to_gpio("PB11");

			gpio_request(btpwr_pin, "btpwr");
			gpio_direction_output(btpwr_pin, 0);
			mdelay(100);
			gpio_direction_output(btpwr_pin, 1);
		}

#ifdef CONFIG_DM_SPI_FLASH
		if (eeprom->config.storage == 's' ||
		    eeprom->id == 8958 ||
		    eeprom->id == 9604 ||
		    eeprom->id == 9613) {

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
		}
#endif
	}

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_0, PHYS_SDRAM_0_SIZE);

	return 0;
}

#ifdef CONFIG_NAND_SUNXI
static void nand_pinmux_setup(void)
{
	unsigned int pin;

	for (pin = SUNXI_GPC(0); pin <= SUNXI_GPC(2); pin++)
		sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_NAND);
	for (pin = SUNXI_GPC(4); pin <= SUNXI_GPC(6); pin++)
		sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_NAND);
	for (pin = SUNXI_GPC(4); pin <= SUNXI_GPC(6); pin++)
		sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_NAND);
	for (pin = SUNXI_GPC(8); pin <= SUNXI_GPC(15); pin++)
		sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_NAND);
}

static void nand_clock_setup(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	setbits_le32(&ccm->ahb_gate0, (CLK_GATE_OPEN << AHB_GATE_OFFSET_NAND0));
	setbits_le32(&ccm->nand0_clk_cfg, CCM_NAND_CTRL_ENABLE | AHB_DIV_1);
}

void board_nand_init(void)
{
	if (eeprom->config.storage != 'n')
		return;

	nand_pinmux_setup();
	nand_clock_setup();
#ifndef CONFIG_SPL_BUILD
	sunxi_nand_init();
#endif
}
#endif /* CONFIG_NAND_SUNXI */

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
		/* SDC2: PC6-PC11 */
		for (pin = SUNXI_GPC(6); pin <= SUNXI_GPC(11); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_SDC2);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}
		break;

	case 3:
		/* SDC3: PI4-PI9 */
		for (pin = SUNXI_GPI(4); pin <= SUNXI_GPI(9); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUNXI_GPI_SDC3);
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

	/* Initialize MMC2 on boards with eMMC */
	if (eeprom->config.storage == 'e') {
		mmc_pinmux_setup(2);
		mmc = sunxi_mmc_init(2);
		if (!mmc) {
			printf("Failed to init MMC2!\n");
			return -1;
		}
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

#ifdef CONFIG_BOARD_EARLY_INIT_R
int board_early_init_r(void)
{
#ifdef CONFIG_MMC
	mmc_pinmux_setup(0);

	if (eeprom->config.storage == 'e')
		mmc_pinmux_setup(2);

	if (olimex_board_is_micro() ||
		olimex_board_is_som_evb())
		mmc_pinmux_setup(3);
#endif
	return 0;
}
#endif /* CONFIG_BOARD_EARLY_INIT_R */

void sunxi_board_init(void)
{
	int power_failed = 0;

	power_failed = axp_init();
	if (power_failed)
		printf("axp_init failed!\n");

	power_failed |= axp_set_dcdc2(CONFIG_AXP_DCDC2_VOLT);
	power_failed |= axp_set_dcdc3(CONFIG_AXP_DCDC3_VOLT);
	power_failed |= axp_set_aldo2(CONFIG_AXP_ALDO2_VOLT);
	power_failed |= axp_set_aldo3(CONFIG_AXP_ALDO3_VOLT);
	power_failed |= axp_set_aldo4(CONFIG_AXP_ALDO4_VOLT);

	printf("DRAM:");
	gd->ram_size = sunxi_dram_init();
	printf(" %d MiB\n", (int)(gd->ram_size >> 20));
	if (!gd->ram_size)
		hang();

	/*
	 * Only clock up the CPU to full speed if we are reasonably
	 * assured it's being powered with suitable core voltage
	 */
	if (!power_failed)
		clock_set_pll1(CONFIG_SYS_CLK_FREQ);
	else
		printf("Failed to set core voltage! Can't set CPU frequency\n");

	if (olimex_i2c_eeprom_read())
		printf("EEPROM: Error\n");
	else if (olimex_eeprom_is_valid())
		printf("EEPROM: Ready\n");
	else
		printf("EEPROM: Corrupted!\n");
}

#ifndef CONFIG_SPL_BUILD

#ifdef CONFIG_USB_GADGET
int g_dnl_board_usb_cable_connected(void)
{
	struct udevice *dev;
	struct phy phy;
	int ret;

	ret = uclass_get_device(UCLASS_USB_DEV_GENERIC, 0, &dev);
	if (ret) {
		pr_err("%s: Cannot find USB device\n", __func__);
		return ret;
	}

	ret = generic_phy_get_by_name(dev, "usb", &phy);
	if (ret) {
		pr_err("failed to get %s USB PHY\n", dev->name);
		return ret;
	}

	ret = generic_phy_init(&phy);
	if (ret) {
		pr_err("failed to init %s USB PHY\n", dev->name);
		return ret;
	}

	return sun4i_usb_phy_vbus_detect(&phy);
	if (ret == 1)
		return -ENODEV;

	return ret;
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
	struct boot_file_head *spl = (void *)(ulong)spl_addr;
	if (memcmp(spl->spl_signature, SPL_SIGNATURE, 3) != 0)
		return; /* signature mismatch, no usable header */

	uint8_t spl_header_version = spl->spl_signature[3];
	if (spl_header_version != SPL_HEADER_VERSION) {
		printf("sunxi SPL version mismatch: expected %u, got %u\n",
		       SPL_HEADER_VERSION, spl_header_version);
		return;
	}
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
	unsigned int sid[4];
	uint8_t mac_addr[6] = { 0, 0, 0, 0, 0, 0 };
	char ethaddr[16], digit[3], strrev[3];
	int i, ret;
	char *p;

	if (olimex_eeprom_is_valid()) {

		if (!env_get("board_id"))
			env_set_ulong("board_id", eeprom->id);

		if (!env_get("board_name"))
			env_set("board_name", olimex_get_board_name());

		if (!env_get("board_rev")) {
			strrev[0] = (eeprom->revision.major < 'A' || eeprom->revision.major > 'Z') ? 0 : eeprom->revision.major;
			strrev[1] = (eeprom->revision.minor < '1' || eeprom->revision.minor > '9') ? 0 : eeprom->revision.minor;
			strrev[2] = 0;
			env_set("board_rev", strrev);
		}

		p = eeprom->mac;
		for (i = 0; i < 6; i++) {
			sprintf(digit, "%c%c",
				(p[i * 2] == 0xFF) ? 'F' : p[i * 2],
				(p[i * 2 + 1] == 0xFF) ? 'F' : p[i * 2 + 1]);
			mac_addr[i] = simple_strtoul(digit, NULL, 16);
		}

		if (fdt_get_alias(fdt, "ethernet0"))
			if (!env_get("ethaddr"))
				eth_env_set_enetaddr("ethaddr", mac_addr);

		if (fdt_get_alias(fdt, "ethernet2")) {
			if (!strncmp(p, "301F9AD", 7))
				mac_addr[0] |= 0x02;
			if (!env_get("eth2addr"))
				eth_env_set_enetaddr("eth2addr", mac_addr);
		}

		/**
		 * Setup mtdparts
		 * A20-SOM204xxxx has both eMMC and SPI flash
		 * A20-OLinuXino-LIME2-e16Gs16M has both eMMC and SPI flash
		 * A20-OLinuXino-LIME2-e4Gs16M has both eMMC and SPI flash
		 */
		if (eeprom->id == 8958 ||
		    eeprom->id == 9604 ||
		    eeprom->id == 9613 ||
		    eeprom->config.storage == 's') {
			env_set("mtdids", SPI_MTDIDS);
			env_set("mtdparts", SPI_MTDPARTS);
		} else if (eeprom->config.storage == 'n') {
			env_set("mtdids", NAND_MTDIDS);
			env_set("mtdparts", NAND_MTDPARTS);
		}
	}

	ret = sunxi_get_sid(sid);
	if (ret == 0 && sid[0] != 0) {

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
	}

	if (!env_get("serial#")) {
		snprintf(serial_string, sizeof(serial_string),
			"%08x", eeprom->serial);
		env_set("serial#", serial_string);
	}

	if (!env_get("fdtfile"))
		env_set("fdtfile", olimex_get_board_fdt());

}

int misc_init_r(void)
{
	__maybe_unused struct udevice *dev;
	__maybe_unused int ret;
	uint boot;

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

#ifdef CONFIG_USB_GADGET
	ret = uclass_first_device(UCLASS_USB_DEV_GENERIC, &dev);
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
	const char *name;
	char *mac = eeprom->mac;
	uint8_t i;

	if (!olimex_eeprom_is_valid()) {
		printf("Model: Unknown\n");
		return 0;
	}

	/**
	 * In case of lowercase revision number, rewrite eeprom
	 */
	if (eeprom->revision.major >= 'a' && eeprom->revision.major <= 'z') {
		eeprom->revision.major -= 0x20;

		olimex_i2c_eeprom_write();
	}

	/* Get board name and compare if with eeprom content */
	name = olimex_get_board_name();

	printf("Model: %s Rev.%c%c", name,
	       (eeprom->revision.major < 'A' || eeprom->revision.major > 'Z') ?
	       0 : eeprom->revision.major,
	       (eeprom->revision.minor < '1' || eeprom->revision.minor > '9') ?
	       0 : eeprom->revision.minor);

	printf("\nSerial:%08X\n", eeprom->serial);
	printf("MAC:   ");
	for (i = 0; i < 12; i += 2 ) {
		if (i < 10)
			printf("%c%c:",
				(mac[i] == 0xFF) ? 'F' : mac[i],
				(mac[i+1] == 0xFF) ? 'F' : mac[i+1]);
		else
			printf("%c%c\n",
				(mac[i] == 0xFF) ? 'F' : mac[i],
				(mac[i+1] == 0xFF) ? 'F' : mac[i+1]);
	}

	return 0;
}

#ifdef CONFIG_MULTI_DTB_FIT
int board_fit_config_name_match(const char *name)
{
	const char *dtb;

	if (!olimex_eeprom_is_valid())
		return -1;

	dtb = olimex_get_board_fdt();
	return (!strncmp(name, dtb, strlen(dtb) - 4)) ? 0 : -1;
}
#endif /* CONFIG_MULTI_DTB_FIT */

#ifdef CONFIG_SET_DFU_ALT_INFO
void set_dfu_alt_info(char *interface, char *devstr)
{
	char *p = NULL;
	int dev;

	printf("interface: %s, devstr: %s\n", interface, devstr);

#ifdef CONFIG_DFU_MMC
	if (!strcmp(interface, "mmc")) {
		dev = simple_strtoul(devstr, NULL, 10);
		if (dev == 0 )
			p = env_get("dfu_alt_info_mmc0");
		else if (dev == 1)
			p = env_get("dfu_alt_info_mmc1");
	}
#endif

#ifdef CONFIG_DFU_RAM
	if (!strcmp(interface, "ram"))
		p = env_get("dfu_alt_info_ram");
#endif

#ifdef CONFIG_DFU_NAND
	if (!strcmp(interface, "nand"))
		p = env_get("dfu_alt_info_nand");
#endif

#ifdef CONFIG_DFU_SF
	if (!strcmp(interface, "sf"))
		p = env_get("dfu_alt_info_sf");
#endif
	env_set("dfu_alt_info", p);
}

#endif /* CONFIG_SET_DFU_ALT_INFO */

#endif /* !CONFIG_SPL_BUILD */
