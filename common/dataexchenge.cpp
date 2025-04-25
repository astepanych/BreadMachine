
#include "dataexchenge.h"
#include <string.h>
#ifndef STM32F407xx
	
#include <esp_log.h>			  
#endif // esp


#ifdef STM32F407xx
#define SIZE_STACK_TACK_PROTOCOL	1024
#else
#define SIZE_STACK_TACK_PROTOCOL	4096
#endif

#ifdef STM32F407xx
#include "GpioDriver.h"
#endif



#define MAX_COUTING_SEMAPHORE 20	//максимальное счетчик семафора
#define SIZE_QUEUE_TX 32			//размер очереди на передачу

static const char *TAG = "user uart";

/*!
    \brief Функция обратного вызова для передачи полученных данных из уарт в протокол обработки
    \param data - данные
    \param len - длина
*/
static void putData(const uint8_t *data, const uint8_t len) {

	objDataExchenge.putByte(data, len);
}

/*!
    \brief Поток протокола обмена
    \param arg - не используется
*/
static void protocolTask(void *arg) {

	objDataExchenge.protocol();
}


/*!
    \brief Функция таймаута тамеров для функицонирования протокола
    \param arg не используется
*/
static void protocolTimeout(TimerHandle_t xTimer) {

	objDataExchenge.stopTimer(xTimer);
	objDataExchenge.postMutex();

}


/*!
    \brief Функция таймаута тамеров для функицонирования протокола
    \param arg не используется
*/
static void timeoutWaitData(TimerHandle_t xTimer) {
	objDataExchenge.stopTimer(xTimer);
	objDataExchenge.resetRxIndex();
}


void DataExchenge::stopTimer(TimerHandle_t xTimer)
{
	xTimerStop(xTimer, 0);
}

void DataExchenge::startTimer() {

}

DataExchenge::DataExchenge() {
	//инициализируем уарт
	indexFill = 0;
	indexWork = 0;
	//создаем семафор и поток
	mutex = xSemaphoreCreateCounting(MAX_COUTING_SEMAPHORE, 0);
	txQueue = xQueueCreate(SIZE_QUEUE_TX, SIZE_PACKAGE);
	BaseType_t res = xTaskCreate(protocolTask, "protocolD", SIZE_STACK_TACK_PROTOCOL, NULL, 9, &taskProtocol);
	
	
	timerProtocol = xTimerCreate("timerProtocol", (100 / portTICK_PERIOD_MS), pdFALSE, (void*)2, protocolTimeout);
	timerDenyTx = xTimerCreate("timerDeny", TIMEOUT_DENY_SEN, pdFALSE, (void*)3, protocolTimeout);
	timerWaitPacket = xTimerCreate("timerwait", TIMEOUT_WAIT_DATA, pdFALSE, (void*)4, timeoutWaitData);
	//xTimerStart(timerWaitPacket,0);
	
	//инициализируем счетчик принятых сообщений значением, которого не может быть, т.к. на счетчик сообщений отведено только 2 бита в заголовке
	countMessageRx = 260;
}

DataExchenge &DataExchenge::instance() {
	static DataExchenge obj; 
	return obj;
}


void DataExchenge::statisticTask()
{
#ifndef STM32F407xx
	TaskStatus_t pxTaskStatus;
	TaskHandle_t xHandle;
	

		xHandle = xTaskGetHandle("Tmr Svc");
	
	vTaskGetInfo(xHandle, &pxTaskStatus, pdTRUE, eInvalid);

	//ESP_LOGI(TAG, "s = %d", pxTaskStatus.usStackHighWaterMark);
#endif
}

void DataExchenge::putByte(const uint8_t *byte, const uint8_t len) {
	xTimerStop(timerWaitPacket, 0);
	//сохраняем данные во внутренний буфер
	for (int i = 0; i < len; i++) {
		bufRx[indexFill] = byte[i];
		indexFill = (indexFill + 1) % SIZE_BUF_RX;
	}

	xTimerStart(timerWaitPacket, 0);
	postMutex(); // постим наш семафор для запуска потока


}

void DataExchenge::resetRxIndex() {
	indexFill = indexWork = 0;
	stateProtocol = StateIdle;
	cntBytes = 0;
}
PackageNetworkFormat pack;
void DataExchenge::sendPackage(int16_t Id, int8_t cmd, int16_t len, uint8_t *data) {

	buildPackage(Id, cmd, len, data, pack);
#ifndef STM32F407xx
	ESP_LOGI(TAG, "s = %d", uxQueueMessagesWaiting(txQueue));
#endif
	//кладем собранный пакет в очередь
	xQueueSend(txQueue, &pack, 0);
	//постим семафор
	postMutex();
}

