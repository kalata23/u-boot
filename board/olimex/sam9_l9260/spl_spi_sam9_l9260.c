#include <common.h>
#include <spl.h>
#include <wait_bit.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>

/* Register offsets */
#define ATMEL_SPI_CR			0x0000
#define ATMEL_SPI_MR			0x0004
#define ATMEL_SPI_RDR			0x0008
#define ATMEL_SPI_TDR			0x000c
#define ATMEL_SPI_SR			0x0010
#define ATMEL_SPI_IER			0x0014
#define ATMEL_SPI_IDR			0x0018
#define ATMEL_SPI_IMR			0x001c
#define ATMEL_SPI_CSR(x)		(0x0030 + 4 * (x))
#define ATMEL_SPI_VERSION		0x00fc

/* Bits in CR */
#define ATMEL_SPI_CR_SPIEN		BIT(0)
#define ATMEL_SPI_CR_SPIDIS		BIT(1)
#define ATMEL_SPI_CR_SWRST		BIT(7)
#define ATMEL_SPI_CR_LASTXFER		BIT(24)

/* Bits in MR */
#define ATMEL_SPI_MR_MSTR		BIT(0)
#define ATMEL_SPI_MR_PS			BIT(1)
#define ATMEL_SPI_MR_PCSDEC		BIT(2)
#define ATMEL_SPI_MR_FDIV		BIT(3)
#define ATMEL_SPI_MR_MODFDIS		BIT(4)
#define ATMEL_SPI_MR_WDRBT		BIT(5)
#define ATMEL_SPI_MR_LLB		BIT(7)
#define ATMEL_SPI_MR_PCS(x)		(((x) & 15) << 16)
#define ATMEL_SPI_MR_DLYBCS(x)		((x) << 24)

/* Bits in RDR */
#define ATMEL_SPI_RDR_RD(x)		(x)
#define ATMEL_SPI_RDR_PCS(x)		((x) << 16)

/* Bits in TDR */
#define ATMEL_SPI_TDR_TD(x)		(x)
#define ATMEL_SPI_TDR_PCS(x)		((x) << 16)
#define ATMEL_SPI_TDR_LASTXFER		BIT(24)

/* Bits in SR/IER/IDR/IMR */
#define ATMEL_SPI_SR_RDRF		BIT(0)
#define ATMEL_SPI_SR_TDRE		BIT(1)
#define ATMEL_SPI_SR_MODF		BIT(2)
#define ATMEL_SPI_SR_OVRES		BIT(3)
#define ATMEL_SPI_SR_ENDRX		BIT(4)
#define ATMEL_SPI_SR_ENDTX		BIT(5)
#define ATMEL_SPI_SR_RXBUFF		BIT(6)
#define ATMEL_SPI_SR_TXBUFE		BIT(7)
#define ATMEL_SPI_SR_NSSR		BIT(8)
#define ATMEL_SPI_SR_TXEMPTY		BIT(9)
#define ATMEL_SPI_SR_SPIENS		BIT(16)

/* Bits in CSRx */
#define ATMEL_SPI_CSRx_CPOL		BIT(0)
#define ATMEL_SPI_CSRx_NCPHA		BIT(1)
#define ATMEL_SPI_CSRx_CSAAT		BIT(3)
#define ATMEL_SPI_CSRx_BITS(x)		((x) << 4)
#define ATMEL_SPI_CSRx_SCBR(x)		((x) << 8)
#define ATMEL_SPI_CSRx_SCBR_MAX		GENMASK(7, 0)
#define ATMEL_SPI_CSRx_DLYBS(x)		((x) << 16)
#define ATMEL_SPI_CSRx_DLYBCT(x)	((x) << 24)

/* Bits in VERSION */
#define ATMEL_SPI_VERSION_REV(x)	((x) & 0xfff)
#define ATMEL_SPI_VERSION_MFN(x)	((x) << 16)

/* Constants for CSRx:BITS */
#define ATMEL_SPI_BITS_8		0
#define ATMEL_SPI_BITS_9		1
#define ATMEL_SPI_BITS_10		2
#define ATMEL_SPI_BITS_11		3
#define ATMEL_SPI_BITS_12		4
#define ATMEL_SPI_BITS_13		5
#define ATMEL_SPI_BITS_14		6
#define ATMEL_SPI_BITS_15		7
#define ATMEL_SPI_BITS_16		8

#define ATMEL_SPI_TIMEOUT		1000000

#define DB_CONTINUOUS_ARRAY_READ        0xE8            /* Continuous array read */
#define DB_BURST_ARRAY_READ             0xE8            /* Burst array read */
#define DB_PAGE_READ                    0xD2            /* Main memory page read */
#define DB_BUF1_READ                    0xD4            /* Buffer 1 read */
#define DB_BUF2_READ                    0xD6            /* Buffer 2 read */
#define DB_STATUS                       0xD7            /* Status Register */

#define DF_PAGES_NUMBER 		4096
#define DF_PAGES_SIZE			528
#define DF_PAGE_OFFSET			10



