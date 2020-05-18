#include "main.h"
#include "logger.h"
#include "can.h"
#include "bootloader.h"

#include "stm32f1xx_hal.h"

extern CAN_HandleTypeDef hcan;
extern IWDG_HandleTypeDef hiwdg;

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
		Error_Handler();
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

	CAN_FilterTypeDef filter_config;
	filter_config.FilterBank = 0;
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

	HAL_StatusTypeDef result = HAL_CAN_ConfigFilter(hcan, &filter_config);

	LOG("Configuring CAN filters my_addr: %d, status: %d", my_address, result);
	if(result != HAL_OK)
	{
		LOG("Error configuring CAN filter");
		Error_Handler();
	}
}

void canpybara_can_init(void)
{
	LOG("Initializing CAN");

	canpybara_configure_filters(&hcan);

	if(HAL_CAN_Start(&hcan) != HAL_OK)
	{
		LOG("Error starting CAN");
		Error_Handler();
	}

	if(HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		LOG("Error activating CAN notification");
		Error_Handler();
	}
}


// TODO: Remove or make these counters really count things..
void capybara_can_report_status(void)
{
	CAN_TxHeaderTypeDef can_tx_header;
	uint8_t can_tx_data[8];

	can_tx_header.StdId = CANPYBARA_REPORT_STATUS;
	can_tx_header.ExtId = 0;
	can_tx_header.IDE = CAN_ID_STD;
	can_tx_header.RTR = CAN_RTR_DATA;
	can_tx_header.DLC = 7;
	can_tx_header.TransmitGlobalTime = DISABLE;

	int i = 0;
	can_tx_data[i++] = 0x02; // bootloader

	// TX
	can_tx_data[i++] = canpybara_tx_frames >> 8;
	can_tx_data[i++] = canpybara_tx_frames;

	// RX
	can_tx_data[i++] = canpybara_rx_frames >> 8;
	can_tx_data[i++] = canpybara_rx_frames;

	// ERR
	can_tx_data[i++] = canpybara_errors >> 8;
	can_tx_data[i++] = canpybara_errors;

	canpybara_can_tx(&can_tx_header, can_tx_data);
}

void canpybara_can_rx(CAN_HandleTypeDef* hcan)
{
	canpybara_rx_frames++;

	CAN_RxHeaderTypeDef can_rx_header;
	uint8_t can_rx_data[8];
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &can_rx_header, can_rx_data) != HAL_OK)
	{
		LOG("Error when getting CAN rx message");
		Error_Handler();
	}

	int request_id = CANPYBARA_CONTROLLER_REQUESTID(can_rx_header.StdId);

	LOG("Request id: %d for %"PRIu32, request_id, can_rx_header.StdId);

	switch(request_id)
	{
		case CANPYBARA_REQUEST_STATUS:
			if (can_rx_header.RTR == CAN_RTR_REMOTE)
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
			if (can_rx_header.DLC == 4)
			{
				canpybara_bootloader_erase(*((uint32_t*)can_rx_data));
			}
			break;

		case CANPYBARA_REQUEST_BOOTLOADER_WRITE:
			if (can_rx_header.DLC == 8)
			{
				canpybara_bootloader_write(*((uint64_t*)can_rx_data));
			}
			break;

		default:
			LOG("Unknown request ID: %d", request_id);
	}
}

void canpybara_can_tx(CAN_TxHeaderTypeDef* can_tx_header, uint8_t data[8])
{
	uint32_t mailbox_no;
	if(HAL_CAN_AddTxMessage(&hcan, can_tx_header, data, &mailbox_no) != HAL_OK)
	{
		LOG("CAN add message failure");
		Error_Handler();
	}
}