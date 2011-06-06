/*****************************************************************************/
/* appli.c  -  Copyright Wavecom S.A. (c) 2006                               */
/*                                                                           */
/* DISCLAIMER OF WARRANTY                                                    */
/* ======================                                                    */
/* This Software is provided free of charge on an 'as is' basis. No warranty */
/* is given by Sierra Wireless in relation to the Software of the uses to    */
/* which it may be put by you,the user, or its merchantability, fitness or   */
/* suitability for any particular purpose or conditions; and/or that the use */
/* of the Software and all documentation relating thereto by the Licensee    */
/* will not infringe any third party copyright or other intellectual         */
/* property rights.Sierra Wireless shall furthermore be under no obligation  */
/* to provide support of any nature for the Software and the Documentation.  */
/*                                                                           */
/* LIMIT OF LIABILITY                                                        */
/* ==================                                                        */
/* In no event shall Sierra Wireless be liable for any loss or damages       */
/* whatsoever or howsoever caused arising directly or indirectly in          */
/* connection with this licence, the Software,its use or otherwise except    */
/* to the extent that such liability may not be lawfully                     */
/* excluded. Notwithstanding the generality of the foregoing, Sierra Wireless*/
/* expressly excludes liability for indirect, special, incidental or         */
/* consequential loss or damage which may arise in respect of the Software   */
/* or its use, or in respect of other equipment or property, or for loss     */
/* of profit, business, revenue, goodwill or anticipated savings.            */
/*                                                                           */
/*****************************************************************************/

/***************************************************************************/
/*  File       : appli.c                                                   
 *                                                
 *  This is the main ADL entry point:                                  
 *    Before WIP application appli_entry_point() is called, any or none
 *    of the flags could be activated for a bearer type configuration:
 *
 *    The TCP/IP stack will be initialized over a :
 *
 *    [OVER_UART_PPP_SERV]:   PPP UART server connection mode
 *    [OVER_UART_PPP_CLIENT]: PPP UART client connection mode
 *    [OVER_GSM_PPP_SERV]:    PPP GSM DATA server connection mode
 *    [OVER_GSM_PPP_CLIENT]:  PPP GSM DATA client connection mode
 *    [OVER_GPRS]:            GPRS connection mode
 *    [ETHERNET_BEARER_USED]: Other case where ETHERNET driver is used
 *
 */

/*
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  22.05.06 | FFT    | 1.0            | Initial revision.
 * ----------+--------+----------------+-------------------------------------
 *  24.07.06 | RFN    |                | Increased wm_apmCustomStackSize
 *           |        |                | for compiling with ARM
 * ----------+--------+----------------+-------------------------------------
 *  24.10.06 | APH    |                | Added the GSM bearers
 * ----------+--------+----------------+-------------------------------------
 *  16.02.10 | SFR    |                | Added ETHERNET_BEARER_USED flag
 * ----------+--------+----------------+-------------------------------------
*/

#include "adl_global.h"
#include "wip.h"

/***************************************************************************/
/*  Defines                                                                */
/***************************************************************************/

/* 
 * -----------------------------------------------
 * Activate any or none of the concerned following
 * flags for the proper case configuration.
 * -----------------------------------------------
 */

/* #define OVER_UART_PPP_SERV           */
/* #define OVER_UART_PPP_CLIENT         */
/* #define OVER_GSM_PPP_SERV            */
/* #define OVER_GSM_PPP_CLIENT          */
/* #define OVER_ETHERNET                */
#define OVER_GPRS
/* #define ETHERNET_BEARER_USED         */


/***************************************************************************/
/*  Mandatory variables (Call stack sizes)                                 */
/*-------------------------------------------------------------------------*/
/*  wm_apmCustomStackSize                                                  */
/*  wm_apmIRQLowLevelStackSize                                             */
/*  wm_apmIRQHighLevelStackSize                                            */
/*-------------------------------------------------------------------------*/
/***************************************************************************/
#ifdef __GNU_GCC__
/*The GCC compiler and GNU Newlib (standard C library) implementation
 require more stack size than ARM compilers. If the GCC compiler is used,
 the Open AT� application has to be declared with greater stack sizes.*/
