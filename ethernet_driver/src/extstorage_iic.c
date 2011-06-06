/********************************************************************************************/
/*  ext_storage_iic.c   -  Copyright Wavecom S.A. (c) 2004                                  */
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

/***************************************************************************/
/*  File       : ext_storage_iic.c                                         */
/*-------------------------------------------------------------------------*/
/*  Object     : Customer application                                      */
/*                                                                         */
/*  contents   : ADL External Storage Sample                               */
/*                                                                         */
/*
    $LogWavecom: U:\projet\mmi\pvcsarch\archives\open-at\SAMPLES\adl\External_Storage\src\ext_storage.c-arc $
 * --------------------------------------------------------------------------
 *  Date     | Author | Revision       | Description
 * ----------+--------+----------------+-------------------------------------
 *  19.12.05 | DPO    | 1.1            | *   Resolution for 31871: External 
 *           |        |                | storage sample update              
 * ----------+--------+----------------+-------------------------------------
 *  24.08.04 | DPO    | 1.0            | Initial revision.                  
 * ----------+--------+----------------+-------------------------------------
*/
/****************************************************************************/

/* includes */
#include "adl_global.h"
#include "drv_m24cxx.h"

/***************************************************************************/
/*  Mandatory variables                                                    */
/*-------------------------------------------------------------------------*/
/*  wm_apmCustomStackSize                                                  */
/*-------------------------------------------------------------------------*/
/***************************************************************************/
#ifdef _STAND_ALONE_E2PROM
const u16 wm_apmCustomStackSize = 2048*3;
#endif /* _STAND_ALONE_E2PROM */
/* consts */

// Used I2C E2P chip
#define USED_E2P_I2C_CHIP   DRV_I2C_E2P_M24C00

// MAX data length
#define MAX_DATA_LENGTH ( 1 << (USED_E2P_I2C_CHIP))

// Max address sizes
static const u32 ChipMaxSizes = DRV_I2C_E2P_M24CXX_READ_MAX_LENGTH(USED_E2P_I2C_CHIP);
// Max write sizes
static const u32 ChipMaxWriteSizes = DRV_I2C_E2P_M24CXX_WRITE_MAX_LENGTH(USED_E2P_I2C_CHIP);

adl_ioDefs_t Power_GPIO_Config_Init[] = {
    (ADL_IO_GPIO | 8 | ADL_IO_DIR_OUT | ADL_IO_LEV_LOW) };
    
adl_ioDefs_t Power_GPIO_Config[] = {
    (ADL_IO_GPIO | 8 ) };


adl_busAccess_t *I2CBus_Access;

// Read buffer
static drv_Read_I2C_E2P_M24CXX_t *ReadBuff = NULL; 
// Write buffer
static drv_Write_I2C_E2P_M24CXX_t *WriteBuff;

// IIC Bus Driver Handle
u32 DRV_HANDLE;
// E2PROM Power Pin Handle
s32 GPIO_E2PROM_HANDLE;

// Trace level
u8 drv_TraceLevel;

/***************************************************************************/
/*  Function   : es_delay                                                  */
/*-------------------------------------------------------------------------*/
/*  Objet      : set delay                                                 */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  paras             |   |   |   |  delay time (in ms)                    */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void es_delay(u32 time)
{
    wip_delay(time);
}
    

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
u32 drv_I2C_E2P_M24CXX_Init ( void )
{
    u32 length = ((USED_E2P_I2C_CHIP > DRV_I2C_E2P_M24C16)?16:8);
    
    adl_busI2CSettings_t I2CSettings =
    {
        (0xA0 >> 1),            // ChipAddress
        ADL_BUS_I2C_CLK_STD     // Clk_Speed 
    };

    
    TRACE((1, "drv_I2C_E2P_M24CXX_Init: %d", length));
    /* Open I2C bus */
    DRV_HANDLE = adl_busSubscribe ( ADL_BUS_I2C,
                                    1,
                                    &I2CSettings );
 
                                   
    adl_busIOCtl(DRV_HANDLE, ADL_BUS_CMD_SET_ADD_SIZE, &length); 

    GPIO_E2PROM_HANDLE = adl_ioSubscribe(1, Power_GPIO_Config_Init,
        0, 0, 0);
    
    return DRV_HANDLE; 
}

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
void drv_I2C_E2P_M24CXX_Close ( void )
{
    TRACE (( drv_TraceLevel, "M24CXX Driver unsubscription" ));
        
    /* Close I2C bus */
    adl_busUnsubscribe ( DRV_HANDLE );
    adl_ioUnsubscribe( GPIO_E2PROM_HANDLE );

    DRV_HANDLE = ERROR;
}