/* Register access macros */
#define spi_readl(reg)					\
	readl(ATMEL_BASE_SPI0 + ATMEL_##reg)
#define spi_writel(reg, value)				\
	writel(value, ATMEL_BASE_SPI0 + ATMEL_##reg)


static int sam9_l9260_spi_init(void)
{
	/* Open PIO for SPI0 */
	at91_set_a_periph(AT91_PIO_PORTA, 0, 0);	/* SPI0_MISO */
	at91_set_a_periph(AT91_PIO_PORTA, 1, 0);	/* SPI0_MOSI */
	at91_set_a_periph(AT91_PIO_PORTA, 2, 0);	/* SPI0_SPCK */
	at91_set_b_periph(AT91_PIO_PORTC, 11, 1);	/* SPIO_CS1 */

	/* Enables the SPI0 Clock */
	at91_periph_clk_enable(ATMEL_ID_SPI0);

	/* Reset SPI0 */
	spi_writel(SPI_CR, ATMEL_SPI_CR_SWRST);
	spi_writel(SPI_CR, ATMEL_SPI_CR_SWRST);

	/* Configure SPI0 in Master Mode with No CS selected */
	spi_writel(SPI_MR, ATMEL_SPI_MR_MSTR | ATMEL_SPI_MR_MODFDIS | ATMEL_SPI_MR_PCS(0xF));

	/* Configure CS1 */
	spi_writel(SPI_CSR(1), ATMEL_SPI_CSRx_NCPHA |
			       ATMEL_SPI_CSRx_SCBR(3) |
			       ATMEL_SPI_CSRx_DLYBS(0x1a) |
			       ATMEL_SPI_CSRx_DLYBCT(1));

	/* SPI_Enable */
	spi_writel(SPI_CR, ATMEL_SPI_CR_SPIEN);

	return 0;
}

static void sam9_l9260_spi_cs_activate(void)
{
	/* Enable CS1 */
	spi_writel(SPI_MR, ATMEL_SPI_MR_MSTR | ATMEL_SPI_MR_MODFDIS | ATMEL_SPI_MR_PCS(0xD));
}

static void sam9_l9260_spi_cs_deactivate(void)
{
	/* Enable CS1 */
	spi_writel(SPI_MR, ATMEL_SPI_MR_MSTR | ATMEL_SPI_MR_MODFDIS | ATMEL_SPI_MR_PCS(0xF));
}

static int sam9_l9260_spi_xfer(unsigned int len, const void *dout, void *din)
{

	u32 len_tx, len_rx;
	u32 status;
	const u8 *txp = dout;
	u8 *rxp = din;
	u8 value;

	sam9_l9260_spi_cs_activate();

	/*
	 * sometimes the RDR is not empty when we get here,
	 * in theory that should not happen, but it DOES happen.
	 * Read it here to be on the safe side.
	 * That also clears the OVRES flag. Required if the
	 * following loop exits due to OVRES!
	 */
	spi_readl(SPI_RDR);

	for (len_tx = 0, len_rx = 0; len_rx < len; ) {
		status = spi_readl(SPI_SR);

		if (status & ATMEL_SPI_SR_OVRES)
			return -1;

		if ((len_tx < len) && (status & ATMEL_SPI_SR_TDRE)) {
			if (txp)
				value = *txp++;
			else
				value = 0;
			spi_writel(SPI_TDR, value);
			len_tx++;
		}

		if (status & ATMEL_SPI_SR_RDRF) {
			value = spi_readl(SPI_RDR);
			if (rxp)
				*rxp++ = value;
			len_rx++;
		}
	}

	/*
	 * Wait until the transfer is completely done before
	 * we deactivate CS.
	 */
	wait_for_bit_le32((void *)(ATMEL_BASE_SPI0 + ATMEL_SPI_SR),
			  ATMEL_SPI_SR_TXEMPTY, true, 1000, false);

	sam9_l9260_spi_cs_deactivate();

	return 0;
}




static int spl_spi_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	int ret = 0;

	u8 data[50];
	struct image_header *header;
	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	sam9_l9260_spi_init();



	while (1) {
		mdelay(1000);
		sam9_l9260_spi_xfer(50, data, data);
		ret ^= 1;
		at91_set_gpio_output(CONFIG_GREEN_LED, ret);
	}

	// sam9_l9260_spi_read_data();

	// spi0_read_data((void *)header, CONFIG_SYS_SPI_U_BOOT_OFFS, 0x40);
	//
        // if (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	// 	image_get_magic(header) == FDT_MAGIC) {
	// 	struct spl_load_info load;
	//
	// 	debug("Found FIT image\n");
	// 	load.dev = NULL;
	// 	load.priv = NULL;
	// 	load.filename = NULL;
	// 	load.bl_len = 1;
	// 	load.read = spi_load_read;
	// 	ret = spl_load_simple_fit(spl_image, &load,
	// 				  CONFIG_SYS_SPI_U_BOOT_OFFS, header);
	// } else {
	// 	ret = spl_parse_image_header(spl_image, header);
	// 	if (ret)
	// 		return ret;
	//
	// 	spi0_read_data((void *)spl_image->load_addr,
	// 		       CONFIG_SYS_SPI_U_BOOT_OFFS, spl_image->size);
	// }
	//
	// spi0_deinit();

	return ret;
}
SPL_LOAD_IMAGE_METHOD("SAM9-L9260 SPI", 0, BOOT_DEVICE_SPI, spl_spi_load_image);
