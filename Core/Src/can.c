#include "main.h"
#include "logger.h"
#include "can.h"
#include "bootloader.h"

#include "stm32f1xx_hal.h"

extern CAN_HandleTypeDef hcan;
extern IWDG_HandleTypeDef hiwdg;

CanTxMsgTypeDef hcan_CAN_Tx;

CanRxMsgTypeDef hcan_CAN_Rx0;
CanRxMsgTypeDef hcan_CAN_Rx1;


uint16_t canpybara_can_get_my_address(void)
{
	uint16_t result = 0;

	uint16_t ADDR_PIN[] = {ADDR0_Pin, ADDR1_Pin, ADDR2_Pin, ADDR3_Pin, ADDR4_Pin, ADDR5_Pin, ADDR6_Pin};
	GPIO_TypeDef * ADDR_PORT[] = {ADDR0_GPIO_Port, ADDR1_GPIO_Port, ADDR2_GPIO_Port, ADDR3_GPIO_Port, ADDR4_GPIO_Port, ADDR5_GPIO_Port, ADDR6_GPIO_Port};
	const size_t INPUTS_ADDR_LEN = sizeof(ADDR_PIN) / sizeof(ADDR_PIN[0]);

	int i;
	for (i = 0; i < INPUTS_ADDR_LEN; ++i)
	{
		if(HAL_GPIO_ReadPin(ADDR_PORT[i], ADDR_PIN[i]) == GPIO_PIN_RESET)
		{
			result |= 1 << i;
		}
	}

	if(result == 0)
	{
		LOG("Invalid module address: %d", result);
		_Error_Handler(__FILE__, __LINE__);
	}

	return result;
}

uint16_t canpybara_tx_frames = 0;
uint16_t canpybara_rx_frames = 0;
uint16_t canpybara_errors = 0;

void canpybara_can_tx_complete(void)
{
	canpybara_tx_frames++;
}

void canpybara_can_error(void)
{
	canpybara_errors++;
}

void canpybara_configure_filters(CAN_HandleTypeDef* hcan)
{
	uint16_t my_address = canpybara_can_get_my_address();

	CAN_FilterConfTypeDef filter_config;
	filter_config.FilterNumber = 0;
	filter_config.FilterMode = CAN_FILTERMODE_IDMASK;
	filter_config.FilterScale = CAN_FILTERSCALE_16BIT;

	// 5 for matching STDID

	// Filter 1 : My addr
	filter_config.FilterIdHigh = my_address << 5;
	filter_config.FilterMaskIdHigh = CANPYBARA_DEVICE_ADDR_BITMASK;

	// Filter 2 : Addr 0
	filter_config.FilterIdLow = 0x000 << 5;
	filter_config.FilterMaskIdLow = CANPYBARA_DEVICE_ADDR_BITMASK;

	filter_config.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	filter_config.FilterActivation = ENABLE;
	filter_config.BankNumber = 0;

	HAL_StatusTypeDef result = HAL_CAN_ConfigFilter(hcan, &filter_config);

	LOG("Configuring CAN filters my_addr: %d, status: %d", my_address, result);
	if(result != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
}

void canpybara_reload_canrx(CAN_HandleTypeDef* hcan)
{
	__HAL_UNLOCK(hcan);
	HAL_StatusTypeDef result = HAL_CAN_Receive_IT(hcan, CAN_FIFO0);

	if(result != HAL_OK)
	{
		LOG("CAN REGISTER STATUS: %"PRIu32, hcan->State);
		LOG("CAN_Receive_IT called status: %d", result);
		_Error_Handler(__FILE__, __LINE__);
	}
}

void canpybara_can_init(void)
{
	LOG("Initializing CAN");

	hcan.pRxMsg = &hcan_CAN_Rx0;
	hcan.pRx1Msg = &hcan_CAN_Rx1;

	canpybara_configure_filters(&hcan);
	canpybara_reload_canrx(&hcan);
}

void capybara_can_report_status(void)
{
	static CanTxMsgTypeDef can_tx;
	can_tx.StdId = CANPYBARA_REPORT_STATUS;
	can_tx.ExtId = 0;
	can_tx.IDE = CAN_ID_STD;
	can_tx.RTR = CAN_RTR_DATA;

	can_tx.DLC = 7;
	int i = 0;
	can_tx.Data[i++] = 0x02; // bootloader

	// TX
	can_tx.Data[i++] = canpybara_tx_frames >> 8;
	can_tx.Data[i++] = canpybara_tx_frames;

	// RX
	can_tx.Data[i++] = canpybara_rx_frames >> 8;
	can_tx.Data[i++] = canpybara_rx_frames;

	// ERR
	can_tx.Data[i++] = canpybara_errors >> 8;
	can_tx.Data[i++] = canpybara_errors;

	canpybara_can_tx(&can_tx);
}

void canpybara_can_rx(CAN_HandleTypeDef* hcan)
{
	canpybara_rx_frames++;

	int request_id = CANPYBARA_CONTROLLER_REQUESTID(hcan->pRxMsg->StdId);

	LOG("Request id: %d for %"PRIu32, request_id, hcan->pRxMsg->StdId);

	switch(request_id)
	{
		case CANPYBARA_REQUEST_STATUS:
			if (hcan->pRxMsg->RTR == CAN_RTR_REMOTE)
			{
				capybara_can_report_status();
			}
			break;

		case CANPYBARA_REQUEST_REBOOT:
			LOG("Received reset request...");
			NVIC_SystemReset();
			break;

		case CANPYBARA_REQUEST_BOOTLOADER:
			canpybara_bootloader_enable();
			break;

		case CANPYBARA_REQUEST_BOOTLOADER_ERASE:
			if (hcan->pRxMsg->DLC == 4)
			{
				canpybara_bootloader_erase(*((uint32_t*)hcan->pRxMsg->Data));
			}
			break;

		case CANPYBARA_REQUEST_BOOTLOADER_WRITE:
			if (hcan->pRxMsg->DLC == 8)
			{
				canpybara_bootloader_write(*((uint64_t*)hcan->pRxMsg->Data));
			}
			break;

		default:
			LOG("Unknown request ID: %d", request_id);
	}
	
	canpybara_reload_canrx(hcan);
}


void canpybara_can_tx(CanTxMsgTypeDef *can_tx)
{
	hcan.pTxMsg = can_tx;

	if(HAL_CAN_Transmit(&hcan, 100) != HAL_OK)
	{
		LOG("Can transmit failure");
		canpybara_can_error();
	}

	canpybara_can_tx_complete();
}
