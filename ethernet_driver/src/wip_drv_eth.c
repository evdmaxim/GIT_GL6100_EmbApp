/********************************************************************************************/
/* wip_drv_eth.c  -  Copyright Wavecom S.A. (c) 2007                                        */
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
/* File    :   wip_drv_eth.c                                                */
/*                                                                          */
/* Scope   :   Ethernet driver generic interface                            */
/*                                                                          */

/*
 * $LogWavecom:$
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  22.04.09 | JGE    | 1.0            | Initial revision.                  
 * ----------+--------+----------------+-------------------------------------
*/
#include "wip.h"
#define __DRV_ETH_PARAM__
#include "wip_drv_eth.h"
#undef __DRV_ETH_PARAM__


extern s32 ( *eth_open[ETH_DRV_LAST]) ( psEthSettings_t Settings ); 


/***************************************************************************/
/*  Function   : ethernet_open                                             */
/*-------------------------------------------------------------------------*/
/*  Object     : open ethernet driver                                      */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/* Settings           | X | X |   |                                        */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
s32 ethernet_open( psEthSettings_t Settings )
{
  s32 ret = ETH_STATUS_ERROR; 

  if( strcmp( Settings->identity, AvailableEthernetDriver[ETH_DRV_TYPE_IESM] ) == 0) 
  {
    if( eth_open[ETH_DRV_TYPE_IESM] != NULL)
      ret = eth_open[ETH_DRV_TYPE_IESM]( Settings );
    else
      wip_debug( "no access Ethernet (%s) driver\n", Settings->identity);
  }
  else if (strcmp( Settings->identity, AvailableEthernetDriver[ETH_DRV_TYPE_DM9000] ) == 0) 
  {
    if( eth_open[ETH_DRV_TYPE_DM9000] != NULL)
      ret = eth_open[ETH_DRV_TYPE_DM9000]( Settings );
    else
      wip_debug( "no access Ethernet (%s) driver\n", Settings->identity);
  }
  //
  //ADD HERE ANY NEW SUPPORTED ETHERNET DRIVER
  //
  else
  {
    wip_debug( "%s Ethernet driver is not supported \n", Settings->identity);
  }

  if( ret < 0)
  {
    wip_debug( "cannot open Ethernet (%s) driver : %d\n", Settings->identity, ret); 
  }

  // return status 
  return ret;
}
