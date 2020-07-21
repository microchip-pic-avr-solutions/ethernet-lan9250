/*
    \file   lan9250.h

    \brief  LAN9250 Ethernet controller driver header file.

    (c) 2018 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip software and any
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party
    license terms applicable to your use of third party software (including open source software) that
    may accompany Microchip software.

   THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER  
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
    FOR A PARTICULAR PURPOSE.

   IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
    SOFTWARE.
*/

#ifndef __LAN9250_H
#define __LAN9250_H

typedef enum
{
    RX_DATA_FIFO                        = 0x00,
    TX_DATA_FIFO                        = 0x20,
    RX_STATUS_FIFO                      = 0x40,
    RX_STATUS_PEEK                      = 0x44,
    TX_STATUS_FIFO                      = 0x48,
    TX_STATUS_PEEK                      = 0x4C,
            
    ID_REV                              = 0x050,            
    IRQ_CFG                             = 0x054,
    INT_STS                             = 0x058,
    INT_EN                              = 0x05C,
    BYTE_TEST                           = 0x064,
            
    //Host MAC and FIFO Interface Logic Registers
    FIFO_INT                            = 0x068, 
    RX_CFG                              = 0x06C,
    TX_CFG                              = 0x070,
    HW_CFG                              = 0x074,
    RX_DP_CTRL                          = 0x078,
    RX_FIFO_INF                         = 0x07c,
    TX_FIFO_INF                         = 0x080,
    PMT_CTRL                            = 0x084,
    GPT_CFG                             = 0x08C,
    GPT_CNT                             = 0x090,
    FREE_RUN                            = 0x09c,
    RX_DROP                             = 0x0A0,
    MAC_CSR_CMD                         = 0x0A4,
    MAC_CSR_DATA                        = 0x0A8,
    AFC_CFG                             = 0x0AC,
    HMAC_RX_LPI_TRANSISTION             = 0x0B0,
    HMAC_RX_LPI_TIME                    = 0x0B4,
    HMAC_TX_LPI_TRANSISTION             = 0x0B8,
    HMAC_TX_LPI_TIME                    = 0x0BC,
            
    //1588 Registers
    LAN_1588_CMD_CTL                    = 0x100,
    LAN_1588_GENERAL_CONFIG             = 0x104,
    LAN_1588_INT_STS                    = 0x108,
    LAN_1588_INT_EN                     = 0x10C,
    LAN_1588_CLOCK_SEC                  = 0x110,
    LAN_1588_CLOCK_NS                   = 0x114,
    LAN_1588_CLOCK_SUBNS                = 0x118,
    LAN_1588_CLOCK_RATE_ADJ             = 0x11C,
    LAN_1588_CLOCK_TEMP_RATE_ADJ        = 0x120,
    LAN_1588_CLOCK_TEMP_RATE_DURATION   = 0x124,
    LAN_1588_CLOCK_STEP_ADJ             = 0x128,
    LAN_1588_CLOCK_TARGET_SEC_A         = 0x12C,
    LAN_1588_CLOCK_TARGET_NS_A          = 0x130,
    LAN_1588_CLOCK_TARGET_RELOAD_SEC_A  = 0x134,
    LAN_1588_CLOCK_TARGET_RELOAD_NS_A   = 0x138,
    LAN_1588_CLOCK_TARGET_SEC_B         = 0x13C,
    LAN_1588_CLOCK_TARGET_NS_B          = 0x140,
    LAN_1588_CLOCK_TARGET_RELOAD_SEC_B  = 0x144,
    LAN_1588_CLOCK_TARGET_RELOAD_NS_B   = 0x148,
    LAN_1588_USER_MAC_HI                = 0x14C,
    LAN_1588_USER_MAC_LO                = 0x150,
    LAN_1588_BANK_PORT_GPIO_SEL         = 0x154,
         
    //BANK 0
    LAN_1588_LATENCY                    = 0x158,
    LAN_1588_ASYM_PEERDLY               = 0x15C,
    LAN_1588_CAP_INFO                   = 0x160,
                   
    //BANK 1
    LAN_1588_RX_PARSE_CONFIG            = 0x158,
    LAN_1588_RX_TIMESTAMP_CONFIG        = 0x15C,
    LAN_1588_RX_TS_INSERT_CONFIG        = 0x160,
    LAN_1588_RX_FILTER_CONFIG           = 0x168,
    LAN_1588_RX_INGRESS_SEC             = 0x16C,
    LAN_1588_RX_INGRESS_NS              = 0x170,
    LAN_1588_RX_MSG_HEADER              = 0x174,
    LAN_1588_RX_PDREQ_SEC               = 0x178,
    LAN_1588_RX_PDREQ_NS                = 0x17C,
    LAN_1588_RX_PDREQ_CF_LOW            = 0x184,
    LAN_1588_RX_CHKSUM_DROPPED_CNT      = 0x188,
    LAN_1588_RX_FILTERED_CNT            = 0x18C,
            
    //BANK 2
    LAN_1588_TX_PARSE_CONFIG            = 0x158,
    LAN_1588_TX_TIMESTAMP_CONFIG        = 0x15C,
    LAN_1588_TX_MOD                     = 0x164,
    LAN_1588_TX_MOD2                    = 0x168,
    LAN_1588_TX_EGRESS_SEC              = 0x16C,
    LAN_1588_TX_EGRESS_NS               = 0x170,
    LAN_1588_TX_MSG_HEADER              = 0x174,
    LAN_1588_TX_DREQ_SEC                = 0x178,
    LAN_1588_TX_DREQ_NS                 = 0x17C,
    LAN_1588_TX_ONE_STEP_SYNC_SEC       = 0x180,
    
    //BANK 3
    LAN_1588_GPIO_CAP_CONFIG            = 0x15C,
    LAN_1588_GPIO_RE_CLOCK_SEC_CAP_x    = 0x16C,
    LAN_1588_GPIO_RE_CLOCK_NS_CAP_x     = 0x170,
    LAN_1588_GPIO_FE_CLOCK_SEC_CAP_x    = 0x178,
    LAN_1588_GPIO_FE_CLOCK_NS_CAP_x     = 0x17C,  
            
    GPIO_CFG                            = 0x1E0,
    GPIO_DATA_DIR                       = 0X1E4,
    GPIO_INT_STS_EN                     = 0X1E8,
    RESET_CTL                           = 0x1F8,
               
}lan9250_registers_t;

