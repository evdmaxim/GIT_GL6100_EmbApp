/********************************************************************************************/
/* wip_drv_eth.h  -  Copyright Wavecom S.A. (c) 2007                                        */
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
/* File    :   wip_drv_eth.h                                                */
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

#ifndef __WIP_DRV_ETH_H_INCLUDED__
#define __WIP_DRV_ETH_H_INCLUDED__

typedef enum
{
   ETH_STATUS_ERROR   = -1L,         // operation failed     (sync/async)
   ETH_STATUS_NORMAL  = 0,           // operation succeeded  (sync)
   ETH_STATUS_PENDING = 0x7FFFFFFEL, // operation is pending (async)
   ETH_STATUS_ALIGN   = 0x7FFFFFFFL
} e_Eth_driverStatus_t;


/*

 * GENERIC CONTAINER for handling Ethernet like Service Provider Interfaces
 */
typedef struct
{        
   e_Eth_driverStatus_t ( *io_control )( u32 Handle, u32 Cmd, void* pParam );
   e_Eth_driverStatus_t ( *close )     ( u32 Handle );
} sItfCont_t, *psItfCont_t, **ppsItfCont_t;


typedef enum
{
  IOCTL_ETHER_READ_MAC_ADDRESS,
  IOCTL_ETHER_LINK_STATUS,
  IOCTL_ETHER_LAST
}
e_Eth_driverIoCtl_t;
/*
 * Identies of the Generic IO control sub opearations (SET / GET )
 */

typedef struct
{
   /*== INPUT ==*/
   char*     identity;

   /*== OUTPUT ==*/
   ppsItfCont_t interface;  /* may be set to NULL */
} sEthSettings_t, *psEthSettings_t;


enum {
  ETH_DRV_TYPE_IESM,
  ETH_DRV_TYPE_DM9000,
  ETH_DRV_LAST
};


//authorized value for identity field in settings param.

#ifndef __DRV_ETH_PARAM__
extern const char* AvailableEthernetDriver[ETH_DRV_LAST];
#else
const char* AvailableEthernetDriver[ETH_DRV_LAST]= { "IESM", "DM9000"};
#endif 

/* Public & Generic interface for driver management */
s32 ethernet_open( psEthSettings_t Settings );


#endif /* __WIP_DRV_ETH_H_INCLUDED__ */
