/*****************************************************************************/
/*  cfg_gprs.c   -  Copyright Wavecom S.A. (c) 2006                          */
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
/*  File       : cfg_gprs.c                                                
 *
 *  Object     : GPRS bearer configuration:
 *
 *  [GPRS_APN]:      Network provider APN
 *  [GPRS_USER]:     login name
 *  [GPRS_PASSWORD]: password
 *  [GPRS_PINCODE]:  PIN code of the SIM card, or NULL if not required
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
#define GPRS_APN      "websfr"
#define GPRS_USER     "a2b"
#define GPRS_PASSWORD "access"
#define GPRS_PINCODE  "0000"

#define CREG_POLLING_PERIOD 20 /* in 100ms steps */

#define ASSERT( pred) \
if( !(pred)) wip_debug( "ASSERTION FAILURE line %i: " #pred "\n", __LINE__)
#define ASSERT_OK( v) ASSERT( 0 == (v))


/***************************************************************************/
/*  Functions                                                              */
/***************************************************************************/

/* Function to be called once the bearer is up and running. */
static void (* appli_entry_point)( void);

/***************************************************************************/
/*  Initialization-related event handlers                                  */
/***************************************************************************/


/***************************************************************************/
/*  Function   : evh_bearer                                                */
/*-------------------------------------------------------------------------*/
/*  Object     : bearer events handler: when the bearer connection is      */
/*               completed, start IP services                              */
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

/****************************************************************************/
/*  Function   : open_and_start_bearer()                                    */
/*--------------------------------------------------------------------------*/
/*  Object : Open and start the GPRS bearer. Normally, the bearer will      */
/*           answer IN_PROGRESS, and the initialization will be finished    */
/*           by the callback evh_bearer().                                  */
/*--------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                            */
/*--------------------+---+---+---+-----------------------------------------*/
/*--------------------+---+---+---+-----------------------------------------*/
/****************************************************************************/
static void open_and_start_bearer( void) {
  int r;
  wip_bearer_t b;

  r = wip_bearerOpen( &b, "GPRS", evh_bearer, NULL);
  ASSERT_OK( r);
  
  r = wip_bearerSetOpts( b, WIP_BOPT_GPRS_APN, GPRS_APN,
                         WIP_BOPT_LOGIN,       GPRS_USER,
                         WIP_BOPT_PASSWORD,    GPRS_PASSWORD,
                         WIP_BOPT_END);
  ASSERT_OK( r);
  r = wip_bearerStart( b);
  ASSERT( 0 == r || WIP_BERR_OK_INPROGRESS == r);
}

static void poll_creg( u8 Id );

/****************************************************************************/
/*  Function   : poll_creg_callback()                                       */
/*--------------------------------------------------------------------------*/
/*  Object : A call to "AT+CREG?" has been done, to check the registration  */
/*           status, and the answer comes back to this handler.             */
/*           Either the registration is completed, and we can actually      */
/*           open and start the bearer, or it isn't, and we shall poll      */
/*           at "AT+CREG?" command again through a timer.                   */
/*--------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                            */
/*--------------------+---+---+---+-----------------------------------------*/
/*  Rsp               | x |   |   |  AT command response                    */
/*--------------------+---+---+---+-----------------------------------------*/
/****************************************************************************/
static bool poll_creg_callback(adl_atResponse_t *Rsp) {
    ascii *rsp;
    ascii regStateString[3];
    s32 regStateInt;
    
    TRACE (( 1, "(poll_creg_callback) Enter." ));

    rsp = (ascii *)adl_memGet(Rsp->StrLength);
    wm_strRemoveCRLF(rsp, Rsp->StrData, Rsp->StrLength);
    
    wm_strGetParameterString(regStateString, Rsp->StrData, 2);
    regStateInt = wm_atoi(regStateString);
    
    if ( 1 == regStateInt || 5 ==regStateInt) {           
        TRACE (( 1, "(poll_creg_callback) Registered on GPRS network." ));
        open_and_start_bearer();
    } else { 
      /* Not ready yet, we'll check again later. Set a one-off timer. */
      adl_tmrSubscribe( FALSE, CREG_POLLING_PERIOD, ADL_TMR_TYPE_100MS,
                        poll_creg);
    }                
    return FALSE;
}

/****************************************************************************/
/*  Function   : poll_creg                                                  */
/*--------------------------------------------------------------------------*/
/*  Object : Monitor the network registration; the only way to do that is   */
/*           through an AT command, so we send that "AT+CREG?" command.     */
/*           Actual reaction will be performed by the callback              */
/*           poll_creg_callback().                                          */
/*--------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                            */
/*--------------------+---+---+---+-----------------------------------------*/
/*  Id                |   |   |   | Dummy parameter that makes the function */
/*                    |   |   |   | callable by a timer's adl_tmrSubscribe()*/
/*--------------------+---+---+---+-----------------------------------------*/
/****************************************************************************/
static void poll_creg( u8 Id ) {
  adl_atCmdCreate( "AT+CREG?", FALSE, poll_creg_callback, ADL_STR_CREG, NULL);
}

/***************************************************************************/
/*  Function   : evh_sim                                                   */
/*-------------------------------------------------------------------------*/
/*  Object     : sim events:                                               */
/*               when SIM initialisation is completed, check the registra- */
/*               tion status; poll_creg()'s callback will actually         */
/*               open the bearer once registration is completed.           */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  event             | X |   |   |SIM event                               */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static void evh_sim( u8 event) {
  TRACE (( 1, "(evh_sim) Enter." ));
  if( ADL_SIM_EVENT_FULL_INIT == event) {
    poll_creg( 0); /* argument 0 is dummy, see poll_reg() "Object" comment */
  }
}


/***************************************************************************/
/*  Function   : cfg_gprs                                                  */
/*-------------------------------------------------------------------------*/
/*  Object     : initialize GPRS connection, then launch entry_point() on  */
/*               success.                                                  */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  entry_point       | X |   |   |Function run after successful connection*/
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void cfg_gprs ( void (* entry_point)(void)) {
  TRACE (( 1, "(cfg_gprs) Enter." ));
  appli_entry_point = entry_point;
  adl_simSubscribe( evh_sim, GPRS_PINCODE);
}