#define TX_CMD_A_INT_ON_COMP                0X80000000
#define TX_CMD_A_BUFFER_END_ALIGN           0x03000000
#define TX_CMD_A_4_BYTE_ALIGN               0x00000000
#define TX_CMD_A_16_BYTE_ALIGN              0x01000000
#define TX_CMD_A_32_BYTE_ALIGN              0x02000000
#define TX_CMD_A_DATA_OFFSET                0x00000000 //Data-offset, can range from 0-31 bytes
#define TX_CMD_A_FIRST_SEG                  0x00002000
#define TX_CMD_A_LAST_SEG                   0x00001000
#define TX_CMD_A_BUFFER_SIZE                0x000007FF
#define TX_CMD_B_PACKET_TAG                 0x05200000 //Using same packet tag for all frames
#define TX_CMD_B_CHECKSUM_ENABLE            0x00004000
#define TX_CMD_B_ADD_CRC_DISABLE            0x00002000
#define TX_CMD_B_DISABLE_PADDING            0x00001000
#define TX_CMD_B_PACKET_BYTE_LENGTH         0x000007FF

// Chip ID and Revision
#define ID_REV_CHIP_ID                      0xFFFF0000
#define ID_REV_CHIP_REVISION                0x0000FFFF //default value is dependent on device revision

// Interrupt configuration register
#define IRQ_CFG_INT_DEAS                    0xA0000000 //Setting interrupt de-assertion interval to 100microseconds
#define IRQ_CFG_INT_DEAS_CLR                0x00004000
#define IRQ_CFG_INT_DEAS_STS                0x00002000
#define IRQ_CFG_IRQ_INT                     0x00001000
#define IRQ_CFG_IRQ_EN                      0x00000100
#define IRQ_CFG_INT_POL                     0x00000010
#define IRQ_CFG_IRQ_CLK_SELECT              0x00000002
#define IRQ_CFG_IRQ_TYPE                    0x00000001