void DataExchenge::buildPackage(int16_t Id, int8_t cmd, int16_t len, uint8_t *data, PackageNetworkFormat &out) {
	memset(&out, 0, sizeof(PackageNetworkFormat));
	out.cmdId = Id;
	out.counter = countMessage;
	out.dataSize = len;
	out.msgType = cmd;
	//наращиваем счетчик
	countMessage = (countMessage + 1) & MASK_COUNT_MESSAGE;
	//копируем данные
	memcpy(out.data, data, len);
	//считаем контрольную сумму
	out.crc = CRC_Calc_s16_CCITT((uint16_t*)&out, (SIZE_PACKAGE_HEADER + SIZE_PACKAGE_DATA) / sizeof(uint16_t));
	
}

void DataExchenge::parsePackage(uint8_t *buf) {
}

int DataExchenge::readPack(uint8_t *buf) {
	return 0;
}

int DataExchenge::writePack(const uint8_t *buf) {
	return 0;
}

void DataExchenge::sendPacket() {
	if (isCollision)
		return;

	//отправляем данные
	sendBytes(bufTxPackage, SIZE_PACKAGE);
	//переходим в состояние ожидания подтверждения
	stateProtocol = StateWaitConfirm;
	//запускаем таймер ожидания подтверждения
	BaseType_t res = xTimerStop(timerProtocol, 1 / portTICK_PERIOD_MS);

	res =  xTimerChangePeriod(timerProtocol, TIMEOUT_WAIT_CONFIRM, 1 / portTICK_PERIOD_MS);

	res = xTimerStart(timerProtocol, 2 / portTICK_PERIOD_MS);
	
#ifndef STM32F407xx
//	ESP_LOGI(TAG, "sendPacket");
#endif
	
}

void DataExchenge::parsePackage() {
	
	PackageNetworkFormat *h = (PackageNetworkFormat*)bufRxPackage;
	
	if (h->counter == countMessageRx && h->cmdId != IdConnectExternSoft && h->cmdId != IdDoneBootloader) //если счетчики совпали, то пакет уже приняли и текущий не обрабатываем
		return;
	countMessageRx = h->counter; // запоминаем считчик принятых сообщений
	procPack(*h);
}

