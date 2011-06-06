/*****************************************************************************/
/*  entry_point.c   -  Copyright Wavecom S.A. (c) 2006                       */
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


/****************************************************************************
 * File    :   entry_point.c                                                        
 *
 * Very basic TCP client application:
 * - Connects a TCP client socket to the TCP server on host [PEER_STRADDR]
 *   on port [PEER_PORT]
 * - Sends all data in [snd_buffer], possibly in several steps, by using
 *   [WIP_CEV_WRITE] events to control data flow.
 * - Puts everything received by the socket into [rcv_buffer], until
 *   the table is full.
 */

/*
 * $LogWavecom:$
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  27.10.06 | FFT    | 1.0            | Initial revision.                  
 * ----------+--------+----------------+-------------------------------------
 *  26.02.09 | MBU    | 1.1            | ANO52586: [OAT] unable to compile the
 *           |        |                | TCP_CLIENT sample application in 
 *           |        |                | Microsoft Visual C++
 * ----------+--------+----------------+-------------------------------------
 */

/***************************************************************************/
/*  Defines                                                                */
/***************************************************************************/

#define PEER_STRADDR    "192.168.1.5"
#define PEER_PORT       2000
#define RCV_BUFFER_SIZE 10240

#include "adl_global.h"
#include "wip.h"

/***************************************************************************/
/*  Globals                                                                */
/***************************************************************************/

/* Whatever data I want to send in [DST_FILENAME]. */
static u8 snd_buffer [] = {
  "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Nunc\n"
  "velit nisl, rhoncus at, tristique et, egestas in, nulla. Sed a\n"
  "pede quis orci sollicitudin euismod. Donec pulvinar tortor at\n"
  "neque. Donec iaculis. Curabitur varius ornare dui. Nullam auctor\n"
  "nisl sit amet ipsum. Etiam urna massa, blandit dapibus, varius\n"
  "ac, rhoncus sit amet, urna. Suspendisse potenti. Curabitur et leo\n"
  "non sapien tristique laoreet. Integer rutrum sapien scelerisque\n"
  "velit. Nunc at tellus. Fusce quis velit. Nunc ornare tortor. In\n"
  "hac habitasse platea dictumst. Sed pretium tincidunt\n"
  "lacus. Nullam lorem mauris, interdum nec, pretium non, euismod\n"
  "sit amet, mauris. Morbi hendrerit quam pretium sapien placerat\n"
  "lobortis. Curabitur consectetuer. Sed dolor. Duis varius enim vel\n"
  "tortor.\n"
  "\n"
  "Phasellus nulla risus, consequat id, vestibulum ut, elementum ac,\n"
  "urna. Vestibulum auctor magna tempor augue. Morbi vel purus eget\n"
  "ligula vulputate eleifend. Quisque urna sem, sagittis a, ornare\n"
  "ac, hendrerit adipiscing, lectus. Sed a magna sit amet sem\n"
  "placerat malesuada. Nunc suscipit est viverra tellus. Morbi sit\n"
  "amet massa quis est ultricies lobortis. Sed pede diam,\n"
  "condimentum sed, malesuada ornare, eleifend eget, tellus. Class\n"
  "aptent taciti sociosqu ad litora torquent per conubia nostra, per\n"
  "inceptos hymenaeos. Proin cursus justo vitae nibh.\n"
  "\n"
  "Aenean vel elit non nisi bibendum lobortis. Cras porttitor odio\n"
  "eu nulla. Aliquam id orci. Phasellus sed nisi. Lorem ipsum dolor\n"
  "sit amet, consectetuer adipiscing elit. Proin id arcu. Praesent\n"
  "imperdiet diam vel quam. Sed lobortis placerat velit. Vestibulum\n"
  "sit amet urna sit amet sem malesuada dapibus. Sed in ipsum in\n"
  "magna nonummy porttitor. Sed nisl. In eu ante non nibh elementum\n"
  "facilisis. Phasellus non felis vel elit lacinia tempus. Fusce\n"
  "aliquet. Cum sociis natoque penatibus et magnis dis parturient\n"
  "montes, nascetur ridiculus mus. Donec sit amet arcu. Curabitur\n"
  "rutrum, erat ac viverra porta, libero pede bibendum lectus, ac\n"
  "tempor dui tellus quis tellus. Nulla interdum. Maecenas tortor\n"
  "lectus, faucibus quis, blandit pulvinar, auctor eu, ligula. Nunc\n"
  "leo enim, sodales congue, laoreet ac, pellentesque vitae, diam.\n"};