// Interrupt status register
#define SW_INT                              0x80000000
#define READY                               0x40000000
#define STS_1588_EVNT                       0x20000000
#define PHY_INT                             0x04000000
#define TXSTOP_INT                          0x02000000
#define RXSTOP_INT                          0x01000000
#define RXDFH_INT                           0x00800000
#define TX_IOC                              0x00200000
#define RXD_INT                             0x00100000
#define GPT_INT                             0x00080000
#define PME_INT                             0x00020000
#define TXSO                                0x00010000
#define RWT                                 0x00008000
#define RXE                                 0x00004000
#define TXE                                 0x00002000
#define GPIO                                0x00001000
#define TDFO                                0x00000400
#define TDFA                                0x00000200
#define TSFF                                0x00000100
#define TSFL                                0x00000080
#define RXDF_INT                            0x00000040
#define RSFF                                0x00000010
#define RSFL                                0x00000008

//Interrupt enable register
#define SW_INT_EN                           0x80000000
#define RAEDY_EN                            0x40000000
#define EN_1588_EVNT                        0x20000000
#define PHY_INT_EN                          0x04000000
#define TXSTOP_INT_EN                       0x02000000
#define RXSTOP_INT_EN                       0x01000000
#define RXDFH_INT_EN                        0x00800000
#define TIOC_INT_EN                         0x00200000
#define RXD_INT_EN                          0x00100000
#define GPT_INT_EN                          0x00080000
#define PME_INT_EN                          0x00020000
#define TXSO_EN                             0x00010000
#define RWT_INT_EN                          0x00008000
#define RXE_INT_EN                          0x00004000
#define TXE_INT_EN                          0x00002000
#define GPIO_EN                             0x00001000
#define TDFO_EN                             0x00000400
#define TDFA_EN                             0x00000200
#define TSFF_EN                             0x00000100
#define TSFL_EN                             0x00000080
#define RXDF_INT_EN                         0x00000040
#define RSFF_EN                             0x00000010
#define RSFL_EN                             0x00000008

// FIFO level interrupt register
#define TX_DATA_AVAILABLE_LEVEL             0x48000000
#define TX_STATUS_LEVEL                     0x00600000
#define RX_STATUS_LEVEL                     0x00000000

// Receive configuration register
#define RX_EA                               0xC0000000
#define RX_DMA_CNT                          0x06000000
#define RX_DUMP                             0x00008000
#define RX_RXDOFF                           0x00000400 // Four byte offset

// Transmit configuration register
#define TXS_DUMP                            0x00008000
#define TXD_DUMP                            0x00004000
#define TXSAC                               0x00000004
#define TX_ON                               0x00000002
#define STOP_TX                             0x00000001

// Hardware configuration register
#define HCR_READY                           0x08000000
#define AMDIX_EN_STRAP_STATE                0x02000000
#define MBO                                 0x00100000
#define TX_FIF_SiZE                         0x00050000 // FIFO SIZE value

// Receive datapath control register
#define RX_FFWD                             0x8000000

// RX FIFO information register
#define RXSUSED                             0x00FF0000
#define RXDUSED                             0x0000FFFF

// TX FIFO information register
#define TXSUSED                             0x00FF0000
#define TXDUSED                             0x0000FFFF
 
// Power management control register
#define PM_MODE                             0xE0000000
#define PM_SLEEP_EN                         0x10000000
#define PM_WAKE                             0x08000000
#define LED_DIS                             0x04000000
#define DIS_1588                            0x02000000
#define DIS_1588_TSU                        0x00400000
#define HMAC_DIS                            0x00080000
#define HMAC_SYS_ONLY_DIS                   0x00040000
#define ED_STS                              0x00010000
#define ED_EN                               0x00004000
#define WOL_EN                              0x00000200
#define PME_TYPE                            0x00000040
#define WOL_STS                             0x00000020
#define PME_IND                             0x00000008
#define PME_POL                             0x00000004
#define PME_EN                              0x00000002
#define PME_READY                           0x00000001


// Host MAC CSR interface command register
#define HMAC_CSR_BUSY                       0x80000000
#define R_nW                                0x40000000
#define CSR_ADDRESS                         0x000000FF

// Host MAC automatic flow control configuration register
#define AFC_HI                              0x00FF0000
#define AFC_LO                              0x0000FF00
#define BACK_DUR                            0x000000F0
#define FCMULT                              0x00000008
#define FCBRD                               0x00000004
#define FCADD                               0x00000002
#define FCANY                               0x00000001

