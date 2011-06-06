/********************************************************************************************/
/* drv_dm9000.c  -  Copyright Wavecom S.A. (c) 2007                                         */
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
/* File    :   drv_dm9000.c                                                 */
/*                                                                          */
/* Scope   :   Davicom DM9000/DM9000A Ethernet driver                       */
/*                                                                          */

/*
 * Known limitations:
 *  - EEPROM is not handled
 */

/*
 * $LogWavecom:$
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  06.03.07 | RFN    | 1.0            | Initial revision.
 * ----------+--------+----------------+-------------------------------------
 *  05.11.07 | RFN    |                | DEV42836: support for OpenAT OS v6
 * ----------+--------+----------------+-------------------------------------
 *  03.11.07 | RFN    | 1.0            | New subscribe function
 *           |        |                | corrected PHY initialization
 *           |        |                | setup reset and power signals
 * ----------+--------+----------------+-------------------------------------
 *  13.03.08 | DGI    | 1.0.1.0        | ANO46388
 * ----------+--------+----------------+-------------------------------------
 *  11.09.08 | DGI    | 1.0.1.1        | ANO48994
 * ----------+--------+----------------+-------------------------------------
*/

#include "wip_drv.h"
#include "adl_gpio.h"
#include "adl_bus.h"
#include "adl_memory.h"
#include "wip_drv_dm9000.h"
#include "dm9000_regs.h"

#include <string.h>

/* driver data */
typedef struct {
  s32           bus_handle;
  volatile bool txbusy;
  u32           indexAddr;
  u32           dataAddr;
  volatile u16 *indexPtr;
  volatile u16 *dataPtr;
  u8            csId;
  s32           io_handle;
  s32           isr_handle;
  u32           addressPin;
  u32           extIntId;
  adl_ioDefs_t  resetGpioId;
  adl_ioDefs_t  powerGpioId;
  wip_ethAddr_t macAddr;
} dm9000data;


/*
 * DM9000 registers access
 */

static u8 read_reg( dm9000data *dp, u8 reg)
{
  *(dp->indexPtr) = reg;
  return *(dp->dataPtr);
}

static void write_reg( dm9000data *dp, u8 reg, u8 val)
{
  *(dp->indexPtr) = reg;
  *(dp->dataPtr)  = val;
}

static void write_index( dm9000data *dp, u8 reg)
{
  *(dp->indexPtr) = reg;
}

static u8 read_index( dm9000data *dp)
{
  return *(dp->indexPtr);
}

static void write_data( dm9000data *dp, u16 val)
{
  *(dp->dataPtr) = val;
}

static u16 read_data( dm9000data *dp)
{
  return *(dp->dataPtr);
}

static void read_buf( dm9000data *dp, void *ptr, int len)
{
  adl_busDirectRead( dp->bus_handle, dp->dataAddr, len, ptr);
}

static void write_buf( dm9000data *dp, const void *ptr, int len)
{
  adl_busDirectWrite( dp->bus_handle, dp->dataAddr, len, (void *) ptr);
}

/*
 * Internal PHY access
 */

static u16 read_phy( dm9000data *dp, u8 reg)
{
  s32 intr_sav;
  u8 status;
  u16 data;

  intr_sav = wip_drvIrqDisable();
  write_reg( dp, EPAR, (1 << EPAR_PHY_ADR_SHIFT) | reg);
  write_reg( dp, EPCR, EPCR_EPOS | EPCR_ERPRR);
  wip_drvIrqRestore( intr_sav);

  do {
    intr_sav = wip_drvIrqDisable();
    status = read_reg( dp, EPCR);
    wip_drvIrqRestore( intr_sav);
  }
  while( status & EPCR_ERRE);

  intr_sav = wip_drvIrqDisable();
  write_reg( dp, EPCR, 0);
  data = read_reg( dp, EPDRL) | (read_reg( dp, EPDRH) << 8);
  wip_drvIrqRestore( intr_sav);

  return data;
}

