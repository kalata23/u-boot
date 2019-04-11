/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012-2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Configuration settings for the Allwinner sunxi series of boards.
 */

#ifndef __A64_OLINUXINO_CONFIG_H
#define __A64_OLINUXINO_CONFIG_H

#include <asm/arch/cpu.h>
#include <linux/stringify.h>

/*
 * A64 specific configuration
 */

#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1
#endif

#define GICD_BASE		0x3021000
#define GICC_BASE		0x3022000

#ifdef CONFIG_OLD_SUNXI_KERNEL_COMPAT
/*
 * The U-Boot workarounds bugs in the outdated buggy sunxi-3.4 kernels at the
 * expense of restricting some features, so the regular machine id values can
 * be used.
 */
# define CONFIG_MACH_TYPE_COMPAT_REV	0
#else
/*
 * A compatibility guard to prevent loading outdated buggy sunxi-3.4 kernels.
 * Only sunxi-3.4 kernels with appropriate fixes applied are able to pass
 * beyond the machine id check.
 */
# define CONFIG_MACH_TYPE_COMPAT_REV	1
#endif

#define CONFIG_BUILD_TARGET "u-boot.itb"
#define CONFIG_SYS_BOOTM_LEN		(32 << 20)


/* Serial & console */
#define CONFIG_SYS_NS16550_SERIAL
/* ns16550 reg in the low bits of cpu reg */
#define CONFIG_SYS_NS16550_CLK		24000000
#ifndef CONFIG_DM_SERIAL
# define CONFIG_SYS_NS16550_REG_SIZE	-4
# define CONFIG_SYS_NS16550_COM1		SUNXI_UART0_BASE
# define CONFIG_SYS_NS16550_COM2		SUNXI_UART1_BASE
# define CONFIG_SYS_NS16550_COM3		SUNXI_UART2_BASE
# define CONFIG_SYS_NS16550_COM4		SUNXI_UART3_BASE
# define CONFIG_SYS_NS16550_COM5		SUNXI_R_UART_BASE
#endif

/* CPU */
#define COUNTER_FREQUENCY		24000000

/*
 * The DRAM Base differs between some models. We cannot use macros for the
 * CONFIG_FOO defines which contain the DRAM base address since they end
 * up unexpanded in include/autoconf.mk .
 *
 * So we have to have this #ifdef #else #endif block for these.
 */
#define SDRAM_OFFSET(x) 0x4##x
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_LOAD_ADDR		0x42000000 /* default load address */
/* V3s do not have enough memory to place code at 0x4a000000 */
/* Note SPL_STACK_R_ADDR is set through Kconfig, we include it here
 * since it needs to fit in with the other values. By also #defining it
 * we get warnings if the Kconfig value mismatches. */
#define CONFIG_SPL_STACK_R_ADDR		0x4fe00000
#define CONFIG_SPL_BSS_START_ADDR	0x4ff80000

#define CONFIG_SPL_BSS_MAX_SIZE		0x00080000 /* 512 KiB */

/*
 * The A80's A1 sram starts at 0x00010000 rather then at 0x00000000 and is
 * slightly bigger. Note that it is possible to map the first 32 KiB of the
 * A1 at 0x00000000 like with older SoCs by writing 0x16aa0001 to the
 * undocumented 0x008000e0 SYS_CTRL register. Where the 16aa is a key and
 * the 1 actually activates the mapping of the first 32 KiB to 0x00000000.
 * A64 and H5 also has SRAM A1 at 0x00010000, but no magic remap register
 * is known yet.
 * H6 has SRAM A1 at 0x00020000.
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SUNXI_SRAM_ADDRESS
/* FIXME: this may be larger on some SoCs */
#define CONFIG_SYS_INIT_RAM_SIZE	0x8000 /* 32 KiB */

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define PHYS_SDRAM_0			CONFIG_SYS_SDRAM_BASE
#define PHYS_SDRAM_0_SIZE		0x80000000 /* 2 GiB */

#ifdef CONFIG_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SUNXI_AHCI
#define CONFIG_SYS_64BIT_LBA
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)
#endif

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_SERIAL_TAG

#ifdef CONFIG_SPL_SPI_SUNXI
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x8000
#endif

#if defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define CONFIG_ENV_OFFSET_REDUND	( CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE		(128 << 10)
#endif

#if defined(CONFIG_ENV_IS_IN_MMC)
/*
 * This is actually (CONFIG_ENV_OFFSET -
 * (CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)), but the value will be used
 * directly in a makefile, without the preprocessor expansion.
 */
