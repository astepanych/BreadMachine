#pragma once

#include <stdint.h>
#include <functional>
#include <vector>

#include "common_project.h"

#include <stdint.h>
#ifdef STM32F407xx
#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#else 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#endif
#include <functional>


#define IsUartInterface 0xffffffff   //признак подключения по RS232

#define SIZE_BUF_RX 256				 //размер буфера приема


#define SIZE_PACKAGE_HEADER 8		//размер заголовка в пакете протокола обмена по RS232
#define SIZE_PACKAGE_DATA 32		//размер данных в пакете протокола обмена по RS232
#define SIZE_PACKAGE_CRC 2			//размер контрольной суммы в пакете протокола обмена по RS232
#define SIZE_PACKAGE (SIZE_PACKAGE_DATA + SIZE_PACKAGE_HEADER + SIZE_PACKAGE_CRC)  //размер  пакета протокола обмена по RS232

#define NUM_ATTEMPS	5				//максимальное количество попыток отправки одного пакета

#define BYTE_CRC_OK    0x77			//байт подтверждения, когда пакет принят успешно
#define BYTE_CRC_BAD    0xAA		//байт подтверждения, когда пакет принят неуспешно

#define CMD_SET  1					//команда установки параметра
#define MASK_COUNT_MESSAGE  0xff		//маска для поля счетчика пакетов в заголовке протокола обмена


#define TIMEOUT_WAIT_CONFIRM (40/portTICK_PERIOD_MS)		    // таймаут ожидания подтверждения в микросекундах
#ifndef STM32F407xx
#define TIMEOUT_COLLISION (80/portTICK_PERIOD_MS)				    // таймаут разрешения коллизий в микросекундах
#else
#define TIMEOUT_COLLISION (40/portTICK_PERIOD_MS)				    // таймаут разрешения коллизий в микросекундах
#endif
#define TIMEOUT_RESEND (10/portTICK_PERIOD_MS)			        // таймаут повторной отправки после таймаута разрешения
#define TIMEOUT_RESEND_AFTER_BAD_CRC (20/portTICK_PERIOD_MS)	// таймаут повторной отправки после полученая байт о неверной контрольной сумме
#define TIMEOUT_WAIT_DATA (10/portTICK_PERIOD_MS)	            // таймаут ожидания данных

#define TIMEOUT_DENY_SEN (30/portTICK_PERIOD_MS)	// таймаут разрешения коллизий в микросекундах





#define objDataExchenge DataExchenge::instance()

/*!
 * \brief Пречисление StateProtocol описывает состояния протокола обмена
 */
enum StateProtocol {
	StateIdle, ///< состояние бездействия
	StateRx, ///< состояние приема данных
	StateWaitConfirm, ///< состояние ожидания подтверждения
	StateCollision,
	///< состояние коллизии
	StateResend, ///< состояние повторной отправки пакета
};



/*!
 * \brief Класс Interface232 реализует интерфейс и протокол обмена по Rs232 пульта и РС
 */
class DataExchenge {
		
public:
	
	void startTimer() ;
	static void statisticTask();
	/*!
	 * \brief instance создает экземпляр класса по шаблону singleshot
	 * \return
	 */
	static DataExchenge &instance();
	
	void stopTimer(TimerHandle_t xTimer);
    
	/*!
	 * \brief putByte заполняет буфер приема
	 * \param byte - данные
	 * \param len - длина
	 */
	void putByte(const uint8_t *byte, const uint8_t len);
	/*!
	 * \brief buildPackage собирает пакет для отправки в последовательный интерфейс и кладет его в очерадь
	 * \param Id - идентификатор пакета
	 * \param cmd - команда
	 * \param len - длина данных
	 * \param data - данные
	 */
	void buildPackage(int16_t Id, int8_t cmd, int16_t len, uint8_t *data, PackageNetworkFormat &out);
	
	void sendPackage(int16_t Id, int8_t cmd, int16_t len, uint8_t *data);
	/*!
	 * \brief parsePackage произоводит разбор пакета и его обработку, пересылку на терминал
	 * \param buf - данные
	 */
	void parsePackage(uint8_t *buf) ;

	int readPack(uint8_t *buf) ;

	int writePack(const uint8_t *buf) ;
	/*!
	 * \brief protocol поток обработки протокола
	 */
	void protocol();
	/*!
	 * \brief postMutex постит семафор для потока обработки протокола
	 */
	void postMutex() { 
		xSemaphoreGive(mutex);
		portYIELD();
	}
	
	/*!
		\brief Отправляет  на РС событие остановки таймера контроля подключения ПК к РС
	*/
	void stopTimerAnalizeConnectToRs();
	/*!
    	\brief сбрасывает индексы приема в 0 и переводит состояние протокола в StateIdle
	*/
	void resetRxIndex();
	
	static uint16_t CRC_Calc_s16_CCITT(uint16_t *PayLoad, uint16_t iBitsNum);
	std::function<void(uint8_t*, uint8_t)> sendBytes; 
	std::function<void(PackageNetworkFormat&)> procPack; 
private:
	/*!
     * \brief Interface232 конструктор
     */
	DataExchenge(); 

	uint8_t alignas(2) bufRx[SIZE_BUF_RX]; ///< буфер приема
	uint8_t alignas(2) bufRxPackage[SIZE_PACKAGE]; ///< приянтый пакет 
	uint8_t alignas(2) bufTxPackage[SIZE_PACKAGE]; ///< пакет для отправки
	uint16_t indexFill; ///< индекс приянтых данных 
	uint16_t indexWork; ///< индекс обработанных данных
	uint16_t cntBytes; ///< количество прияных данных пакета
	xSemaphoreHandle mutex; ///< семафор для потока протокола
	TaskHandle_t taskProtocol; ///< дескриптор потока протокола
	uint16_t stateProtocol; ///< состояние протокола обмена
	xQueueHandle txQueue; ///< очередь на передачу
	xTimerHandle timerProtocol; ///< таймер ожидания подтверждения, разрешения коллизий
	xTimerHandle timerDenyTx; ///< таймер запрета передачи данных после получения подтверждения об успешной отправке пакета
	xTimerHandle timerWaitPacket; ///< таймер ожидания данных
	uint16_t countAttemps; ///< счетчик попыток отправки пакета
	uint8_t countMessage; ///< счетчик номера пакета на передачу 
	uint8_t countMessageRx; ///< счетчик принятых пакетов
	bool isCollision; ///< состояние коллизии
	/*!
    	\brief отправляет пакет в уарт и переводит состояние протокола StateWaitConfirm
	*/
	void sendPacket();	                                      
	
	/*!
    	\brief производит обработку принятых пакетов и(или) их ретрансляцию на терминал
	*/
	void parsePackage();    
	
	void sendByte(const uint8_t byte);

};