static void write_phy( dm9000data *dp, u8 reg, u16 data)
{
  s32 intr_sav;
  u8 status;

  intr_sav = wip_drvIrqDisable();
  write_reg( dp, EPAR, (1 << EPAR_PHY_ADR_SHIFT) | reg);
  write_reg( dp, EPDRL, data);
  write_reg( dp, EPDRH, data>>8);
  write_reg( dp, EPCR, EPCR_EPOS | EPCR_ERPRW);
  wip_drvIrqRestore( intr_sav);

  do {
    intr_sav = wip_drvIrqDisable();
    status = read_reg( dp, EPCR);
    wip_drvIrqRestore( intr_sav);
  }
  while( status & EPCR_ERRE);

  intr_sav = wip_drvIrqDisable();
  write_reg( dp, EPCR, 0);
  wip_drvIrqRestore( intr_sav);
}


/*
 * Interrupt handler
 */
static void drv_intr( wip_drvData_t *drvp)
{
  u8 istatus, sav_imask;
  dm9000data * dp;
  int looprx;

  dp = drvp->drv_data;

  /* mask interrupts */
  sav_imask = read_reg( dp, IMR);
  write_reg( dp, IMR, sav_imask & ~(IMR_LNKCHGI|IMR_UDRUNI|IMR_ROOI|
                                    IMR_ROI|IMR_PTI|IMR_PRI));

  istatus = read_reg( dp, ISR);
  write_reg( dp, ISR, istatus); /* clear interrupts */

  /*
   * RX interrupt
   */
  if( istatus & ISR_PRS) do {
    u16 status, pktlen;

    /* dummy read ? */
    read_reg( dp, MRCMDX);
    status = read_data( dp);

    switch( status & 0xff) {

    case 0x01:  /* packet ready */
    {
      wip_drvBuf_t *bufp;

      /* read packet header */
      write_index( dp, MRCMD);
      status = read_data( dp);
      pktlen = read_data( dp);

      /* discard invalid frames */
      if( (status & 0xbf00) || (pktlen < 64) || (pktlen > 1518)) {
        bufp = NULL;
      }
      else {
        bufp = wip_drvBufAlloc( drvp, pktlen-4);
      }

      /* read packet data */
      if( bufp != NULL) {
        pktlen -= 4;  /* discard CRC */
        read_buf( dp, bufp->buf_datap, (pktlen+1)/2);
        read_data( dp); read_data( dp);   /* skip CRC */
        wip_drvBufEnqueue( drvp, bufp);
      }
      else {
        /* flush discarded packet data */
        while( pktlen >= 16) {
          read_data( dp); read_data( dp);
          read_data( dp); read_data( dp);
          read_data( dp); read_data( dp);
          read_data( dp); read_data( dp);
          pktlen -= 16;
        }
        while( pktlen > 0) {
          read_data( dp);
          pktlen -= 2;
        }
      }
      looprx = 1; /* check next packet */
      break;
    }

    case 0x00:  /* no more packet to read */
      looprx = 0;
      break;

    default:
      looprx = 0;
      wip_debug( "[DM9000] bad status: %04x\n", status);
      /* stop RX -> need to reset chip ??? */
      write_reg( dp, RCR, read_reg( dp, RCR) & ~RCR_RXEN);
      break;
    }

  } while( looprx);

  /*
   * TX interrupt
   */
  if( istatus & ISR_PTS) {
    u8 nsr;
    /* read and clear tx status */
    nsr = read_reg( dp, NSR);
    write_reg( dp, NSR, nsr);
    /* data transmitted ? */
    {
      wip_drvBuf_t *bufp;
      /* get next buffer to transmit */
      bufp = wip_drvBufDequeue( drvp);
      if( bufp != NULL) {
        dp->txbusy = TRUE;
        /* transfert buffer data to controller */
        write_index( dp, MWCMD);
        write_buf( dp, bufp->buf_datap, (bufp->buf_datalen+1)/2);
        /* write length of buffer and request tx */
        write_reg( dp, TXPLH, bufp->buf_datalen>>8);
        write_reg( dp, TXPLL, bufp->buf_datalen & 0xff);
        write_reg( dp, TCR, TCR_TXREQ);
        /* release buffer */
        wip_drvBufFree( drvp, bufp);
      }
      else {
        dp->txbusy = FALSE;
      }
    }
  }

  /* restore interrupt mask */
  write_reg( dp, IMR, sav_imask);
}