// General purpose timer configuration register
#define TIMER_EN                            0x20000000
#define GPT_LOAD                            0x0000FFFF

// General purpose timer count register
#define GPT_CNT                             0x0000FFFF

// LED configuration register
#define LED_FUN                             0x00000700
#define LED_EN                              0x00000007

// GPIO configuration register
#define GPIO_CH_SEL                         0x07000000
#define GPIO_POL                            0x00070000
#define GPIO_OE                             0x00000700
#define GPIOBUF                             0x00000007

// GPIO data & direction register
#define GPIODIR                             0x00070000
#define GPIOD                               0x00000007

// GPIO interrupt status and enable register
#define GPIO_INT_EN                         0x00070000
#define GPIO_INT                            0x00000007

// Host MAC addressable registers
typedef enum
{
    HMAC_CR                                 = 0x01,
    HMAC_ADDRH                              = 0x02,
    HMAC_ADDRL                              = 0x03,
    HMAC_HASHH                              = 0x04,
    HMAC_HASHL                              = 0x05,
    HMAC_MII_ACC                            = 0x06,
    HMAC_MII_DATA                           = 0x07,
    HMAC_FLOW                               = 0x08,
    HMAC_VLAN1                              = 0x09,
    HMAC_VLAN2                              = 0x0A,
    HMAC_WUFF                               = 0x0B,
    HMAC_WUCSR                              = 0x0C,
    HMAC_COE_CR                             = 0x0D,
    HMAC_EEE_TW_TX_SYS                      = 0x0E,
    HMAC_EEE_TX_LPI_REQ_DELAY               = 0x0F,
         
}lan_mac_registers_t;

// Host MAC control register
#define RXALL                               0x80000000
#define HMAC_EEE_ENABLE                     0x02000000
#define RCVOWN                              0x00400000
#define LOOPBK                              0x00200000
#define FDPX                                0x00100000
#define MCPAS                               0x00080000
#define PRMS                                0x00040000
#define INVFILT                             0x00020000
#define PASSBAD                             0x00010000
#define HO                                  0x00008000
#define HPFILT                              0x00002000
#define BCAST                               0x00000800
#define DISTRY                              0x00000400
#define PADSTR                              0x00000100
#define BOLMT                               0x000000C0
#define DFCHK                               0x00000020
#define TXEN                                0x00000008
#define RXEN                                0x00000004

// Host MAC address high register
#define PHYSICAL_ADDRESS                    0x0000FFFF

// Host MAC MII access register
#define PHY_ADDR                            0x0000F800
#define MIIRDA                              0x000007C0
#define MIIWnR                              0x00000002
#define MIIBZY                              0x00000001

// Host MAC MII data register
#define MII_DATA                            0x0000FFFF

// Host MAC flow control register
#define FCPT                                0xFFFF0000
#define FCPASS                              0x00000004
#define FCEN                                0x00000002
#define FCBSY                               0x00000001

// Host MAC VLAN1 tag register
#define VTI1                                0x0000FFFF

// Host MAC VLAN2 tag register
#define VTI2                                0xFFFF0000

// Host MAC wake-up control and status register
#define WFF_PTR_RST                         0x80000000
#define GUE                                 0x00000200
#define WOL_WAIT_SLEEP                      0x00000100
#define PFDA_FR                             0x00000080
#define WUFR                                0x00000040
#define MPR                                 0x00000020
#define BCAST_FR                            0x00000010
#define PFDA_EN                             0x00000008
#define WUEN                                0x00000004
#define MPEN                                0x00000002
#define BCST_EN                             0x00000001

// Host MAC checksum offload engine control register
#define TX_COE_EN                           0x00010000
#define RX_COE_MODE                         0x00000002
#define RX_COE_EN                           0x00000001

// Host MAC EEE time wait TX system register
#define TX_DELAY_AFTER_TX_LPI_REMOVAL       0x00FFFFFF

