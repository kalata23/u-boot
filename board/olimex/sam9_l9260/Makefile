# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2003-2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2008
# Stelian Pop <stelian@popies.net>
# Lead Tech Design <www.leadtechdesign.com>
#
# (C) Copyright 2012
# Markus Hubig <mhubig@imko.de>
# IMKO GmbH <www.imko.de>
#
# (C) Copyright 2014
# Heiko Schocher <hs@denx.de>
# DENX Software Engineering GmbH

obj-y	+= board.o
obj-$(CONFIG_AT91_LED) += led.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_SPI_SAM9_L9260)	+= spl_spi_sam9_l9260.o
endif
