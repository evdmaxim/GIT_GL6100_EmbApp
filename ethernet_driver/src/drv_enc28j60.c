/********************************************************************************************/
/* drv_enc28j60.c  -  Copyright Wavecom S.A. (c) 2007                                       */
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
/* File    :   drv_enc28j60.c                                               */
/*                                                                          */
/* Scope   :   Microchip ENC28J60 Ethernet driver                           */
/*                                                                          */

/*
 * $LogWavecom:$
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  03.12.07 | RFN    | 1.0            | Initial revision.                  
 * ----------+--------+----------------+-------------------------------------
*/

#include "wip_drv.h"
#include "adl_gpio.h"
#include "adl_bus.h"
#include "adl_memory.h"

#include "adl_ctx.h"
#include "adl_msg.h"
#include "adl_TimerHandler.h"

#include "wip_drv_eth.h"
#include "wip_drv_enc28j60.h"
#include "enc28j60_regs.h"
#include "adl_global.h"
#include "extstorage_iic.h"

#include <string.h>


#define USE_LIMIT_NB_INTR
//#define USE_LIMIT_RCV_DATA

// Message identifier ( 0x454E4332 == "ENC2" )
#define DRV_ENC28J60_MESSAGE_ID   0x454E4332
// Message filter
static /*const*/ adl_msgFilter_t ENC28J60_MsgFilter = { 0xFFFFFFFF, DRV_ENC28J60_MESSAGE_ID, ADL_MSG_ID_COMP_EQUAL, ADL_CTX_ALL };
// Message handle
static s32 ENC28J60_MsgHandle;
// Timer Handle
static adl_tmr_t *Timer_handle;

/* constants */
#ifdef USE_LIMIT_NB_INTR
#define MAX_NB_INTR           200
#elif defined USE_LIMIT_RCV_DATA
#define BANDWITH_LIMIT        200000 //in bytes 
#endif
#define CTX_FREEZE_DURATION   12     //in ticks of 18.5ms   

/* Global variables */

/* bandwith limitation */
#ifdef USE_LIMIT_NB_INTR
static int  IntrCount = 0;
#elif defined USE_LIMIT_RCV_DATA
static bool BdwLimitReached = FALSE;
static int  SumRxPktLen = 0;
#endif

static int istatus;
static int  PktCount  = 0;


/* bus access mode */
static adl_busAccess_t BusAccessMode;

/* buffer where to copy the read items */
static u8 ReadBuffer[10];
/* data buffer to write on the bus */
static u8 WriteBuffer[10];

static wip_drvBuf_t *ReadBufp;
static wip_drvBuf_t *WriteBufp;


s32 eth_handle;


static s8 LinkStatus;
static wip_ethAddr_t MacAddr;
static sItfCont_t DrvPubItf; 
static sItfCont_t * psItfCont = & DrvPubItf;
s32 ( *eth_open[ETH_DRV_LAST]) ( psEthSettings_t Settings ) = { drv_open_iesm, NULL }; 


/* driver data */
typedef struct {
  s32           bus_handle;
  volatile bool txbusy;
  int           txtimeo;
  s32           io_handle;
  s32           isr_handle;
  u32           spiId;
  adl_ioDefs_t  csGpioId;
  u32           extIntId;
  adl_ioDefs_t  resetGpioId;
  adl_ioDefs_t  powerGpioId;
  wip_ethAddr_t macAddr;
} enc28data;

static enc28data * drv_datap;

/* using adl bus functions */

#define OP_RD     (0<<5)
#define OP_WR     (2<<5)
#define OP_SET    (4<<5)
#define OP_CLR    (5<<5)

#define OP_RDBUF  0x3a
#define OP_WRBUF  0x7a
#define OP_RST    0xff


static u8 read_reg( enc28data *dp, u8 reg)
{
  BusAccessMode.Address = 0;
  BusAccessMode.Opcode  = (OP_RD|reg)<<24;
  adl_busRead( dp->bus_handle, &BusAccessMode, 1, (void *) ReadBuffer);

  return ReadBuffer[0];  
}

static u8 read_mac_reg( enc28data *dp, u8 reg)
{
  BusAccessMode.Address = 0;
  BusAccessMode.Opcode  = (OP_RD|reg)<<24;
  adl_busRead( dp->bus_handle, &BusAccessMode, 2, (void *) ReadBuffer);

  return ReadBuffer[1];  /* skip dummy byte */
}

static void write_reg( enc28data *dp, u8 reg, u8 bval)
{
  WriteBuffer[0] = bval;
  BusAccessMode.Address = 0;
  BusAccessMode.Opcode  = (OP_WR|reg)<<24;
  adl_busWrite( dp->bus_handle, &BusAccessMode, 1, (void *) WriteBuffer);
}

static void set_reg( enc28data *dp, u8 reg, u8 bval)
{
  WriteBuffer[0] = bval;
  BusAccessMode.Address = 0;
  BusAccessMode.Opcode  = (OP_SET|reg)<<24;
  adl_busWrite( dp->bus_handle, &BusAccessMode, 1, (void *) WriteBuffer);
}

static void clr_reg( enc28data *dp, u8 reg, u8 bval)
{
  WriteBuffer[0] = bval;
  BusAccessMode.Address = 0;
  BusAccessMode.Opcode  = (OP_CLR|reg)<<24;
  adl_busWrite( dp->bus_handle, &BusAccessMode, 1, (void *) WriteBuffer);
}

static void read_buf( enc28data *dp, void *buf, int len)
{
  BusAccessMode.Address = 0;
  BusAccessMode.Opcode  = OP_RDBUF<<24;
  adl_busRead( dp->bus_handle, &BusAccessMode, len, buf);
}

