/*****************************************************************************/
/*  cfg_eth.c   -  Copyright Wavecom S.A. (c) 2006                           */
/*                                                                           */
/*                                                                           */
/* DISCLAIMER OF WARRANTY                                                    */
/* ======================                                                    */
/* This Software is provided free of charge on an 'as is' basis. No warranty */
/* is given by Wavecom S.A. in relation to the Software of the uses to which */
/* it may be put by you, the user, or its merchantability, fitness or        */
/* suitability for any particular purpose or conditions; and/or that the use */
/* of the Software and all documentation relating thereto by the Licensee    */
/* will not infringe any third party copyright or other intellectual property*/
/* rights. Wavecom S.A. shall furthermore be under no obligation             */
/* to provide support of any nature for the Software and the Documentation.  */
/*                                                                           */
/* LIMIT OF LIABILITY                                                        */
/* ==================                                                        */
/* In no event shall Wavecom S.A. be liable for any loss or damages          */
/* whatsoever or howsoever caused arising directly or indirectly in          */
/* connection with this licence, the Software, its use or otherwise except to*/
/* the extent that such liability may not be lawfully excluded.              */
/* Notwithstanding the generality of the foregoing, Wavecom S.A. expressly   */
/* excludes liability for indirect, special, incidental or consequential loss*/
/* or damage which may arise in respect of the Software or its use, or in    */
/* respect of other equipment or property, or for loss of profit, business,  */
/* revenue, goodwill or anticipated savings.                                 */
/*                                                                           */
/*****************************************************************************/

/***************************************************************************/
/*  File       : cfg_eth.c                                                
 *
 *  Object     : Ethernet bearer configuration
 *
 *
 */

/*
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  27.04.09 | JGE    | 1.0            | Initial revision.
 * ----------+--------+----------------+-------------------------------------
 */
#include "adl_global.h"
#include "wip.h"
#include "wip_drv_eth.h"
#include "wip_drv.h"

/***************************************************************************/
/*  Functions                                                              */
/***************************************************************************/

/* Function to be called after driver subscription */
static void (* appli_entry_point)( void);

/***************************************************************************/
/*  Variables                                                              */
/***************************************************************************/

sEthSettings_t Drv_Settings;
u32            Drv_Handle;

wip_bearer_t   Bearer_Handle;

/*********************************************************************/
/* Due to Memory problem, the dedicated memory space for MAC address */
/* is allocated to 7 byte, where the most significant byte is dummy  */
/* This is only used when there is memory problem occcures           */
/*********************************************************************/
wip_ethAddr_t mac_addr = { 0x02, 0x26, 0x87, 0x00, 0x00, 0x00 };


/***************************************************************************/
/*  Defines                                                                */
/***************************************************************************/
/* 
 * -----------------------------------------------
 * Activate any of the concerned following
 * flags for the proper case configuration.
 * -----------------------------------------------
 */
#define __DRV_ETH_IESM__
//#define __DRV_ETH_DM9000__


/***************************************************************************/
/*  Function   : show_config                                               */
/*-------------------------------------------------------------------------*/
/*  Object     : display Ethernet bearer configuration                     */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  br                | X |   |   | bearer identifier                      */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static void show_config( wip_bearer_t br)
{
  wip_in_addr_t ip1, ip2;
  char ip1_str[16], ip2_str[16];
  
  wip_debug( "\n*** Ethernet bearer configuration ***\n\n");
  
  if( wip_bearerGetOpts( br,
                         WIP_BOPT_IP_ADDR,      &ip1,
                         WIP_BOPT_IP_NETMASK,   &ip2,
                         WIP_BOPT_END) == OK) {
    wip_inet_ntoa( ip1, ip1_str, sizeof(ip1_str));
    wip_inet_ntoa( ip2, ip2_str, sizeof(ip2_str));
    wip_debug( "  Local: %s Netmask: %s\n", ip1_str, ip2_str);
  }
  if( wip_bearerGetOpts( br,
                         WIP_BOPT_IP_GW,   &ip1,
                         WIP_BOPT_END) == OK) {
    wip_inet_ntoa( ip1, ip1_str, sizeof(ip1_str));
    wip_debug( "  Default gateway: %s\n", ip1_str);
  }
  if( wip_bearerGetOpts( br,
                         WIP_BOPT_IP_DNS1, &ip1,
                         WIP_BOPT_IP_DNS2, &ip2,
                         WIP_BOPT_END) == OK) {
    wip_inet_ntoa( ip1, ip1_str, sizeof(ip1_str));
    wip_inet_ntoa( ip2, ip2_str, sizeof(ip2_str));
    wip_debug( "  DNS1: %s DNS2: %s\n", ip1_str, ip2_str);
  }

  wip_debug( "\n");
  
  /* start test application */
  appli_entry_point();
}