#define DECLARE_CALL_STACK(X)   const u16 wm_apmCustomStackSize = X*3
#define DECLARE_LOWIRQ_STACK(X) const u32 wm_apmIRQLowLevelStackSize = X*3
#define DECLARE_HIGHIRQ_STACK(X) const u32 wm_apmIRQHighLevelStackSize = X*3
#else /* #ifndef __GNU_GCC__ */
#define DECLARE_CALL_STACK(X)   const u16 wm_apmCustomStackSize = X
#define DECLARE_LOWIRQ_STACK(X) const u32 wm_apmIRQLowLevelStackSize = X
#define DECLARE_HIGHIRQ_STACK(X) const u32 wm_apmIRQHighLevelStackSize = X
#endif /* #ifndef __GNU_GCC__ */
/* Call stack size declaration */


/***************************************************************************/
/*  Prototypes                                                             */
/***************************************************************************/
void cfg_uart_ppp_client( void (*entry_point) (void));
void cfg_uart_ppp_serv  ( void (*entry_point) (void));
void cfg_gsm_ppp_client ( void (*entry_point) (void));
void cfg_gsm_ppp_serv   ( void (*entry_point) (void));
void cfg_gprs           ( void (*entry_point) (void));
void cfg_eth            ( void (*entry_point) (void));

void appli_entry_point  ( void);


/***************************************************************************/
/*  Mandatory Stack size Declaration                                       */
/*-------------------------------------------------------------------------*/
/*  No more WIP version < 4.00 supported                                   */
/*-------------------------------------------------------------------------*/
/***************************************************************************/

#if  defined( ETHERNET_BEARER_USED ) || defined( OVER_ETHERNET )
DECLARE_CALL_STACK(8192);
/* Ethernet driver uses high level interrupt handler */
DECLARE_LOWIRQ_STACK(1024);
DECLARE_HIGHIRQ_STACK(1024);
#else
DECLARE_CALL_STACK(4096);
#endif


/***************************************************************************/
/*  Function   : adl_main                                                  */
/*-------------------------------------------------------------------------*/
/*  Object     : Customer application initialisation                       */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  InitType          | x |   |   |  Application start mode reason         */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void adl_main ( adl_InitType_e InitType )
{
  int r;

  TRACE (( 1, "Embedded Application : Main" ));

  /* Initialize the stack */
  r = wip_netInitOpts( 
      WIP_NET_OPT_DEBUG_PORT, WIP_NET_DEBUG_PORT_UART1, /* WIP traces on UART1 */
      /* WIP_NET_OPT_DEBUG_PORT, WIP_NET_DEBUG_PORT_UART2, */
      /* WIP_NET_OPT_DEBUG_PORT, WIP_NET_DEBUG_PORT_TRACE, */
      WIP_NET_OPT_END);

  /* Depending on the activated flag, take the proper config */
#if defined( OVER_UART_PPP_SERV)
  /* Initialize PPP server over UART */
  cfg_uart_ppp_serv( appli_entry_point);

#elif defined( OVER_UART_PPP_CLIENT)
  /* Initialize PPP client over UART */
  cfg_uart_ppp_client( appli_entry_point);

#elif defined( OVER_GSM_PPP_SERV)
  /* Initialize PPP server over GSM  */
  cfg_gsm_ppp_serv( appli_entry_point);

#elif defined( OVER_GSM_PPP_CLIENT)
  /* Initialize PPP client over GSM  */
  cfg_gsm_ppp_client( appli_entry_point);

#elif defined( OVER_GPRS)
  /* Initialize GPRS connection      */
  cfg_gprs( appli_entry_point);

#elif defined( OVER_ETHERNET)
  /* initialize Ethernet connection */
  cfg_eth( appli_entry_point);

#else
  /* Don't initialize any bearer; only 
   * localhost=127.0.0.1 will be reachable. */
  appli_entry_point();

#endif
}