static void write_buf( enc28data *dp, const void *buf, int len)
{
  BusAccessMode.Address = 0;
  BusAccessMode.Opcode  = OP_WRBUF<<24;
  adl_busWrite( dp->bus_handle, &BusAccessMode, len, (void *) buf);
}

static void set_bank( enc28data *dp, int n)
{
  BusAccessMode.Address = 0;
  
  switch( n) {
  case 0:
    BusAccessMode.Opcode = (OP_CLR|ECON1)<<24;    /* clear bits 0-1 */    
    WriteBuffer[0] = 3;  //val[0] = 3;
    adl_busWrite( dp->bus_handle, &BusAccessMode, 1, (void *) WriteBuffer);
    break;
    
  case 1:
    BusAccessMode.Opcode = (OP_SET|ECON1)<<24;    /* set bit 0 */
    WriteBuffer[0] = 1;  //val[0] = 1;
    adl_busWrite( dp->bus_handle, &BusAccessMode, 1, (void *) WriteBuffer);
    BusAccessMode.Opcode = (OP_CLR|ECON1)<<24;    /* clear bit 1 */
    WriteBuffer[0] = 2;  //val[0] = 2;
    adl_busWrite( dp->bus_handle, &BusAccessMode, 1, (void *) WriteBuffer);
    break;
    
  case 2:
    BusAccessMode.Opcode = (OP_CLR|ECON1)<<24;    /* clear bit 0 */
    WriteBuffer[0] = 1; //val[0] = 1;
    adl_busWrite( dp->bus_handle, &BusAccessMode, 1, (void *) WriteBuffer);
    BusAccessMode.Opcode = (OP_SET|ECON1)<<24;    /* set bit 1 */
    WriteBuffer[0] = 2; //val[0] = 2;
    adl_busWrite( dp->bus_handle, &BusAccessMode, 1, (void *) WriteBuffer);
    break;
    
  case 3:
    BusAccessMode.Opcode = (OP_SET|ECON1)<<24;    /* set bits 0-1 */
    WriteBuffer[0] = 3; //val[0] = 3;
    adl_busWrite( dp->bus_handle, &BusAccessMode, 1, (void *) WriteBuffer);
    break;
  }
}


/****************************************************************************
 * ENC28J60 DRIVER
 ****************************************************************************/

/* internal memory */
#define TXSTART    (8192-(1+1514+7))  /* one frame */
#define RXSTART       0   /* must be zero (ERRATA) */
#define RXSTOP     (TXSTART-1)   /* must be odd (ERRATA) */


/*
 * Internal PHY access
 *
 * note: interrupt must be masked when calling these functions
 */

static u16 read_phy( enc28data *dp, u8 reg)
{
  u16 data;
  int i;

  set_bank( dp, 2);
  /* start read operation */
  write_reg( dp, MIREGADR, reg);
  write_reg( dp, MICMD, MICMD_MIIRD);
  set_bank( dp, 3);
  /* wait until operation is complete */
  for( i=0; i<100; i++) {
    u8 status;
    status = read_mac_reg( dp, MISTAT);
    if( !(status & MISTAT_BUSY)) break;
  }
  set_bank( dp, 2);
  write_reg( dp, MICMD, 0);
  /* get result */
  data = read_mac_reg( dp, MIRDL) |
        (read_mac_reg( dp, MIRDH) << 8);

  return data;
}

static void write_phy( enc28data *dp, u8 reg, u16 data)
{
  int i;

  set_bank( dp, 2);
  /* start write operation */
  write_reg( dp, MIREGADR, reg);
  write_reg( dp, MIWRL, data);
  write_reg( dp, MIWRH, data>>8);
  set_bank( dp, 3);
  /* wait until operation is complete */
  for( i=0; i<100; i++) {
    u8 status;
    status = read_mac_reg( dp, MISTAT);
    if( !(status & MISTAT_BUSY)) break;
  }
}

#ifdef USE_LIMIT_RCV_DATA
void ResetSumRcvData( u8 ID, void *Context )
{
  //wip_debug( "[ENC28J60] ID= %d, Reset Sum (= %d) of received packet data over last 500ms\n", ID, SumRxPktLen);
  
  SumRxPktLen = 0;  
  
  Timer_handle = adl_tmrSubscribe ( FALSE, 5, ADL_TMR_TYPE_100MS, ResetSumRcvData );  
}

static void drv_MessageHandler ( u32 MsgIdentifier, adl_ctxID_e Source, u32 Length, void * Data )
{
  // Incoming message, sent from High level interruption handler
  //wip_debug( "[ENC28J60] drv_MessageHandler: (%08X, %d, %d, %d)", MsgIdentifier, Source, Length, * ( (u32*) Data ) );
  
  //wip_debug( "[ENC28J60] start timer 500ms\n");

  //stop any currently running timer
  adl_tmrUnSubscribe( Timer_handle, ResetSumRcvData, ADL_TMR_TYPE_100MS);
  
  
  //before starting the new one      
  Timer_handle = adl_tmrSubscribe ( FALSE, 5, ADL_TMR_TYPE_100MS, ResetSumRcvData );
}
#endif 


/*
 * Interrupt handler
 */