/*
 * Set input and multicast filter
 */
static void drv_set_filter( wip_drvEthData_t *drvp)
{
  s32 intr_sav;
  dm9000data * dp;

  dp = drvp->eth_drvdata.drv_data;

  if( drvp->eth_promisc) {
    /* promiscuous mode */
    intr_sav = wip_drvIrqDisable();
    write_reg( dp, RCR, read_reg( dp, RCR) | (RCR_ALL|RCR_PRMSC));
    wip_drvIrqRestore( intr_sav);
  }
  else if( drvp->eth_allmulti) {
    /* all multicast mode */
    intr_sav = wip_drvIrqDisable();
    write_reg( dp, RCR, (read_reg( dp, RCR) & ~RCR_PRMSC) | RCR_ALL);
    wip_drvIrqRestore( intr_sav);
  }
  else {
    int i;
    u16 hash[4];

    /* set multicast filter */
    hash[0] = 0x0000;
    hash[1] = 0x0000;
    hash[2] = 0x0000;
    hash[3] = 0x8000;  /* receive broadcasts */
    for( i=0; i<drvp->eth_mcast_nb; i++) {
      int val;
      /* compute CRC of multicast address,
         keep 6-bits as hash index */
      val = wip_ethCRC32( drvp->eth_mcast[i], 6) >> 26;
      /* bits are reversed */
      val = ((val & 0x20) >> 5) |
            ((val & 0x10) >> 3) |
            ((val & 0x08) >> 1) |
            ((val & 0x04) << 1) |
            ((val & 0x02) << 3) |
            ((val & 0x01) << 5);
      hash[val>>4] |= 1 << (val & 0xf);
    }

    intr_sav = wip_drvIrqDisable();
    for( i=0; i<4; i++) {
      write_reg( dp, MAR+2*i,   hash[i] & 0xff);
      write_reg( dp, MAR+2*i+1, hash[i] >> 8);
    }
    write_reg( dp, RCR, read_reg( dp, RCR) & ~(RCR_PRMSC|RCR_ALL));
    wip_drvIrqRestore( intr_sav);
  }
}

/*
 * Set PHY configuration
 */
static void drv_set_phy( wip_drvEthData_t *drvp)
{
  u16 val;
  dm9000data * dp;

  dp = drvp->eth_drvdata.drv_data;

  if( drvp->eth_linkcfg == WIP_ETH_LINK_AUTONEG) {
    wip_debug( "[DM9000] set autoneg: %04x\n", drvp->eth_linkadv);
    /* set advertisement register */
    val = ANAR_SELECT;
    if( drvp->eth_linkadv & WIP_ETH_LINKCAP_10BASET) {
      val |= ANAR_10_HDX;
    }
    if( drvp->eth_linkadv & WIP_ETH_LINKCAP_10BASET_FD) {
      val |= ANAR_10_FDX;
    }
    if( drvp->eth_linkadv & WIP_ETH_LINKCAP_100BASETX) {
      val |= ANAR_TX_HDX;
    }
    if( drvp->eth_linkadv & WIP_ETH_LINKCAP_100BASETX_FD) {
      val |= ANAR_TX_FDX;
    }
    write_phy( dp, ANAR, val);
    /* start auto-negociation */
    val = BMCR_ANEG|BMCR_RESTART;
  }
  else {
    /* select a mode */
    wip_debug( "[DM9000] set link config: %d\n", drvp->eth_linkcfg);
    switch( drvp->eth_linkcfg ) {
    case WIP_ETH_LINK_10BASET:
    default:
      val = 0;
      break;
    case WIP_ETH_LINK_10BASET_FD:
      val = BMCR_DPX;
      break;
    case WIP_ETH_LINK_100BASETX:
      val = BMCR_SPEED;
      break;
    case WIP_ETH_LINK_100BASETX_FD:
      val = BMCR_SPEED|BMCR_DPX;
      break;
    }
  }
  /* configure phy */
  write_phy( dp, BMCR, val);

  /* reset link status */
  drvp->eth_link = WIP_ETH_LINK_DOWN;
}


