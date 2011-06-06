/********************************************************************************************/
/* dm9000_regs.h  -  Copyright Wavecom S.A. (c) 2007                                        */
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
/* File    :   dm9000_regs.h                                                */
/*                                                                          */
/* Scope   :   Davicom DM9000/DM9000A Ethernet controller registers         */ 
/*                                                                          */

/*
 * $LogWavecom:$
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  06.03.07 | RFN    | 1.0            | Initial revision.                  
 * ----------+--------+----------------+-------------------------------------
*/

#ifndef __DM9000_REGS_H_INCLUDED__
#define __DM9000_REGS_H_INCLUDED__

/*
 * reference: Davicom Semiconductor, Inc.
 *            DM9000A Ethernet Controller with General Processor Interface
 *            DATA SHEET
 *            version: DM9000A-DS-F01
 */

/* registers offsets */
#define NCR     0x00
#define NSR     0x01
#define TCR     0x02
#define TSR_I   0x03
#define TSR_II  0x04
#define RCR     0x05
#define RSR     0x06
#define ROCR    0x07
#define BPTR    0x08
#define FCTR    0x09
#define FCR     0x0a
#define EPCR    0x0b
#define EPAR    0x0c
#define EPDRL   0x0d
#define EPDRH   0x0e
#define WCR     0x0f
#define PAR     0x10    /* 0x10-0x15 */
#define MAR     0x16    /* 0x16-0x1d */
#define GPCR    0x1e
#define GPR     0x1f
#define TRPAL   0x22
#define TRPAH   0x23
#define RWPAL   0x24
#define RWPAH   0x25
#define VIDL    0x28
#define VIDH    0x29
#define PIDL    0x2a
#define PIDH    0x2b
#define CHIPR   0x2c
#define TCR2    0x2d    /* DM9000A */
#define OCR     0x2e    /* DM9000A */
#define SMCR    0x2f
#define ETXCSR  0x30    /* DM9000A */
#define TCSCR   0x31    /* DM9000A */
#define RCSCSR  0x32    /* DM9000A */
#define MPAR    0x33    /* DM9000A */
#define LEDCR   0x34    /* DM9000A */
#define BUSCR   0x38    /* DM9000A */
#define INTCR   0x39    /* DM9000A */
#define SCCR    0x50    /* DM9000A */
#define RSCCR   0x51    /* DM9000A */
#define MRCMDX  0xf0
#define MRCMDX1 0xf1    /* DM9000A */
#define MRCMD   0xf2
#define MRRL    0xf4
#define MRRH    0xf5
#define MWCMDX  0xf6
#define MWCMD   0xf8
#define MWRL    0xfa
#define MWRH    0xfb
#define TXPLL   0xfc
#define TXPLH   0xfd
#define ISR     0xfe
#define IMR     0xff

#define NCR_WAKEEN      (1<<6)
#define NCR_FCOL        (1<<4)
#define NCR_FDX         (1<<3)
#define NCR_LBK         (3<<1)
#define NCR_LBK_NORMAL    (0<<1)
#define NCR_LBK_MAC_LOOP  (1<<1)
#define NCR_LBK_PHY_LOOP  (2<<1)
#define NCR_RST         (1<<0)

#define NSR_SPEED       (1<<7)
#define NSR_LINKST      (1<<6)
#define NSR_WAKEST      (1<<5)
#define NSR_TX2END      (1<<3)
#define NSR_TX1END      (1<<2)
#define NSR_RXOV        (1<<1)

#define TCR_TJDIS       (1<<6)
#define TCR_EXCECM      (1<<5)
#define TCR_PAD_DIS2    (1<<4)
#define TCR_CRC_DIS2    (1<<3)
#define TCR_PAD_DIS1    (1<<2)
#define TCR_CRC_DIS1    (1<<1)
#define TCR_TXREQ       (1<<0)

#define TSR_TJTO        (1<<7)
#define TSR_LC          (1<<6)
#define TSR_NC          (1<<5)
#define TSR_LCOL        (1<<4)
#define TSR_COL         (1<<3)
#define TSR_ECOL        (1<<2)