static void drv_intr( wip_drvData_t *drvp)
{
  int nbLoop=0;  
#ifdef USE_LIMIT_RCV_DATA  
  int sum=0;
#endif      
  
#ifdef USE_LIMIT_NB_INTR  
  IntrCount++;
#endif  

  drv_datap = drvp->drv_data;

  /* interrupt are triggered by front, ensure front will be generated
   * at end of handler if interrupts are still active */
  clr_reg( drv_datap, EIE, EIE_INTIE); 
        
      
 while( 1) {

  //prevent from infinite loop
  if( ++nbLoop > 50 ) 
  {
    wip_debug( "[ENC28J60] WARNING nbLoop= %d\n", nbLoop);
    break;
  }

  istatus = read_reg( drv_datap, EIR);
  if( (istatus & 0xcb) == 0) break;
  
  
  /* ERRATA 4 : set bank 1 before reading EPKTCNT */
  set_bank(drv_datap, 1); 
  PktCount = read_reg( drv_datap, EPKTCNT);    
  if((PktCount==0) && (istatus & EIR_PKTIF))
  {
    wip_debug( "[ENC28J60] nbPacket= %d while PKTIF is set\n", PktCount);  
  }
  /* return to bank 0 */
  set_bank(drv_datap, 0); 
      

  /*
   * get RX data
   *
   */
  if( PktCount > 0) {
  //if( istatus & EIR_PKTIF) {

    //u8 buf[6];
    int pktlen, pktstat;     

    /* read next pkt ptr and status vector */
    read_buf( drv_datap, ReadBuffer, 6);
    //read_buf( dp, buf, 6);

    /* length of packet */
    pktlen = (int) ReadBuffer[2] | (ReadBuffer[3] << 8);
    /* status */
    pktstat = (int) ReadBuffer[4] | (ReadBuffer[5] << 8);

    //if(BdwLimitReached == FALSE) {
          
      if( (pktstat & 0x80) && (pktlen >= 64) && (pktlen <= 1518)) {
        /* RX OK */
        //wip_drvBuf_t *bufp;
              
        pktlen -= 4;  /* skip crc */
        ReadBufp = wip_drvBufAlloc( drvp, pktlen);
        if( ReadBufp != NULL) {
          /* read packet data */
          read_buf( drv_datap, ReadBufp->buf_datap, ReadBufp->buf_datalen);
          wip_drvBufEnqueue( drvp, ReadBufp);
        }        
      }
    //}

    /* free buffer space, ensure odd value (ERRATA) */
    pktlen = ((int) ReadBuffer[0] | (ReadBuffer[1] << 8)) - 1;
    if( (pktlen < 0) || (pktlen > RXSTOP)) {
      pktlen = RXSTOP;
    }
    write_reg( drv_datap, ERXRDPTL, (u8) pktlen);
    write_reg( drv_datap, ERXRDPTH, (u8) (pktlen >> 8));
    set_reg( drv_datap, ECON2, ECON2_PKTDEC);

    /* prepare read ptr for next packet */
    write_reg( drv_datap, ERDPTL, ReadBuffer[0]);
    write_reg( drv_datap, ERDPTH, ReadBuffer[1]);
    

#ifdef USE_LIMIT_RCV_DATA
    /* increment total sum length of pkt received */
    sum += pktlen;
    SumRxPktLen += pktlen;
    
    /* Is Bandwith limit reached ? */
    if( ( (SumRxPktLen >= BANDWITH_LIMIT) || (sum >= BANDWITH_LIMIT) )
       && (BdwLimitReached == FALSE))
    {
      /* yes */      
      BdwLimitReached = TRUE; 
            
      /* disable packet reception */
      //clr_reg( dp, ECON1, ECON1_RXEN);
      
      wip_debug( "[ENC28J60] BW limit reached: sum= %d, total= %d \n", sum, SumRxPktLen ); 
    }
#endif //USE_LIMIT_RCV_DATA
    
    
    /* update received packet counter */
    set_bank(drv_datap, 1); 
    PktCount = read_reg( drv_datap, EPKTCNT);
    set_bank(drv_datap, 0);        
    
    
    continue;
  } /* flush all rx packets */
    

  /* RX error */
  if( istatus & EIR_RXERIF) {
    clr_reg( drv_datap, EIR, EIR_RXERIF);
    wip_debug( "[ENC28J60] RX error\n"); 
  }

  if( istatus & (EIR_TXERIF|EIR_TXIF)) {
    /*
     * TX complete
     */
    //wip_drvBuf_t *bufp;

    /* just in case... */
    clr_reg( drv_datap, ECON1, ECON1_TXRTS);

    /* check for error */
    if( istatus & EIR_TXERIF) {
      u8 stat;
      stat = read_reg( drv_datap, ESTAT);
      clr_reg( drv_datap, ESTAT, ESTAT_LATECOL|ESTAT_TXABRT);
      wip_debug( "[ENC28J60] tx error (ESTAT=%02x)\n", stat);
    }

    /* clear interrupt */
    set_reg( drv_datap, ECON1, ECON1_TXRST);
    clr_reg( drv_datap, ECON1, ECON1_TXRST);
    clr_reg( drv_datap, EIR, EIR_TXERIF|EIR_TXIF);

    /* get next buffer to transmit */
    WriteBufp = wip_drvBufDequeue( drvp);
    if( WriteBufp != NULL) {
      u16 etxnd;

      drv_datap->txbusy = TRUE;
      drv_datap->txtimeo = 2;

      /* set write pointer at beginning of tx area */
      /* first byte is control */
      write_reg( drv_datap, EWRPTL, (u8)(TXSTART+1));
      write_reg( drv_datap, EWRPTH, (TXSTART+1)>>8);

      /* set end of packet */
      etxnd = TXSTART + 1 + WriteBufp->buf_datalen - 1;
      write_reg( drv_datap, ETXNDL, etxnd);
      write_reg( drv_datap, ETXNDH, etxnd>>8);

      /* write ethernet frame */
      write_buf( drv_datap, WriteBufp->buf_datap, WriteBufp->buf_datalen);

      /* start tx */
      set_reg( drv_datap, ECON1, ECON1_TXRTS);

      /* release buffer */
      wip_drvBufFree( drvp, WriteBufp);
    }
    else {
      drv_datap->txbusy = FALSE;
      drv_datap->txtimeo = 0;
    }
  }
 
 } /* end while */ 
 
   
   
  /* re-enable interrupts if bandwith limit not reached */
#ifdef USE_LIMIT_NB_INTR
  if( IntrCount > MAX_NB_INTR) { 
#elif defined USE_LIMIT_RCV_DATA
  if( BdwLimitReached == TRUE) {    
#endif 
    
    /* disable packet reception */
    clr_reg( drv_datap, ECON1, ECON1_RXEN);   
        
    /* put the current execution to sleep for a duration of 200 ms */
    adl_ctxSleep(CTX_FREEZE_DURATION); //duration in ticks of 18.5ms   
        
    /* reset variables */
#ifdef USE_LIMIT_NB_INTR    
    IntrCount       = 0;
#elif defined USE_LIMIT_RCV_DATA     
    BdwLimitReached = FALSE;
    SumRxPktLen     = 0;
#endif    
        
    /* enable packet reception and interrupts */  
    set_reg( drv_datap, ECON1, ECON1_RXEN);         

#ifdef USE_LIMIT_RCV_DATA
    //adl_msgSend ( 0, DRV_ENC28J60_MESSAGE_ID, 0, NULL ); // send message to ADL main task (0)  
#endif
  }
  
  /* if interrupts are still active this will trigger handler again */
  set_reg( drv_datap, EIE, EIE_INTIE);  
  
}


/*
 * Send a packet
 */
static void drv_output( wip_drvData_t *drvp)
{
  s32 intr_sav;
  wip_drvBuf_t *bufp;
  enc28data * dp;

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
    u16 etxnd;

    /* reset tx (just in case...) */
    set_reg( dp, ECON1, ECON1_TXRST);
    clr_reg( dp, ECON1, ECON1_TXRST);
    clr_reg( dp, EIR, EIR_TXERIF|EIR_TXIF);

    /* transfert buffer data to controller */
    /* arm tx watchdog */
    dp->txbusy = TRUE;
    dp->txtimeo = 2;

    /* set write pointer at beginning of tx area */
    /* first byte is control */
    write_reg( dp, EWRPTL, (u8)(TXSTART+1));
    write_reg( dp, EWRPTH, (TXSTART+1)>>8);

    /* set end of packet (point to last byte) */
    etxnd = TXSTART + 1 + bufp->buf_datalen - 1;
    write_reg( dp, ETXNDL, etxnd);
    write_reg( dp, ETXNDH, etxnd>>8);

    /* write ethernet frame */
    write_buf( dp, bufp->buf_datap, bufp->buf_datalen);

    /* start tx */
    set_reg( dp, ECON1, ECON1_TXRTS);

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
  wip_ethLink_e old_link;
  u16 status;
  enc28data * dp;

  dp = drvp->eth_drvdata.drv_data;

  old_link = drvp->eth_link;

  intr_sav = wip_drvIrqDisable();

  /* get link status */
  status = read_phy( dp, PHSTAT2);
  set_bank( dp, 0);

  /* tx watchdog */
  if( (dp->txtimeo > 0) && (--dp->txtimeo == 0)) {
    /* tx is stall (!) */
    wip_debug( "[ENC28J60] tx watchdog\n");
    /* reset tx */
    clr_reg( dp, ECON1, ECON1_TXRTS);
    set_reg( dp, ECON1, ECON1_TXRST);
    clr_reg( dp, ECON1, ECON1_TXRST);
    /* clear interrupt */
    clr_reg( dp, EIR, EIR_TXERIF|EIR_TXIF);
    dp->txbusy = FALSE;
    dp->txtimeo = 0;
    wip_drvIrqRestore( intr_sav);
    /* try to send next packet */
    drv_output( (wip_drvData_t *) drvp);
  }
  else {
    wip_drvIrqRestore( intr_sav);
  }

  if( status & PHSTAT2_LSTAT) {
    /* link is up */
    if( status & PHSTAT2_DPXSTAT) {
      /* full-duplex operation */
      drvp->eth_link = WIP_ETH_LINK_10BASET_FD;

      LinkStatus = 0;
    }
    else {
      /* half-duplex operation */
      drvp->eth_link = WIP_ETH_LINK_10BASET;

      LinkStatus = 0;
    }
  }
  else {
    /* link is down */
    drvp->eth_link = WIP_ETH_LINK_DOWN;

    LinkStatus = 1;
  }

  if( old_link != drvp->eth_link) {
    wip_debug( "[ENC28J60] link status: %d\n", drvp->eth_link);
  }
}


/*
 * Set input and multicast filter
 */
static void drv_set_filter( wip_drvEthData_t *drvp)
{
  s32 intr_sav;
  enc28data * dp;

  dp = drvp->eth_drvdata.drv_data;

  intr_sav = wip_drvIrqDisable();

  set_bank( dp, 1);

  if( drvp->eth_promisc) {
    /* promiscuous mode */
    write_reg( dp, ERXFCON, ERXFCON_CRCEN);
  }
  else if( drvp->eth_allmulti) {
    /* all multicast mode */
    write_reg( dp, ERXFCON, ERXFCON_UCEN|ERXFCON_MCEN|ERXFCON_BCEN|ERXFCON_CRCEN);
  }
  else {
    int i;
    u8 ht[8];

    /* use hash table */
    write_reg( dp, ERXFCON, ERXFCON_UCEN|ERXFCON_HTEN|ERXFCON_BCEN|ERXFCON_CRCEN);

    /* set multicast filter */
    ht[0] = ht[1] = ht[2] = ht[3] = ht[4] = ht[5] = ht[6] = ht[7] = 0;
    for( i=0; i<drvp->eth_mcast_nb; i++) {
      int val;
      /* compute CRC of multicast address,
         keep bits 28-23 as hash index */
      val = (wip_ethCRC32( drvp->eth_mcast[i], 6) >> 23) & 0x3f;
      ht[val>>3] |= 1 << (val & 0x7);
    }
    for( i=0; i<8; i++) {
      write_reg( dp, EHT0 + i, ht[i]);
    }
  }

  set_bank( dp, 0);

  wip_drvIrqRestore( intr_sav);
}

/*
 * Set PHY configuration
 *
 * only 10baseT is supported, no auto-negociation
 */
static void drv_set_phy( wip_drvEthData_t *drvp)
{
  s32 intr_sav;
  u16 val;
  enc28data * dp;

  dp = drvp->eth_drvdata.drv_data;

  if( drvp->eth_linkcfg == WIP_ETH_LINK_10BASET_FD) {
    /* full-duplex */
    wip_debug( "[ENC28J60] set full-duplex\n");
    intr_sav = wip_drvIrqDisable();
    /* configure MAC for full-duplex */
    set_bank( dp, 2);
    val = read_mac_reg( dp, MACON3);
    val |= MACON3_FULDPX;
    write_reg( dp, MACON3, val);
    write_reg( dp, MABBIPG, 0x15);
    val = read_phy( dp, PHCON1);
    val |= PHCON1_PDPXMD;
    write_phy( dp, PHCON1, val);
    set_bank( dp, 0);
    wip_drvIrqRestore( intr_sav);
  }
  else {
    /* default is half-duplex */
    wip_debug( "[ENC28J60] set half-duplex\n");
    intr_sav = wip_drvIrqDisable();
    /* configure MAC for half-duplex */
    set_bank( dp, 2);
    val = read_mac_reg( dp, MACON3);
    val &= ~MACON3_FULDPX;
    write_reg( dp, MACON3, val);
    write_reg( dp, MABBIPG, 0x12);
    val = read_phy( dp, PHCON1);
    val &= ~PHCON1_PDPXMD;
    write_phy( dp, PHCON1, val);
    set_bank( dp, 0);
    wip_drvIrqRestore( intr_sav);
  }

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
  enc28data * dp;

  dp = drvp->eth_drvdata.drv_data;

  wip_debug( "[ENC28J60] init\n");

  dp->txbusy  = FALSE;
  dp->txtimeo = 0;

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
  /* clear power save flags */
  write_reg( dp, ECON2, ECON2_AUTOINC);
  wip_delay( 50);
  i = read_reg( dp, ESTAT);
  if( !(i & ESTAT_CLKRDY)) {
    wip_debug( "[ENC28J60] cannot get CLKRDY signal\n");
    goto failure;
  }

  set_bank( dp, 0);
  /* prepare rx circular buffer */
  /* start of buffer */
  write_reg( dp, ERXSTL, (u8)RXSTART);
  write_reg( dp, ERXSTH, RXSTART>>8);
  /* current buffer limit */
  write_reg( dp, ERXRDPTL, (u8)RXSTOP);
  write_reg( dp, ERXRDPTH, RXSTOP>>8);
  /* end of buffer */
  write_reg( dp, ERXNDL, (u8)RXSTOP);
  write_reg( dp, ERXNDH, RXSTOP>>8);
  /* point to first buffer */
  write_reg( dp, ERDPTL, 0);
  write_reg( dp, ERDPTH, 0);

  /* prepare tx buffer */
  /* set start of buffer */
  write_reg( dp, ETXSTL, (u8)TXSTART);
  write_reg( dp, ETXSTH, TXSTART>>8);
  /* write control byte */
  write_reg( dp, EWRPTL, (u8)TXSTART);
  write_reg( dp, EWRPTH, TXSTART>>8);
  {
    u8 ctrl;
    ctrl = 0;
    write_buf( dp, &ctrl, 1);
  }

  /* configure MAC */
  set_bank( dp, 2);

  /* Enable the receive portion of the MAC */
  write_reg( dp, MACON1, MACON1_TXPAUS|MACON1_RXPAUS|MACON1_MARXEN);

  /* Pad packets to 60 bytes, add CRC, and check Type/Length field */
  write_reg( dp, MACON3, MACON3_PADCFG_PAD60|MACON3_TXCRCEN|MACON3_FRMLNEN);

  /* Allow infinite deferals if the medium is continuously busy
     (do not time out a transmission if the half duplex medium is
     completely saturated with other people's data) */
  write_reg( dp, MACON4, MACON4_DEFER);

  /* Late collisions occur beyond 63+8 bytes (8 bytes for preamble/start of frame delimiter)
     55 is all that is needed for IEEE 802.3, but ENC28J60 B5 errata for improper link pulse
     collisions will occur less often with a larger number. */
  write_reg( dp, MACLCON2, 63);

  /* Set non-back-to-back inter-packet gap to 9.6us.  The back-to-back
     inter-packet gap (MABBIPG) is set by drv_set_phy() which is called
     later. */
  write_reg( dp, MAIPGL, 0x12);
  write_reg( dp, MAIPGH, 0x0C);

  /* Set the maximum packet size which the controller will accept */
  write_reg( dp, MAMXFLL, (u8) 1518);
  write_reg( dp, MAMXFLH, 1518>>8);

  /* set MAC address */
  if( (drvp->eth_addr[0] == 0) && (drvp->eth_addr[1] == 0) &&
      (drvp->eth_addr[2] == 0) && (drvp->eth_addr[3] == 0) &&
      (drvp->eth_addr[4] == 0) && (drvp->eth_addr[5] == 0)) {
    /* no address provided by bearer, use the one initialized by driver */
    memcpy( drvp->eth_addr, dp->macAddr, 6);
  }
  wip_debug( "[ENC28J60] mac address: %02x:%02x:%02x:%02x:%02x:%02x\n",
             drvp->eth_addr[0], drvp->eth_addr[1], drvp->eth_addr[2],
             drvp->eth_addr[3], drvp->eth_addr[4], drvp->eth_addr[5]);
  set_bank( dp, 3);
  write_reg( dp, MAADR1, drvp->eth_addr[0]);
  write_reg( dp, MAADR2, drvp->eth_addr[1]);
  write_reg( dp, MAADR3, drvp->eth_addr[2]);
  write_reg( dp, MAADR4, drvp->eth_addr[3]);
  write_reg( dp, MAADR5, drvp->eth_addr[4]);
  write_reg( dp, MAADR6, drvp->eth_addr[5]);

  /* Disable CLKOUT */
  write_reg( dp, ECOCON, 0x00);

  /* installs interrupt handler */
  dp->isr_handle = wip_drvIsrSubscribe( (wip_drvData_t *) drvp,
                                        drv_intr,
                                        dp->extIntId,
                                        WIP_DRV_IRQ_TRIGGER_FALLING_EDGE);
  if( dp->isr_handle < 0) {
    wip_debug( "[ENC28J60] irq subscribe failed: %d\n", dp->isr_handle);
    goto failure;
  }

  /* set multicast filter */
  drv_set_filter( drvp);

  /* disable loopback in half-duplex mode */
  write_phy( dp, PHCON2, PHCON2_HDLDIS);

  /* configure PHY */
  drv_set_phy( drvp);


  /* from here, keep bank 0 active */
  set_bank( dp, 0);

  /* enable interrupts, start rx */
  write_reg( dp, EIE, EIE_INTIE|EIE_PKTIE|EIE_TXIE|EIE_TXERIE|EIE_RXERIE);
  set_reg( dp, ECON1, ECON1_RXEN);

  /* initialization successfull */
  return 0;

failure:
  /* failed to initialize controller */
  /* enter sleep mode */
  write_reg( dp, ECON2, ECON2_PWRSV | ECON2_VRPS);
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
static void drv_shutdown( wip_drvEthData_t *drvp)
{
  s32 intr_sav;
  u8 status;
  enc28data * dp;
  int i;

  dp = drvp->eth_drvdata.drv_data;

  wip_debug( "[ENC28J60] shutdown\n");

  intr_sav = wip_drvIrqDisable();
  /* stop rx */
  write_reg( dp, ECON1, 0); /* clear ECON1.RXEN */
  /* disable interrupts */
  write_reg( dp, EIE, 0);
  wip_drvIrqRestore( intr_sav);

  /* remove interrupt handler */
  wip_drvIsrUnsubscribe( dp->isr_handle);
  dp->isr_handle = -1;

  /* ensure end of rx */
  for( i=0; i<10; i++) {
    status = read_reg( dp, ESTAT);
    if( !(status & ESTAT_RXBUSY)) break;
  }
  /* ensure end of tx */
  for( i=0; i<10; i++) {
    status = read_reg( dp, ECON1);
    if( !(status & ECON1_TXRTS)) break;
  }

  /* enter sleep mode */
  write_reg( dp, ECON2, ECON2_PWRSV | ECON2_VRPS);

  /* disable power */
  if( dp->powerGpioId != (adl_ioDefs_t) -1) {
    adl_ioDefs_t iocfg[1];
    iocfg[0] = dp->powerGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;
    adl_ioWrite( dp->io_handle, 1, iocfg);
  }
}

/*
 * Release all driver resources
 */

static void drv_release( wip_drvData_t *drvp)
{
  enc28data * dp = drvp->drv_data;

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
    drv_shutdown( (wip_drvEthData_t *) drvp);
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
 * Driver registration
 */
static s32 wip_drvSubscribe_ENC28J60(
  const ascii *         name,
  u32                   spiId,
  adl_ioDefs_t          csGpioId,
  u32                   extIntId,
  adl_ioDefs_t          resetGpioId,
  adl_ioDefs_t          powerGpioId,
  const wip_ethAddr_t * macAddr)
{
  adl_busSPISettings_t buscfg;
  s32 siz;
  adl_ioDefs_t iocfg[2];
  enc28data * dp;
  s32 hnd;
  s32 SpiClock;

  wip_debug( "[ENC28J60] : subscribe to driver\n");
  wip_debug( "[ENC28J60] register: SPI=%d CS=%08x EXTINT=%d\n",
             spiId, csGpioId, extIntId);
  wip_debug( "[ENC28J60]           RST=%08x PWR=%08x\n",
             resetGpioId, powerGpioId);

  /* allocate driver data */
  dp = adl_memGet( sizeof(enc28data));
  if( dp == NULL) {
    wip_debug( "[ENC28J60] cannot allocate data\n");
    return -1; /* XXX */
  }
  dp->isr_handle  = -1;
  dp->spiId       = spiId;
  dp->csGpioId    = csGpioId;
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
   * Initialize SPI bus
   */

  /* ENC28J60 chipset controller supports 20 MHZ SPI BUS frequency max */
  adl_regGetHWInteger ( "spi_01_Master_MaxFreqClock", &SpiClock);
  if (SpiClock == 26000)
  {
  	buscfg.Clk_Speed          = 1;
  }
  else
  {
  	buscfg.Clk_Speed          = 0;
  }
  wip_debug( "[ENC28J60] spi_01_Master_MaxFreqClock=%d , Clk_Speed=%d\n", SpiClock, buscfg.Clk_Speed );
  
  buscfg.Clk_Mode           = ADL_BUS_SPI_CLK_MODE_0; /* SPI mode 0,0 */
  buscfg.ChipSelect         = ADL_BUS_SPI_ADDR_CS_GPIO;
  buscfg.ChipSelectPolarity = ADL_BUS_SPI_CS_POL_LOW;
  buscfg.LsbFirst           = ADL_BUS_SPI_MSB_FIRST;
  buscfg.GpioChipSelect     = dp->csGpioId;
  buscfg.LoadSignal         = ADL_BUS_SPI_LOAD_UNUSED;
  buscfg.DataLinesConf      = ADL_BUS_SPI_DATA_UNIDIR;
  buscfg.MasterMode         = ADL_BUS_SPI_MASTER_MODE;
  buscfg.BusySignal         = ADL_BUS_SPI_BUSY_UNUSED;
  dp->bus_handle = adl_busSubscribe( ADL_BUS_ID_SPI, dp->spiId, &buscfg);
  if( dp->bus_handle < 0) {
    wip_debug( "[ENC28J60] bus subscribe failed (%d)\n", dp->bus_handle);
    adl_memRelease( dp);
    return -1;
  }

  siz = 0;
  adl_busIOCtl( dp->bus_handle, ADL_BUS_CMD_SET_ADD_SIZE, &siz);
  siz = 8;
  adl_busIOCtl( dp->bus_handle, ADL_BUS_CMD_SET_OP_SIZE, &siz);
  adl_busIOCtl( dp->bus_handle, ADL_BUS_CMD_SET_DATA_SIZE, &siz);

  /*
   * Initialize GPIOs: disable power
   */
  iocfg[0] = dp->resetGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;
  if( dp->powerGpioId != (adl_ioDefs_t) -1) {
    iocfg[1] = dp->powerGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW;
    dp->io_handle = adl_ioSubscribe( 2, iocfg, 0, 0, 0);
  }
  else {
    dp->io_handle = adl_ioSubscribe( 1, iocfg, 0, 0, 0);
  }
  if( dp->io_handle < 0) {
    wip_debug( "[ENC28J60] gpio subscribe failed (%d)\n", dp->io_handle);
    adl_busUnsubscribe( dp->bus_handle);
    adl_memRelease( dp);
    return -1;
  }

  /*
   * If there is no power control line, set controller in power-down mode
   */
  if( dp->powerGpioId == (adl_ioDefs_t) -1) {
    /* deactive reset */
    wip_delay( 50);
    iocfg[0] = dp->resetGpioId | ADL_IO_DIR_OUT | ADL_IO_LEV_HIGH;
    adl_ioWrite( dp->io_handle, 1, iocfg);
    wip_delay( 50);
    /* enter sleep mode */
    write_reg( dp, ECON2, ECON2_PWRSV | ECON2_VRPS);
  }

  /* register driver */
  hnd = wip_drvSubscribe( name, WIP_BEARER_ETHER, drv_ctl, dp);
  if( hnd < 0) {
    wip_debug( "[ENC28J60] driver subscribe failed (%d)\n", hnd);
    adl_busUnsubscribe( dp->bus_handle);
    adl_ioUnsubscribe( dp->io_handle);
    adl_memRelease( dp);
  }
  else
  {
    eth_handle = hnd;
      
#ifdef USE_LIMIT_RCV_DATA 
    // Message handler definition
    ENC28J60_MsgHandle = adl_msgSubscribe ( &ENC28J60_MsgFilter, drv_MessageHandler );
#endif    
  }

  return hnd;
}
static void wip_drvUnsubscribe_ENC28J60(u32 handle)
{
  wip_debug( "[ENC28J60] : unsubscribe to driver\n");
  wip_drvUnsubscribe(handle);
  drv_I2C_E2P_M24CXX_Close();/*Added for NEW53415*/

#ifdef USE_LIMIT_RCV_DATA     
  // Unsubscribe Message handler
  adl_msgUnsubscribe ( ENC28J60_MsgHandle );
#endif 
}

/***************************************************************************/
/*  Function   : u8_CheckSum                                               */
/*-------------------------------------------------------------------------*/
/*  Object     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*                    |   |   |   |                                        */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static u8 u8_CheckSum(u8 * Data, u16 Length)
{
  u8 uReturn = 0;
  while (Length-- > 0)
  {
      uReturn += *Data++;
  }
  uReturn ^= 0xff;
  return uReturn;
}

/***************************************************************************/
/*  Function   : drv_MacAddressRead                                        */
/*-------------------------------------------------------------------------*/
/*  Object     : Read MAC address                                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*                    |   |   |   |                                        */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static s32 drv_MacAddressRead(wip_ethAddr_t * MacAdr, adl_atPort_e Port)
{
    s32 ret = OK;
    drv_Read_I2C_E2P_M24CXX_t *ReadBuff;

    ReadBuff = adl_memGet ( sizeof ( drv_Read_I2C_E2P_M24CXX_t ) 
        + sizeof(wip_ethAddr_t_7BYTE) + 1 );
    ReadBuff->Length = sizeof(wip_ethAddr_t_7BYTE) + 1;

    /* Shift address offset to 0x01 */
    ReadBuff->Address = 0;
    if ( !drv_I2C_E2P_M24CXX_Read ( ReadBuff, 0))
    {
        // Build response string
        if (u8_CheckSum(ReadBuff->Data + sizeof(u8), sizeof(wip_ethAddr_t)) == 
            ReadBuff->Data[sizeof(wip_ethAddr_t_7BYTE)])
        {
            wm_memcpy((u8*)MacAdr, ReadBuff->Data + sizeof(u8), sizeof(wip_ethAddr_t));
            if (Port)
            {
                ascii * StrResult = adl_memGet ( sizeof(wip_ethAddr_t) * 2 + 30 );
                // I2C chip: 4 digits address
                wm_sprintf ( StrResult, "\r\n+%s: \"", "MAC" );
                wm_ibuftohexa ( StrResult + wm_strlen ( StrResult )
                    , (u8*)MacAdr, sizeof(wip_ethAddr_t) );
                wm_strcat ( StrResult, "\"\r\n" );
                adl_atSendResponse ( Port, StrResult );
                adl_memRelease ( StrResult );
            }
        }
        else
        {
            if (Port)
            {
                ascii * StrResult = adl_memGet ( (sizeof(wip_ethAddr_t) + 1) * 2 + 30 );
                // I2C chip: 4 digits address
                wm_sprintf ( StrResult, "\r\n%s: \"", "MAC Error: " );
                wm_ibuftohexa ( StrResult + wm_strlen ( StrResult )
                    , ReadBuff->Data, sizeof(wip_ethAddr_t) + 1 );
                wm_sprintf ( StrResult + wm_strlen ( StrResult )
                    , " -> %2x", u8_CheckSum(ReadBuff->Data, sizeof(wip_ethAddr_t)) );
                wm_strcat ( StrResult, "\"\r\n" );
                adl_atSendResponse ( Port, StrResult );
                adl_memRelease ( StrResult );
            }
            wm_memcpy(MacAdr, ReadBuff->Data + sizeof(u8), sizeof(wip_ethAddr_t));
            ret = ERROR;
        }
    }
    else
    {
      ret = ERROR;
    }

    adl_memRelease ( ReadBuff );
    return ret;
}

/***************************************************************************/
/*  Function   : wip_drvSubscribe_FASTRACK                                 */
/*-------------------------------------------------------------------------*/
/*  Object     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static s32 wip_drvSubscribe_FASTRACK( const ascii *name )
{
  s32 ret = ETH_STATUS_ERROR; 

  ret = drv_e2pInit();
  if (ret < 0)
  {
    wip_debug( "[ENC28J60] : e2pInit failed\n");
    return ret;
  }

  ret = drv_MacAddressRead(&MacAddr, ADL_AT_UNS);
  
  if (ret < 0)
  {
    wip_debug( "[ENC28J60] : cannot read Mac address\n");
    return ret;
  }
  else
  {
    wip_debug( "[ENC28J60] : MAC address read: %02x-%02x-%02x-%02x-%02x-%02x\n\n",
                MacAddr[0], MacAddr[1], MacAddr[2],
                MacAddr[3], MacAddr[4], MacAddr[5] );

    //step2 : subscribe to IESM Ethernet driver 
    /* subscribe ethernet IESM driver and return   */
    /* handle on success, or error code on failure */
    ret = wip_drvSubscribe_ENC28J60(
                  "ETHER",
                  2,                  // SPI2 
                  ADL_IO_GPIO|35,     // GPIO35 (CS) 
                  0,                  // INT0 
                  ADL_IO_GPIO|19,     // GPIO19 (RST) 
                  ADL_IO_GPIO|20,     // GPIO20 (EN_VCC) 
                  NULL); 

    if( ret < 0) {
      wip_debug( "[ENC28J60] : driver subscription failed: %d\n", ret);
    }
    else {
      wip_debug( "[ENC28J60] : driver subscription successful\n");
    }
  }

  return ret;
}


/***************************************************************************/
/*  Function   : drv_close_iesm                                            */
/*-------------------------------------------------------------------------*/
/*  Object     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static e_Eth_driverStatus_t drv_close_iesm( u32 Handle )
{
  wip_debug( "[ENC28J60] : closing driver\n");

  // call IESM driver unsubscribe function 
  wip_drvUnsubscribe_ENC28J60( Handle);

#ifdef USE_LIMIT_RCV_DATA     
  // Unsubscribe Message handler
  adl_msgUnsubscribe ( ENC28J60_MsgHandle );
#endif 

  // return status
  return ETH_STATUS_NORMAL;
}

/***************************************************************************/
/*  Function   : drv_io_control_iesm                                       */
/*-------------------------------------------------------------------------*/
/*  Object     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static e_Eth_driverStatus_t drv_io_control_iesm( u32 Handle, u32 Cmd, void* pParam )
{
  e_Eth_driverStatus_t status = ETH_STATUS_ERROR;

  switch(Cmd)
  {
    case IOCTL_ETHER_READ_MAC_ADDRESS: 
    {
      memcpy(pParam, &MacAddr, sizeof(wip_ethAddr_t));
      status = ETH_STATUS_NORMAL; 

      wip_debug( "[ENC28J60] : io_ctrl : MAC address read: %02x-%02x-%02x-%02x-%02x-%02x\n\n",
                  MacAddr[0], MacAddr[1], MacAddr[2],
                  MacAddr[3], MacAddr[4], MacAddr[5] );
    }
    break;

    case IOCTL_ETHER_LINK_STATUS: 
    { 
      *((s8 *)pParam) = LinkStatus;
      status = ETH_STATUS_NORMAL;

      wip_debug( "[ENC28J60] : io_ctrl : link status: %d\n", LinkStatus);
    }
    break;
    
    default:
    {
      wip_debug( "[ENC28J60] : io_ctrl : unknown command %d\n", Cmd);
    }
    break;
  }

  return status; 
}

/***************************************************************************/
/*  Function   : drv_open_iesm                                             */
/*-------------------------------------------------------------------------*/
/*  Object     : open IESM ethernet driver                                 */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/* Settings           | X | X |   | interface settings                     */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static s32 drv_open_iesm( psEthSettings_t Settings )
{
  s32 ret = ETH_STATUS_ERROR;  

  wip_debug( "[ENC28J60] : opening %s driver\n", Settings->identity);

  /* subscribe to the driver */
  ret = wip_drvSubscribe_FASTRACK( "ETHER"); 

  if( ret >= 0) { 
  
    /* return management interface for IESM driver */    
    DrvPubItf.close      = drv_close_iesm;
    DrvPubItf.io_control = drv_io_control_iesm;
  }
  else {

    DrvPubItf.close      = NULL; 
    DrvPubItf.io_control = NULL; 

    wip_debug( "[ENC28J60] : cannot subscribe to driver : %d\n", ret ); 
  }

  // register the interface
  Settings->interface = &psItfCont;

  // return status
  return ret; 
}

