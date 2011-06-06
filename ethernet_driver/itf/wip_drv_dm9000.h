/********************************************************************************************/
/* wip_drv_dm9000.h  -  Copyright Wavecom S.A. (c) 2007                                     */
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
/* File    :   wip_drv_dm9000.h                                             */
/*                                                                          */
/* Scope   :   Davicom DM9000/DM9000A Ethernet driver                       */ 
/*                                                                          */

/*
 * $LogWavecom:$
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  06.03.07 | RFN    | 1.0            | Initial revision.                  
 * ----------+--------+----------------+-------------------------------------
 *  03.12.07 | RFN    | 1.0            | New subscribe function                  
 * ----------+--------+----------------+-------------------------------------
*/

#ifndef __WIP_DRV_DM9000_H_INCLUDED__
#define __WIP_DRV_DM9000_H_INCLUDED__

#include "wip_drv.h"

/* driver registration */
s32 wip_drvSubscribe_DM9000(
  const ascii *         name,           /* name of bearer */
  u8                    csId,           /* chip select id */
  u32                   addressPin,     /* address pin */
  u32                   extIntId,       /* external interrupt id */
  adl_ioDefs_t          resetGpioId,    /* GPIO id of RESET line */
  adl_ioDefs_t          powerGpioId,    /* GPIO id of POWER_EN line */
  const wip_ethAddr_t * macAddr);       /* MAC address */

/* set powerGpioId to ((adl_ioDefs_t) -1) if not used */
/* set macAddr to NULL if address is set with bearer API */

#endif /* __WIP_DRV_DM9000_H_INCLUDED__ */