/***************************************************************************/
/*  Function   : drv_I2C_E2P_M24CXX_ComputeBusAccess                       */
/*-------------------------------------------------------------------------*/
/*  Object     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
// Bus access structure compute function
static adl_busAccess_t * drv_I2C_E2P_M24CXX_ComputeBusAccess ( u32 Address )
{
    // Compute Bus Access structure from provided opcode & address
    static adl_busAccess_t iicBus_Access = { 0, 0 };
    u8 AddressMax = 32;
    u8 length = ((USED_E2P_I2C_CHIP > DRV_I2C_E2P_M24C16)?16:8);
    
    TRACE((1, "drv_I2C_E2P_M24CXX_ComputeBusAccess: %d", length));
    // Check if address is required
    if ( Address != 0xFFFF )
    {
        // Set address
        iicBus_Access.Address = Address << ( AddressMax - length );
    }
    else
    {
        // Reset address
        length = iicBus_Access.Address = 0;
    }
    
    TRACE (( drv_TraceLevel, "[M24CXX Driver] compute access (address : %x (length %d))", iicBus_Access.Address, length ));
    
    // Return structure
    return &iicBus_Access;
}

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
/*  sRet              | x |   |   | Reading result OK / ERRROR             */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
u32 drv_I2C_E2P_M24CXX_Read ( drv_Read_I2C_E2P_M24CXX_t * pRead,
                              AsyncReadCtxt_t * AsyncReadCtxt )
{
    s32 sRet;
    I2CBus_Access = drv_I2C_E2P_M24CXX_ComputeBusAccess ( pRead->Address );
    
    TRACE (( drv_TraceLevel, "M24CXX Driver read" ));
    
    adl_ioWriteSingle ( GPIO_E2PROM_HANDLE, Power_GPIO_Config, 1 );
    
    es_delay(1000);
    TRACE (( drv_TraceLevel, "ADL Bus read" ));

    // I2C bus read
    sRet = adl_busRead( DRV_HANDLE,
                  I2CBus_Access,
                  pRead->Length,
                  pRead->Data);
    if (sRet < OK)
    {
        TRACE((drv_TraceLevel, "BusReadError: %d", sRet));
    }

//    adl_ioWriteSingle ( GPIO_E2PROM_HANDLE, Power_GPIO_Config, 0 );
    return sRet;
}

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
/*  sRet              | x |   |   | Writing result OK / ERRROR             */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
u32 drv_I2C_E2P_M24CXX_Write ( drv_Write_I2C_E2P_M24CXX_t * pWrite )
{
    u32 sRet;
    u32 Position = 0;
    WriteBuff = pWrite;
    I2CBus_Access = drv_I2C_E2P_M24CXX_ComputeBusAccess ( WriteBuff->Address );
    
    TRACE (( drv_TraceLevel, "M24CXX Driver write" ));
    DUMP ( drv_TraceLevel, WriteBuff->Data, WriteBuff->Length );


    adl_ioWriteSingle ( GPIO_E2PROM_HANDLE, Power_GPIO_Config, 1 );

    es_delay(1000);
     
    for (Position; Position < WriteBuff->Length; Position++)
    {
        I2CBus_Access = 
            drv_I2C_E2P_M24CXX_ComputeBusAccess ( WriteBuff->Address + Position);
        // I2C bus write 
        sRet = adl_busWrite ( DRV_HANDLE,
                       I2CBus_Access,
                       1,
                       &WriteBuff->Data[Position] );
        if (sRet < 0)
        {
            break;
        }
        es_delay(20);
    }

    es_delay(1000);

//    adl_ioWriteSingle ( GPIO_E2PROM_HANDLE, Power_GPIO_Config, 0 );
    return sRet;
}


/***************************************************************************/
/*  Function   : drv_e2pInit                                               */
/*-------------------------------------------------------------------------*/
/*  Objet      :                                                           */
/*                                                                         */
/*  Return     :                                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*                    |   |   |   |                                        */
/***************************************************************************/
s32 drv_e2pInit(void)
{
    drv_TraceLevel = 2;
    
    TRACE (( drv_TraceLevel, "External Storage init" ));
    
    // Init external memory driver
    if ( drv_I2C_E2P_M24CXX_Init() )
    {
        TRACE (( drv_TraceLevel, "External Memory driver subscribed" ));
        
        // Subscribe to READ/WRITE/SET MODE commands
        return OK;
    }
    return ERROR;
}