/*
 * Initialize controller
 */
static s32 drv_init( wip_drvEthData_t *drvp)
{
  int i;
  adl_ioDefs_t iocfg[2];
  dm9000data * dp;

  dp = drvp->eth_drvdata.drv_data;

  wip_debug( "[DM9000] init\n");

  /*
   * enable power, reset controller
   */
  iocfg[0] = dp->resetGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;
  if( dp->powerGpioId != (adl_ioDefs_t) -1) {
    iocfg[1] = dp->powerGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_HIGH;
    adl_ioWrite( dp->io_handle, 2, iocfg);
  }
  else {
    adl_ioWrite( dp->io_handle, 1, iocfg);
  }
  wip_delay( 50);
  iocfg[0] = dp->resetGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_HIGH;
  adl_ioWrite( dp->io_handle, 1, iocfg);
  wip_delay( 50);

  /* power up controller */
  write_reg( dp, RSCCR, 0);
  wip_delay( 50);

  /* power up PHY */
  write_reg( dp, GPR, read_reg( dp, GPR) & ~GPR_PHYD);

  /* reset */
  write_reg( dp, NCR, NCR_RST);
  wip_delay( 50);
  write_reg( dp, NCR, 0);

  dp->txbusy = FALSE;

  /* initialize controller */
  write_reg( dp, TCR, 0);
  write_reg( dp, RCR, RCR_DIS_LONG | RCR_DIS_CRC);

  write_reg( dp, SMCR, 0);
  /* clear status bits */
  write_reg( dp, NSR, NSR_WAKEST | NSR_TX1END | NSR_TX2END);
  /* clear interrupts */
  write_reg( dp, ISR, ISR_ROOS | ISR_ROS | ISR_PTS | ISR_PRS);

  /* installs interrupt handler */
  dp->isr_handle = wip_drvIsrSubscribe( (wip_drvData_t *) drvp,
                                        drv_intr,
                                        dp->extIntId,
                                        WIP_DRV_IRQ_TRIGGER_RISING_EDGE);
  if( dp->isr_handle < 0) {
    wip_debug( "[DM9000] irq subscribe failed: %d\n", dp->isr_handle);
    goto failure;
  }

  /* power-on PHY */
//  write_reg( dp, GPR, 0);  /* already done */

  /* set MAC address */
  if( (drvp->eth_addr[0] == 0) && (drvp->eth_addr[1] == 0) &&
      (drvp->eth_addr[2] == 0) && (drvp->eth_addr[3] == 0) &&
      (drvp->eth_addr[4] == 0) && (drvp->eth_addr[5] == 0)) {
    /* no address provided by bearer */
    if( (dp->macAddr[0] == 0) && (dp->macAddr[1] == 0) &&
        (dp->macAddr[2] == 0) && (dp->macAddr[3] == 0) &&
        (dp->macAddr[4] == 0) && (dp->macAddr[5] == 0)) {
      /* try to get address stored in eeprom */
      /* XXX TO DO */
    }
    else {
      /* use the one initialized by driver */
      memcpy( drvp->eth_addr, dp->macAddr, 6);
    }
  }
  wip_debug( "[DM9000] mac address: %02x:%02x:%02x:%02x:%02x:%02x\n",
              drvp->eth_addr[0], drvp->eth_addr[1], drvp->eth_addr[2],
              drvp->eth_addr[3], drvp->eth_addr[4], drvp->eth_addr[5]);
  for( i=0; i<6; i++) {
    write_reg( dp, PAR+i, drvp->eth_addr[i]);
  }

  /* set multicast filter */
  drv_set_filter( drvp);

  /* enable interrupts, start rx */
  write_reg( dp, IMR, IMR_PAR | IMR_PTI | IMR_PRI);
  write_reg( dp, RCR, read_reg( dp, RCR) | RCR_RXEN);

  /* configure PHY */
  drv_set_phy( drvp);

  /* initialization successfull */
  return 0;

failure:
  /* failed to initialize controller */
  /* power down PHY */
  write_reg( dp, GPR, read_reg( dp, GPR) | GPR_PHYD);
  /* power down controller */
  write_reg( dp, SCCR, SCCR_DIS_CLK);
  /* disable power */
  if( dp->powerGpioId != (adl_ioDefs_t) -1) {
    adl_ioDefs_t iocfg[1];
    iocfg[0] = dp->powerGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;
    adl_ioWrite( dp->io_handle, 1, iocfg);
  }
  return -1;
}


