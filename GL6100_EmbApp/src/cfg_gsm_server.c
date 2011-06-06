/*****************************************************************************/
/*  cfg_gsm_server.c   -  Copyright Wavecom S.A. (c) 2006                    */
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
/*  File       : cfg_gsm_server.c            
 *
 *  Object     : PPP GSM DATA server bearer configuration:
 *
 *  [PPP_USER]:          login name
 *  [GPRS_PASSWORD]:     password
 *  [PPP_LOCAL_STRADDR]: Module's address
 *  [PPP_DEST_STRADDR]:  PPP client's address
 *  [GSM_REMOTE_DIALER]: Remote GSM caller number to check
 *  [GSM_PINCODE]:       PIN code of the SIM card, or NULL if not required
 *
 */

/*
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  23.10.06 | APH    | 1.0            | Initial revision.
 * ----------+--------+----------------+-------------------------------------
 */

#include "adl_global.h"
#include "wip.h"

/***************************************************************************/
/*  Defines                                                                */
/***************************************************************************/
#define PPP_USER          "wipuser"     /* Expected user name       */
#define PPP_PASSWORD      "WU#passwd"   /* Password server checks   */
#define PPP_LOCAL_STRADDR "192.168.1.4" /* Module's address         */
#define PPP_DEST_STRADDR  "192.168.1.5" /* PPP client's address     */
#define GSM_REMOTE_DIALER "0612019964"  /* Remote GSM caller number */
#define GSM_PINCODE       "8464"        /* GSM pin code             */


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
static void evh_sim( u8 event);
static s8 evh_ppp_auth( wip_bearer_t b, wip_bearerServerEvent_t *ev, void *ctx);
static void evh_bearer( wip_bearer_t b, s8 event, void *ctx);



/***************************************************************************/
/*  Function   : evh_sim                                                   */
/*-------------------------------------------------------------------------*/
/*  Object     : sim events:                                               */
/*               when SIM initialisation is completed, open the GSM        */
/*               server bearer                                             */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  event             | X |   |   | SIM event                              */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static void evh_sim( u8 event) {
  int r;
  wip_in_addr_t local, dest;
  wip_bearer_t b;

  if( ADL_SIM_EVENT_FULL_INIT != event) return;

  wip_inet_aton( PPP_LOCAL_STRADDR, &local);
  wip_inet_aton( PPP_DEST_STRADDR, &dest);

  r = wip_bearerOpen( &b, "GSM", evh_bearer, NULL);
  ASSERT_OK( r);

  r = wip_bearerSetOpts( b, WIP_BOPT_IP_ADDR,  local,
                         WIP_BOPT_IP_DST_ADDR, dest,
                         WIP_BOPT_IP_SETDNS,   FALSE,
                         WIP_BOPT_IP_SETGW,    FALSE,
                         WIP_BOPT_RESTART,     TRUE,
                         WIP_BOPT_END);
  ASSERT_OK( r);
  r = wip_bearerStartServer( b, evh_ppp_auth, NULL);
  ASSERT_OK( r);
}


/***************************************************************************/
/*  Function   : evh_ppp_auth                                              */
/*-------------------------------------------------------------------------*/
/*  Object     : ppp authentication events: in PPP server mode, when a     */
/*               client authenticates itself, fill the password            */
/*               so that it can be checked.                                */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  b                 | X |   |   | bearer identifier                      */
/*  event             | X |   |   | bearer event type                      */
/*  ctx               |   |   |   | unused                                 */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static s8 evh_ppp_auth( wip_bearer_t b,
                        wip_bearerServerEvent_t *ev,
                        void *ctx) {

  if( WIP_BEV_DIAL_CALL == ev->kind) {
    /* Make sure that we're called by [GSM_REMOTE_DIALER], not by
     * some random hacker... */
    ev->content.dial_call.phonenb = GSM_REMOTE_DIALER;
    return 1;
  }

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
/*  Function   :  evh_bearer                                               */
/*-------------------------------------------------------------------------*/
/*  Object     : bearer events handler: when the bearer connection         */
/*               is completed, start IP services                           */
/*               (used both in PPP and GPRS modes).                        */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  b                 | X |   |   | bearer identifier                      */
/*  event             | X |   |   | bearer event type                      */
/*  ctx               |   |   |   | unused                                 */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static void evh_bearer( wip_bearer_t b, s8 event, void *ctx) {
  if( WIP_BEV_IP_CONNECTED == event) { appli_entry_point(); }
}


/***************************************************************************/
/*  Function   : cfg_gsm_ppp_serv                                          */
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
void cfg_gsm_ppp_serv( void (*entry_point)(void)) {
  appli_entry_point = entry_point;
  adl_simSubscribe( evh_sim, GSM_PINCODE);
}

