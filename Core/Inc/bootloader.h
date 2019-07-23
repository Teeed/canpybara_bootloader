#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>


#define BOOTLOADER_FLASH_BASE 0x08000000

// first 16k for bootloader
#define BOOTLOADER_MEMORY_SIZE 0x4000 
#define BOOTLOADER_APP_START_ADDR (BOOTLOADER_FLASH_BASE + BOOTLOADER_MEMORY_SIZE)
#define BOOTLOADER_APP_END_ADDR (BOOTLOADER_FLASH_BASE + 0x3FF*64)
#define BOOTLOADER_APP_RESET_HANDLER (BOOTLOADER_APP_START_ADDR + 4)

#define BOOTLOADER_TIMEOUT 5000
#define BOOTLOADER_ACTIVE_TIMEOUT 30000

void canpybara_bootloader_loop(void);
void canpybara_bootloader_enable(void);
void canpybara_bootloader_check_loop(void);
void canpybara_bootloader_send_discovery(void);

void canpybara_bootloader_erase(uint32_t addr);
void canpybara_bootloader_write(uint64_t bytes);

#endif
