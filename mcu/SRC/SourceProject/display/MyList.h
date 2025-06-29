#pragma once
#include "Widget.h"
#include "WorkMode.h"
#include <list>
#include <string>
#include <vector>
#include <functional>

struct ElementList
{
	uint16_t addrColor;
	uint16_t addrText;
};

/**
 * @class MyList
 * @brief �������������� ������ � ������� ���������
 * 
 * @extends Widget
 * 
 * ����� ������������� ���������� ���:
 * - ����������/�������� ���������
 * - ��������� (�����/����)
 * - �������� ������ WorkMode
 * - ���������� �����������
 * - ���������� ���������� ���������
 * 
 * @note ������������ ��� ������������ ������ � ������������� ���������.
 */
class MyList : public Widget {
public:
    /**
     * @brief �����������
     * @param p ��������� �� ������� �������
     * @param maxNumItem ������������ ���������� ������������ ���������
     */
    MyList(DisplayDriver *p, uint16_t maxNumItem);

    /**
     * @brief ���������� ������� ������
     * @param key ��� ������� (��������, ������� �����/����)
     * @return Widget* ��������� ������ � ������� ���������
     */
    Widget* keyEvent(uint16_t key) override;

    /**
     * @brief ��������� ������� � ������
     * @param el ������ �� ������� ���� ElementList
     */
    void addItemHard(ElementList &el);

    /**
     * @brief ����������� ������ WorkMode
     * @param p ��������� �� ������ � ������� WorkMode
     */
    void setWModes(std::vector<WorkMode> *p) { pWModes = p; };

    /**
     * @brief ���������� ����� ��������
     * @param index ������� ��������
     * @return std::string ����� ��������
     */
    std::string text(uint16_t index);

    /**
     * @brief ������ � �������� �� �������
     * @param index ������� ��������
     * @return ElementList ������ �� �������
     */
    ElementList at(int index);

    /**
     * @brief ���������� ���������� ���������
     * @return int ������ ������
     */
    int length();

    /**
     * @brief ������� ������
     */
    void clear();

    /**
     * @brief ��������� ����������� ������
     * @details ������ ���� ���������� ��� ���������� ����������
     */
    void updateDisplay() override;

    /**
     * @brief ���������� ��������� ������
     */
    void resetWidget() override;

    /**
     * @brief ������������� ����� �������� ������ ���������
     * @param addr ����� � ������ ��� ���������� ��������
     */
    void setAddrScrollValue(uint16_t addr) { addrScrollValue = addr; };

    /**
     * @brief ������ ��� ��������� ������
     * @param index ����� ��������� ������
     */
    std::function<void(int)> changeValue;

    /**
     * @brief ������������� ������� ��������� ������
     * @param newIndex ������ ��� ������
     * @note �������� ������ changeValue
     */
    inline void setIndex(uint16_t newIndex) {
        currentIndex = newIndex;
        changeValue(currentIndex);
    }

    /**
     * @brief ���������� ������� ��������� ������
     * @return uint16_t �������� ������
     */
    inline uint16_t currentIndexValue() { return currentIndex; };

protected:
    std::vector<ElementList> lst; ///< ��������� ���������
    std::vector<WorkMode> *pWModes; ///< �������� ������ WorkMode
    uint16_t indexDisplay; ///< ������ ������� �������� ��������
    int16_t startIndex; ///< �������� ��� ���������
    int16_t indexCommon; ///< ����� ������ ��� ���������
    uint16_t maxItemDisplay; ///< ����. ������� ���������
    uint16_t maxPosItemDisplay; ///< ����. ������� ���������
    uint16_t currentIndex; ///< ������� ��������� ������
    uint16_t currentPos; ///< ������� ������� ���������
    uint16_t addrScrollValue; ///< ����� �������� ���������
    const uint16_t maxValueScroll = 84; ///< ������������ �������� ���������
};