#define CONFIG_BOARD_SIZE_LIMIT		0x7e000
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_MAX_DEVICE	4
#endif

#if defined(CONFIG_ENV_IS_NOWHERE)
#define CONFIG_ENV_SIZE			(128 << 10)
#endif

#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (64 << 20))
/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE	1024	/* Print Buffer Size */

/* standalone support */
#define CONFIG_STANDALONE_LOAD_ADDR	CONFIG_SYS_LOAD_ADDR

/* FLASH and environment organization */

#define CONFIG_SYS_MONITOR_LEN		(768 << 10)	/* 768 KiB */

#ifndef CONFIG_ARM64		/* AArch64 FEL support is not ready yet */
#define CONFIG_SPL_BOARD_LOAD_IMAGE
#endif

/*
 * We cannot use expressions here, because expressions won't be evaluated in
 * autoconf.mk.
 */

#define CONFIG_SPL_TEXT_BASE		0x10060		/* sram start+header */
#define CONFIG_SPL_MAX_SIZE		0x7fa0		/* 32 KiB */

/* end of SRAM A2 for now, as SRAM A1 is pretty tight for an ARM64 build */
#define LOW_LEVEL_SRAM_STACK		0x00054000

#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

#define CONFIG_SPL_PAD_TO		32768		/* decimal for 'dd' */


/* I2C */
#ifndef CONFIG_DM_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SPEED		400000
#define CONFIG_SYS_I2C_SLAVE		0x7f
#endif

#define CONFIG_VIDEO_LCD_I2C_BUS	-1 /* NA, but necessary to compile */
#define LCD_OLINUXINO_EEPROM_BUS	0


/* PMU */
#if defined CONFIG_AXP152_POWER || defined CONFIG_AXP209_POWER || \
    defined CONFIG_AXP221_POWER || defined CONFIG_AXP818_POWER || \
    defined CONFIG_SY8106A_POWER
#endif

#ifdef CONFIG_REQUIRE_SERIAL_CONSOLE
#define OF_STDOUT_PATH		"/soc@01c00000/serial@01c28000:115200"
#endif /* ifdef CONFIG_REQUIRE_SERIAL_CONSOLE */

/* GPIO */
#define CONFIG_SUNXI_GPIO

#ifdef CONFIG_VIDEO_DE2
/*
 * The amount of RAM to keep free at the top of RAM when relocating u-boot,
 * to use as framebuffer. This must be a multiple of 4096.
 */
#define CONFIG_SUNXI_MAX_FB_SIZE (16 << 20)

#define CONFIG_VIDEO_STD_TIMINGS
#define VIDEO_LINE_LEN (pGD->plnSizeX)

/* allow both serial and cfb console. */
/* stop x86 thinking in cfbconsole from trying to init a pc keyboard */

#endif /* CONFIG_VIDEO_DE2 */

#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 1
#endif

#ifdef CONFIG_USB_KEYBOARD
#define CONFIG_PREBOOT
#endif

#ifndef CONFIG_SPL_BUILD

/*
 * Boards seem to come with at least 512MB of DRAM.
 * The kernel should go at 512K, which is the default text offset (that will
 * be adjusted at runtime if needed).
 * There is no compression for arm64 kernels (yet), so leave some space
 * for really big kernels, say 256MB for now.
 * Scripts, PXE and DTBs should go afterwards, leaving the rest for the initrd.
 * Align the initrd to a 2MB page.
 */
#define BOOTM_SIZE	__stringify(0xa000000)
#define KERNEL_ADDR_R	__stringify(SDRAM_OFFSET(0080000))
#define FDT_ADDR_R	__stringify(SDRAM_OFFSET(FA00000))
#define SCRIPT_ADDR_R	__stringify(SDRAM_OFFSET(FC00000))
#define PXEFILE_ADDR_R	__stringify(SDRAM_OFFSET(FD00000))
#define RAMDISK_ADDR_R	__stringify(SDRAM_OFFSET(FE00000))

#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=" BOOTM_SIZE "\0" \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0" \
	"pxefile_addr_r=" PXEFILE_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0"

#define DFU_ALT_INFO_RAM \
	"dfu_alt_info_ram=" \
	"kernel ram " KERNEL_ADDR_R " 0x1000000;" \
	"fdt ram " FDT_ADDR_R " 0x100000;" \
	"ramdisk ram " RAMDISK_ADDR_R " 0x4000000\0"

#ifdef CONFIG_DFU
#define CONFIG_SET_DFU_ALT_INFO

