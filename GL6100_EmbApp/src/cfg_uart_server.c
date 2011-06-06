/*****************************************************************************/
/*  cfg_uart_server.c   -  Copyright Wavecom S.A. (c) 2006                   */
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
/*  File       : cfg_uart_server.c            
 *
 *  Object     : PPP GSM DATA server bearer configuration:
 *
 *  [PPP_BEARER]:        UART Id
 *  [PPP_USER]:          login name
 *  [PPP_PASSWORD]:      password
 *  [PPP_LOCAL_STRADDR]: Module's address
 *  [PPP_DEST_STRADDR]:  PPP client's address
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
#define PPP_LOCAL_STRADDR "192.168.1.4" /* Module's address     */
#define PPP_DEST_STRADDR  "192.168.1.5" /* PPP client's address */


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
static s8 evh_ppp_auth( wip_bearer_t b, wip_bearerServerEvent_t *ev, void *ctx);
static void evh_bearer( wip_bearer_t b, s8 event, void *ctx);


/***************************************************************************/
/*  Function   : evh_ppp_auth                                              */
/*-------------------------------------------------------------------------*/
/*  Object     : ppp authentication events: in PPP server mode, when       */
/*               a client authenticates itself, fill the password so that  */
/*               t can be checked                                          */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  b                 | X |   |   | bearer identifier                      */
/*  ev                | X |   |   | bearer event                           */
/*  ctx               | X |   |   | passed context                         */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static s8 evh_ppp_auth( wip_bearer_t b,
                        wip_bearerServerEvent_t *ev,
                        void *ctx) {

  if( WIP_BEV_PPP_AUTH_PEER == ev->kind) {
    /* Make sure the user name is [PPP_USER] */
    if( strncmp( ev->content.ppp_auth.user,
                 PPP_USER,
                 ev->content.ppp_auth.userlen)) return 0;
    /* Fill in expected password. */
    ev->content.ppp_auth.secret    = PPP_PASSWORD;
    ev->content.ppp_auth.secretlen = sizeof( PPP_PASSWORD) - 1;
    return 1;
  }

  return 0;
}


/***************************************************************************/
/*  Function   : evh_bearer                                                */
/*-------------------------------------------------------------------------*/
/*  Object     : bearer events handler: when the bearer connection         */
/*                is completed, start application                          */
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
/*  Function   : cfg_uart_ppp_serv                                         */
/*-------------------------------------------------------------------------*/
/*  Object     : initialize PPP server, then launch entry_point() upon     */
/*               succesfull client connection.                             */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  entry_point       | X |   |   |Function run after successful connection*/
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void cfg_uart_ppp_serv( void (*entry_point)(void)) {
  int r;
  wip_in_addr_t local, dest;
  wip_bearer_t b;

  appli_entry_point = entry_point;

  wip_inet_aton( PPP_LOCAL_STRADDR, &local);
  wip_inet_aton( PPP_DEST_STRADDR, &dest);

  r = wip_bearerOpen( &b, PPP_BEARER, evh_bearer, NULL);
  ASSERT_OK( r);

  /* if Open AT application on WCPU aims at addressing an other IP @ 
   * than PPP peer or 127.0.0.1, it has to enable the IP Gateway 
   * functionality. This is done using WIP_BOPT_IP_SETGW at TRUE 
   */
  r = wip_bearerSetOpts( b, WIP_BOPT_IP_ADDR,  local,
                         WIP_BOPT_IP_DST_ADDR, dest,
                         WIP_BOPT_IP_SETDNS,   FALSE,
                         WIP_BOPT_IP_SETGW,    TRUE,
                         WIP_BOPT_RESTART,     FALSE,
                         WIP_BOPT_END);
  ASSERT_OK( r);
  
  r = wip_bearerStartServer( b, evh_ppp_auth, NULL);
  ASSERT_OK( r);
}

