#pragma once

enum ReturnCodeKey
{
	ReturnCodeKeyStart               = 0x1000,
	ReturnCodeKeyCancelSettings,
	ReturnCodeKeyApplySettings,
	ReturnCodeKeyStop,
	ReturnCodeKeySaveProgramm,
	ReturnCodeKeyUpSelectProgramm,
	ReturnCodeKeyDownSelectProgramm,
	ReturnCodeKeyInMenuListProgramms,
	ReturnCodeKeyInMenuSettingsProgramms,
	ReturnCodeKeyExitFromMenu,
	ReturnCodeKeyViewWorkMode,
	ReturnCodeKeyEditWorkMode,
	ReturnCodeKeyAddWorkMode,
	ReturnCodeKeyDeleteWorkMode,
	
		
	ReturnCodeKeySwitchWorkModeUp  = 0x8001,
	ReturnCodeKeySwitchWorkModeDown,	//0x8002
	ReturnCodeKeySaveWorkMode,			//0x8003
	ReturnCodeKeyAddStageWorkMode,		//0x8004
	ReturnCodeKeyDeleteStageWorkMode	//0x8005
};