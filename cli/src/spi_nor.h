/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 */

#ifndef __LINUX_MTD_SPI_NOR_H
#define __LINUX_MTD_SPI_NOR_H

/* Flash opcodes. */
#define SPINOR_OP_WRDI 0x04       /* Write disable */
#define SPINOR_OP_WREN 0x06       /* Write enable */
#define SPINOR_OP_RDSR 0x05       /* Read status register */
#define SPINOR_OP_WRSR 0x01       /* Write status register 1 byte */
#define SPINOR_OP_RDSR2 0x3f      /* Read status register 2 */
#define SPINOR_OP_WRSR2 0x3e      /* Write status register 2 */
#define SPINOR_OP_READ 0x03       /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST 0x0b  /* Read data bytes (high frequency) */
#define SPINOR_OP_READ_1_1_2 0x3b /* Read data bytes (Dual Output SPI) */
#define SPINOR_OP_READ_1_2_2 0xbb /* Read data bytes (Dual I/O SPI) */
#define SPINOR_OP_READ_1_1_4 0x6b /* Read data bytes (Quad Output SPI) */
#define SPINOR_OP_READ_1_4_4 0xeb /* Read data bytes (Quad I/O SPI) */
#define SPINOR_OP_READ_1_1_8 0x8b /* Read data bytes (Octal Output SPI) */
#define SPINOR_OP_READ_1_8_8 0xcb /* Read data bytes (Octal I/O SPI) */
#define SPINOR_OP_PP 0x02         /* Page program (up to 256 bytes) */
#define SPINOR_OP_PP_1_1_4 0x32   /* Quad page program */
#define SPINOR_OP_PP_1_4_4 0x38   /* Quad page program */
#define SPINOR_OP_PP_1_1_8 0x82   /* Octal page program */
#define SPINOR_OP_PP_1_8_8 0xc2   /* Octal page program */
#define SPINOR_OP_BE_4K 0x20      /* Erase 4KiB block */
#define SPINOR_OP_BE_4K_PMC 0xd7  /* Erase 4KiB block on PMC chips */
#define SPINOR_OP_BE_32K 0x52     /* Erase 32KiB block */
#define SPINOR_OP_CHIP_ERASE 0xc7 /* Erase whole flash chip */
#define SPINOR_OP_SE 0xd8         /* Sector erase (usually 64KiB) */
#define SPINOR_OP_RDID 0x9f       /* Read JEDEC ID */
#define SPINOR_OP_RDSFDP 0x5a     /* Read SFDP */
#define SPINOR_OP_RDCR 0x35       /* Read configuration register */
#define SPINOR_OP_RDFSR 0x70      /* Read flag status register */
#define SPINOR_OP_CLFSR 0x50      /* Clear flag status register */
#define SPINOR_OP_RDEAR 0xc8      /* Read Extended Address Register */
#define SPINOR_OP_WREAR 0xc5      /* Write Extended Address Register */
#define SPINOR_OP_WRCR 0x81       /* Write Configuration register */

/* reset commands */
#define SPINOR_OP_RST_EN 0x66  /* Reset flash enable must be followed by the RESET MEMORY */
#define SPINOR_OP_RST_MEM 0x99 /* Reset flash memory */

/* 4-byte address opcodes - used on Spansion and some Macronix flashes. */
#define SPINOR_OP_READ_4B 0x13       /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST_4B 0x0c  /* Read data bytes (high frequency) */
#define SPINOR_OP_READ_1_1_2_4B 0x3c /* Read data bytes (Dual Output SPI) */
#define SPINOR_OP_READ_1_2_2_4B 0xbc /* Read data bytes (Dual I/O SPI) */
#define SPINOR_OP_READ_1_1_4_4B 0x6c /* Read data bytes (Quad Output SPI) */
#define SPINOR_OP_READ_1_4_4_4B 0xec /* Read data bytes (Quad I/O SPI) */
#define SPINOR_OP_READ_1_1_8_4B 0x7c /* Read data bytes (Octal Output SPI) */
#define SPINOR_OP_READ_1_8_8_4B 0xcc /* Read data bytes (Octal I/O SPI) */
#define SPINOR_OP_PP_4B 0x12         /* Page program (up to 256 bytes) */
#define SPINOR_OP_PP_1_1_4_4B 0x34   /* Quad page program */
#define SPINOR_OP_PP_1_4_4_4B 0x3e   /* Quad page program */
#define SPINOR_OP_PP_1_1_8_4B 0x84   /* Octal page program */
#define SPINOR_OP_PP_1_8_8_4B 0x8e   /* Octal page program */
#define SPINOR_OP_BE_4K_4B 0x21      /* Erase 4KiB block */
#define SPINOR_OP_BE_32K_4B 0x5c     /* Erase 32KiB block */
#define SPINOR_OP_SE_4B 0xdc         /* Sector erase (usually 64KiB) */

/* Double Transfer Rate opcodes - defined in JEDEC JESD216B. */
#define SPINOR_OP_READ_1_1_1_DTR 0x0d
#define SPINOR_OP_READ_1_2_2_DTR 0xbd
#define SPINOR_OP_READ_1_4_4_DTR 0xed

#define SPINOR_OP_READ_1_1_1_DTR_4B 0x0e
#define SPINOR_OP_READ_1_2_2_DTR_4B 0xbe
#define SPINOR_OP_READ_1_4_4_DTR_4B 0xee

/* Used for SST flashes only. */
#define SPINOR_OP_BP 0x02     /* Byte program */
#define SPINOR_OP_AAI_WP 0xad /* Auto address increment word program */

#define GLOBAL_BLKPROT_UNLK 0x98 /* Clear global write protection bits */
/* Used for S3AN flashes only */
#define SPINOR_OP_XSE 0x50   /* Sector erase */
#define SPINOR_OP_XPP 0x82   /* Page program */
#define SPINOR_OP_XRDSR 0xd7 /* Read status register */

#define XSR_PAGESIZE BIT(0) /* Page size in Po2 or Linear */
#define XSR_RDY BIT(7)      /* Ready */

/* Used for Macronix and Winbond flashes. */
#define SPINOR_OP_EN4B 0xb7 /* Enter 4-byte mode */
#define SPINOR_OP_EX4B 0xe9 /* Exit 4-byte mode */

/* Used for Spansion flashes only. */
#define SPINOR_OP_BRWR 0x17 /* Bank register write */
#define SPINOR_OP_BRRD 0x16 /* Bank register read */
#define SPINOR_OP_CLSR 0x30 /* Clear status register 1 */

/* Used for Micron flashes only. */
#define SPINOR_OP_RD_EVCR 0x65 /* Read EVCR register */
#define SPINOR_OP_WD_EVCR 0x61 /* Write EVCR register */

/* For Octal SPI Macronix flashes only */
#define SPINOR_OP_WR_CFG_REG2 0x72 /* Write Config register2 */

/* For Octal SPI Macronix flashes only */
#define SPINOR_MACRONIX_CFG2_OCTAL_DDR 0x2

/* For Micron flashes only */
#define SPINOR_VCR_OCTAL_DDR 0xE7 /* VCR BYTE0 value for Octal DDR mode */

/* Status Register bits. */
#define SR_WIP BIT(0) /* Write in progress */
#define SR_WEL BIT(1) /* Write enable latch */

/* Flag Status Register bits */
#define FSR_READY BIT(7)  /* Device status, 0 = Busy, 1 = Ready */
#define FSR_E_ERR BIT(5)  /* Erase operation status */
#define FSR_P_ERR BIT(4)  /* Program operation status */
#define FSR_PT_ERR BIT(1) /* Protection error bit */

#endif