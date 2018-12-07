/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2010
 * Achim Ehrlich <aehrlich@taskit.de>
 * taskit GmbH <www.taskit.de>
 *
 * (C) Copyright 2012
 * Markus Hubig <mhubig@imko.de>
 * IMKO GmbH <www.imko.de>
 *
 * (C) Copyright 2014
 * Heiko Schocher <hs@denx.de>
 * DENX Software Engineering GmbH
 *
 * Configuation settings for the smartweb.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>
#include <linux/sizes.h>

/*
 * Warning: changing CONFIG_SYS_TEXT_BASE requires adapting the initial boot
 * program. Since the linker has to swallow that define, we must use a pure
 * hex number here!
 */

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000	/* 18.432MHz crystal */

/* misc settings */
#define CONFIG_CMDLINE_TAG		/* pass commandline to Kernel */
#define CONFIG_SETUP_MEMORY_TAGS	/* pass memory defs to kernel */
#define CONFIG_INITRD_TAG		/* pass initrd param to kernel */
#define CONFIG_SKIP_LOWLEVEL_INIT_ONLY	/* U-Boot is loaded by a bootloader */

/* We set the max number of command args high to avoid HUSH bugs. */
#define CONFIG_SYS_MAXARGS    32

/* setting board specific options */
#define CONFIG_ENV_OVERWRITE    1 /* Overwrite ethaddr / serial# */
#define CONFIG_SYS_AUTOLOAD "yes"
#define CONFIG_RESET_TO_RETRY

/* The LED PINs */
#define CONFIG_AT91_LED
#define CONFIG_GREEN_LED		AT91_PIN_PA6
#define CONFIG_YELLOW_LED		AT91_PIN_PA9

/* Configure SPI */
#define CONFIG_SPI_DATAFLASH_WRITE_VERIFY
#define SAM9_L9260_SPI_MASK	(1 << 5)
#define SAM9_L9260_SPI_CS_PIN	AT91_PIN_PC11

/*
 * SDRAM: 1 bank, 64 MB, base address 0x20000000
 * Already initialized before u-boot gets started.
 */
#define CONFIG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CONFIG_SYS_SDRAM_SIZE		(64 * SZ_1M)

/*
 * Perform a SDRAM Memtest from the start of SDRAM
 * till the beginning of the U-Boot position in RAM.
 */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_TEXT_BASE - 0x100000)

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN \
	ROUND(3 * CONFIG_ENV_SIZE + (4 * SZ_1M), 0x1000)


/* general purpose I/O */
#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */
#define CONFIG_AT91_GPIO		/* enable the GPIO features */
#define CONFIG_AT91_GPIO_PULLUP	1	/* keep pullups on peripheral pins */

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define CONFIG_USART_ID			ATMEL_ID_SYS

/*
 * Ethernet configuration
 *
 */
#define CONFIG_MACB
#define CONFIG_MII			/* use MII inteface */
#define CONFIG_NET_RETRY_COUNT	20      /* # of DHCP/BOOTP retries */
#define CONFIG_AT91_WANTS_COMMON_PHY

/* BOOTP and DHCP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_NFSBOOTCOMMAND						\
	"setenv autoload yes; setenv autoboot yes; "			\
	"setenv bootargs ${basicargs} ${mtdparts} "			\
	"root=/dev/nfs ip=dhcp nfsroot=${serverip}:/srv/nfs/rootfs; "	\
	"dhcp"

/* USB configuration */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE	ATMEL_UHP_BASE
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"at91sam9260"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2

/* USB DFU support */
#define CONFIG_USB_GADGET_AT91

/* DFU class support */
#define CONFIG_SYS_DFU_DATA_BUF_SIZE	SZ_1M
#define DFU_MANIFEST_POLL_TIMEOUT	25000

/* General Boot Parameter */
#define CONFIG_SYS_CBSIZE		512

/*
 * RAM Memory address where to put the
 * Linux Kernel befor starting.
 */
#define CONFIG_SYS_LOAD_ADDR		0x22000000

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET		0x4200
#define CONFIG_ENV_OFFSET_REDUND	0x6300
#define CONFIG_ENV_SIZE			0x2100
#define CONFIG_ENV_SECT_SIZE		CONFIG_ENV_SIZE

/* NAND flash settings */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PC13

#if 0
#define MEM_LAYOUT_ENV_SETTINGS \
	"scriptaddr=0x21000000\0" \
	"fdt_addr_r=0x21100000\0" \
	"pxefile_addr_r=0x21120000\0" \
	"kernel_addr_r=0x22000000\0" \
	"ramdisk_addr_r=0x23000000\0"

/* Basic environment settings */
#define BOOT_TARGET_DEVICES(func) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>
#endif

#if 0
#define CONFIG_BOOTCOMMAND	"setenv serverip 192.168.0.20; " \
				"setenv bootargs console=ttyS0,115200 loglevel=8; " \
				"dhcp 0x22000000 SAM9-L9260/zImage; " \
				"tftp 0x23000000 SAM9-L9260/rootfs.cpio.uboot; " \
				"tftp 0x21800000 SAM9-L9260/at91sam9260ek.dtb; " \
				"bootz 0x22000000 0x23000000 0x21800000\0"
#endif

#define	CONFIG_EXTRA_ENV_SETTINGS \
	"update_uboot=" \
		"setenv serverip 192.168.0.20; " \
		"dhcp 0x22000000 SAM9-L9260/u-boot.bin; " \
		"sf probe; " \
		"sf erase 0x8400 0x84000; " \
		"sf write 0x22000000 0x8400 0x84000\0" \
	"mtdparts="CONFIG_MTDPARTS_DEFAULT"\0"

/*
 * Initial stack pointer: 4k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above that
 * address while providing maximum stack area below.
 */
#define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM1 + 0x1000 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MASTER_CLOCK		(198656000/2)
#define AT91_PLL_LOCK_TIMEOUT		1000000
#define CONFIG_SYS_AT91_PLLA		0x2060bf09
#define CONFIG_SYS_MCKR			0x100
#define CONFIG_SYS_MCKR_CSS		(0x02 | CONFIG_SYS_MCKR)
#define CONFIG_SYS_AT91_PLLB		0x10483f0e

#endif /* __CONFIG_H */