/*
 * Shutdown controller
 */
static void drv_shutdown( wip_drvData_t *drvp)
{
  s32 intr_sav;
  dm9000data * dp;

  dp = drvp->drv_data;

  wip_debug( "[DM9000] shutdown\n");

  intr_sav = wip_drvIrqDisable();
  /* stop rx */
  write_reg( dp, RCR, 0);
  /* disable interrupts */
  write_reg( dp, IMR, read_reg( dp, IMR) & ~(IMR_LNKCHGI|IMR_UDRUNI|IMR_ROOI|
                                             IMR_ROI|IMR_PTI|IMR_PRI));
  /* power down PHY */
  write_reg( dp, GPR, read_reg( dp, GPR) | GPR_PHYD);
  wip_drvIrqRestore( intr_sav);

  /* remove interrupt handler */
  wip_drvIsrUnsubscribe( dp->isr_handle);
  dp->isr_handle = -1;

  /* power down controller */
  write_reg( dp, SCCR, SCCR_DIS_CLK);

  /* disable power */
  if( dp->powerGpioId != (adl_ioDefs_t) -1) {
    adl_ioDefs_t iocfg[1];
    iocfg[0] = dp->powerGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;
    adl_ioWrite( dp->io_handle, 1, iocfg);
  }
}


/*
 * Send a packet
 */
static void drv_output( wip_drvData_t *drvp)
{
  s32 intr_sav;
  wip_drvBuf_t *bufp;
  dm9000data * dp;

  dp = drvp->drv_data;

  intr_sav = wip_drvIrqDisable();
  if( dp->txbusy) {
    /* driver busy */
    wip_drvIrqRestore( intr_sav);
    return;
  }

  /* get next buffer to transmit */
  bufp = wip_drvBufDequeue( drvp);
  if( bufp != NULL) {
    dp->txbusy = TRUE;
    /* transfert buffer data to controller */
    write_index( dp, MWCMD);
    write_buf( dp, bufp->buf_datap, (bufp->buf_datalen+1)/2);
    /* write length of buffer and request tx */
    write_reg( dp, TXPLH, bufp->buf_datalen>>8);
    write_reg( dp, TXPLL, bufp->buf_datalen & 0xff);
    write_reg( dp, TCR, TCR_TXREQ);
    /* release buffer */
    wip_drvBufFree( drvp, bufp);
  }

  /* restore interrupts */
  wip_drvIrqRestore( intr_sav);
}


/*
 * Timer: check link status
 */
static void drv_timer( wip_drvEthData_t *drvp)
{
  s32 intr_sav;
  u8 nsr, ncr;
  wip_ethLink_e old_link;
  dm9000data * dp;

  dp = drvp->eth_drvdata.drv_data;

  old_link = drvp->eth_link;

  /* check link status */
  intr_sav = wip_drvIrqDisable();
  nsr = read_reg( dp, NSR);
  if( nsr & NSR_LINKST) {
    ncr = read_reg( dp, NCR);
  }
  wip_drvIrqRestore( intr_sav);
  if( nsr & NSR_LINKST) {
    /* link is up, get speed and duplex mode */
    if( nsr & NSR_SPEED) {
      if( ncr & NCR_FDX)  drvp->eth_link = WIP_ETH_LINK_10BASET_FD;
      else                drvp->eth_link = WIP_ETH_LINK_10BASET;
    }
    else {
      if( ncr & NCR_FDX)  drvp->eth_link = WIP_ETH_LINK_100BASETX_FD;
      else                drvp->eth_link = WIP_ETH_LINK_100BASETX;
    }
  }
  else {
    /* link is down */
    drvp->eth_link = WIP_ETH_LINK_DOWN;
  }

  if( old_link != drvp->eth_link) {
    wip_debug( "[DM9000] link status: %d\n", drvp->eth_link);
  }
}