// MDIO Manageable Device (MMD) register
// Device address in decimal
#define PCS                                 3
#define AUTO_NEGOTIATION                    7
#define VENDOR_SPECIFIC                     30
//Index in decimal
#define PHY_PCS_CTL1                        0
#define PHY_PCS_STAT1                       1
#define PHY_PCS_MMD_PRESENT1                5
#define PHY_PCS_MMD_PRESENT2                6
#define PHY_EEE_CAP                         20
#define PHY_EEE_WAKE_ERR                    22
#define PHY_AN_MMD_PRESENT1                 5
#define PHY_AN_MMD_PRESENT2                 6
#define PHY_EEE_ADV                         60
#define PHY_EEE_LPV_ADV                     61
#define PHY_VEND_SPEC_MMD1_DEVID1           2
#define PHY_VEND_SPEC_MMD1_DEVID2           3
#define PHY_VEND_SPEC_MMD1_PRESENT1         5
#define PHY_VEND_SPEC_MMD1_PRESENT2         6
#define PHY_VEND_SPEC_MMD1_STAT             8
#define PHY_VEND_SPEC_MMD1_PKG_ID1          14
#define PHY_VEND_SPEC_MMD1_PKG_ID2          15

// PHY PCS control 1 register
#define CLOCK_STOP_ENABLE

// PHY PCS status 1 register
#define PHY_TX_LPI_RECEIVED                 0x0800
#define PHY_RX_LPI_RECEIVED                 0x0400
#define PHY_TX_LPI_INDICATION               0x0200
#define PHY_RX_LPI_INDICATION               0x0100
#define PHY_CLOCK_STOP_CAPABLE              0x0040

// PHY PCS MMD devices present 1 register
#define PHY_PCS_MMD_PRESENT1_AUTO_NEGOTIATION_PRESENT       0x0080
#define PHY_PCS_MMD_PRESENT1_TC_PRESENT                     0x0040
#define PHY_PCS_MMD_PRESENT1_DTE_XS_PRESENT                 0x0020
#define PHY_PCS_MMD_PRESENT1_PHY_XS_PRESENT                 0x0010
#define PHY_PCS_MMD_PRESENT1_PCS_PRESENT                    0x0008
#define PHY_PCS_MMD_PRESENT1_WIS_PRESENT                    0x0004
#define PHY_PCS_MMD_PRESENT1_PMD_PMA_PESENT                 0x0002
#define PHY_PCS_MMD_PRESENT1_CLAUSE22_REGISTERS_PRESENT     0x0001
      
// PHY PCS MMD devices present 2 register
#define PCS_VENDOR_SPECIFIC_DEVICE_2_PRESENT                0x8000
#define PCS_VENDOR_SPECIFIC_DEVICE_1_PRESENT                0x4000
#define PCS_CLAUSE_22_EXTENSION_PRESENT                     0x2000

// PHY EEE capability register
#define PHY_EEE_CAP_10GBASE_KR_EEE                          0x0040
#define PHY_EEE_CAP_10GBASE_KX4_EEE                         0x0020
#define PHY_EEE_CAP_10GBASE_KX_EEE                          0x0010
#define PHY_EEE_CAP_10GBASE_T_EEE                           0x0008
#define PHY_EEE_CAP_1000BASE_T_EEE                          0x0004
#define PHY_EEE_CAP_100BASE_TX_EEE                          0x0002

// PHY auto-negotiation MMD devices present 1 register
#define PHY_AN_MMD_PRESENT1_AUTO_NEGOTIATION_PRESENT        0x0080
#define PHY_AN_MMD_PRESENT1_TC_PRESENT                      0x0040
#define PHY_AN_MMD_PRESENT1_DTE_XS_PRESENT                  0x0020
#define PHY_AN_MMD_PRESENT1_PHY_XS_PRESENT                  0x0010
#define PHY_AN_MMD_PRESENT1_PCS_PRESENT                     0x0008
#define PHY_AN_MMD_PRESENT1_WIS_PRESENT                     0x0004
#define PHY_AN_MMD_PRESENT1_PMD_PMA_PRESENT                 0x0002
#define PHY_AN_MMD_PRESENT1_CLAUSE_22_REGISTERS_PRESENT     0x0001

// PHY auto-negotiation MMD devices present 2 register
#define AN_VENDOR_SPECIFIC_DEVICE_2_PRESENT                 0x8000
#define AN_VENDOR_SPECIFIC_DEVICE_1_PRESENT                 0x4000
#define AN_CLAUSE_22_EXTENSION_PRESENT                      0x2000

