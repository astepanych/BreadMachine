#pragma once
#include <stdint.h>
#include <Uart3.h>
#include <Uart1.h>
#include <string>
#include "typedef.h"
#include "1251/utf8_to_cp1251.h"

/**
 * @file display_driver.h
 * @brief ������������ ���� ��� ������ � ��������
 * 
 * �������� ��������� �������, ������� � ����� ��� ���������� ��������.
 */

/** @defgroup display_consts ��������� �������
 *  @{
 */

/// ����� ������� �������� �������
constexpr uint16_t AddrCurrentPage = 0x0014;

/// ����� ������� �������
constexpr uint16_t AddrOverlay = 0x00e8;

/**
 * @brief ����� ������ �� ���
 * @details ����� � ������ �������, � ������� �������� ����� ������ �� ��� ��� �����������
 */
constexpr uint16_t CmdSoftVersion = 0x2000;


constexpr uint16_t AddrRtc = 0x0010;
constexpr uint16_t AddrDamper = 0x1200;
constexpr uint16_t AddrFan = 0x1202;

constexpr uint16_t CmdDateTime = 0x2014;
constexpr uint16_t CmdSetDateTime = 0x009c;
constexpr uint16_t CmdNumProgramm = 0x4000;
constexpr uint16_t AddrScrollMainList = 0x4002;
constexpr uint16_t AddrProgressBar = 0x4004;
constexpr uint16_t AddrNumStage = 0x4006;
constexpr uint16_t AddrNumWater = 0x4008;
constexpr uint16_t AddrNumTemperature = 0x400a;
constexpr uint16_t AddrNumDamper = 0x400E;
constexpr uint16_t AddrNumFan = 0x4010;
constexpr uint16_t AddrNumTime = 0x4012;
constexpr uint16_t AddrNumTemperatureMeasure = 0x4020;
constexpr uint16_t AddrNumDurationNew = 0x4028;
constexpr uint16_t AddrMessage = 0x4040;
constexpr uint16_t AddrNumTimeMode = 0x4080;
constexpr uint16_t AddrNumTimeModeEdit = 0x40a0;

constexpr uint16_t AddrStages = 0x7000;
constexpr uint16_t CmdPaintFillRectangle = 0x0004;	
constexpr uint16_t addrMainItem = 0x2220;
constexpr uint16_t addrNameProg = 0x2280;

constexpr uint16_t addrCurrentSound = 0x2300;
constexpr uint16_t addrCurrentVolume = 0x2301;


constexpr uint16_t addrPassword = 0x6870;
constexpr uint16_t addrK1 = 0x6860;
constexpr uint16_t addrK2 = 0x6864;
constexpr uint16_t addrPeriod = 0x6868;
constexpr uint16_t addrAddWater = 0x686a;


constexpr uint16_t addrUpT = 0x6890;
constexpr uint16_t addrDownT = 0x6892;
constexpr uint16_t addrEnFan = 0x6894;
constexpr uint16_t addrWater = 0x6896;
constexpr uint16_t addrStrTempTest = 0x6f40;
constexpr uint16_t addrDamperOpen = 0x6898;
constexpr uint16_t addrDamperClose = 0x689a;
constexpr uint16_t addrGreenLed = 0x689C;
constexpr uint16_t addrYellowLed = 0x689E;
constexpr uint16_t addrMainRele = 0x689f;

constexpr uint16_t addrStateWifi = 0x68E0;
constexpr uint16_t addrStateWifiIcon = 0x68E2;
constexpr uint16_t addrStateWifiSSID = 0x68A0;
constexpr uint16_t addrStateWifiPassword = 0x68c0;

constexpr uint16_t CmdCofirm = 0x4f4b;

/**
 * @brief ��������� �������������� ��� ���������
 * 
 * ������������ ��� ������ ��������� �� �������
 */
struct Rectangle {
    u16be beginX; ///< ��������� ���������� X
    u16be beginY; ///< ��������� ���������� Y
    u16be endX; ///< �������� ���������� X
    u16be endY; ///< �������� ���������� Y
    u16be color; ///< ���� ��������������
};



/**
 * @class DisplayDriver
 * @brief ������� ��� ������ � ��������
 * 
 * ����� ������������� ���������� ��� �������� ������ � ������ �� �������,
 * ��������� �������� ������ � ���������� ���������� �������.
 */
