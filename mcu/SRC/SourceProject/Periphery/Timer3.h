#pragma once

#include <functional>

/**
 * @file timer3.h
 * @brief ������� ��� ������ � �������� 3
 * @details �������� ��������� � ������� ���������� ���������� �������� 3.
 */

/**
 * @def TimeoutTim3
 * @brief ������� �� ��������� ��� ������� 3
 * @note �������� 999 ������������� 1 ������� (��� ������� ������� 1 ���)
 */
#define TimeoutTim3 (999)

/**
 * @class Timer3
 * @brief ����� ��� ���������� �������� 3
 * 
 * ��������� ���������� �������������, ������� � ��������� ������� 3,
 * � ����� ��������� ������� �������� ����� callback-�������.
 */
class Timer3 {
public:
    /**
     * @brief ����������� ������ Timer3
     */
    Timer3();
    
    /**
     * @brief ���������� ������ Timer3
     */
    ~Timer3();
    
    /**
     * @brief �������� ��������� ������ (��������)
     * @return ��������� �� ������������ ��������� Timer3
     */
    static Timer3* instance() {return m_instance;};
    
    /**
     * @brief ������������� ������� 3
     * @details ����������� ���������� ������ 3 � �����������:
     * - ������������: 0
     * - ������: 999 (1 ��� ��� 1 ���)
     * - �����: �����
     */
    static void init();
    
    /**
     * @brief ������ ������� 3
     */
    static void start();
    
    /**
     * @brief ��������� ������� 3
     */
    static void stop();
    
    /**
     * @brief Callback-������� ��������� ��������
     * @details ���������������� �������, ���������� �� ��������� �������� �������.
     * 
     * ������ �������������:
     * @code
     * Timer3::handlerTimeout = []() {
     *     // �������� �� ��������
     * };
     * @endcode
     */
    static std::function<void()> handlerTimeout;
        
private:
    static Timer3 *m_instance; ///< ��������� �� ������������ ��������� ������
    
    /**
     * @brief ����� �������� ������� (� �����)
     * @note �������� �� ��������� 999 (1 ��� ��� 1 ���)
     */
    static const int m_timeout = 999;
};