#ifdef CONFIG_DFU_RAM
#define DFU_ALT_INFO_RAM \
	"dfu_alt_info_ram=" \
	"kernel ram " KERNEL_ADDR_R " 0x1000000;" \
	"fdt ram " FDT_ADDR_R " 0x100000;" \
	"ramdisk ram " RAMDISK_ADDR_R " 0x4000000\0"
#else
#define DFU_ALT_INFO_RAM ""
#endif

#ifdef CONFIG_DFU_SF
#define DFU_ALT_INFO_SF \
	"dfu_alt_info_sf=" \
		"SPI.u-boot raw 0 0x200000;" \
		"SPI.u-boot-env raw 0x200000 0x20000;" \
		"SPI.u-boot-env.backup raw 0x220000 0x20000\0"
#else
#define DFU_ALT_INFO_SF ""
#endif

#define DFU_ALT_INFO \
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_SF

#endif /* CONFIG_DFU */

#ifdef CONFIG_SPI_FLASH
#define SPI_MTDIDS "nor0=flash.0"
#define SPI_MTDPARTS "mtdparts=flash.0:2m(SPI.u-boot),128k(SPI.u-boot-env),128k(SPI.u-boot-env.backup),-(SPI.user)"
#endif

#ifdef CONFIG_MMC
#if CONFIG_MMC_SUNXI_SLOT_EXTRA != -1
#define BOOTENV_DEV_MMC_AUTO(devtypeu, devtypel, instance)		\
	BOOTENV_DEV_MMC(MMC, mmc, 0)					\
	BOOTENV_DEV_MMC(MMC, mmc, 1)					\
	"bootcmd_mmc_auto="						\
		"if test ${mmc_bootdev} -eq 1; then "			\
			"run bootcmd_mmc1; "				\
			"run bootcmd_mmc0; "				\
		"elif test ${mmc_bootdev} -eq 0; then "			\
			"run bootcmd_mmc0; "				\
			"run bootcmd_mmc1; "				\
		"fi\0"

#define BOOTENV_DEV_NAME_MMC_AUTO(devtypeu, devtypel, instance) \
	"mmc_auto "

#define BOOT_TARGET_DEVICES_MMC(func) func(MMC_AUTO, mmc_auto, na)
#else
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0)
#endif
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#ifdef CONFIG_AHCI
#define BOOT_TARGET_DEVICES_SCSI(func) func(SCSI, scsi, 0)
#else
#define BOOT_TARGET_DEVICES_SCSI(func)
#endif

#ifdef CONFIG_USB_STORAGE
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

/* FEL boot support, auto-execute boot.scr if a script address was provided */
#define BOOTENV_DEV_FEL(devtypeu, devtypel, instance) \
	"bootcmd_fel=" \
		"if test -n ${fel_booted} && test -n ${fel_scriptaddr}; then " \
			"echo '(FEL boot)'; " \
			"source ${fel_scriptaddr}; " \
		"fi\0"
#define BOOTENV_DEV_NAME_FEL(devtypeu, devtypel, instance) \
	"fel "

#define BOOT_TARGET_DEVICES(func) \
	func(FEL, fel, na) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_SCSI(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#ifdef CONFIG_USB_KEYBOARD
#define CONSOLE_STDIN_SETTINGS \
	"preboot=usb start\0" \
	"stdin=serial,usbkbd\0"
#else
#define CONSOLE_STDIN_SETTINGS \
	"stdin=serial\0"
#endif

#ifdef CONFIG_VIDEO
#define CONSOLE_STDOUT_SETTINGS \
	"stdout=serial,vga\0" \
	"stderr=serial,vga\0"
#elif CONFIG_DM_VIDEO
#define CONFIG_SYS_WHITE_ON_BLACK
#define CONSOLE_STDOUT_SETTINGS \
	"stdout=serial,vidconsole\0" \
	"stderr=serial,vidconsole\0"
#else
#define CONSOLE_STDOUT_SETTINGS \
	"stdout=serial\0" \
	"stderr=serial\0"
#endif
#define CONSOLE_ENV_SETTINGS \
	CONSOLE_STDIN_SETTINGS \
	CONSOLE_STDOUT_SETTINGS

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONSOLE_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	DFU_ALT_INFO \
	"console=ttyS0,115200\0" \
	BOOTENV

#else /* ifndef CONFIG_SPL_BUILD */
#define CONFIG_EXTRA_ENV_SETTINGS
#endif

#endif /* __A64_OLINUXINO_CONFIG_H */