// PHY EEE advertisement register
#define PHY_EEE_ADV_100BASE_TX_EEE                          0x0002

// PHY EEE link partner advertisement register
#define PHY_EEE_LP_ADV_10GBASE_KR_EEE                       0x0040
#define PHY_EEE_LP_ADV_10GBASE_KX4_EEE                      0x0020
#define PHY_EEE_LP_ADV_10GBASE_KX_EEE                       0x0010
#define PHY_EEE_LP_ADV_10GBASE_T_EEE                        0x0008
#define PHY_EEE_LP_ADV_1000BASE_T_EEE                       0x0004
#define PHY_EEE_LP_ADV_100BASE_TX_EEE                       0x0002

// Phy control and status registers
typedef enum
{
    PHY_BASIC_CONTROL                       = 0x00000000,
    PHY_BASIC_STATUS                        = 0x00000001,
    PHY_ID_MSB                              = 0x00000002,
    PHY_ID_LSB                              = 0x00000003,
    PHY_AN_ADV                              = 0x00000004,
    PHY_AN_LP_BASE_ABILITY                  = 0x00000005,
    PHY_AN_EXP                              = 0x00000006,
    PHY_AN_NP_TX                            = 0x00000007,
    PHY_AN_NP_RX                            = 0x00000008,
    PHY_MMD_ACCESS                          = 0x0000000D,
    PHY_MMD_ADDR_DATA                       = 0x0000000E,
    PHY_EDPD_CFG                            = 0x00000010,
    PHY_MODE_CONTROL_STATUS                 = 0x00000011,
    PHY_SPECIAL_MODES                       = 0x00000012,
    PHY_TDR_PAT_DELAY                       = 0x0000018,
    PHY_TDR_CONTROL_STAT                    = 0x00000019,
    PHY_SYM_ERR_COUNTER                     = 0x0000001A,
    PHY_SPECIAL_CONTROL_STATUS_IND          = 0x0000001B,
    PHY_CABLE_LEN                           = 0x0000001C,
    PHY_INTERRUPT_SOURCE                    = 0x0000001D,
    PHY_INTERRUPT_MASK                      = 0x0000001E,
    PHY_SPECIAL_CONTROL_STATUS              = 0x0000001F,
             
}phy_registers_t;

// Phy basic control register
#define PHY_SRST                            0x8000
#define PHY_LOOPBACK                        0x4000
#define PHY_SPEED_SEL_LSB                   0x2000
#define PHY_AN                              0x1000
#define PHY_PWR_DWN                         0x0800
#define PHY_RST_AN                          0x0200
#define PHY_DUPLEX                          0x0100
#define PHY_COL_TEST                        0x0080

// Phy basic status register
#define PHY_100BASE_T4                      0x8000
#define PHY_100BASE_X_FULL_DUPLEX           0x4000
#define PHY_100BASE_X_HALF_DUPLEX           0x2000
#define PHY_10BASE_X_FULL_DUPLEX            0x1000
#define PHY_10BASE_X_HALF_DUPLEX            0x0800
#define PHY_100BASE_T2_FULL_DUPLEX          0x0400
#define PHY_100BASE_T2_HALF_DUPLEX          0x0200
#define PHY_EXTENDED_STATUS                 0x0100
#define PHY_UNIDIRECTIONAL_ABILITY          0x0080
#define PHY_MF_PREAMBLE_SUPPRESSION         0x0040
#define PHY_AUTO_NEGOTIATION_COMPLETE       0x0020
#define PHY_REMOTE_FAULT                    0x0010
#define PHY_AUTO_NEGOTIATION_ABILITY        0x0008
#define PHY_LINK_STATUS                     0x0004
#define PHY_JABBER_DETECT                   0x0002
#define PHY_EXTENDED_CAPABILITY             0x0001

// Phy identification MSB register
#define PHY_MSB_ID                          0xFFFF

// Phy identification LSB register
#define PHY_LSB_ID                          0xFC00
#define MODEL_NUMBER                        0x03F0
#define REVISION_NUMBER                     0x000F