/*
 * Release all driver resources
 */

static void drv_release( wip_drvData_t *drvp)
{
  dm9000data * dp = drvp->drv_data;

  /* release bus */
  adl_busUnsubscribe( dp->bus_handle);

  /* release io */
  adl_ioUnsubscribe( dp->io_handle);

  /* release memory */
  adl_memRelease( dp);
}


/*
 * Driver entry point
 */
static s32 drv_ctl( wip_drvData_t *drvp, wip_drvCtl_e cmd, void *arg)
{
  switch( cmd) {

  case WIP_DRVCTL_UP:
    return drv_init( (wip_drvEthData_t *) drvp);

  case WIP_DRVCTL_DOWN:
    drv_shutdown( drvp);
    break;

  case WIP_DRVCTL_OUTPUT:
    drv_output( drvp);
    break;

  case WIP_DRVCTL_TIMER:
    drv_timer( (wip_drvEthData_t *) drvp);
    break;

  case WIP_DRVCTL_ETH_SETFILTER:
    drv_set_filter( (wip_drvEthData_t *) drvp);
    break;

  case WIP_DRVCTL_ETH_SETPHY:
    drv_set_phy( (wip_drvEthData_t *) drvp);
    break;

  case WIP_DRVCTL_UNSUBSCRIBE:
    drv_release( drvp);
    break;

  default:
    return -1;  /* XXX */
  }

  return 0;
}

/*
 * driver registration
 */