#define RCR_WTDIS       (1<<6)
#define RCR_DIS_LONG    (1<<5)
#define RCR_DIS_CRC     (1<<4)
#define RCR_ALL         (1<<3)
#define RCR_RUNT        (1<<2)
#define RCR_PRMSC       (1<<1)
#define RCR_RXEN        (1<<0)

#define RSR_RF          (1<<7)
#define RSR_MF          (1<<6)
#define RSR_LCS         (1<<5)
#define RSR_RWTO        (1<<4)
#define RSR_PLE         (1<<3)
#define RSR_AE          (1<<2)
#define RSR_CE          (1<<1)
#define RSR_FOE         (1<<0)

#define ROCR_RXFU       (1<<7)
#define ROCR_ROC        (0x7f<<0)

#define BPTR_BPHW       (0xf<<4)
#define BPTR_BPHW_0K      (0x0<<4)
#define BPTR_BPHW_1K      (0x1<<4)
#define BPTR_BPHW_2K      (0x2<<4)
#define BPTR_BPHW_3K      (0x3<<4)
#define BPTR_BPHW_4K      (0x4<<4)
#define BPTR_BPHW_5K      (0x5<<4)
#define BPTR_BPHW_6K      (0x6<<4)
#define BPTR_BPHW_7K      (0x7<<4)
#define BPTR_BPHW_8K      (0x8<<4)
#define BPTR_BPHW_9K      (0x9<<4)
#define BPTR_BPHW_10K     (0xa<<4)
#define BPTR_BPHW_11K     (0xb<<4)
#define BPTR_BPHW_12K     (0xc<<4)
#define BPTR_BPHW_13K     (0xd<<4)
#define BPTR_BPHW_14K     (0xe<<4)
#define BPTR_BPHW_15K     (0xf<<4)
#define BPTR_JPT        (0xf<<0)
#define BPTR_JPT_5us      (0x0<<0)
#define BPTR_JPT_10us     (0x1<<0)
#define BPTR_JPT_15us     (0x2<<0)
#define BPTR_JPT_25us     (0x3<<0)
#define BPTR_JPT_50us     (0x4<<0)
#define BPTR_JPT_100us    (0x5<<0)
#define BPTR_JPT_150us    (0x6<<0)
#define BPTR_JPT_200us    (0x7<<0)
#define BPTR_JPT_250us    (0x8<<0)
#define BPTR_JPT_300us    (0x9<<0)
#define BPTR_JPT_350us    (0xa<<0)
#define BPTR_JPT_400us    (0xb<<0)
#define BPTR_JPT_450us    (0xc<<0)
#define BPTR_JPT_500us    (0xd<<0)
#define BPTR_JPT_550us    (0xe<<0)
#define BPTR_JPT_600us    (0xf<<0)

#define FCTR_HWOT       (0xf<<4)
#define FCTR_HWOT_0K      (0x0<<4)
#define FCTR_HWOT_1K      (0x1<<4)
#define FCTR_HWOT_2K      (0x2<<4)
#define FCTR_HWOT_3K      (0x3<<4)
#define FCTR_HWOT_4K      (0x4<<4)
#define FCTR_HWOT_5K      (0x5<<4)
#define FCTR_HWOT_6K      (0x6<<4)
#define FCTR_HWOT_7K      (0x7<<4)
#define FCTR_HWOT_8K      (0x8<<4)
#define FCTR_HWOT_9K      (0x9<<4)
#define FCTR_HWOT_10K     (0xa<<4)
#define FCTR_HWOT_11K     (0xb<<4)
#define FCTR_HWOT_12K     (0xc<<4)
#define FCTR_HWOT_13K     (0xd<<4)
#define FCTR_HWOT_14K     (0xe<<4)
#define FCTR_HWOT_15K     (0xf<<4)
#define FCTR_LWOT       (0xf<<0)
#define FCTR_LWOT_0K      (0x0<<0)
#define FCTR_LWOT_1K      (0x1<<0)
#define FCTR_LWOT_2K      (0x2<<0)
#define FCTR_LWOT_3K      (0x3<<0)
#define FCTR_LWOT_4K      (0x4<<0)
#define FCTR_LWOT_5K      (0x5<<0)
#define FCTR_LWOT_6K      (0x6<<0)
#define FCTR_LWOT_7K      (0x7<<0)
#define FCTR_LWOT_8K      (0x8<<0)
#define FCTR_LWOT_9K      (0x9<<0)
#define FCTR_LWOT_10K     (0xa<<0)
#define FCTR_LWOT_11K     (0xb<<0)
#define FCTR_LWOT_12K     (0xc<<0)
#define FCTR_LWOT_13K     (0xd<<0)
#define FCTR_LWOT_14K     (0xe<<0)
#define FCTR_LWOT_15K     (0xf<<0)