void DataExchenge::protocol() {
	isCollision = false;

	while (true) {
		//высим на семафоре
		xSemaphoreTake(mutex, portMAX_DELAY);
		
		switch (stateProtocol) {
			case StateIdle: //состояние ничегонеделания
				if (indexFill != indexWork) {
					//если индекс заполнения и индекс обработки не совпадают
					cntBytes = 0;
					stateProtocol = StateRx; //переходим в состояния приема данных
				}
				else {
					if (xTimerIsTimerActive(timerDenyTx) || isCollision)//если запущен таймер запрета передачи или была коллизия, то ничего не достаем из очериди и не начинаем передачу
					break;
					//если принятых данных нет, то смотрим очередь на передачу на наличие данных для передачи
					if (xQueueReceive(txQueue, bufTxPackage, 0) == pdTRUE) {
						//пытаемся извлечь из очереди данные
						//если есть данные - отправляем их
						sendPacket();
						countAttemps = 0; //сбрасываем счетчик попыток отправки
					}
					break;
				}
			case StateRx: 
				if (indexFill == indexWork) {
					//если нет данных для обработки - переходим в состояние ничегонеделания
					//stateProtocol = StateIdle;
					break;
				}
				do {
					//из буфера приема достаем байты, количеством SIZE_PACKAGE
					bufRxPackage[cntBytes] = bufRx[indexWork];
					indexWork = (indexWork + 1) % SIZE_BUF_RX;
					cntBytes++;
					//если насобирали нужное нам количество или данные в буфере приема закончились - выходим 
					if (indexWork == indexFill || cntBytes >= SIZE_PACKAGE)
						break;
				} while (true);
				//ESP_LOGI(TAG, "len = %d", cntBytes);
				//байт нужное нам количество
				if (cntBytes >= SIZE_PACKAGE) {
					xTimerStop(timerWaitPacket,0);
					
					//считаем контрольную сумму
					uint16_t crcCalc = CRC_Calc_s16_CCITT((uint16_t*)bufRxPackage, (SIZE_PACKAGE_HEADER + SIZE_PACKAGE_DATA) / sizeof(uint16_t));
					//полученная контрольная сумма
					uint16_t *crcRx = (uint16_t*)(bufRxPackage + SIZE_PACKAGE_HEADER + SIZE_PACKAGE_DATA);
					//сравнение контрольных сумм
					if (crcCalc == *crcRx) {
						//если совпали - отправляем подтверждение, что все хорошо
						sendByte(BYTE_CRC_OK);
#ifndef STM32F407xx
						//ESP_LOGI(TAG, "BYTE_CRC_OK");
#endif
						//парсим пакет дальше
						parsePackage();
						if (isCollision) {
							
							stateProtocol = StateResend;
							xTimerStop(timerProtocol, 0);
							xTimerChangePeriod(timerProtocol, TIMEOUT_RESEND, 1);
							xTimerStart(timerProtocol, 0);
							isCollision = false;
							break;
						}
						
					}
					else {
						//если не совпали - отправляем подтверждение, что все плохо
						sendByte(BYTE_CRC_BAD);
						indexFill = indexWork = 0; //сбрасываем индексы приема
					}
					//переходим в состояние ожидания
					stateProtocol = StateIdle;
					if (uxQueueMessagesWaiting(txQueue) > 0) {
						
						postMutex();
					}
				}

				break;
			case StateWaitConfirm: {

					//состояние ожидания подтверждения
					uint8_t confirm = BYTE_CRC_BAD;
					if (indexWork != indexFill) {
					
						//если приняли данные
						confirm = bufRx[indexWork]; //достаем байт подтверждеия
#ifndef STM32F407xx
		//				ESP_LOGI(TAG, "0x%x", confirm);
#endif
						indexWork = (indexWork + 1) % SIZE_BUF_RX;
						//получили подтверждение успешной отправки пакета
						xTimerStop(timerProtocol,0);
						if (confirm == BYTE_CRC_OK) {
							xTimerStop(timerWaitPacket, 0);
							
							stateProtocol = StateIdle;
							xTimerStart(timerDenyTx, 0); //запускаем таймера для разрешения коллизий
							if (uxQueueMessagesWaiting(txQueue) > 0 || indexWork != indexFill) {
								postMutex();
							}
							break;
						}
						//получили байт о неуспешной отправке подтверждения - повторяем пакет
						if (confirm == BYTE_CRC_BAD) {
							xTimerStop(timerWaitPacket,0);
							stateProtocol = StateResend;
							xTimerStop(timerProtocol, 0);
							xTimerChangePeriod(timerProtocol, TIMEOUT_RESEND_AFTER_BAD_CRC, 1);
							xTimerStart(timerProtocol, 0);
						}
						//если получили байт отличный от байт успешной и неуспешной  отправки - то это коллизия
						if (confirm != BYTE_CRC_BAD && confirm != BYTE_CRC_OK) {
						
							stateProtocol = StateIdle;
							//	indexFill = indexWork = 0;		//сбрасываем индексы приема
							isCollision = true;
							break;
						}
					}
				
					BaseType_t res = xTimerIsTimerActive(timerProtocol);

					//если истек таймер ожидания подтверждения 
					if (res == pdFALSE) {
						stateProtocol = StateResend;
						xTimerStop(timerProtocol, 0);
						xTimerChangePeriod(timerProtocol, TIMEOUT_RESEND, 1);
						xTimerStart(timerProtocol, 0); //запускаем таймер
					}
					break;
				}
			case StateResend://состояние повтора пакета
				if (xTimerIsTimerActive(timerProtocol))
					break;
				countAttemps++;
				if (countAttemps < NUM_ATTEMPS) {
					sendPacket();
				}
				else {
					stateProtocol = StateIdle;
					if (uxQueueMessagesWaiting(txQueue) > 0) {
						postMutex();
					}
				}
				break;
		
			default:
				break;
		}
	}
}

/*!
    \brief Рассчитывает контрольную сумму последовательности
    \param PayLoad поледоветельность для которой будет рассчитываться контрольная сумма
    \param iBitsNum длина последовательности
    \return посчитанная контрольная сумма
*/
uint16_t DataExchenge::CRC_Calc_s16_CCITT(uint16_t *PayLoad, uint16_t iBitsNum) {
	// Update the CRC for transmitted and received data using
	// the CCITT 16bit algorithm (X^16 + X^12 + X^5 + 1).
	uint16_t i;
	uint16_t crc = 0;

	for (i = 0; i < iBitsNum; i++) {
		crc = ((crc >> 8) & 0xFF) | (crc << 8); //перестановка полуслов
		crc ^= PayLoad[i];
		crc ^= ((crc & 0xff) >> 4);
		crc ^= (crc << 12);
		crc ^= ((crc & 0xff) << 5);
	}
	return crc;
}

void DataExchenge::sendByte(const uint8_t byte)
{
	sendBytes((uint8_t*)&byte, 1);
}