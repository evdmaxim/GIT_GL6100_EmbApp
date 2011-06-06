/********************************************************************************************/
/* enc28j60_regs.h  -  Copyright Wavecom S.A. (c) 2007                                      */
/*                                                                                          */
/*                                                                                          */
/* DISCLAIMER OF WARRANTY                                                                   */
/* ======================                                                                   */
/* This Software is provided free of charge on an 'as is' basis. No warranty is given       */
/* by Wavecom S.A. in relation to the Software of the uses to which it may be put by you,   */
/* the user, or its merchantability, fitness or suitability for any particular purpose      */
/* or conditions; and/or that the use of the Software and all documentation relating        */
/* thereto by the Licensee will not infringe any third party copyright or other             */
/* intellectual property rights. Wavecom S.A. shall furthermore be under no obligation      */
/* to provide support of any nature for the Software and the Documentation.                 */
/*                                                                                          */
/* LIMIT OF LIABILITY                                                                       */
/* ==================                                                                       */
/* In no event shall Wavecom S.A. be liable for any loss or damages whatsoever or howsoever */
/* caused arising directly or indirectly in connection with this licence, the Software,     */
/* its use or otherwise except to the extent that such liability may not be lawfully        */
/* excluded. Notwithstanding the generality of the foregoing, Wavecom S.A. expressly        */
/* excludes liability for indirect, special, incidental or consequential loss or damage     */
/* which may arise in respect of the Software or its use, or in respect of other equipment  */
/* or property, or for loss of profit, business, revenue, goodwill or anticipated savings.  */
/*                                                                                          */
/********************************************************************************************/

/****************************************************************************/
/* File    :   enc28j60_regs.h                                              */
/*                                                                          */
/* Scope   :   Microchip ENC28J60 Ethernet controller registers             */
/*                                                                          */

/*
 * $LogWavecom:$
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  16.10.07 | RFN    | 1.0            | Initial revision.                  
 * ----------+--------+----------------+-------------------------------------
*/

#ifndef __ENC28J60_REGS_H_INCLUDED__
#define __ENC28J60_REGS_H_INCLUDED__

/*
 * reference: Microchip Technology Inc.
 *            ENC28J60 Data Sheet
 *            Stand-Alone Ethernet Controller with SPI Interface
 *            reference: DS39662B
 */

/* registers offsets */

/* common registers */
#define EIE       0x1b
#define EIR       0x1c
#define ESTAT     0x1d
#define ECON2     0x1e
#define ECON1     0x1f

#define EIE_INTIE     (1<<7)
#define EIE_PKTIE     (1<<6)
#define EIE_DMAIE     (1<<5)
#define EIE_LINKIE    (1<<4)
#define EIE_TXIE      (1<<3)
#define EIE_TXERIE    (1<<1)
#define EIE_RXERIE    (1<<0)

#define EIR_PKTIF     (1<<6)
#define EIR_DMAIF     (1<<5)
#define EIR_LINKIF    (1<<4)
#define EIR_TXIF      (1<<3)
#define EIR_TXERIF    (1<<1)
#define EIR_RXERIF    (1<<0)

#define ESTAT_INT     (1<<7)
#define ESTAT_BUFER   (1<<6)
#define ESTAT_LATECOL (1<<4)
#define ESTAT_RXBUSY  (1<<2)
#define ESTAT_TXABRT  (1<<1)
#define ESTAT_CLKRDY  (1<<0)

#define ECON2_AUTOINC (1<<7)
#define ECON2_PKTDEC  (1<<6)
#define ECON2_PWRSV   (1<<5)
#define ECON2_VRPS    (1<<3)

#define ECON1_TXRST   (1<<7)
#define ECON1_RXRST   (1<<6)
#define ECON1_DMAST   (1<<5)
#define ECON1_CSUMEN  (1<<4)
#define ECON1_TXRTS   (1<<3)
#define ECON1_RXEN    (1<<2)
#define ECON1_BSEL    (3<<0)
#define ECON1_BSEL_BANK0  (0<<0)
#define ECON1_BSEL_BANK1  (1<<0)
#define ECON1_BSEL_BANK2  (2<<0)
#define ECON1_BSEL_BANK3  (3<<0)

