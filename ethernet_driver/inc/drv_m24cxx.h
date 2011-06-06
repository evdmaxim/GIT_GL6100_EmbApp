/********************************************************************************************/
/*  drv_m24cxx.h   -  Copyright Wavecom S.A. (c) 2005                                       */
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
/* File    :   drv_m24cxx.h                                                 */
/*                                                                          */
/* Scope   :   I2C ST M24CXX EEPROM driver definitions                      */
/*                                                                          */
/*
    $Log:   U:/projet/mmi/pvcsarch/archives/open-at/SAMPLES/adl/Libraries/Drivers/itf/drv_m24cxx.h-arc  $
 * 
 *    Rev 1.0   Dec 19 2005 16:54:18   DPO
 * Initial revision.
*/
/****************************************************************************/

#ifndef __DRV_STM24CXX_H__
#define __DRV_STM24CXX_H__



/********************/
/* Driver Constants */
/********************/

// Chip type
typedef enum
{
    DRV_I2C_E2P_M24C00 = 4, //  128bits E2P
    DRV_I2C_E2P_M24C01 = 7, //  1 Kbits E2P
    DRV_I2C_E2P_M24C02,     //  2 Kbits E2P
    DRV_I2C_E2P_M24C04,     //  4 Kbits E2P
    DRV_I2C_E2P_M24C08,     //  8 Kbits E2P
    DRV_I2C_E2P_M24C16,     // 16 Kbits E2P
    DRV_I2C_E2P_M24C32,     // 32 Kbits E2P
    DRV_I2C_E2P_M24C64,     // 64 Kbits E2P
} drv_I2C_E2P_M24CXX_Type_e;

// Write max length (write page length)

#define DRV_I2C_E2P_M24CXX_WRITE_MAX_LENGTH(_CHIP) \
    ((_CHIP > DRV_I2C_E2P_M24C00) ? 16 : 1)

// Address max length (in bits)
#define DRV_I2C_E2P_M24CXX_ADDR_MAX_LENGTH(_typ)    ( _typ )

// Read max length (whole memory size in bytes)
#define DRV_I2C_E2P_M24CXX_READ_MAX_LENGTH(_typ)    ( 1 << DRV_I2C_E2P_M24CXX_ADDR_MAX_LENGTH(_typ) )

/*********************************************************************/
/* Due to Memory problem, the dedicated memory space for MAC address */
/* is allocated to 7 byte, where the most significant byte is dummy  */
/* This is only used when there is memory problem occcures           */
/*********************************************************************/
/* MAC address with dummy byte */
typedef u8 wip_ethAddr_t_7BYTE[7];


/*******************/
/* Driver Settings */
/*******************/

// Driver configuration
typedef struct
{
    drv_I2C_E2P_M24CXX_Type_e   Type;           // Chip type
    u32                         ChipAddress;    // Chip Address
} drv_Settings_I2C_E2P_M24CXX_t;

// Asynchronous reading context
typedef struct
{
    u32 ByteLength; // Byte length for the reading
    u32 Port;       // port where to send the reading result
} AsyncReadCtxt_t;


/*******************************/
/* Driver Read/Write structure */
/*******************************/

typedef struct
{
    u32 Address;    // Read/Write Start Address
                    // 12 less significant bits used for M24C32
                    // 13 less significant bits used for M24C64
    u32 Length;     // Data buffer length
    u8  Data [ 1 ]; // Data buffer
} drv_Read_I2C_E2P_M24CXX_t, drv_Write_I2C_E2P_M24CXX_t;


/***************************************************************************/
/*  Function   : drv_I2C_E2P_M24CXX_Init                                   */
/*-------------------------------------------------------------------------*/
/*  Object     : Subscribe to the peripheral IIC bus                       */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
u32 drv_I2C_E2P_M24CXX_Init ( void );

/***************************************************************************/
/*  Function   : drv_I2C_E2P_M24CXX_Close                                  */
/*-------------------------------------------------------------------------*/
/*  Object     : Unsubscribe to the peripheral IIC bus                     */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void drv_I2C_E2P_M24CXX_Close( void );

/***************************************************************************/
/*  Function   : drv_I2C_E2P_M24CXX_Read                                   */
/*-------------------------------------------------------------------------*/
/*  Object     : Read on the the peripheral IIC bus                        */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  pRead             | x |   |   | Reading buffer                         */
/*--------------------+---+---+---+----------------------------------------*/
/*  AsyncReadCtxt     | x |   |   | Asynchronous Reading Context           */
/*--------------------+---+---+---+----------------------------------------*/
/*  sRet              |   | x |   | Reading result OK / ERRROR             */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
u32 drv_I2C_E2P_M24CXX_Read ( drv_Read_I2C_E2P_M24CXX_t * pRead
                             , AsyncReadCtxt_t * AsyncReadCtxt );

/***************************************************************************/
/*  Function   : drv_I2C_E2P_M24CXX_Write                                  */
/*-------------------------------------------------------------------------*/
/*  Object     : Write on the the peripheral IIC bus                       */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  pWrite            | x |   |   | Writing buffer                         */
/*--------------------+---+---+---+----------------------------------------*/
/*  sRet              |   | x |   | Writing result OK / ERRROR             */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
u32 drv_I2C_E2P_M24CXX_Write ( drv_Write_I2C_E2P_M24CXX_t * pWrite );

/***************************************************************************/
/*  Function   : busLowIrqHandler                                          */
/*-------------------------------------------------------------------------*/
/*  Objet      : IRQ low level handler for Bus asynchronous operation      */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  Source            | X |   |   |  holding command parameter             */
/*--------------------+---+---+---+----------------------------------------*/
/*  NotificationLevel | X |   |   |  holding command parameter             */
/*--------------------+---+---+---+----------------------------------------*/
/*  Data              | X |   |   |  holding command parameter             */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
bool busLowIrqHandler(adl_irqID_e Source, adl_irqNotificationLevel_e NotificationLevel, adl_irqEventData_t *Data);

/***************************************************************************/
/*  Function   : drv_e2pSetSync                                            */
/*-------------------------------------------------------------------------*/
/*  Objet      :                                                           */
/*                                                                         */
/*  Return     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
s32 drv_e2pSetSync(void);

/***************************************************************************/
/*  Function   : drv_e2pSetAsyn                                            */
/*-------------------------------------------------------------------------*/
/*  Objet      :                                                           */
/*                                                                         */
/*  Return     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
s32 drv_e2pSetAsyn(void);

s32 drv_e2pInit(void);
#endif // __DRV_STM24CXX_H__

