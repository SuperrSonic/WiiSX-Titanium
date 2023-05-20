/**
 * Wii64 - InputManager.cpp
 * Copyright (C) 2009 sepp256
 *
 * Wii64 homepage: http://www.emulatemii.com
 * email address: sepp256@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
**/

#include "InputManager.h"
#include "FocusManager.h"
#include "CursorManager.h"
#include "../gc_input/controller.h"


//void ShutdownWii();

extern char shutdown;

extern "C" {
//void SysReset();
extern int stop;
}

static void ExitWii(void)
{
	// this exits to loader or sys menu if no stub
	// but only works if the menu gui is being drawn
	shutdown = 2;
	
	// so now force call the menu here
	stop = 1;
	
	// this hard resets to system menu
	// might fail on real wii, and might not save data
	//SysReset();
};

//extern char menuActive;
extern GXRModeObj *vmode;
u8 dimSwitch = 1;

static void reset_cb(void)
{
	//if(menuActive == 0)
	//	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	
	//if(menuActive && dimSwitch < 2) {
	if(dimSwitch < 2) {
		u8 night = dimSwitch ? 0 : 22;
		dimSwitch ^= 1;

		u8 sharp[7] = {0, 0, 21, night, 21, 0, 0};
		u8* vfilter = sharp;

		GX_SetCopyFilter(vmode->aa,vmode->sample_pattern,GX_TRUE,vfilter);
		GX_Flush();

		VIDEO_Configure(vmode);
		VIDEO_Flush();
	} //else {
		// reset game
	//}
}

namespace menu {
	
Input::Input()
{
	PAD_Init();
#ifdef HW_RVL
	CONF_Init();
	WPAD_Init();
	WPAD_SetIdleTimeout(120);
	WPAD_SetVRes(WPAD_CHAN_ALL, 640, 480);
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR); 
	//WPAD_SetPowerButtonCallback((WPADShutdownCallback)ShutdownWii);
	SYS_SetPowerCallback(ExitWii);
	SYS_SetResetCallback(reset_cb); // dim, only enable for testing
#endif
//	VIDEO_SetPostRetraceCallback (PAD_ScanPads);
}

Input::~Input()
{
}

void Input::refreshInput()
{
	if(padNeedScan){ gc_connected = PAD_ScanPads(); padNeedScan = 0; }
	PAD_Read(gcPad);
	PAD_Clamp(gcPad);
#ifdef HW_RVL
	if(wpadNeedScan){ WPAD_ScanPads(); wpadNeedScan = 0; }
//	WPAD_ScanPads();
	wiiPad = WPAD_Data(0);
#endif
}

#ifdef HW_RVL
WPADData* Input::getWpad()
{
	return wiiPad;
}
#endif

PADStatus* Input::getPad()
{
	return &gcPad[0];
}

void Input::clearInputData()
{
	Focus::getInstance().clearInputData();
	Cursor::getInstance().clearInputData();
}

} //namespace menu 
