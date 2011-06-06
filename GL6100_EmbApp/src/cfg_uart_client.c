/*****************************************************************************/
/*  cfg_uart_client.c   -  Copyright Wavecom S.A. (c) 2006                   */
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
/*  File       : cfg_uart_client.c            
 *
 *  Object     : PPP UART client bearer configuration:
 *
 *  [PPP_BEARER]:    UART Id
 *  [PPP_USER]:      login name
 *  [GPRS_PASSWORD]: password
 *
 */

/*
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  22.05.06 | FFT    | 1.0            | Initial revision.
 * ----------+--------+----------------+-------------------------------------
 *  26.10.06 | APH    |                | Update function headers
 * ----------+--------+----------------+-------------------------------------
 */

#include "adl_global.h"
#include "wip.h"


/***************************************************************************/
/*  Defines                                                                */
/***************************************************************************/
#define PPP_BEARER        "UART2"
#define PPP_USER          "wipuser"
#define PPP_PASSWORD      "WU#passwd"



#define ASSERT( pred) \
if( !(pred)) wip_debug( "ASSERTION FAILURE line %i: " #pred "\n", __LINE__)
#define ASSERT_OK( v) ASSERT( 0 == (v))


/***************************************************************************/
/*  Functions                                                              */
/***************************************************************************/
static void (* appli_entry_point)( void);

/***************************************************************************/
/*  Initialization-related event handlers                                  */
/***************************************************************************/
static void evh_bearer( wip_bearer_t b, s8 event, void *ctx);


/***************************************************************************/
/*  Function   : evh_bearer                                                */
/*-------------------------------------------------------------------------*/
/*  Object     : bearer events handler: when the bearer connection is      */
/*               completed, start IP services                              */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  b                 | X |   |   | bearer identifier                      */
/*  event             | X |   |   | bearer event                           */
/*  ctx               | X |   |   | passed context                         */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static void evh_bearer( wip_bearer_t b, s8 event, void *ctx) {
  if( WIP_BEV_IP_CONNECTED == event) { appli_entry_point(); }
}


/***************************************************************************/
/*  Function   : cfg_uart_ppp_client                                       */
/*-------------------------------------------------------------------------*/
/*  Object     : initialize PPP client connection, then launch             */
/*               entry_point() on success                                  */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  entry_point       | X |   |   |Function run after successful connection*/
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void cfg_uart_ppp_client ( void (* entry_point)(void)) {
  int r;
  wip_bearer_t b;

  appli_entry_point = entry_point;

  r = wip_bearerOpen( &b, PPP_BEARER, evh_bearer, NULL);
  ASSERT_OK( r);
  
  r = wip_bearerSetOpts( b,
                         WIP_BOPT_LOGIN,       PPP_USER,
                         WIP_BOPT_PASSWORD,    PPP_PASSWORD,
                         WIP_BOPT_END);
  ASSERT_OK( r);
  r = wip_bearerStart( b);
  ASSERT( 0 == r || WIP_BERR_OK_INPROGRESS == r);
}

