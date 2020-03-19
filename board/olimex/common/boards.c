/*
 * Copyright (C) 2018 Olimex Ltd.
 *   Author: Stefan Mavrodiev <stefan@olimex.com>
 *
 * SPDX-License-Identifier: (GPL-2.0+ OR MIT)
 */
#include <common.h>
#include "board_detect.h"
#include "boards.h"

struct olinuxino_boards olinuxino_boards[] = {
#if defined(CONFIG_TARGET_A20_OLINUXINO)
	/* A20-OLinuXino-Lime */
	{
		OLINUXINO_BOARD(7739, "A20-OLinuXino-LIME", "sun7i-a20-olinuxino-lime.dtb")
		OLINUXINO_CONFIG(NONE, -1, MBYTES(512), COM)
	},
	{
		OLINUXINO_BOARD(7743, "A20-OLinuXino-LIME-n4GB", "sun7i-a20-olinuxino-lime.dtb")
		OLINUXINO_CONFIG(NAND, GBYTES(4), MBYTES(512), COM)
	},
	{
		OLINUXINO_BOARD(8934, "A20-OLinuXino-LIME-n8G", "sun7i-a20-olinuxino-lime.dtb")
		OLINUXINO_CONFIG(NAND, GBYTES(8), MBYTES(512), COM)
	},
	{
		OLINUXINO_BOARD(9076, "A20-OLinuXino-LIME-s16MB", "sun7i-a20-olinuxino-lime.dtb")
		OLINUXINO_CONFIG(SPI, MBYTES(16), MBYTES(512), COM)
	},
	{
		OLINUXINO_BOARD(9516, "A20-OLinuXino-LIME-e16Gs16M", "sun7i-a20-olinuxino-lime-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(16), MBYTES(512), COM)

	},
	{
		OLINUXINO_BOARD(9696, "A20-OLinuXino-LIME-e4Gs16M", "sun7i-a20-olinuxino-lime-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), MBYTES(512), COM)

	},

	/* T2-OLinuXino-Lime */
	{
		OLINUXINO_BOARD(9211, "T2-OLinuXino-LIME-IND", "sun7i-a20-olinuxino-lime.dtb")
		OLINUXINO_CONFIG(NONE, -1, MBYTES(512), IND)
	},
	{
		OLINUXINO_BOARD(9215, "T2-OLinuXino-LIME-s16M-IND", "sun7i-a20-olinuxino-lime.dtb")
		OLINUXINO_CONFIG(SPI, MBYTES(16), MBYTES(512), IND)
	},
	{
		OLINUXINO_BOARD(9219, "T2-OLinuXino-LIME-e4G-IND", "sun7i-a20-olinuxino-lime-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), MBYTES(512), IND)
	},
	{
		OLINUXINO_BOARD(9734, "T2-OLinuXino-LIME-e4Gs16M-IND", "sun7i-a20-olinuxino-lime-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), MBYTES(512), IND)

	},

	/* A20-OLinuXino-Lime2 */
	{
		OLINUXINO_BOARD(7701, "A20-OLinuXino-LIME2", "sun7i-a20-olinuxino-lime2.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(8340, "A20-OLinuXino-LIME2-e4GB", "sun7i-a20-olinuxino-lime2-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(9166, "A20-OLinuXino-LIME2-e16GB", "sun7i-a20-olinuxino-lime2-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(16), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(7624, "A20-OLinuXino-LIME2-n4GB", "sun7i-a20-olinuxino-lime2.dtb")
		OLINUXINO_CONFIG(NAND, GBYTES(4), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(8910, "A20-OLinuXino-LIME2-n8GB", "sun7i-a20-olinuxino-lime2.dtb")
		OLINUXINO_CONFIG(NAND, GBYTES(8), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(8946, "A20-OLinuXino-LIME2-s16MB", "sun7i-a20-olinuxino-lime2.dtb")
		OLINUXINO_CONFIG(SPI, MBYTES(16), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(9604, "A20-OLinuXino-LIME2-e16Gs16M", "sun7i-a20-olinuxino-lime2-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(16), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(9613, "A20-OLinuXino-LIME2-e4Gs16M", "sun7i-a20-olinuxino-lime2-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(9905, "A20-OLinuXino-LIME2-G2", "sun7i-a20-olinuxino-lime2.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), COM)
	},


	/* T2-OLinuXino-Lime2 */
	{
		OLINUXINO_BOARD(9239 , "T2-OLinuXino-LIME2-IND", "sun7i-a20-olinuxino-lime2.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(9247 , "T2-OLinuXino-LIME2-s16MB-IND", "sun7i-a20-olinuxino-lime2.dtb")
		OLINUXINO_CONFIG(SPI, MBYTES(16), GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(9243 , "T2-OLinuXino-LIME2-e8Gs16M-IND", "sun7i-a20-olinuxino-lime2-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(8), GBYTES(1), IND)
	},

	/* A20-OLinuXino-MICRO */
	{
		OLINUXINO_BOARD(4614, "A20-OLinuXino-MICRO", "sun7i-a20-olinuxino-micro.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(4615, "A20-OLinuXino-MICRO-n4GB", "sun7i-a20-olinuxino-micro.dtb")
		OLINUXINO_CONFIG(NAND, GBYTES(4), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(8661, "A20-OLinuXino-MICRO-e4GB-IND", "sun7i-a20-olinuxino-micro-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(8828, "A20-OLinuXino-MICRO-IND", "sun7i-a20-olinuxino-micro.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(8832, "A20-OLinuXino-MICRO-e4GB", "sun7i-a20-olinuxino-micro-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(8918, "A20-OLinuXino-MICRO-n8G", "sun7i-a20-olinuxino-micro.dtb")
		OLINUXINO_CONFIG(NAND, GBYTES(8), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(9042, "A20-OLinuXino-MICRO-e16G", "sun7i-a20-olinuxino-micro-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(16), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(9231, "A20-OLinuXino-MICRO-s16MB", "sun7i-a20-olinuxino-micro.dtb")
		OLINUXINO_CONFIG(SPI, MBYTES(16), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(9684, "A20-OLinuXino-MICRO-e4Gs16M", "sun7i-a20-olinuxino-micro-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(9689, "A20-OLinuXino-MICRO-e16Gs16M", "sun7i-a20-olinuxino-micro-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(16), GBYTES(1), COM)
	},

	/* T2-OLinuXino-MICRO */
	{
		OLINUXINO_BOARD(9223, "T2-OLinuXino-MICRO-IND", "sun7i-a20-olinuxino-micro.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(9227, "T2-OLinuXino-MICRO-e4GB-IND", "sun7i-a20-olinuxino-micro-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(9235, "T2-OLinuXino-MICRO-s16MB-IND", "sun7i-a20-olinuxino-micro.dtb")
		OLINUXINO_CONFIG(SPI, MBYTES(16), GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(9739, "T2-OLinuXino-MICRO-e4Gs16M-IND", "sun7i-a20-olinuxino-micro-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(9789, "T2-OLinuXino-MICRO-e4Gs16M-IND", "sun7i-a20-olinuxino-micro-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), GBYTES(1), IND)
	},

	/* A20-SOM */
	{
		OLINUXINO_BOARD(4673, "A20-SOM-n4GB", "sun7i-a20-olimex-som-evb.dtb")
		OLINUXINO_CONFIG(NAND, GBYTES(4), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(7664, "A20-SOM", "sun7i-a20-olimex-som-evb.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(8849, "A20-SOM-IND", "sun7i-a20-olimex-som-evb.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(8922, "A20-SOM-n8GB", "sun7i-a20-olimex-som-evb.dtb")
		OLINUXINO_CONFIG(NAND, GBYTES(8), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(9155, "A20-SOM-e16GB", "sun7i-a20-olimex-som-evb-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(16), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(9148, "A20-SOM-e16GB-IND", "sun7i-a20-olimex-som-evb-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(16), GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(9047, "A20-SOM-e16Gs16M", "sun7i-a20-olimex-som-evb-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(16), GBYTES(1), COM)
	},

	/* T2-SOM */
	{
		OLINUXINO_BOARD(9259, "T2-SOM-IND", "sun7i-a20-olimex-som-evb.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(9827, "T2-SOM-e8Gs16M-IND", "sun7i-a20-olimex-som-evb-emmc.dtb")
		OLINUXINO_CONFIG(NONE, GBYTES(8), GBYTES(1), IND)
	},




	/* A20-SOM204 */
	{
		OLINUXINO_BOARD(8991, "A20-SOM204-1G", "sun7i-a20-olimex-som204-evb.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(8958, "A20-SOM204-1Gs16Me16G-MC", "sun7i-a20-olimex-som204-evb-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(16), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(10257, "A20-SOM204-1G-M", "sun7i-a20-olimex-som204-evb.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), COM)
	},

	/* T2-SOM204 */
	{
		OLINUXINO_BOARD(10157, "T2-SOM204-1Gs16Me4G-C-I", "sun7i-a20-olimex-som204-evb-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(10234, "T2-SOM204-1Gs16Me8G-MC-I", "sun7i-a20-olimex-som204-evb-emmc.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(8), GBYTES(1), IND)
	},
	{
		OLINUXINO_BOARD(10238, "T2-SOM204-1G-I", "sun7i-a20-olimex-som204-evb.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), IND)
	},
#endif
#if CONFIG_TARGET_A64_OLINUXINO
	{
		OLINUXINO_BOARD(8861, "A64-OLinuXino-2Ge8G-IND", "sun50i-a64-olinuxino-2Ge8G.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(8), GBYTES(2), IND)
	},
	{
		OLINUXINO_BOARD(9065, "A64-OLinuXino-1Gs16M", "sun50i-a64-olinuxino-1Gs16M.dtb")
		OLINUXINO_CONFIG(SPI, MBYTES(16), GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(8367, "A64-OLinuXino-1Ge4GW", "sun50i-a64-olinuxino-1Ge4GW.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(4), GBYTES(2), COM)
	},
	{
		OLINUXINO_BOARD(8857, "A64-OLinuXino-1G", "sun50i-a64-olinuxino-1G.dtb")
		OLINUXINO_CONFIG(NONE, -1, GBYTES(1), COM)
	},
	{
		OLINUXINO_BOARD(9849, "A64-OLinuXino-1Ge16GW", "sun50i-a64-olinuxino-1Ge16GW.dtb")
		OLINUXINO_CONFIG(EMMC, GBYTES(16), GBYTES(2), COM)
	},
#endif
	/* END */
	{
		.id = 0
	},
};


const char *olimex_get_board_name()
{
#if defined(CONFIG_TARGET_A20_OLINUXINO) || defined(CONFIG_TARGET_A64_OLINUXINO)
	struct olinuxino_boards *board = olinuxino_boards;

	while (board->id) {
		if (eeprom->id == board->id)
			return board->name;
		board++;
	}
	return "";
#elif defined(CONFIG_TARGET_A33_OLINUXINO)
	return "A33-OLinuXino-n8GB";
#endif
}

const char *olimex_get_board_fdt()
{
#if defined(CONFIG_TARGET_A20_OLINUXINO) || defined(CONFIG_TARGET_A64_OLINUXINO)
	struct olinuxino_boards *board = olinuxino_boards;

	while (board->id) {
		if (eeprom->id == board->id)
			return board->fdt;
		board++;
	}
	return "";
#elif defined(CONFIG_TARGET_A33_OLINUXINO)
	return "sun8i-a33-olinuxino.dtb";
#endif
}

#ifdef CONFIG_TARGET_A20_OLINUXINO
bool olimex_board_is_micro()
{
	switch (eeprom->id) {
		case 4614:
		case 8832:
		case 9042:
		case 8661:
		case 8828:
		case 4615:
		case 8918:
		case 9231:
		case 9684:
		case 9689:

		case 9223:
		case 9235:
		case 9227:
		case 9739:
		case 9789:
			return true;

		default:
			return false;
	}
}

bool olimex_board_is_lime()
{
	switch (eeprom->id) {
		case 7739:
		case 7743:
		case 8934:
		case 9076:
		case 9516:
		case 9696:

		case 9211:
		case 9215:
		case 9219:
		case 9734:
			return true;

		default:
			return false;
	}
}

bool olimex_board_is_lime2()
{
	switch (eeprom->id) {
		case 7701:
		case 8340:
		case 9166:
		case 7624:
		case 8910:
		case 8946:

		case 9239:
		case 9247:
		case 9243:
		case 9604:
		case 9613:
		case 9905:
			return true;

		default:
			return false;
	}
}

bool olimex_board_is_som_evb()
{
	switch (eeprom->id) {
		case 4673:
		case 7664:
		case 8849:
		case 8922:
		case 9155:
		case 9148:
		case 9047:

		case 9259:
		case 9827:
			return true;

		default:
			return false;
	}
}
bool olimex_board_is_som204_evb()
{
	switch (eeprom->id) {
		case 8991:
		case 8958:
		case 10157:
		case 10234:
		case 10257:
			return true;

		default:
			return false;
	}
}

bool olimex_board_has_spi(void)
{
	if (eeprom->config.storage == 's')
		return true;

	/* Some boards have both eMMC and SPI */
	switch (eeprom->id) {
		case 8958:		// A20-SOM204-1Gs16Me16G-MC
		case 10157:		// T2-SOM204-1Gs16Me4G-C-I

		case 9604:		// A20-OLinuXino-LIME2-e16Gs16M
		case 9613:		// A20-OLinuXino-LIME2-e4Gs16M"
		case 9243:		// T2-OLinuXino-LIME2-e8Gs16M-IND

		case 9047:		// A20-SOM-e16Gs16M

		case 9684:		// A20-OLinuXino-MICRO-e4Gs16M
		case 9689:		// A20-OLinuXino-MICRO-e16Gs16M
		case 9739:		// T2-OLinuXino-MICRO-e4Gs16M-IND
		case 9789:		// T2-OLinuXino-MICRO-e4Gs16M-IND

		case 9516:		// A20-OLinuXino-LIME-e16Gs16M
		case 9696:		// A20-OLinuXino-LIME-e4Gs16M
		case 9734:		// T2-OLinuXino-LIME-e4Gs16M-IND

		case 9827:		// T2-SOM-e8Gs16M-IND
			return true;

		default:
			return false;
	}
}
#endif

const char * olimex_get_lcd_pwr_pin()
{
#if defined(CONFIG_TARGET_A20_OLINUXINO)
	if (olimex_board_is_som_evb())
		return "PH7";
	else if (olimex_board_is_som204_evb())
		return "PC24";
	else
		return "PH8";
#elif defined(CONFIG_TARGET_A33_OLINUXINO)
		return "PB2";
#elif defined(CONFIG_TARGET_A64_OLINUXINO)
		return "PD23";
#endif
}

const char * olimex_get_lcd_pwm_pin()
{
#if defined(CONFIG_TARGET_A20_OLINUXINO)
	return "PB2";
#elif defined(CONFIG_TARGET_A33_OLINUXINO)
	return "PH0";
#elif defined(CONFIG_TARGET_A64_OLINUXINO)
	return "PD22";
#endif
}

const char *olimex_get_lcd_irq_pin()
{
#if defined(CONFIG_TARGET_A20_OLINUXINO)
	if (olimex_board_is_som204_evb())
		return "PH2";
	else if (olimex_board_is_som_evb())
		return NULL;			// Not yes supported
	else if (olimex_board_is_lime2())
		return "PH10";
	else
		return "PH12";
#elif defined(CONFIG_TARGET_A33_OLINUXINO)
	return "PB5";
#elif defined(CONFIG_TARGET_A64_OLINUXINO)
	return "PH7";
#endif
}

const char *olimex_get_lcd_rst_pin()
{
#if defined(CONFIG_TARGET_A20_OLINUXINO)
	if (olimex_board_is_som204_evb())
		return "PI1";
	else if (olimex_board_is_som_evb())
		return NULL;			// Not yes supported
	else if (olimex_board_is_lime2())
		return "PH11";
	else
		return "PB13";
#elif defined(CONFIG_TARGET_A33_OLINUXINO)
	return "PB6";
#elif defined(CONFIG_TARGET_A64_OLINUXINO)
	return "PH8";
#endif
}

const char *olimex_get_usb_vbus_pin(uint8_t port)
{
	switch (port) {
#if defined(CONFIG_TARGET_A20_OLINUXINO)
	case 0:
		if (olimex_board_is_som204_evb() || olimex_board_is_lime2())
			return "PC17";
		else
			return "PB9";
	case 1:
		return "PH6";
	case 2:
		return "PH3";
#elif defined(CONFIG_TARGET_A33_OLINUXINO)
	case 0:
		return "AXP0-VBUS-ENABLE";
#elif defined(CONFIG_TARGET_A64_OLINUXINO)
		return NULL;	// Driven by AXP803
#endif
	default:
		return NULL;
	}

}

const char *olimex_get_usb_vbus_det_pin()
{
#if defined(CONFIG_TARGET_A20_OLINUXINO)
	return "PH5";
#elif defined(CONFIG_TARGET_A33_OLINUXINO)
	return "AXP0-VBUS-DETECT";
#elif defined(CONFIG_TARGET_A64_OLINUXINO)
	return "PH6";
#endif
}

const char *olimex_get_usb_id_pin()
{
#if defined(CONFIG_TARGET_A20_OLINUXINO)
	return "PH4";
#elif defined(CONFIG_TARGET_A33_OLINUXINO)
	return "PB3";
#elif defined(CONFIG_TARGET_A64_OLINUXINO)
	return "PH9";
#endif
}