// Phy auto-negotiation advertisement register
#define PHY_AN_ADV_NEXT_PAGE                0x8000
#define PHY_AN_ADV_REMOTE_FAULT             0x2000
#define PHY_AN_ADV_EXTENDED_NEXT_PAGE       0x1000
#define PHY_AN_ADV_ASYMMETRIC_PAUSE         0x0800
#define PHY_AN_ADV_SYMMETRIC_PAUSE          0x0400
#define PHY_AN_ADV_100BASE_X_FULL_DUPLEX    0x0100
#define PHY_AN_ADV_100BASE_X_HALF_DUPLEX    0x0080
#define PHY_AN_ADV_10BASE_T_FULL_DUPLEX     0x0040
#define PHY_AN_ADV_10BASE_T_HALF_DUPLEX     0x0020
#define PHY_AN_ADV_SELECTOR_FIELD           0x0001

// PHY auto-negotiation link partner base page ability register
#define PHY_AN_LP_BASE_ABILITY_NEXT_PAGE                    0x8000
#define PHY_AN_LP_BASE_ABILITY_ACKNOWLEDGE                  0x4000
#define PHY_AN_LP_BASE_ABILITY_REMOTE_FAULT                 0x2000
#define PHY_AN_LP_BASE_ABILITY_EXTENDED_NEXT_PAGE           0x1000
#define PHY_AN_LP_BASE_ABILITY_ASYMMETRIC_PAUSE             0x0800
#define PHY_AN_LP_BASE_ABILITY_PAUSE                        0x0400
#define PHY_AN_LP_BASE_ABILITY_100BASE_T4                   0x0200
#define PHY_AN_LP_BASE_ABILITY_100BASE_X_FULL_DUPLEX        0x0100
#define PHY_AN_LP_BASE_ABILITY_100BASE_X_HALF_DUPLEX        0x0080
#define PHY_AN_LP_BASE_ABILITY_10BASE_T_FULL_DUPLEX         0x0040
#define PHY_AN_LP_BASE_ABILITY_10BASE_T_HALF_DUPLEX         0x0020
#define PHY_AN_LP_BASE_ABILITY_SELECTOR_FIELD               0x001F

// PHY auto-negotiation expansion register
#define RECEIVE_NEXT_PAGE_LOCATION_ABLE                     0x0040
#define RECEIVED_NEXT_PAGE_STORAGE_LOCATION                 0x0020
#define PARALLEL_DETECTION_FAULT                            0x0010
#define LINK_PARTNER_NEXT_PAGE_ABLE                         0x0008
#define NEXT_PAGE_ABLE                                      0x0004
#define PAGE_RECEIVED                                       0x0002
#define LINK_PARTNER_AUTO_NEGOTIATION_ABLE                  0x0001

// PHY auto-negotiation next page TX register
#define PHY_AN_NP_TX_NEXT_PAGE                              0x8000
#define PHY_AN_NP_TX_MASSAGEPAGE                            0x2000
#define PHY_AN_NP_TX_ACKNOWLEDGE_2                          0x1000 
#define PHY_AN_NP_TX_TOGGLE                                 0x0800
#define PHY_AN_NP_TX_MESSAGE_CODE                           0x07FF

// PHY auto-negotiation next page RX register
#define PHY_AN_NP_RX_NEXT_PAGE                              0x8000
#define PHY_AN_NP_RX_MASSAGEPAGE                            0x2000
#define PHY_AN_NP_RX_ACKNOWLEDGE_2                          0x1000 
#define PHY_AN_NP_RX_TOGGLE                                 0x0800
#define PHY_AN_NP_RX_MESSAGE_CODE                           0x07FF

// PHY MMD access control register
#define MMD_FUNCTION                                        0xC000
#define MMD_DEVICE_ADDRESS                                  0x001F

// PHYEDPD NLP / Crossover Time / EEE configuration register
#define EDPD_TX_NLP_ENABLE                                  0x8000
#define EDPD_TX_NLP_INTERVAL_TIMER_SELECT                   0x6000
#define EDPD_RX_SINGLE_NLP_WAKE_ENABLE                      0x1000
#define EDPD_RX_NLP_MAX_INTERVAL_DETECT_SELECT              0x0C00
#define PHYEEEEN                                            0x0004
#define PHY_EXTEND_CROSSOVER                                0x0002
#define EXTENDED_MANUAL_10BY100_AUTO_MDIX_CROSSOVER_TIME    0x0001

