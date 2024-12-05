#include "uart1_driver.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"

static const char *TAG = "uart";

#define RX_BUF_SIZE_UART0 380 //размер буфера приема

#define BIT_RATE_UART0 115200 //скорость уарта0

//перемаплены пины согласно схеме КЛСИ.468389.024 Э3
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_18)

#define sizeQueue  16 //размер очереди событий

#define UARTNUM UART_NUM_2

static fnxProcessRxData processReadedData0;

/*!
    \brief оправляет данные в уарт0
    \param data данные
    \param len длина
    \return возвращает количество отправленых данных
*/
int  sendDataToUart(const uint8_t* data, const uint8_t len) {
	
	const int txBytes = uart_write_bytes(UARTNUM, data, len);
#ifndef STM32F407xx
	//ESP_LOGI(TAG, "txBytes = %d", txBytes);
#endif
	return txBytes;
}
/*!
    \brief отправляет байт в уарт0
    \param data отправляемый байт
    \return возвращает количество отправленых данных
*/
int  sendByteToUart(const uint8_t data) {
	
	const int txBytes = uart_write_bytes(UARTNUM, &data, 1);
	return txBytes;
}

//буфер для приема данных по уарт0
static uint8_t gBufRxUart0[RX_BUF_SIZE_UART0];
//очередь событий уарт0
static QueueHandle_t uart0Queue = 0;

/*!
    \brief поток обработки событий уарт0
    \param arg Summary for arg
*/
static void rx_task0(void *arg) {
	
	//настройки уарта
	const uart_config_t uart_config =  {
		.baud_rate = BIT_RATE_UART0,
		//скорость
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB,
	};
	const uint16_t sizeThresholdRx = 7; //размер очереди приема уарта  
	//инициализация драйвера перенесена в таск чтобы прерывания уарта обрабатывались на ядре 1, т.к. на 0 ядре не удавалось разместить прерывания
	//данный таск выполняется на втором ядре и прерывания так же будут обрабатыватся на этом же ядре
	//https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/system/intr_alloc.html#_CPPv413esp_intr_dumpP4FILE
	esp_err_t res = uart_driver_install(UARTNUM, RX_BUF_SIZE_UART0 * 2, 0, sizeQueue, &uart0Queue, ESP_INTR_FLAG_LEVEL3);
	if (res != ESP_OK)
		asm(" nop;");
	uart_param_config(UARTNUM, &uart_config);
	uart_set_pin(UARTNUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_set_rx_full_threshold(UARTNUM, sizeThresholdRx);
	size_t availibleBytes; //количество доступных байт для чтения
	uart_event_t ev; //событие

	while (1) {
		//ожидаем событие
		xQueueReceive(uart0Queue, &ev, portMAX_DELAY);
		if (ev.type == UART_DATA) { //событие о принятии данных
			
			uart_get_buffered_data_len(UARTNUM, &availibleBytes); //получаем количество доступных данных для чтения
			if (availibleBytes != 0) { //если есть данные
				const uint8_t rxBytes = uart_read_bytes(UARTNUM, gBufRxUart0, availibleBytes, 0); //читаем данные
				//ESP_LOGI(TAG, "%d ", rxBytes);
				if (rxBytes > 0) { // если прочитли данные приняты
					processReadedData0(gBufRxUart0, rxBytes); //отправляем их на обработку
				}
				
			} 
		}
	}
}


/*!
    \brief Инициализирует уарт0
    \param cbFunc указатель на функцию обратного вызова, обрабатывающую принятые данные из уарт0
*/
void initUart(fnxProcessRxData cbFunc) {
	processReadedData0=cbFunc;
	const uint16_t sizeStackTask = 4096;	//размер стека для потока обработки событий
	// We won't use a buffer for sending data.
	uart0Queue = xQueueCreate(sizeQueue, sizeof(uart_event_t)); //создаем очередь
	//запускаем таск на ядре 1, т.к. на ядре 0 не получалось разместить прерывания
	//и в связи с тем, что перерывания размещаются на том ядре, на котором происходит инициализация драйвера - поток rx_task0 перенесен на ядро 1
	xTaskCreatePinnedToCore(rx_task0, "uart_rx_task", sizeStackTask, NULL, 10, NULL, 0);

}
