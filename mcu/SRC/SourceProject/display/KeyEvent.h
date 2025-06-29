#pragma once
/**
 * @enum ReturnCodeKey
 * @brief Коды возврата для обработки нажатий клавиш
 * 
 * Перечисление содержит коды действий, связанных с навигацией по меню
 * и управлением программой. Группировка кодов выполнена по функциональности.
 */
enum ReturnCodeKey {
    /// @name Основные команды управления (группа 0x1000)
    /// @{
    ReturnCodeKeyStart = 0x1000, ///< Запуск программы
    ReturnCodeKeyExtendedSettings, ///< Вход в расширенные настройки
    ReturnCodeKeyApplySettings, ///< Применить настройки
    ReturnCodeKeyStop, ///< Остановка программы
    ReturnCodeKeySaveProgramm, ///< Сохранить программу
    ReturnCodeKeyUpSelectProgramm, ///< Выбор программы вверх
    ReturnCodeKeyDownSelectProgramm,  ///< Выбор программы вниз
    ReturnCodeKeyInMenuListProgramms, ///< Нахождение в меню списка программ
    ReturnCodeKeyInMenuSettingsProgramms, ///< Нахождение в настройках программы
    ReturnCodeKeyExitFromMenu, ///< Выход из меню
    ReturnCodeKeyViewWorkMode, ///< Просмотр режима работы
    ReturnCodeKeyEditWorkMode, ///< Редактирование режима работы
    ReturnCodeKeyAddWorkMode, ///< Добавление режима работы
    ReturnCodeKeyDeleteWorkMode,///< Удаление режима работы
    ReturnCodeKeyMainSettings, ///< Основные настройки
    ReturnCodeKeyRunSettings, ///< Настройки запуска
    /// @}

    /// @name Команды WiFi меню (группа 0x1010)
    /// @{
    ReturnCodeKeyWifiMenu = 0x1010, ///< Вход в меню WiFi
    ReturnCodeKeyWifiMenuExit, ///< Выход из меню WiFi
    ReturnCodeKeyWifiMenuApply, ///< Применить настройки WiFi
    ReturnCodeKeyExitMenuTest, ///< Выход из меню тестирования
    ReturnCodeKeySoundTest,  ///< Тест звука
    ReturnCodeKeyInMenuTest, ///< Нахождение в меню тестирования
    ReturnCodeKeyPlaySoundTest, ///< Воспроизвести тестовый звук
    /// @}

    /// @name Команды сообщений (группа 0x2000)
    /// @{
    ReturnCodeKeyHideMsg = 0x2000, ///< Скрыть сообщение
    /// @}

    /// @name Команды управления режимами работы (группа 0x8000)
    /// @{
    ReturnCodeKeySwitchWorkModeUp = 0x8001, ///< Переключить режим работы вверх
    ReturnCodeKeySwitchWorkModeDown, ///< Переключить режим работы вниз (0x8002)
    ReturnCodeKeySaveWorkMode, ///< Сохранить режим работы (0x8003)
    ReturnCodeKeyAddStageWorkMode, ///< Добавить этап в режим работы (0x8004)
    ReturnCodeKeyDeleteStageWorkMode               ///< Удалить этап из режима работы (0x8005)
    /// @}
};