s32 wip_drvSubscribe_DM9000(
  const ascii *         name,
  u8                    csId,
  u32                   addressPin,
  u32                   extIntId,
  adl_ioDefs_t          resetGpioId,
  adl_ioDefs_t          powerGpioId,
  const wip_ethAddr_t * macAddr)
{
  adl_busParallelSettings_t cfg;
  u32 bus_ptr;
  adl_ioDefs_t iocfg[2];
  dm9000data * dp;
  s32 hnd;

  wip_debug( "[DM9000] register: CS=%d AddrPin=%08x EXTINT=%d\n",
             csId, addressPin, extIntId);
  wip_debug( "[DM9000]           RST=%08x PWR=%08x\n",
              resetGpioId, powerGpioId);

  /* allocate driver data */
  dp = adl_memGet( sizeof(dm9000data));
  if( dp == NULL) {
    wip_debug( "[DM9000] cannot allocate data\n");
    return -1; /* XXX */
  }
  dp->isr_handle  = -1;
  dp->csId        = csId;
  dp->addressPin  = addressPin;
  dp->extIntId    = extIntId;
  dp->resetGpioId = resetGpioId & ADL_IO_LABEL_MSK;
  if( powerGpioId != (adl_ioDefs_t) -1) {
    dp->powerGpioId = powerGpioId & ADL_IO_LABEL_MSK;
  }
  else {
    dp->powerGpioId = (adl_ioDefs_t) -1;
  }
  if( macAddr != NULL) {
    memcpy( dp->macAddr, macAddr, sizeof(wip_ethAddr_t));
  }
  else {
    memset( dp->macAddr, 0, sizeof(wip_ethAddr_t));
  }

  /*
   * Initialize parallel bus
   */
  memset( &cfg, 0, sizeof(cfg));
  cfg.Width                         = ADL_BUS_PARALLEL_WIDTH_16_BITS;
  cfg.Mode                          = ADL_BUS_PARALLEL_MODE_ASYNC_INTEL;
  cfg.ReadCfg.AccessTime            = 1;
  cfg.ReadCfg.SetupTime             = 1;
  cfg.ReadCfg.HoldTime              = 1;
  cfg.ReadCfg.TurnaroundTime        = 2;
  cfg.ReadCfg.OpToOpTurnaroundTime  = 0;
  cfg.WriteCfg.AccessTime           = 1;
  cfg.WriteCfg.SetupTime            = 1;
  cfg.WriteCfg.HoldTime             = 1;
  cfg.WriteCfg.TurnaroundTime       = 2;
  cfg.WriteCfg.OpToOpTurnaroundTime = 0;
  cfg.Cs.Type                       = ADL_BUS_PARA_CS_TYPE_CS;
  cfg.Cs.Id                         = dp->csId;
  cfg.AdressPin                     = dp->addressPin;
  dp->bus_handle = adl_busSubscribe( ADL_BUS_ID_PARALLEL, 1, &cfg);
  if( dp->bus_handle < 0) {
    wip_debug( "[DM9000] bus subscribe failed (%d)\n", dp->bus_handle);
    adl_memRelease( dp);
    return -1;
  }

  dp->indexAddr = 0;
  dp->dataAddr  = dp->addressPin;

  hnd = adl_busIOCtl( dp->bus_handle, ADL_BUS_CMD_PARA_GET_ADDRESS, &bus_ptr);
  dp->indexPtr = (volatile u16 *) (bus_ptr);
  dp->dataPtr  = (volatile u16 *) (bus_ptr + dp->addressPin);

  /*
   * Initialize GPIOs: enable power, reset controller
   */
  iocfg[0] = dp->resetGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;
  if( dp->powerGpioId != (adl_ioDefs_t) -1) {
    iocfg[1] = dp->powerGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_HIGH;
    dp->io_handle = adl_ioSubscribe( 2, iocfg, 0, 0, 0);
  }
  else {
    dp->io_handle = adl_ioSubscribe( 1, iocfg, 0, 0, 0);
  }
  if( dp->io_handle < 0) {
    wip_debug( "[DM9000] gpio subscribe failed (%d0)\n", dp->io_handle);
    adl_busUnsubscribe( dp->bus_handle);
    adl_memRelease( dp);
    return -1;
  }
  /* deactive reset */
  wip_delay( 50);
  iocfg[0] = dp->resetGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_HIGH;
  adl_ioWrite( dp->io_handle, 1, iocfg);
  wip_delay( 50);

  /* check that DM9000 board is present */
  {
    u16 vid, pid;

    vid = read_reg( dp, VIDL) | (read_reg( dp, VIDH) << 8);
    pid = read_reg( dp, PIDL) | (read_reg( dp, PIDH) << 8);

    if( (vid != VID_CHECK) || (pid != PID_CHECK)) {
      wip_debug( "[DM9000] invalid chip id: %04X-%04X\n", vid, pid);
      adl_busUnsubscribe( dp->bus_handle);
      adl_ioUnsubscribe( dp->io_handle);
      adl_memRelease( dp);
      return -1;
    }
  }

  if( dp->powerGpioId == (adl_ioDefs_t) -1) {
    /* no power control line, */
    /* power down PHY */
    write_reg( dp, GPR, read_reg( dp, GPR) | GPR_PHYD);
    /* power down controller */
    write_reg( dp, SCCR, SCCR_DIS_CLK);
  }
  else {
    /* power off */
    iocfg[0] = dp->powerGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;
    adl_ioWrite( dp->io_handle, 1, iocfg);
  }

  /* register driver */
  hnd = wip_drvSubscribe( name, WIP_BEARER_ETHER, drv_ctl, dp);
  if( hnd < 0) {
    wip_debug( "[DM9000] driver subscribe failed (%d)\n");
    adl_busUnsubscribe( dp->bus_handle);
    adl_ioUnsubscribe( dp->io_handle);
    adl_memRelease( dp);
  }
  return hnd;
}