// PHY mode control / status register
#define EDPWRDOWN                                           0x2000
#define ALTINT                                              0x0040
#define ENERGYON                                            0x0002

// PHY special modes register
#define FX_MODE                                             0x0400
#define MODE                                                0x00E0
#define PHYADD                                              0x0001

// PHY TDR patterns / delay control register
#define TDR_DELAY_IN                                        0x8000
#define TDR_LINE_BREAK_COUNTER                              0x7000
#define TDR_PATTERN_HIGH                                    0x0FC
#define TDR_PATTERN_LOW                                     0x003F

// PHY TDR control / status register
#define TDR_ENABLE                                          0x8000
#define TDR_ANALOG_TO_DIGITAL_FILTER_ENABLE                 0x4000
#define TDR_CHANNEL_CABLE_TYPE                              0x0600
#define TDR_CHANNEL_STATUS                                  0x0100
#define TDR_CHANNEL_LENGTH                                  0x00FF

//  PHY Special control/status indication register
#define AMDIXCTRL                                           0x8000
#define AMDIXEN                                             0x4000
#define AMDIXSTATE                                          0x2000
#define SQEOFF                                              0x0800
#define FEFI_EN                                             0x0020
#define XPOL                                                0x0010

// PHY cable length register
#define CBLN                                                0xF000

// PHY Interrupt source flags register
#define INT9_LINK_UP                                        0x0200
#define INT7_ENERGY_ON                                      0x0080
#define INT6_AUTONEG                                        0x0040
#define INT5_REMOTE_FAULT                                   0x0020
#define INT4_LINK_DOWN                                      0x0010
#define INT3_AUTONEG_LP_ACKNOWLEDGE                         0x0008
#define INT2_PARALLEL_DETECT_FAULT                          0x0004
#define INT1_AUTONEG_PAGE_RECEIVED                          0x0002

// PHY interrupt mask register
#define INT9_MASK                                           0x0200
#define INT7_MASK                                           0x0080
#define INT6_MASK                                           0x0040
#define INT5_MASK                                           0x0020
#define INT4_MASK                                           0x0010
#define INT3_MASK                                           0x0008
#define INT2_MASK                                           0x0004
#define INT1_MASK                                           0x0002

// PHY special control/status register
#define AUTODONE                                            0x8000
#define RESERVED_DATA                                       0x0040
#define SPEED_INDICATION                                    0x001C

// Reset control register 
#define HM_AC_RST                                           0x00000020
#define PHY_RST                                             0x00000002
#define DIGITAL_RST                                         0x00000001

// Power management control register
#define PMT_CTRL_PM_MODE                                    0xE0000000
#define PMT_CTRL_PM_SLEEP_EN                                0x10000000
#define PMT_CTRL_PM_WAKE                                    0x08000000
#define PMT_CTRL_LED_DIS                                    0x04000000
#define PMT_CTRL_1588_DIS                                   0x02000000
#define PMT_CTRL_1588_TSU_DIS                               0x00400000
#define PMT_CTRL_HMAC_DIS                                   0x00080000
#define PMT_CTRL_HMAC_SYS_ONLY_DIS                          0x00040000
#define PMT_CTRL_ED_STS                                     0x00010000
#define PMT_CTRL_ED_EN                                      0x00004000
#define PMT_CTRL_WOL_EN                                     0x00000200
#define PMT_CTRL_PME_TYPE                                   0x00000040
#define PMT_CTRL_WOL_STS                                    0x00000020
#define PMT_CTRL_PME_IND                                    0x00000008
#define PMT_CTRL_PME_POL                                    0x00000004
#define PMT_CTRL_PME_EN                                     0x00000002
#define PMT_CTRL_READY                                      0x00000001

#define TX_FIFO_LOW_THRESHOLD    (1600)
#define SPI_ENABLE               (0xFF)
#define SQI_ENABLE               (0x38)
#define SPI_READ                 (0x03)
#define SPI_WRITE                (0x02)
#define PACKET_LENGTH            (0x3FFF)
#define BUFFER_SIZE              (0xFFFF)
#define INTERRUPT_STATUS_CLEAR   (0xFFFFFFFF) // Writing to 1 to clear the interrupt status register
#define WAIT_DEVICE_READY        (0x5) // Waiting for the device to be ready after a reset or read

#endif // __LAN9250_H