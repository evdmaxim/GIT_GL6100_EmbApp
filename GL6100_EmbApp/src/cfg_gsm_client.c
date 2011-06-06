/*****************************************************************************/
/*  cfg_gsm_client.c   -  Copyright Wavecom S.A. (c) 2006                    */
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
/*  File       : cfg_gsm_client.c            
 *
 *  Object     : PPP GSM DATA client bearer configuration:
 *
 *  [REMOTE_GSM_DIALNB]: Remote number to call
 *  [PPP_USER]:          login name
 *  [GPRS_PASSWORD]:     password
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

#define PPP_USER          "wipuser"    /* Expected user name         */
#define PPP_PASSWORD      "WU#passwd"  /* Password server checks     */
#define GSM_PINCODE       "0000"       /* GSM pin code               */
#define REMOTE_GSM_DIALNB "0615546937" /* Remote GSM module phone nb */


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
static void evh_bearer( wip_bearer_t b, s8 event, void *ctx);


/***************************************************************************/
/*  Function   : evh_sim                                                   */
/*-------------------------------------------------------------------------*/
/*  Object     : sim events:                                               */
/*               when SIM initialisation is completed, open the GSM        */
/*               client bearer                                             */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  event             | X |   |   | SIM event                              */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static void evh_sim( u8 event) {
  int r;
  wip_bearer_t b;

  if( ADL_SIM_EVENT_FULL_INIT != event) return;

  r = wip_bearerOpen( &b, "GSM", evh_bearer, NULL);
  ASSERT_OK( r);
  
  r = wip_bearerSetOpts( b,
                         WIP_BOPT_LOGIN,        PPP_USER,
                         WIP_BOPT_PASSWORD,     PPP_PASSWORD,
                         WIP_BOPT_DIAL_PHONENB, REMOTE_GSM_DIALNB,
                         WIP_BOPT_END);
  ASSERT_OK( r);
  r = wip_bearerStart( b);
  ASSERT( 0 == r || WIP_BERR_OK_INPROGRESS == r);
}


/***************************************************************************/
/*  Function   : evh_bearer                                                */
/*-------------------------------------------------------------------------*/
/*  Object     : bearer events handler:                                    */
/*               when the bearer connection is completed,                  */
/*               start IP services                                         */
/*                                                                         */
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
/*  Function   : cfg_ppp_client                                            */
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
void cfg_gsm_ppp_client ( void (* entry_point)(void)) {
  appli_entry_point = entry_point;
  adl_simSubscribe( evh_sim, GSM_PINCODE);
}