#define FCR_TXP0        (1<<7)
#define FCR_TXPF        (1<<6)
#define FCR_TXPEN       (1<<5)
#define FCR_BKPA        (1<<4)
#define FCR_BKPM        (1<<3)
#define FCR_RXPS        (1<<2)
#define FCR_RXPCS       (1<<1)
#define FCR_FLCE        (1<<0)

#define EPCR_REEP       (1<<5)
#define EPCR_WEP        (1<<4)
#define EPCR_EPOS       (1<<3)
#define EPCR_ERPRR      (1<<2)
#define EPCR_ERPRW      (1<<1)
#define EPCR_ERRE       (1<<0)

#define EPAR_PHY_ADR    (3<<6)
#define EPAR_PHY_ADR_SHIFT  6
#define EPAR_EROA       (0x1f<<0)

#define WCR_LINKEN      (1<<5)
#define WCR_SAMPLEEN    (1<<4)
#define WCR_MAGICEN     (1<<3)
#define WCR_LINKST      (1<<2)
#define WCR_SAMPLEST    (1<<1)
#define WCR_MAGICST     (1<<0)

#define GPCR_GPC6       (1<<6)    /* DM9000A */
#define GPCR_GPC5       (1<<5)    /* DM9000A */
#define GPCR_GPC4       (1<<4)    /* DM9000A */
#define GPCR_GPC3       (1<<3)
#define GPCR_GPC2       (1<<2)
#define GPCR_GPC1       (1<<1)

#define GPR_GPO6        (1<<6)    /* DM9000A */
#define GPR_GPO5        (1<<5)    /* DM9000A */
#define GPR_GPO4        (1<<4)    /* DM9000A */
#define GPR_GPIO3       (1<<3)
#define GPR_GPIO2       (1<<2)
#define GPR_GPIO1       (1<<1)
#define GPR_PHYD        (1<<0)

#define VID_CHECK   0x0a46
#define PID_CHECK   0x9000

#define TCR2_LED        (1<<7)
#define TCR2_RLCP       (1<<6)
#define TCR2_DTU        (1<<5)
#define TCR2_ONEPM      (1<<4)
#define TCR2_IFGS       (0xf<<0)
#define TCR2_IFGS_64      (0x8<<0)
#define TCR2_IFGS_72      (0x9<<0)
#define TCR2_IFGS_80      (0xa<<0)
#define TCR2_IFGS_88      (0xb<<0)
#define TCR2_IFGS_96      (0xc<<0)
#define TCR2_IFGS_104     (0xd<<0)
#define TCR2_IFGS_112     (0xe<<0)
#define TCR2_IFGS_120     (0xf<<0)

#define OCR_SCC         (3<<6)
#define OCR_SCC_50MHz     (0<<6)
#define OCR_SCC_20MHz     (1<<6)
#define OCR_SCC_100MHz    (2<<6)
#define OCR_SOE         (1<<4)
#define OCR_SCS         (1<<3)
#define OCR_PHYOP       (7<<0)

#define SMCR_SM_EN      (1<<7)
#define SMCR_FLC        (1<<2)
#define SMCR_FB1        (1<<1)
#define SMCR_FB0        (1<<0)

