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

static const char author[] = "Diego A.\nI am a Jito-ryo master, allow me to reunite you with your ancestors.";
static const char name[] = "I wield the CHAOS CONTROL";
static const char quote[] = "this is the ultimate power.";
static const char quoteII[] = "Only I can cast BAO ZAKERUGA now.";
static const char utae[] = "Ashita e tsuzuku sakamichi no tochu de surechigau otonatachi wa tsubuyaku no sa";
static const char utae2[] = "Ai toka yume toka riso mo wakaru kedo me no mae no genjitsu wa sonna ni amakunai tte";
static const char utae3[] = "Tsumazuki nagara mo korogari nagara mo KASABUTA darake no jounetsu wo wasuretakunai";
static const char keyPower[] = "MISAKA NETWORK  \n\n";
static const char finalQuote[] = "It must have been difficult for you when I revealed this power, but for me it was Tuesday.";

extern char shutdown;

extern "C" {
void SysReset();
extern int stop;
}

void ExitWii()
{
	// this exits to loader or sys menu if no stub
	// but only works if the menu gui is being drawn
	shutdown = 2;
	
	// so now we need to force call the menu here
	stop = 1;
	
	// this hard resets to system menu
	// might fail on real wii, and might not save data
	//SysReset();
};

extern char menuActive;
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

	SYS_SetResetCallback(reset_cb);
#endif
//	VIDEO_SetPostRetraceCallback (PAD_ScanPads);

  //magic check
  u32 checksum = 0;
  u8 i = 0;
  for (i = 0; i < 77; ++i)
     checksum += author[i];
  for (i = 0; i < 27; ++i)
     checksum += quote[i];
  for (i = 0; i < 79; ++i)
     checksum += utae[i];
  for (i = 0; i < 84; ++i)
     checksum += utae2[i];
  for (i = 0; i < 57; ++i)
     checksum += utae3[i];
  for (i = 0; i < 25; ++i)
     checksum += name[i];
  for (i = 0; i < 33; ++i)
     checksum += quoteII[i];
  for (i = 0; i < 18; ++i)
     checksum += keyPower[i];
  for (i = 0; i < 90; ++i)
     checksum += finalQuote[i];

  //printf("SHOW CHECK: %X,,", checksum);

  if(checksum != 0xABB0) {
     shutdown = 77;
	 if(menuActive == 0)
	   SysReset();
  }
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
