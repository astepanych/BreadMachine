#pragma once
/**
 * @enum ReturnCodeKey
 * @brief ���� �������� ��� ��������� ������� ������
 * 
 * ������������ �������� ���� ��������, ��������� � ���������� �� ����
 * � ����������� ����������. ����������� ����� ��������� �� ����������������.
 */
enum ReturnCodeKey {
    /// @name �������� ������� ���������� (������ 0x1000)
    /// @{
    ReturnCodeKeyStart = 0x1000, ///< ������ ���������
    ReturnCodeKeyExtendedSettings, ///< ���� � ����������� ���������
    ReturnCodeKeyApplySettings, ///< ��������� ���������
    ReturnCodeKeyStop, ///< ��������� ���������
    ReturnCodeKeySaveProgramm, ///< ��������� ���������
    ReturnCodeKeyUpSelectProgramm, ///< ����� ��������� �����
    ReturnCodeKeyDownSelectProgramm,  ///< ����� ��������� ����
    ReturnCodeKeyInMenuListProgramms, ///< ���������� � ���� ������ ��������
    ReturnCodeKeyInMenuSettingsProgramms, ///< ���������� � ���������� ���������
    ReturnCodeKeyExitFromMenu, ///< ����� �� ����
    ReturnCodeKeyViewWorkMode, ///< �������� ������ ������
    ReturnCodeKeyEditWorkMode, ///< �������������� ������ ������
    ReturnCodeKeyAddWorkMode, ///< ���������� ������ ������
    ReturnCodeKeyDeleteWorkMode,///< �������� ������ ������
    ReturnCodeKeyMainSettings, ///< �������� ���������
    ReturnCodeKeyRunSettings, ///< ��������� �������
    /// @}

    /// @name ������� WiFi ���� (������ 0x1010)
    /// @{
    ReturnCodeKeyWifiMenu = 0x1010, ///< ���� � ���� WiFi
    ReturnCodeKeyWifiMenuExit, ///< ����� �� ���� WiFi
    ReturnCodeKeyWifiMenuApply, ///< ��������� ��������� WiFi
    ReturnCodeKeyExitMenuTest, ///< ����� �� ���� ������������
    ReturnCodeKeySoundTest,  ///< ���� �����
    ReturnCodeKeyInMenuTest, ///< ���������� � ���� ������������
    ReturnCodeKeyPlaySoundTest, ///< ������������� �������� ����
    /// @}

    /// @name ������� ��������� (������ 0x2000)
    /// @{
    ReturnCodeKeyHideMsg = 0x2000, ///< ������ ���������
    /// @}

    /// @name ������� ���������� �������� ������ (������ 0x8000)
    /// @{
    ReturnCodeKeySwitchWorkModeUp = 0x8001, ///< ����������� ����� ������ �����
    ReturnCodeKeySwitchWorkModeDown, ///< ����������� ����� ������ ���� (0x8002)
    ReturnCodeKeySaveWorkMode, ///< ��������� ����� ������ (0x8003)
    ReturnCodeKeyAddStageWorkMode, ///< �������� ���� � ����� ������ (0x8004)
    ReturnCodeKeyDeleteStageWorkMode               ///< ������� ���� �� ������ ������ (0x8005)
    /// @}
};