#define ETXCSR_ETE      (1<<7)
#define ETXCSR_ETS2     (1<<6)
#define ETXCSR_ETS1     (1<<5)
#define ETXCSR_ETT      (3<<0)
#define ETXCSR_ETT_12_5   (0<<0)
#define ETXCSR_ETT_25     (1<<0)
#define ETXCSR_ETT_50     (2<<0)
#define ETXCSR_ETT_75     (3<<0)

#define TCSCR_UDPCSE    (1<<2)
#define TCSCR_TCPCSE    (1<<1)
#define TCSCR_IPCSE     (1<<0)

#define RCSCSR_UDPS     (1<<7)
#define RCSCSR_TCPS     (1<<6)
#define RCSCSR_IPS      (1<<5)
#define RCSCSR_UDPP     (1<<4)
#define RCSCSR_TCPP     (1<<3)
#define RCSCSR_IPP      (1<<2)
#define RCSCSR_RCSEN    (1<<1)
#define RCSCSR_DCSE     (1<<0)

#define MPAR_ADR_EN     (1<<7)
#define MPAR_EPHYADR    (0x1f<<0)

#define LEDCR_GPIO      (1<<1)
#define LEDCR_MII       (1<<0)

#define BUSCR_CURR      (7<<5)
#define BUSCR_CURR_2mA    (0<<5)
#define BUSCR_CURR_4mA    (1<<5)
#define BUSCR_CURR_6mA    (2<<5)
#define BUSCR_CURR_8mA    (3<<5)
#define BUSCR_CURR_10mA   (4<<5)
#define BUSCR_CURR_12mA   (5<<5)
#define BUSCR_CURR_14mA   (6<<5)
#define BUSCR_CURR_16mA   (7<<5)
#define BUSCR_EST       (1<<3)
#define BUSCR_IOW_SPIKE (1<<1)
#define BUSCR_IOR_SPIKE (1<<0)

#define INTCR_INT_TYPE  (1<<1)
#define INTCR_INT_POL   (1<<0)

#define SCCR_DIS_CLK    (1<<0)

#define ISR_IOMODE      (3<<6)
#define ISR_IOMODE_16     (0<<6)
#define ISR_IOMODE_32     (1<<6)  /* DM9000 */
#define ISR_IOMODE_8      (2<<6)
#define ISR_LNKCHGS     (1<<5)    /* DM9000A */
#define ISR_UDRUNS      (1<<4)    /* DM9000A */
#define ISR_ROOS        (1<<3)
#define ISR_ROS         (1<<2)
#define ISR_PTS         (1<<1)
#define ISR_PRS         (1<<0)

#define IMR_PAR         (1<<7)
#define IMR_LNKCHGI     (1<<5)    /* DM9000A */
#define IMR_UDRUNI      (1<<4)    /* DM9000A */
#define IMR_ROOI        (1<<3)
#define IMR_ROI         (1<<2)
#define IMR_PTI         (1<<1)
#define IMR_PRI         (1<<0)

/* PHY registers */
#define BMCR      0x00
#define BMSR      0x01
#define PHYID1    0x02
#define PHYID2    0x03
#define ANAR      0x04
#define ANLPAR    0x05
#define ANER      0x06
#define DSCR      0x16
#define DSCSR     0x17
#define TBTCSR    0x18
#define PWDOR     0x19
#define SPCFG     0x20

#define BMCR_RST      (1<<15)
#define BMCR_LOOPBK   (1<<14)
#define BMCR_SPEED    (1<<13)
#define BMCR_ANEG     (1<<12)
#define BMCR_PWRDWN   (1<<11)
#define BMCR_ISOLATE  (1<<10)
#define BMCR_RESTART  (1<<9)
#define BMCR_DPX      (1<<8)
#define BMCR_COLTST   (1<<7)

#define ANAR_TX_FDX   (1<<8)
#define ANAR_TX_HDX   (1<<7)
#define ANAR_10_FDX   (1<<6)
#define ANAR_10_HDX   (1<<5)
#define ANAR_SELECT   1

#endif /* __DM9000_REGS_H_INCLUDED__ */