/* How many bytes of [buffer] have already been sent. */
static int snd_offset = 0;

static char rcv_buffer [RCV_BUFFER_SIZE];
static int  rcv_offset = 0;

/***************************************************************************/
/*  Function prototypes                                                    */
/***************************************************************************/
static void evh( wip_event_t *ev, void *ctx);


/***************************************************************************/
/*  Function   : appli_entry_point                                         */
/*-------------------------------------------------------------------------*/
/*  Object     : Called once the WIP IP stack is fully initialized.        */
/*               This is the starting point of user applications.          */
/*-------------------------------------------------------------------------*/
/***************************************************************************/
void appli_entry_point() {
  wip_channel_t socket;
  wip_debug( "[SAMPLE]: connecting to client %s:%i...\n", PEER_STRADDR, PEER_PORT);
  socket = wip_TCPClientCreate( PEER_STRADDR, PEER_PORT, evh, NULL);
  if( ! socket) { wip_debug( "[SAMPLE] Can't connect\n"); return; }
}


/***************************************************************************/
/*  Function   : evh_data                                                  */
/*-------------------------------------------------------------------------*/
/*  Object     : Handling events happenning on the TCP client socket.      */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  ev                | X |   |   |  WIP event                             */
/*--------------------+---+---+---+----------------------------------------*/
/*  ctx               | X |   |   |  user data (unused)                    */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
static void evh( wip_event_t *ev, void *ctx) {
  
  switch( ev->kind) {

  case WIP_CEV_OPEN: {
    wip_debug ("[SAMPLE] Connection established successfully\n");
    break;
  }
  case WIP_CEV_READ: {
    int nread;
    wip_debug ("[SAMPLE] Some data arrived\n");
    nread = wip_read( ev->channel, rcv_buffer + rcv_offset, 
                      sizeof( rcv_buffer) - rcv_offset);
    if( nread < 0) { wip_debug( "[SAMPLE] read error %i\n", nread); return; }
    rcv_offset += nread;
    if( rcv_offset == sizeof( rcv_buffer)) {
      wip_debug( "[SAMPLE] Reception capacity exceeded, won't read more\n");
    } else {
      wip_debug( "[SAMPLE] Wrote %i bytes of data from network to rcv_buffer. "
                 "%i bytes remain available in rcv_buffer\n",
                 nread, sizeof( rcv_buffer) - rcv_offset);
    }
    break;
  }
  case WIP_CEV_WRITE: {
    int nwrite;
    wip_debug ("[SAMPLE] Can send more data\n");
    nwrite = wip_write( ev->channel, snd_buffer + snd_offset,
                        sizeof( snd_buffer) - snd_offset);
    if( nwrite < 0) { wip_debug( "[SAMPLE] write error %i\n", nwrite); return; }
    snd_offset += nwrite;
    if( snd_offset == sizeof( snd_buffer)) {
      wip_debug( "[SAMPLE] Everything has been sent, won't send more.\n");
    } else {
      wip_debug( "[SAMPLE] Wrote %i bytes. "
                 "%i bytes left to send in snd_buffer\n",
                 nwrite, sizeof( snd_buffer) - snd_offset);
    }
    break;
  }
  case WIP_CEV_ERROR: {
    wip_debug( "[SAMPLE] Error %i on socket. Closing.\n", 
               ev->content.error.errnum);
    wip_close( ev->channel);
    break;
  }
  case WIP_CEV_PEER_CLOSE: {
    wip_debug( "[SAMPLE] Connection closed by peer\n");
    wip_close( ev->channel);
    break;
  }
  }
}
