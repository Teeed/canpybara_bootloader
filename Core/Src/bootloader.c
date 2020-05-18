#include "stm32f1xx_hal.h"

#include "bootloader.h"
#include "can.h"
#include "logger.h"

#include <stdint.h>

uint8_t canpybara_bootloader_enabled = 0;
uint32_t canpybara_bootloader_last_active = 0;
uint32_t canpybara_bootloader_addr = 0;

typedef void (*pFunction)(void);

void canpybara_bootloader_boot_app(void)
{
	__disable_irq();

	uint32_t app_reset_handler_addr = *(__IO uint32_t *)(BOOTLOADER_APP_RESET_HANDLER); // reset handler
	pFunction application_main = (pFunction)app_reset_handler_addr;
	
	HAL_RCC_DeInit();
	HAL_DeInit();

	__set_MSP(*(__IO uint32_t *)(BOOTLOADER_APP_START_ADDR));
	application_main();
} 


void canpybara_bootloader_enable(void)
{
	LOG("Got magic packet, entering bootloader");
	canpybara_bootloader_enabled = 1;

	canpybara_bootloader_last_active = HAL_GetTick();
}


void canpybara_bootloader_send_discovery(void)
{
	static CanTxMsgTypeDef can_tx;
	can_tx.StdId = CANPYBARA_REPORT_BOOTLOADER;
	can_tx.ExtId = 0;
	can_tx.IDE = CAN_ID_STD;
	can_tx.RTR = CAN_RTR_DATA;

	can_tx.DLC = 1;
	int i = 0;
	can_tx.Data[i++] = 0x00; // protocol version 00

	canpybara_can_tx(&can_tx);
}

static void canpybara_bootloader_active_loop(void)
{

}

static uint8_t canpybara_ignore_address(uint32_t address)
{
	if(! canpybara_bootloader_enabled)
	{
		LOG("Tired bootloader CMD with bootloader not enabled, ignoring");

		return 1;
	}

	LOG("Addr req: %"PRIu32, address);

	if(address < BOOTLOADER_APP_START_ADDR)
	{
		LOG("Too low, ignoring");

		return 1;
	}

	if(address > BOOTLOADER_APP_END_ADDR)
	{
		LOG("Too high, ignoring");

		return 1;
	}

	return 0;
}

void canpybara_bootloader_erase(uint32_t addr)
{
	if(canpybara_ignore_address(addr))
	{
		return;
	}

	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef erase_init_struct;
	erase_init_struct.TypeErase = FLASH_TYPEERASE_PAGES;
	erase_init_struct.PageAddress = addr;
	erase_init_struct.NbPages = 1;

	uint32_t page_error;
	HAL_FLASHEx_Erase(&erase_init_struct, &page_error);

	HAL_FLASH_Lock();

	static CanTxMsgTypeDef can_tx;
	can_tx.StdId = CANPYBARA_REPORT_BOOTLOADER_ERASE;
	can_tx.ExtId = 0;
	can_tx.IDE = CAN_ID_STD;
	can_tx.RTR = CAN_RTR_DATA;

	can_tx.DLC = 1;
	int i = 0;
	can_tx.Data[i++] = page_error == 0xFFFFFFFF; 

	canpybara_can_tx(&can_tx);
	canpybara_bootloader_last_active = HAL_GetTick();
	canpybara_bootloader_addr = addr;
}


void canpybara_bootloader_write(uint64_t bytes)
{
	if(canpybara_ignore_address(canpybara_bootloader_addr))
	{
		return;
	}

	HAL_FLASH_Unlock();
	HAL_StatusTypeDef result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, canpybara_bootloader_addr, bytes);
	HAL_FLASH_Lock();

	static CanTxMsgTypeDef can_tx;
	can_tx.StdId = CANPYBARA_REPORT_BOOTLOADER_WRITE;
	can_tx.ExtId = 0;
	can_tx.IDE = CAN_ID_STD;
	can_tx.RTR = CAN_RTR_DATA;

	can_tx.DLC = 1;
	int i = 0;
	can_tx.Data[i++] = result != HAL_ERROR; 

	canpybara_can_tx(&can_tx);
	canpybara_bootloader_last_active = HAL_GetTick();
	canpybara_bootloader_addr += sizeof(uint64_t);
}

void canpybara_bootloader_loop(void)
{
	if(canpybara_bootloader_enabled)
	{
		canpybara_bootloader_active_loop();

		if((HAL_GetTick()-canpybara_bootloader_last_active) > BOOTLOADER_ACTIVE_TIMEOUT)
		{
			LOG("Bootloader active timeout, issuing reset");
			for(;;); // IWDG will issue reset
		}
	}
	else
	{
		if(HAL_GetTick() > BOOTLOADER_TIMEOUT)
		{
			LOG("Bootloader timeout, jumping to application");
			canpybara_bootloader_boot_app();
		}
	}
}
