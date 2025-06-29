#pragma once
#include "MyList.h"

/**
 * @file work_mode_edit.h
 * @brief ������������ ���� ��� �������������� ������� �������
 * @details �������� ����������� ������� � ����� ��� ������ � ���������� ������� �������
 */

/**
 * @defgroup display_addresses ������ �������
 * @brief ������ �������� ������� ��� ����������� � �������������� ������� �������
 * @{
 */
#define StartAddrListEditItemsVP 0x64a0  ///< ��������� ����� ������ ������� (��������)
#define StartAddrListEditItemsSP 0x6400  ///< ��������� ����� ������ ������� (��������������)
#define OffsetColorsText 3               ///< �������� ��� ������ ������
#define StepListEditVP 0x40              ///< ��� ����� ���������� � ������ ���������
#define StepListEditSP 0x20              ///< ��� ����� ���������� � ������ ��������������
#define AddrScrolBar 0x65e0              ///< ����� ������ ���������

/// @name ������ ����� � ������ ���������
/// @{
#define AddrNameV  0x6620        ///< ����� �������� ������ (��������)
#define AddrNumStageV  0x6660     ///< ����� ������ ����� (��������)
#define AddrTimeStageV  0x6670    ///< ����� ������� ����� (��������)
#define AddrTempStageV  0x6680    ///< ����� ����������� ����� (��������)
#define AddrWaterStageV  0x6682   ///< ����� ������ ���� ����� (��������)
#define AddrDamperStageV  0x6684  ///< ����� ��������� �������� ����� (��������)
#define AddrFanStageV  0x6686     ///< ����� �������� ����������� ����� (��������)
/// @}

/// @name ������ ����� � ������ ��������������
/// @{
#define AddrNameE  0x6720         ///< ����� �������� ������ (��������������)
#define AddrNumStageE  0x6760     ///< ����� ������ ����� (��������������)
#define AddrTimeStageE  0x6770    ///< ����� ������� ����� (��������������)
#define AddrTempStageE  0x6780    ///< ����� ����������� ����� (��������������)
#define AddrWaterStageE  0x6782   ///< ����� ������ ���� ����� (��������������)
#define AddrDamperStageE  0x6784  ///< ����� ��������� �������� ����� (��������������)
#define AddrFanStageE  0x6786     ///< ����� �������� ����������� ����� (��������������)
/// @}
/** @} */ // ����� ������ display_addresses

/**
 * @enum StateEditWorkMode
 * @brief ��������� ��������� ������� �������
 */
typedef enum 
{
    StateView = 0,  ///< ����� ���������
    StateEdit,      ///< ����� ��������������
    StateNew        ///< ����� �������� ������
} StateEditWorkMode;

/**
 * @class WorkModeEdit
 * @brief ����� ��� �������������� ������� �������
 * @extends MyList
 * 
 * ������������� ���������� ��� ���������, �������������� � ��������
 * ������� ������� � ������������ �� �������.
 */
class WorkModeEdit : public MyList
{
public:
    /**
     * @brief ����������� ��������� ������� �������
     * @param p ��������� �� ������� �������
     * @param maxNumItem ������������ ���������� ���������
     */
    WorkModeEdit(DisplayDriver *p, uint16_t maxNumItem);
    
    /**
     * @brief ���������� ������� ������
     * @param key ��� ������� �������
     * @return ��������� �� ������ ��� ��������� �������
     */
    Widget* keyEvent(uint16_t key);
    
    /**
     * @brief ��������� ���������� ������
     * @param id ������������� ���������
     * @param len ����� ������
     * @param data ��������� �� ������
     */
    void changeParams(const uint16_t id, uint8_t len, uint8_t* data);
    
    /**
     * @brief Callback-������� ���������� ������� �������
     * @details ���������� ��� ���������� ��������� � ������� �������
     */
    std::function<void()> saveWorkModes;
    
private:
    /**
     * @brief ��������� �������� �������� ������
     * @param addrItem ����� ����������� �� �������
     */
    void paintNameWorkMode(uint16_t addrItem);
    
    /**
     * @brief ��������� ���������� �������� ������
     * @param isEdited ���� ������ ��������������
     */
    void paintSettingsWorkMode(bool isEdited = false);
    
    /**
     * @brief ����� ���� ��������� ���������� ������
     */
    void printAllTimeMode();
    
    /**
     * @brief ����������� � ���������� ���������
     * @param buf ����� ������
     * @param len ����� ������
     */
    void toMyCodec(uint8_t *buf, const uint16_t len);
    
    /**
     * @brief ����������� � ��������� Windows-1251
     * @param buf ����� ������
     * @param len ����� ������
     */
    void to1251(uint8_t *buf, const uint16_t len);
    
    WorkMode tempWMode;              ///< ��������� ������� ����� ��� ��������������
    int16_t currentStage;            ///< ������� ���� ��������������
    StateEditWorkMode stateEdited;   ///< ������� ��������� ���������
};

