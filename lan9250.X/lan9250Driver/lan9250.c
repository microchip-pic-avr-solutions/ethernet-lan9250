/*
    \file   lan9250.c

    \brief  LAN9250 Ethernet controller driver source file.

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

#include "../mcc_generated_files/mcc.h"
#include "lan9250.h"
#include <stdint.h>
#include <stddef.h>
#include "memory_buffer.h"
#include "../mcc_generated_files/TCPIPLibrary/ethernet_driver.h"
#include "../mcc_generated_files/spi1_types.h"
#include "../mcc_generated_files/drivers/spi_master.h"
#include "../mcc_generated_files/TCPIPLibrary/network.h"

// Packet write in progress, not ready for transmit
#define ETH_WRITE_IN_PROGRESS           (0x0001 << 0)
// Packet complete, in queue for transmit
#define ETH_TX_QUEUED                   (0x0001 << 1)
// Flag for pool management - free or allocated
#define ETH_ALLOCATED                   (0x0001 << 2)

// adjust these parameters for the MAC...
#define RAMSIZE                         (16384)
#define MAX_TX_PACKET_SIZE              (1518)
#define MIN_TX_PACKET_SIZE              (64)
#define MAX_TX_PACKETS                  (20)

#define TX_STATUS_VECTOR_SIZE           (4)

// typical memory map for the MAC buffers
#define TXSTART                         (0x0000)
#define TXEND                           (0x1200) 
#define RXSTART                         (0x0000) 
#define RXEND                           (0x2940) 

#define MIN_TX_PACKET                   (MIN_TX_PACKET_SIZE + TX_STATUS_VECTOR_SIZE)
#define TX_BUFFER_SIZE                  (TXEND  - TXSTART) 

#define TX_BUFFER_MID                   ((TXSTART + TXEND) >> 1 )

#define SetBit( bitField, bitMask )     do{ bitField = bitField | bitMask; } while(0)
#define ClearBit( bitField, bitMask )   do{ bitField = bitField & (~bitMask); } while(0)
#define CheckBit( bitField, bitMask )   (bool)(bitField & bitMask)

// Define a temporary register for passing data to inline assembly
// This is to work around the 110110 LSB errata and to control RDPTR WRPTR update counts4

static txPacket_t  *pHead;
static txPacket_t  *pTail;
static txPacket_t txData[MAX_TX_PACKETS];
static receiveStatusVector_t rxPacketStatusVector;
static uint16_t nextPacketPointer;
static uint8_t ethListSize;
volatile ethernetDriver_t ethData;
const mac48Address_t *eth_MAC;
static uint8_t timeout;
static uint32_t temp;
static uint32_t txCommandA;
static uint32_t txCommandB;
static uint16_t cmdMask = 0x0000;

static error_msg ETH_SendQueued(void);
static void ETH_PacketListReset(void);
static txPacket_t* ETH_NewPacket(void);
static void ETH_RemovePacket(txPacket_t* packetHandle);

static uint32_t LAN_Regread32(lan9250_registers_t a);
static void LAN_Regwrite32(lan9250_registers_t, uint32_t);
static void LAN_Bfc(lan9250_registers_t, uint32_t) ;
static void LAN_Bfs(lan9250_registers_t, uint32_t);

static uint32_t LAN_MACRead32(lan_mac_registers_t);
static void LAN_MACWrite32(lan_mac_registers_t, uint32_t);
static void MAC_Bfc(lan_mac_registers_t, uint32_t);
static void MAC_Bfs(lan_mac_registers_t, uint32_t);

static void LAN_Phywrite(phy_registers_t, uint16_t);
static uint16_t LAN_Phyread(phy_registers_t);

static void LAN_Enable_IRQ(void);
static void LAN_Disable_IRQ(void);
/*******************************************************************************/