/***************************************************************************/
/*  Function   : evh_bearer                                                */
/*-------------------------------------------------------------------------*/
/*  Object     : bearer events handler                                     */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  br                | X |   |   | bearer identifier                      */
/*  event             | X |   |   | bearer event                           */
/*  ctx               | X |   |   | passed context                         */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static void evh_bearer( wip_bearer_t br, s8 event, void *ctx)
{
  if( WIP_BEV_IP_CONNECTED == event) {
    show_config( br);    
  }
}

/***************************************************************************/
/*  Function   : open_and_start_bearer                                     */
/*-------------------------------------------------------------------------*/
/*  Object     : open and start Ethernet bearer                            */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
s32 open_and_start_bearer( void)
{
  s8 ret;
  wip_ethAddr_t mac_addr;

  wip_debug( "open_and_start_bearer\n");

  //read MAC address
  if(Drv_Settings.interface && *Drv_Settings.interface)
  {
    if((*Drv_Settings.interface)->io_control)
    {
      ret = (*Drv_Settings.interface)->io_control( Drv_Handle, /*Cmd=*/IOCTL_ETHER_READ_MAC_ADDRESS, &mac_addr ); 
    }
    if( ret == ETH_STATUS_ERROR) {
      wip_debug( "cannot read Mac address: %d\n", ret);
    }
  } 

  /* Open then Start Ethernet bearer */
  ret = wip_bearerOpen( &Bearer_Handle, "ETHER", evh_bearer, NULL);
  if( ret != OK) {
    wip_debug( "cannot open bearer: %d\n", ret);
  }
  else {

    /* set MAC address */
    /* set automatic configuration with DHCP */
    ret = wip_bearerSetOpts( Bearer_Handle,
                             WIP_BOPT_ETH_ADDR, &mac_addr,
                             WIP_BOPT_IP_DHCP,  TRUE,
                             WIP_BOPT_END);
    if( ret != OK) {
      wip_debug( "cannot set bearer options: %d\n", ret);
    }
    else {

      ret = wip_bearerStart( Bearer_Handle);
      if( (ret != OK) && (ret != WIP_BERR_OK_INPROGRESS)) {
        wip_debug( "cannot start bearer: %d\n", ret);
      }
      else {
        wip_debug( "\n  Ethernet bearer started,\n");
        wip_debug( "  MAC address: %02x-%02x-%02x-%02x-%02x-%02x\n\n",
                   mac_addr[0], mac_addr[1], mac_addr[2],
                   mac_addr[3], mac_addr[4], mac_addr[5]);
      }

      if( ret == OK) {
        show_config( Bearer_Handle);
      }
    }
  }

  return ret;
}

/***************************************************************************/
/*  Function   : cfg_eth                                                   */
/*-------------------------------------------------------------------------*/
/*  Object     : initialize Ethernet connection, then launch entry_point() */
/*               on success.                                               */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  entry_point       | X |   |   |Function run after successful connection*/
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void cfg_eth( void (* entry_point)(void)) 
{
  s32 ret;

  TRACE (( 1, "(cfg_eth) Enter." ));

  /* memorize entry_point */
  appli_entry_point = entry_point; 

#ifdef __DRV_ETH_IESM__

  Drv_Settings.identity = (char *) AvailableEthernetDriver[ETH_DRV_TYPE_IESM];

#elif __DRV_ETH_DM9000__

  Drv_Settings.identity = (char *) AvailableEthernetDriver[ETH_DRV_TYPE_DM9000];

#else

  #error need Ethernet driver to be defined

#endif

  ret = ethernet_open( &Drv_Settings );

  if( ret < 0)
  {
    //error
    TRACE (( 1, "Cannot install Ethernet (%s) driver: %d\n", Drv_Settings.identity, ret));
  }
  else
  {
    // memorize driver handle
    Drv_Handle = ret;

    // open and start Ethernet bearer
    open_and_start_bearer();
  }
}
