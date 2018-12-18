/*
 * Copyright (C) 2018 Olimex Ltd.
 *   Author: Stefan Mavrodiev <stefan@olimex.com>
 *
 * SPDX-License-Identifier: (GPL-2.0+ OR MIT)
 */
#ifndef __BOARD_DETECT_H
#define __BOARD_DETECT_H

/**
 * Location in the RAM.
 * Content should be transfered after relocation
 */
#define OLIMEX_EEPROM_DATA ((struct olimex_eeprom *)CONFIG_SYS_SDRAM_BASE)
extern struct olimex_eeprom *eeprom;

/**
 * Define EEPROM bus and address
 */
#define OLIMEX_EEPROM_BUS		I2C_1
#define OLIMEX_EEPROM_ADDRESS		0x50
#define OLIMEX_EEPROM_MAGIC_HEADER	0x4f4caa55

/**
 * In the board has corrupted EEPROM (or it's very old without one)
 * a sector in the MMC can be used as backup storage.
 */
#define OLIMEX_MMC_SECTOR		0x0F	/* One sector before SPL */


struct olimex_eeprom {
	uint32_t header;
	uint32_t id;
	struct {
		char major;
		char minor;
	} revision;
	uint32_t serial;
	struct {
		uint8_t storage;
		uint8_t size;
		uint8_t ram;
		uint8_t grade;
	} config;
	char mac[12];
	uint8_t reserved[222];
	uint32_t crc;
} __attribute__ ((__packed__));

/* I2C access functions */
int olimex_i2c_eeprom_read(void);
int olimex_i2c_eeprom_write(void);
int olimex_i2c_eeprom_erase(void);

/* Raw MMC access function */
int olimex_mmc_eeprom_read(void);
int olimex_mmc_eeprom_write(void);
int olimex_mmc_eeprom_erase(void);

bool olimex_eeprom_is_valid(void);

#endif	/* __BOARD_DETECT_H */
