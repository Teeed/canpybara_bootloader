#include "main.h"

#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

__attribute__((used)) int _write(int file, char *data, int len) 
{
   // arbitrary timeout 1000
   HAL_StatusTypeDef status =
      HAL_UART_Transmit(&huart1, (uint8_t*)data, len, 1000);

   // return # of bytes written - as best we can tell
   return (status == HAL_OK ? len : 0);
}