bool ETH_CheckLinkUp(void)
{
     uint16_t linkup;
    
    linkup = LAN_Phyread(PHY_BASIC_STATUS);
    
    if((linkup & PHY_LINK_STATUS)>0)
    {
        ethData.up = true;
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * "Allocate" a new packet element and link it into the chained list
 * @param 
 * @return  packet address
 */
static txPacket_t* ETH_NewPacket(void)
{
    uint8_t index = 0;

    if( ethListSize == MAX_TX_PACKETS )
    {
        return NULL;
    }

    while( index < MAX_TX_PACKETS )
    {
        if( CheckBit(txData[index].flags, ETH_ALLOCATED) == false )
        {
            // reset all flags
            txData[index].flags = 0;                        
            // allocated = true - mark the handle as allocated
            SetBit(txData[index].flags, ETH_ALLOCATED);     

            txData[index].packetEnd = TXEND;

            txData[index].prevPacket = NULL;
            txData[index].nextPacket = pHead;

            if( pHead != NULL )
            {
                pHead->prevPacket = &txData[index];
                txData[index].packetStart = pHead->packetEnd + TX_STATUS_VECTOR_SIZE;
                    
                // Try to keep a 2byte alignment
                if( txData[index].packetStart & 0x0001 )
                {
                    // Make sure the end of the packet is odd, so the beginning of the next one is even
                    ++ txData[index].packetStart;
                }
            }
            else
            {
                txData[index].packetStart = TXSTART;
                pTail = (txPacket_t*)&txData[index];
            }

            pHead = (txPacket_t*)&txData[index];

            ethListSize ++;
            return (&txData[index]);
        }
        index ++;
    }
    return NULL;
}

/**
 * Reset the Ethernet Packet List
 * @param 
 * @return
 */
static void ETH_PacketListReset(void)
{
    uint16_t index = 0;
    uint8_t* ptr = (uint8_t*)&txData;
    ethListSize = 0;

    pHead = NULL;
    pTail = NULL;

    while( index < (MAX_TX_PACKETS * sizeof(txPacket_t)) )
    {
        ptr[index] = 0;
        index++;
    }
}

/**
 * Unlink a packet element from the chained list and "deallocate" it
 * @param   packetHandle
 * @return 
 */
static void ETH_RemovePacket(txPacket_t* pPacket)
{    
#ifdef VALIDATE_ALLOCATED_PTR
    uint8_t index = 0;
#endif /* VALIDATE_ALLOCATED_PTR */

    if( (pPacket == NULL) || (ethListSize == 0) )
    {
        return;
    }

#ifdef VALIDATE_ALLOCATED_PTR
    while( index < MAX_TX_PACKETS )
    {
        if( (pPacket == &txData[index]) && (txData[index].allocated == true) )
        {
            break;
        }
        index ++;
    }
    if( index == MAX_TX_PACKETS )
    {
        return;
    }
#endif  /* VALIDATE_ALLOCATED_PTR */

    // Unlink from the chained list
    if( pPacket->nextPacket == NULL )
    {
        pTail = pPacket->prevPacket;
        if( pTail != NULL )
        {
            pTail->nextPacket = NULL;
        }
    }

    if( pPacket->prevPacket == NULL )
    {
        pHead = pPacket->nextPacket;
        if( pHead != NULL )
        {
            pHead->prevPacket = NULL;
        }
    }

    // Deallocate
    pPacket->flags = 0;
    pPacket->prevPacket = NULL;
    pPacket->nextPacket = NULL;

    ethListSize --;

    return;
}

/**
 * Release SPI Bus
 */
void ETH_CloseSPI(void)
{
    spi1_close();
}
 
/**
  * Connect SPI Bus
  */
void ETH_OpenSPI(void)
{
    while (!spi_master_open(MAC));
}

/**
 * Ethernet Initialization - Initializes TX/RX Buffer, MAC and PHY
 */
void ETH_Init(void)
{
    uint32_t low_addr;
    uint32_t high_addr;
    txCommandA = (TX_CMD_A_INT_ON_COMP | TX_CMD_A_4_BYTE_ALIGN | TX_CMD_A_FIRST_SEG | TX_CMD_A_LAST_SEG);
    txCommandB = (TX_CMD_B_PACKET_TAG);
    
    ETH_OpenSPI();
    RXBuffer_Init();
    ethData.error = false; // no error
    ethData.up = false; // no link
    ethData.linkChange = false;
    
    ETH_PacketListReset();
    ethData.saveRDPT = 0;
    ETH_SendSystemReset(); // software reset
    //Wait for device ready bit to set after reset
    timeout = WAIT_DEVICE_READY;
    do{
        temp = LAN_Regread32(HW_CFG);
    }while((--timeout)&&(temp & HCR_READY));

    eth_MAC = MAC_getAddress();
    high_addr = ((eth_MAC->mac_array[5] << 8) | (eth_MAC->mac_array[4]));
    low_addr =(((uint32_t)eth_MAC -> mac_array[3] << 24) | ((uint32_t)eth_MAC->mac_array[2] << 16) | ((uint32_t)eth_MAC->mac_array[1] << 8) | ((uint32_t)eth_MAC->mac_array[0]));
    LAN_MACWrite32(HMAC_ADDRH, high_addr);
    LAN_MACWrite32(HMAC_ADDRL, low_addr);
    
    LAN_Regwrite32(HW_CFG, (MBO | TX_FIF_SiZE));
    LAN_Regwrite32(AFC_CFG, 0x006E3741);
    LAN_Bfs(IRQ_CFG, (IRQ_CFG_INT_DEAS | IRQ_CFG_IRQ_EN | IRQ_CFG_INT_POL | IRQ_CFG_IRQ_TYPE));
    LAN_Regwrite32(INT_STS, INTERRUPT_STATUS_CLEAR);
    LAN_Regwrite32(INT_EN, (SW_INT_EN | RXSTOP_INT_EN | RXE_INT_EN | TDFA_EN | RSFL_EN | PHY_INT_EN));
    LAN_Regwrite32(FIFO_INT, (TX_DATA_AVAILABLE_LEVEL | TX_STATUS_LEVEL | RX_STATUS_LEVEL));
    LAN_Regwrite32(RX_CFG, (RX_DMA_CNT | RX_RXDOFF));
    LAN_Bfs(TX_CFG, (TX_ON)); 
    LAN_Regwrite32(PMT_CTRL, (PM_WAKE | DIS_1588 | DIS_1588_TSU | ED_EN | WOL_EN | PME_TYPE | WOL_STS | PME_POL| PME_EN));

    LAN_MACWrite32(HMAC_CR, (FDPX | PADSTR | RXEN | TXEN));
    
    LAN_Phywrite(PHY_BASIC_CONTROL, (PHY_SPEED_SEL_LSB | PHY_AN | PHY_DUPLEX));  
    LAN_Phywrite(PHY_AN_ADV, (PHY_AN_ADV_ASYMMETRIC_PAUSE | PHY_AN_ADV_SYMMETRIC_PAUSE | PHY_AN_ADV_100BASE_X_FULL_DUPLEX | PHY_AN_ADV_100BASE_X_HALF_DUPLEX | PHY_AN_ADV_10BASE_T_FULL_DUPLEX | PHY_AN_ADV_10BASE_T_HALF_DUPLEX | PHY_AN_ADV_SELECTOR_FIELD));
    LAN_Phywrite(PHY_SPECIAL_MODES, (MODE | PHYADD));
    LAN_Phywrite(PHY_SPECIAL_CONTROL_STATUS_IND, (AMDIXCTRL | AMDIXEN | SQEOFF));
    LAN_Phywrite(PHY_INTERRUPT_MASK, (INT9_LINK_UP | INT7_ENERGY_ON | INT4_LINK_DOWN));
    LAN_Phywrite(PHY_SPECIAL_CONTROL_STATUS, (RESERVED_DATA));    
}

void ETH_EventHandler()
{
    uint16_t phyint;
    uint32_t intsts;
    phyint= LAN_Phyread(PHY_INTERRUPT_SOURCE);
    intsts = LAN_Regread32(INT_STS);
    
    if((intsts & SW_INT)>0)
    {
        LAN_Bfs(INT_STS, SW_INT);        
    }
    
    if((intsts & RXSTOP_INT)>0)
    {
        LAN_Bfs(INT_STS, RXSTOP_INT);        
    }
    
    if((intsts & RXE)>0)
    {
        LAN_Bfs(INT_STS, RXE);
    }
    
    if((intsts & TDFA)>0)
    {
        LAN_Bfs(INT_STS, TDFA);        
    }
    
    if((intsts & PHY_INT)>0)
    {
        if(phyint & (INT9_LINK_UP | INT4_LINK_DOWN))
        {
            ethData.linkChange = true;
            ethData.up = false;
            ETH_CheckLinkUp();           
        }        
    }
    
    if(intsts & RSFL)
    {
        LAN_Bfs(INT_STS, RSFL);
        if(ethData.pktReady == false)
        {
            ethData.pktReady = true;
        }
    }
    LAN_Enable_IRQ();
}

/**
 * Retrieve information about last received packet and the address of the next ones
 */
void ETH_NextPacketUpdate(void)
{
    uint16_t rxLen;
    uint32_t dummyOffset;
    uint32_t receiveStatus;
    uint16_t cmd;
    uint8_t cmd0;
    uint8_t cmd1;
    
    cmd = RX_DATA_FIFO | cmdMask;
    cmd0 = cmd >> 8;
    cmd1 = cmd & 0xFF;
    // Disable all interrupts before reading data from RX FIFO
    LAN_Disable_IRQ();
    // Initialize the RX buffer
    RXBuffer_Init();
    receiveStatus = LAN_Regread32(RX_STATUS_FIFO);
    
    // Calculating the length of received packet
    rxLen = ((receiveStatus >> 16)& PACKET_LENGTH);
    dummyOffset = LAN_Regread32(RX_DATA_FIFO);
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(SPI_READ);
    ETH_SPI_WRITE8(cmd0);
    ETH_SPI_WRITE8(cmd1);   
    // Reading the packet from the RX FIFO 
    BufferReceive(&dataBuffers.rxBuffer, (rxLen));
    ETH_NCS_HIGH();
    
    // Enable all the required interrupts after reading a packet from RX FIFO
    LAN_Enable_IRQ();
}

/**
 * Read 32bit value from the system control and status registers
 * @param address
 */
static uint32_t LAN_Regread32(lan9250_registers_t address)
{
    uint32_t value;
    
    uint16_t cmd;
    uint8_t cmd0;
    uint8_t cmd1;
    
    cmd = address | cmdMask;
    cmd0 = cmd >> 8;
    cmd1 = cmd & 0xFF;
   
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(SPI_READ);
    ETH_SPI_WRITE8(cmd0);
    ETH_SPI_WRITE8(cmd1);
    ((char *) &value)[0] = ETH_SPI_READ8();
    ((char *) &value)[1] = ETH_SPI_READ8();
    ((char *) &value)[2] = ETH_SPI_READ8();
    ((char *) &value)[3] = ETH_SPI_READ8();
    ETH_NCS_HIGH();
    
    return value;   
}

/**
 * Write 32bit value to the system control and status registers
 * @param address
 * @param value
 */
static void LAN_Regwrite32(lan9250_registers_t address, uint32_t value)
{
    uint16_t cmd;
    uint8_t cmd0;
    uint8_t cmd1;

    cmd = address | cmdMask;
    cmd0 = cmd >> 8;
    cmd1 = cmd & 0xFF;   
    ETH_NCS_LOW();
    ETH_SPI_WRITE8(SPI_WRITE);
    ETH_SPI_WRITE8(cmd0);
    ETH_SPI_WRITE8(cmd1);
    ETH_SPI_WRITE8(((char *) &value)[0]);
    ETH_SPI_WRITE8(((char *) &value)[1]);
    ETH_SPI_WRITE8(((char *) &value)[2]);
    ETH_SPI_WRITE8(((char *) &value)[3]);
    ETH_NCS_HIGH();
}

/**
 * Bit Field Clear
 * @param address
 * @param value
 */
static void LAN_Bfc(lan9250_registers_t address, uint32_t value) 
{
    uint32_t temp;
    ETH_NCS_LOW();
    temp = LAN_Regread32(address);
    temp = (temp & (~value));
    LAN_Regwrite32(address, temp);
    ETH_NCS_HIGH();
}

/**
 * Bit Field Set
 * @param address
 * @param value
 */
static void LAN_Bfs(lan9250_registers_t address, uint32_t value)
{
    uint32_t temp;
    ETH_NCS_LOW();
    temp = LAN_Regread32(address);
    temp = temp | value;
    LAN_Regwrite32(address, temp);
    ETH_NCS_HIGH();   
}

/**
 * Read 32bit value from the indirect MAC registers
 * @param offset
 */
static uint32_t LAN_MACRead32(lan_mac_registers_t offset)
{
    while(LAN_Regread32(MAC_CSR_CMD)& HMAC_CSR_BUSY); 
    LAN_Regwrite32(MAC_CSR_CMD, ((offset & 0xFF) | HMAC_CSR_BUSY | R_nW));
    
    LAN_Regread32(BYTE_TEST);
    while(LAN_Regread32(MAC_CSR_CMD)& HMAC_CSR_BUSY); //wait for the busy flag to clear
    return LAN_Regread32(MAC_CSR_DATA);
}

/**
 * Write 32bit value to the indirect MAC registers
 * @param offset
 * @param data
 */
static void LAN_MACWrite32(lan_mac_registers_t offset, uint32_t data)
{
    while(LAN_Regread32(MAC_CSR_CMD)& HMAC_CSR_BUSY); //wait for the busy flag to clear
    LAN_Regwrite32(MAC_CSR_DATA, data);
    LAN_Regwrite32(MAC_CSR_CMD, (HMAC_CSR_BUSY | offset));
    
    LAN_Regread32(BYTE_TEST);
    while(LAN_Regread32(MAC_CSR_CMD)& HMAC_CSR_BUSY); //wait for the busy flag to clear
}

/**
 * Bit Field Clear
 * @param address
 * @param value
 */
static void MAC_Bfc(lan_mac_registers_t address, uint32_t value) 
{
    uint32_t temp;
    ETH_NCS_LOW();
    temp = LAN_MACRead32(address);
    temp = (temp & (~value));
    LAN_MACWrite32(address, temp);
    ETH_NCS_HIGH();
}

/**
 * Bit Field Set
 * @param address
 * @param value
 */
static void MAC_Bfs(lan_mac_registers_t address, uint32_t value)
{
    uint32_t temp;
    ETH_NCS_LOW();
    temp = LAN_MACRead32(address);
    temp = temp | value;
    LAN_MACWrite32(address, temp);
    ETH_NCS_HIGH();   
}

/**
 * Read 16bit value from the indirect PHY registers
 * @param index
 */
static uint16_t LAN_Phyread(phy_registers_t index)
{
    uint16_t value;
    uint32_t cmd;
    cmd = ((phy_add & 0x1F) << 11) | ((index & 0x1F) << 6);
    
    while(LAN_MACRead32(HMAC_MII_ACC)& MIIBZY); //wait for the busy flag to clear
    LAN_MACWrite32(HMAC_MII_ACC, cmd);
    while(LAN_MACRead32(HMAC_MII_ACC)& MIIBZY); //wait for the busy flag to clear
    value =  LAN_MACRead32(HMAC_MII_DATA);   
    
    return value;
}

/**
 * Write 16bit value to the indirect PHY registers
 * @param index
 * @param data
 */
static void LAN_Phywrite(phy_registers_t index, uint16_t data)
{
    uint32_t cmd;
    cmd = ((phy_add & 0x1F) << 11) | ((index & 0x1F)<< 6) | MIIWnR;
    while((LAN_MACRead32(HMAC_MII_ACC))& MIIBZY); //wait for the busy flag to clear
    LAN_MACWrite32(HMAC_MII_DATA, data);
    LAN_MACWrite32(HMAC_MII_ACC, cmd);
    while(LAN_MACRead32(HMAC_MII_ACC)& MIIBZY); //wait for the busy flag to clear
}


/**
 *Enable the required Interrupts
 */
static void LAN_Enable_IRQ()
{
    LAN_Regwrite32(INT_EN, (SW_INT_EN | RXSTOP_INT_EN | RXE_INT_EN | TDFA_EN | RSFL_EN | PHY_INT_EN));
}

/**
 *Disable all the Interrupts
 */
static void LAN_Disable_IRQ()
{
    LAN_Regwrite32(INT_EN,0);
    LAN_Regwrite32(INT_STS, INTERRUPT_STATUS_CLEAR);
}

/**
 * Returns the available space size in the Ethernet TX Buffer
 * @return available space left in the TX buffer
 */
uint16_t ETH_GetFreeTxBufferSize(void)
{
    uint16_t free_tx;
    free_tx = (LAN_Regread32(TX_FIFO_INF)) & BUFFER_SIZE;
    return ((uint16_t)(free_tx));
}

/**
 * start a packet.
 * If the Ethernet transmitter is idle, then start a packet.  Return is SUCCESS if the packet was started.
 * @param dest_mac
 * @param type
 * @return SUCCESS if packet started.  BUFFER_BUSY or TX_LOGIC_NOT_IDLE if buffer or transmitter is busy respectively
 */
error_msg ETH_WriteStart(const mac48Address_t *dest_mac, uint16_t type)
{
    TXBuffer_Init();
    txPacket_t* ethPacket = NULL;

    if( (ETH_GetFreeTxBufferSize()) < TX_FIFO_LOW_THRESHOLD )
    {
        return BUFFER_BUSY;
    }
   
    ethPacket = (txPacket_t*)ETH_NewPacket();

    if( ethPacket == NULL )
    {
        // No more available packets
        return BUFFER_BUSY;
    }

    SetBit(ethPacket->flags, ETH_WRITE_IN_PROGRESS);    // writeInProgress = true;
    
    ETH_ResetByteCount(); 
            
    BufferWrite8(&dataBuffers.txBuffer, dest_mac->mac_array[0]);
    BufferWrite8(&dataBuffers.txBuffer, dest_mac->mac_array[1]);
    BufferWrite8(&dataBuffers.txBuffer, dest_mac->mac_array[2]);
    BufferWrite8(&dataBuffers.txBuffer, dest_mac->mac_array[3]);
    BufferWrite8(&dataBuffers.txBuffer, dest_mac->mac_array[4]);
    BufferWrite8(&dataBuffers.txBuffer, dest_mac->mac_array[5]);
    BufferWrite8(&dataBuffers.txBuffer, eth_MAC->mac_array[0]);
    BufferWrite8(&dataBuffers.txBuffer, eth_MAC->mac_array[1]);
    BufferWrite8(&dataBuffers.txBuffer, eth_MAC->mac_array[2]);
    BufferWrite8(&dataBuffers.txBuffer, eth_MAC->mac_array[3]);
    BufferWrite8(&dataBuffers.txBuffer, eth_MAC->mac_array[4]);
    BufferWrite8(&dataBuffers.txBuffer, eth_MAC->mac_array[5]);
    BufferWrite16(&dataBuffers.txBuffer, type);
    
    return SUCCESS;
}

/**
 * Clears the RX buffer
 */
void ETH_Flush(void)
{
    ethData.pktReady = false;
    if((LAN_Regread32(RX_FIFO_INF) & RXDUSED) > 0xF)
    {
        LAN_Bfs(RX_DP_CTRL, RX_FFWD);
    }
}

/**
 * Clear all the receiver contents and reset
 */
void ETH_ResetReceiver(void)
{
    uint32_t intsts;
    ethData.pktReady = false;
    
    MAC_Bfc(HMAC_CR, RXEN);
    intsts = LAN_Regread32(INT_STS);
    while(!(intsts & RXSTOP_INT));
    LAN_Bfs(RX_CFG, RX_DUMP);
    LAN_Bfs(INT_STS, RXSTOP_INT);
    MAC_Bfs(HMAC_CR, RXEN);
}

/**
 * System Software Reset
 */
void ETH_SendSystemReset(void)
{
   LAN_Regwrite32(RESET_CTL, (DIGITAL_RST));    
}

/**
 * Calculate the TX Checksum - Software Checksum
 * @param position
 * @param length
 * @param seed
 * @return
 */
uint16_t ETH_TxComputeChecksum(uint16_t position, uint16_t length, uint16_t seed)
{
    uint16_t i;
    uint32_t checksum;
    uint16_t value;

    checksum = seed;      
    for(i = 0; i < length; i++)
    {
        while(length > 1)
        {
            value = 0;
            ((char *)&value)[1] = BufferPeekCopy( &dataBuffers.txBuffer, position);
            position++;
            ((char *)&value)[0] = BufferPeekCopy( &dataBuffers.txBuffer, position);
            position++;
            
            checksum += value;
            length -= 2;
        }
        if(length)
        {
            value = 0;
            ((char *)&value)[1] = BufferPeekCopy( &dataBuffers.txBuffer, position);
            ((char *)&value)[0] = 0;
            checksum += value;
        }
    }
    
    // wrap the checksum
    while(checksum >> 16)
    {
        checksum = ((checksum & 0x0FFFF) + (checksum>>16));
    }
    
    // invert the number
    checksum = (uint16_t)(~checksum);
   
    checksum = htons(checksum);
    return (uint16_t)checksum;
}

/**
 * Calculate RX checksum - Software checksum
 * @param length
 * @param seed
 * @return
 */
uint16_t ETH_RxComputeChecksum(uint16_t length, uint16_t seed)
{
    uint32_t checksum;
    uint16_t value;
    uint16_t i;
    uint16_t offset = 0;
       
    checksum = seed;
    
    for(i = 0; i < length; i++)
     {         
         while(length > 1)
         {
             value = 0;
             ((char *)&value)[1] = BufferPeekCopy( &dataBuffers.rxBuffer, offset);
             offset++;
             ((char *)&value)[0] = BufferPeekCopy( &dataBuffers.rxBuffer, offset);
             offset++;
             checksum += value;
             length -= 2;
         }
         if(length)
         {
             value = 0;
             ((char *)&value)[1] = BufferPeekCopy( &dataBuffers.rxBuffer, offset);
             ((char *)&value)[0] = 0;
             checksum += value;
         }
     }
    // wrap the checksum
    while(checksum >> 16)
    {
        checksum = ((checksum & 0x0FFFF) + (checksum>>16));
    }

    // invert the number
    checksum = ~checksum;
    checksum = (uint16_t)checksum;

    // Return the resulting checksum
    return (uint16_t)(((checksum & 0xFF00) >> 8) | ((checksum & 0x00FF) << 8));
}

/**
 * Enqueue the latest written packet and start the transmission of a queued packet
 * @return
 */
error_msg ETH_SendQueued(void)
{
    if( pHead->flags & ETH_TX_QUEUED )
    {
        ClearBit( pHead->flags, ETH_TX_QUEUED);         // Setting the TX queue flag to false if there are no packets in the TX queue
        ETH_RemovePacket(pTail);
        return SUCCESS;
    }
    else 
    {
        return BUFFER_BUSY;
    }
}

/**
 * Start the Transmission
 * @return
 */
error_msg ETH_Send(void)
{  
    uint32_t txstatus;
    uint16_t cmd;
    uint8_t cmd0;
    uint8_t cmd1;
    cmd = (TX_DATA_FIFO | cmdMask);
    cmd0 = cmd >> 8;
    cmd1 = cmd & 0xFF;
    
    if (ethData.up == 0)
    {
        return LINK_NOT_FOUND;
    }

    if( ethListSize == 0 )
    {
        return BUFFER_BUSY;
    }
    
    LAN_Regwrite32(TX_DATA_FIFO, (txCommandA | dataBuffers.txBuffer.dataLength));
    LAN_Regwrite32(TX_DATA_FIFO, (txCommandB | dataBuffers.txBuffer.dataLength));

    ETH_NCS_LOW();
    ETH_SPI_WRITE8(SPI_WRITE);
    ETH_SPI_WRITE8(cmd0);
    ETH_SPI_WRITE8(cmd1);    
    BufferSend(&dataBuffers.txBuffer);    
    ETH_NCS_HIGH();
    // wait before reading the transmit status
    timeout = WAIT_DEVICE_READY;
    while(--timeout)
    {
        LAN_Regread32(BYTE_TEST);
    }    
    
    //Read the status of the transmitted packet
    txstatus = LAN_Regread32(TX_STATUS_FIFO);
    ClearBit( pHead->flags, ETH_WRITE_IN_PROGRESS);     // writeInProgress = false
    SetBit( pHead->flags, ETH_TX_QUEUED);               // Setting the TX queue flag to true if there are packets in the TX queue

    if( ethListSize > 1 )
    {
        return TX_QUEUED;
    }
    return ETH_SendQueued();
}

/**
 * To get the MAC address
 * @param macAddr
 */
void ETH_GetMAC(uint8_t *macAddr)
{
    uint32_t high_addr;
    uint32_t low_addr;
    high_addr = LAN_MACRead32(HMAC_ADDRH);
    low_addr = LAN_MACRead32(HMAC_ADDRL);
    *macAddr++ = (low_addr & 0xFF);
    *macAddr++ = (low_addr >> 8) & 0xFF;
    *macAddr++ = (low_addr >> 16) & 0xFF;
    *macAddr++ = (low_addr >> 24) & 0xFF;
    *macAddr++ = (high_addr & 0xFF);
    *macAddr++ = (high_addr >> 8) & 0xFF;
}

/**
 * To set the MAC address
 * @param macAddr
 */
void ETH_SetMAC(uint8_t *macAddr)
{
    uint32_t low_addr;
    uint32_t high_addr;
      
    low_addr   = (*macAddr++);
    low_addr  |= (*macAddr++ << 8);
    low_addr  |= ((uint32_t)*macAddr++ << 16);
    low_addr  |= ((uint32_t)*macAddr++ << 24);
    high_addr  = (*macAddr++);
    high_addr |= (*macAddr++ << 8);
    
    LAN_MACWrite32(HMAC_ADDRH, high_addr);
    LAN_MACWrite32(HMAC_ADDRL, low_addr);
}

void ETH_SaveRDPT(void)
{
    ethData.saveRDPT = (volatile uint16_t)dataBuffers.rxBuffer.currentLocation;
}

uint16_t ETH_GetReadPtr(void)
{
    return (uint16_t)dataBuffers.rxBuffer.currentLocation;
}

void ETH_SetReadPtr(uint16_t rdptr)
{  
    dataBuffers.rxBuffer.currentLocation = rdptr;
}

void ETH_ResetByteCount(void)
{
    ethData.saveWRPT = dataBuffers.txBuffer.dataLength;
}

uint16_t ETH_GetByteCount(void)
{
    return (dataBuffers.txBuffer.dataLength - ethData.saveWRPT);
}