/* bank 0 */
#define ERDPTL    0x00
#define ERDPTH    0x01
#define EWRPTL    0x02
#define EWRPTH    0x03
#define ETXSTL    0x04
#define ETXSTH    0x05
#define ETXNDL    0x06
#define ETXNDH    0x07
#define ERXSTL    0x08
#define ERXSTH    0x09
#define ERXNDL    0x0a
#define ERXNDH    0x0b
#define ERXRDPTL  0x0c
#define ERXRDPTH  0x0d
#define ERXWRPTL  0x0e
#define ERXWRPTH  0x0f
#define EDMASTL   0x10
#define EDMASTH   0x11
#define EDMANDL   0x12
#define EDMANDH   0x13
#define EDMADSTL  0x14
#define EDMADSTH  0x15
#define EDMACSL   0x16
#define EDMACSH   0x17

/* bank 1 */
#define EHT0      0x00
#define EHT1      0x01
#define EHT2      0x02
#define EHT3      0x03
#define EHT4      0x04
#define EHT5      0x05
#define EHT6      0x06
#define EHT7      0x07
#define EPMM0     0x08
#define EPMM1     0x09
#define EPMM2     0x0a
#define EPMM3     0x0b
#define EPMM4     0x0c
#define EPMM5     0x0d
#define EPMM6     0x0e
#define EPMM7     0x0f
#define EPMCSL    0x10
#define EPMCSH    0x11
#define EPMOL     0x14
#define EPMOH     0x15
#define ERXFCON   0x18
#define EPKTCNT   0x19

#define ERXFCON_UCEN    (1<<7)
#define ERXFCON_ANDOR   (1<<6)
#define ERXFCON_CRCEN   (1<<5)
#define ERXFCON_PMEN    (1<<4)
#define ERXFCON_MPEN    (1<<3)
#define ERXFCON_HTEN    (1<<2)
#define ERXFCON_MCEN    (1<<1)
#define ERXFCON_BCEN    (1<<0)

/* bank 2 */
#define MACON1		0x00
#define MACON3		0x02
#define MACON4		0x03
#define MABBIPG		0x04
#define MAIPGL		0x06
#define MAIPGH		0x07
#define MACLCON1	0x08
#define MACLCON2	0x09
#define MAMXFLL		0x0A
#define MAMXFLH		0x0B
#define MICMD		  0x12
#define MIREGADR	0x14
#define MIWRL		  0x16
#define MIWRH		  0x17
#define MIRDL		  0x18
#define MIRDH		  0x19

#define MACON1_TXPAUS   (1<<3)
#define MACON1_RXPAUS   (1<<2)
#define MACON1_PASSALL  (1<<1)
#define MACON1_MARXEN   (1<<0)

#define MACON3_PADCFG   (7<<5)
#define MACON3_PADCFG_NOPAD   (0<<5)
#define MACON3_PADCFG_PAD60   (1<<5)
#define MACON3_PADCFG_PAD64   (3<<5)
#define MACON3_PADCFG_PADVLAN (5<<5)
#define MACON3_TXCRCEN  (1<<4)
#define MACON3_PHDREN   (1<<3)
#define MACON3_HFRMEN   (1<<2)
#define MACON3_FRMLNEN  (1<<1)
#define MACON3_FULDPX   (1<<0)

#define MACON4_DEFER    (1<<6)
#define MACON4_BPEN     (1<<5)
#define MACON4_NOBKOFF  (1<<4)

#define MICMD_MIISCAN   (1<<1)
#define MICMD_MIIRD     (1<<0)

