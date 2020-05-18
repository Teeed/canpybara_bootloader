#ifndef _CAN_H
#define _CAN_H

#include "stm32f1xx_hal.h"
/*
 
CONTROLLER -> ALLREADER
 0 T T T T 0 0 0 0 0 0

 
CONTROLLER -> READER
 0 T T T T X X X X X X


CONTROLLER <- READER
 1 T T T T X X X X X X
 
 1 0 0 0 0 1 1 1 1 1 1

T T T T
-------
0 0 0 0
       <-  VERSIONINFO
           Wersja, info, bardziej dla ludzi, 8 znakow
 
0 0 0 1
        -> STATUSREQ [RTR]
       <-  STATUS
           00   00 00 00 00 00 00
           ^       TX    RX    ERR
           | 
            - mode (00 - normal, 01 - wiegand, 02 - bootloader)

0 1 1 0
        -> REBOOT 

1 0 0 0
       <- BOOTLOADER DISCO
          00
          ^- protocol version

       -> BOOTLOADER ENABLE

1 0 0 1
       -> BOOTLOADER ERASE PAGE (SETADDR)
          AA AA AA AA
            ADDRESS

       <- BOOTLOADER ERASE ACK

1 0 1 0
       -> BOOTLOADER WRITE BYTES
          XX XX XX XX XX XX XX XX 
          ^- bytes

       <- BOOTLOADER WRITE ACK
          01
          ^- success


*/

// Matches bit 11 (direction) and device ID (X part of addr)
#define CANPYBARA_DEVICE_ADDR_BITMASK 0x87E0

#define CANPYBARA_DEVICE_ADDR_LEN 6

#define CANPYBARA_CONTROLER_ADDR (canpybara_can_get_my_address() | 1<<10)
#define CANPYBARA_CLIENT_REPORT(id) (CANPYBARA_CONTROLER_ADDR | ((id & 0xF) << CANPYBARA_DEVICE_ADDR_LEN))


#define CANPYBARA_REQUEST_BOOTLOADER 0x8
#define CANPYBARA_REQUEST_BOOTLOADER_ERASE (CANPYBARA_REQUEST_BOOTLOADER+1)
#define CANPYBARA_REQUEST_BOOTLOADER_WRITE (CANPYBARA_REQUEST_BOOTLOADER+2)

#define CANPYBARA_REQUEST_STATUS 0x01
#define CANPYBARA_REQUEST_REBOOT 0x06

#define CANPYBARA_REPORT_BOOTLOADER CANPYBARA_CLIENT_REPORT(CANPYBARA_REQUEST_BOOTLOADER)
#define CANPYBARA_REPORT_BOOTLOADER_ERASE CANPYBARA_CLIENT_REPORT(CANPYBARA_REQUEST_BOOTLOADER_ERASE)
#define CANPYBARA_REPORT_BOOTLOADER_WRITE CANPYBARA_CLIENT_REPORT(CANPYBARA_REQUEST_BOOTLOADER_WRITE)

#define CANPYBARA_REPORT_STATUS CANPYBARA_CLIENT_REPORT(CANPYBARA_REQUEST_STATUS)


#define CANPYBARA_CONTROLLER_REQUESTID(id) ((id >> CANPYBARA_DEVICE_ADDR_LEN) & 0xF)


void canpybara_configure_filters(CAN_HandleTypeDef* hcan);
void canpybara_can_init(void);

void canpybara_can_tx(CAN_TxHeaderTypeDef* can_tx_header, uint8_t can_tx_data[8]);

void canpybara_can_tx_complete(void);
void canpybara_can_error(void);

uint16_t canpybara_can_get_my_address(void);
void canpybara_can_rx(CAN_HandleTypeDef* hcan);

#endif