class DisplayDriver {
    /**
     * @enum StateParcePacketDisplay
     * @brief ��������� �������� ������� �� �������
     */
    enum StateParcePacketDisplay {
        StateWaitByte1, ///< �������� ������� ����� ������
        StateWaitByte2, ///< �������� ������� ����� ������
        StateWaitLen, ///< �������� ����� ����� ������
        StateReadByte       ///< ������ ������ ������
    };

public:
    /**
     * @brief ����������� �������� �������
     */
    DisplayDriver();
    
    /**
     * @brief �������� ��������� �������� (��������)
     * @return ��������� �� ��������� DisplayDriver
     */
    static DisplayDriver* instance() {return m_instance;};
    
    /**
     * @brief ������������� �������
     */
    static void initDisplay();    
    
    /**
     * @brief ��������� ���� �� �������
     * @param byte ���� ��� ��������
     */
    static void putByte(const uint8_t byte); 
    
    /**
     * @brief ��������� ������ �� �������
     * @param id ������������� �������/������
     * @param len ����� ������
     * @param data ��������� �� ������
     */
    static void sendToDisplay(uint16_t id, uint8_t len, uint8_t *data);
    
    /**
     * @brief ��������� ������ �� �������
     * @param id ������������� �������/������
     * @param str ������ ��� ��������
     */
    static void sendToDisplay(const uint16_t id, const std::string &str);
    
    /**
     * @brief ��������� ������ �� ������� (C-style)
     * @param id ������������� �������/������
     * @param len ����� ������
     * @param data ��������� �� ������
     */
    static void sendToDisplayStr(uint16_t id, uint8_t len, char *data);

    /**
     * @brief ��������� 16-������ �������� �� �������
     * @param id ������������� �������/������
     * @param data �������� ��� ��������
     */
    static void sendToDisplay(uint16_t id, uint16_t data);
    
    /**
     * @brief �������� ������ � �������
     * @param id ������������� �������/������
     * @param data ����� ��� ������
     * @param len ��������� ����� ������
     */
    static void getDataFromDisplay(uint16_t id, uint16_t data, uint8_t len);
    
    /**
     * @brief ��������� float �������� �� �������
     * @param id ������������� �������/������
     * @param data �������� ��� ��������
     */
    static void sendToDisplayF(uint16_t id, float &data);
    
    /**
     * @brief ����������� �������� �������
     * @param page ����� ��������
     */
    static void switchPage(uint16_t page);
    
    /**
     * @brief �������� ��������� �� �������
     * @param idPage ������������� ��������
     * @param ipMessage ������������� ���������
     */
    static void showMessage(uint16_t idPage, uint16_t ipMessage);
    
    /**
     * @brief ������ ��������� �� �������
     */
    static void hideMessage();
    
    /**
     * @brief ������ ��� ����� ������ �� �������
     * 
     * ������� ����� ������� ��� ��������� ����� ������� �� �������
     */
    static std::function<void(const uint16_t, uint8_t, uint8_t*)> newCmd;
    
    /**
     * @brief ����� �������
     */
    static void reset();
    
    /**
     * @brief ������������� ����
     * @param numSound ����� �����
     * @param volume ��������� (0-255)
     */
    void playSound(uint8_t numSound, uint8_t volume);
    
private:
    static DisplayDriver *m_instance; ///< ��������� �� ��������� (��������)
    
    /// ��������� ���� ������ 1
    static const uint8_t startByte1 = 0x5a;
    
    /// ��������� ���� ������ 2
    static const uint8_t startByte2 = 0xa5;
    
    /// ���� ������� ������
    static const uint8_t cmdByteWrite = 0x82;
    
    /// ���� ������� ������
    static const uint8_t cmdByteRead = 0x83;
    
    static uint8_t bufParce[SizeBuffer]; ///< ����� ��� �������� �������
    static uint8_t lenPacket; ///< ����� �������� ������
    static uint8_t currentIndex; ///< ������� ������ � ������
    
    /**
     * @brief ������ ��� ������ � ��������
     * @param p ��������� ������ (�� ������������)
     */
    static void taskDisplay(void *p);
    
    /**
     * @brief ������� ������ �� �������
     * @param len ����� ������
     * @param data ������ ������
     */
    static void parsePackFromDisplay(uint8_t len, uint8_t *data);
        
    static int cntPutByte; ///< ������� ������������ ����
    static xQueueHandle xQueueDisplay; ///< ������� ��� �������
    static xSemaphoreHandle xSemDisplay; ///< ������� ��� �������
    TaskHandle_t xHandleDisplay = NULL; ///< ����� ������ �������
    static StateParcePacketDisplay statePacket; ///< ������� ��������� �������
};