/* bank 3 */
#define MAADR5		0x00
#define MAADR6		0x01
#define MAADR3		0x02
#define MAADR4		0x03
#define MAADR1		0x04
#define MAADR2		0x05
#define EBSTSD		0x06
#define EBSTCON		0x07
#define EBSTCSL		0x08
#define EBSTCSH		0x09
#define MISTAT		0x0A
#define EREVID		0x12
#define ECOCON		0x15
#define EFLOCON		0x17
#define EPAUSL		0x18
#define EPAUSH		0x19

#define EBSTCON_PSV     (7<<5)
#define EBSTCON_PSV_0       (0<<5)
#define EBSTCON_PSV_1       (1<<5)
#define EBSTCON_PSV_2       (2<<5)
#define EBSTCON_PSV_3       (3<<5)
#define EBSTCON_PSV_4       (4<<5)
#define EBSTCON_PSV_5       (5<<5)
#define EBSTCON_PSV_6       (6<<5)
#define EBSTCON_PSV_7       (7<<5)
#define EBSTCON_PSEL    (1<<4)
#define EBSTCON_TMSEL   (3<<2)

#define EBSTCON_TME     (1<<1)
#define EBSTCON_BISTST  (1<<0)

#define MISTAT_NVALID   (1<<2)
#define MISTAT_SCAN     (1<<1)
#define MISTAT_BUSY     (1<<0)

#define ECOCON_COCON    (7<<0)
#define ECOCON_COCON_DIS    (0<<0)
#define ECOCON_COCON_DIV1   (1<<0)
#define ECOCON_COCON_DIV2   (2<<0)
#define ECOCON_COCON_DIV3   (3<<0)
#define ECOCON_COCON_DIV4   (4<<0)
#define ECOCON_COCON_DIV8   (5<<0)

#define EFLOCON_FULDPXS (1<<2)
#define EFLOCON_FCEN    (3<<0)
#define EFLOCON_FCEN_OFF    (0<<0)
#define EFLOCON_FCEN_ON     (1<<0)
#define EFLOCON_FCEN_SEND   (1<<0)
#define EFLOCON_FCEN_PERIOD (2<<0)
#define EFLOCON_FCEN_SEND0  (3<<0)

/* PHY registers */
#define PHCON1    0x00
#define PHSTAT1   0x01
#define PHID1     0x02
#define PHID2     0x03
#define PHCON2    0x10
#define PHSTAT2   0x11
#define PHIE      0x12
#define PHIR      0x13
#define PHLCON    0x14

#define PHCON1_PRST     (1<<15)
#define PHCON1_PLOOPBK  (1<<14)
#define PHCON1_PPWRSV   (1<<11)
#define PHCON1_PDPXMD   (1<<8)

#define PHSTAT1_PFDPX   (1<<12)
#define PHSTAT1_PHDPX   (1<<11)
#define PHSTAT1_LLSTAT  (1<<2)
#define PHSTAT1_JBSTAT  (1<<1)

#define PHCON2_FRCLNK   (1<<14)
#define PHCON2_TXDIS    (1<<13)
#define PHCON2_JABBER   (1<<10)
#define PHCON2_HDLDIS   (1<<8)

#define PHSTAT2_TXSTAT  (1<<13)
#define PHSTAT2_RXSTAT  (1<<12)
#define PHSTAT2_COLSTAT (1<<11)
#define PHSTAT2_LSTAT   (1<<10)
#define PHSTAT2_DPXSTAT (1<<9)
#define PHSTAT2_PLRITY  (1<<5)

#define PHIE_PLNKIE     (1<<4)
#define PHIE_PGEIE      (1<<1)

#define PHIR_PLNKIF     (1<<4)
#define PHIR_PGIF       (1<<2)

#define PHLCON_LACFG    (0xf<<8)
#define PHLCON_LBCFG    (0xf<<4)
#define PHLCON_LFRQ     (3<<2)
#define PHLCON_STRCH    (1<<1)

#endif /* __ENC28J60_REGS_H_INCLUDED__ */
