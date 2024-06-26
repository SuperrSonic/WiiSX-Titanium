/**
 * WiiSX - MainFrame.cpp
 * Copyright (C) 2009 sepp256
 *
 * WiiSX homepage: http://www.emulatemii.com
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

#include "MenuContext.h"
#include "MainFrame.h"
#include "SettingsFrame.h"
#include "../libgui/Button.h"
#include "../libgui/Gui.h"
#include "../libgui/InputStatusBar.h"
#include "../libgui/resources.h"
//#include "../libgui/InputManager.h"
#include "../libgui/FocusManager.h"
#include "../libgui/CursorManager.h"
#include "../libgui/MessageBox.h"
//#include "../main/wii64config.h"
#ifdef DEBUGON
# include <debug.h>
#endif
extern "C" {
#ifdef WII
#include <di/di.h>
#endif 
/*#include "../gc_memory/memory.h"
#include "../gc_memory/Saves.h"
#include "../main/plugin.h"
#include "../main/savestates.h"*/
#include "../fileBrowser/fileBrowser.h"
#include "../fileBrowser/fileBrowser-libfat.h"
#include "../fileBrowser/fileBrowser-CARD.h"
#include "../fileBrowser/fileBrowser-SMB.h"
//#include "../main/gc_dvd.h"
}
#include <ogc/dvd.h>

void Func_LoadROM();
void Func_CurrentROM();
void Func_Settings();
void Func_Credits();
void Func_ExitToLoader();
void Func_PlayGame();

bool autoboot_warn = false;
bool need_reset = false;
extern GXRModeObj *vmode;

#define NUM_MAIN_BUTTONS 5
#define FRAME_BUTTONS mainFrameButtons
#define FRAME_STRINGS mainFrameStrings

static const char FRAME_STRINGS[7][20] =
	{ "Load Game",
	  "Game Data",
	  "Settings",
	  "Exit",
	  "Play Game",
	  "Resume Game",
	  "Reset"};


struct ButtonInfo
{
	menu::Button	*button;
	int				buttonStyle;
	const char*		buttonString;
	float			x;
	float			y;
	float			width;
	float			height;
	int				focusUp;
	int				focusDown;
	int				focusLeft;
	int				focusRight;
	ButtonFunc		clickedFunc;
	ButtonFunc		returnFunc;
} FRAME_BUTTONS[NUM_MAIN_BUTTONS] =
{ //	button	buttonStyle	buttonString		x		y		width	height	Up	Dwn	Lft	Rt	clickFunc				returnFunc
	{	NULL,	BTN_A_NRM,	FRAME_STRINGS[0],	315.0,	autoboot_warn ? 500.0 : 60.0,	200.0,	56.0,	 5,	 1,	-1,	-1,	Func_LoadROM,			NULL }, // Load ROM
	{	NULL,	BTN_A_NRM,	FRAME_STRINGS[1],	315.0,	autoboot_warn ? 60.0 : 120.0,	200.0,	56.0,	 0,	 2,	-1,	-1,	Func_CurrentROM,		NULL }, // Current ROM
	{	NULL,	BTN_A_NRM,	FRAME_STRINGS[2],	315.0,	autoboot_warn ? 120.0 : 180.0,	200.0,	56.0,	 1,	 3,	-1,	-1,	Func_Settings,			NULL }, // Settings
	{	NULL,	BTN_A_NRM,	FRAME_STRINGS[3],	315.0,	autoboot_warn ? 180.0 : 240.0,	200.0,	56.0,	 2,	 4,	-1,	-1,	Func_ExitToLoader,		NULL }, // Exit to Loader
	{	NULL,	BTN_A_NRM,	FRAME_STRINGS[4],	315.0,	autoboot_warn ? 240.0 : 300.0,	200.0,	56.0,	 3,	 5,	-1,	-1,	Func_PlayGame,			NULL }, // Play/Resume Game
};

MainFrame::MainFrame()
{
	inputStatusBar = new menu::InputStatusBar(450,100);
	add(inputStatusBar);

	for (int i = 0; i < NUM_MAIN_BUTTONS; i++)
		FRAME_BUTTONS[i].button = new menu::Button(FRAME_BUTTONS[i].buttonStyle, &FRAME_BUTTONS[i].buttonString, 
										FRAME_BUTTONS[i].x, FRAME_BUTTONS[i].y, 
										FRAME_BUTTONS[i].width, FRAME_BUTTONS[i].height);

	for (int i = 0; i < NUM_MAIN_BUTTONS; i++)
	{
		/* The "get it done" way */
		if (autoboot_warn && i != 1)
		   if (FRAME_BUTTONS[i].focusUp != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_UP, FRAME_BUTTONS[FRAME_BUTTONS[i].focusUp].button);
		if (!autoboot_warn && i != 0)
		   if (FRAME_BUTTONS[i].focusUp != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_UP, FRAME_BUTTONS[FRAME_BUTTONS[i].focusUp].button);
		if (i != 4)
		   if (FRAME_BUTTONS[i].focusDown != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_DOWN, FRAME_BUTTONS[FRAME_BUTTONS[i].focusDown].button);
		if (FRAME_BUTTONS[i].focusLeft != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_LEFT, FRAME_BUTTONS[FRAME_BUTTONS[i].focusLeft].button);
		if (FRAME_BUTTONS[i].focusRight != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_RIGHT, FRAME_BUTTONS[FRAME_BUTTONS[i].focusRight].button);
		FRAME_BUTTONS[i].button->setActive(true);
		if (FRAME_BUTTONS[i].clickedFunc) FRAME_BUTTONS[i].button->setClicked(FRAME_BUTTONS[i].clickedFunc);
		if (FRAME_BUTTONS[i].returnFunc) FRAME_BUTTONS[i].button->setReturn(FRAME_BUTTONS[i].returnFunc);
		add(FRAME_BUTTONS[i].button);
		menu::Cursor::getInstance().addComponent(this, FRAME_BUTTONS[i].button, FRAME_BUTTONS[i].x, 
												FRAME_BUTTONS[i].x+FRAME_BUTTONS[i].width, FRAME_BUTTONS[i].y, 
												FRAME_BUTTONS[i].y+FRAME_BUTTONS[i].height);
	}
	setDefaultFocus(FRAME_BUTTONS[autoboot_warn ? 1 : 0].button);
	setEnabled(true);

}

MainFrame::~MainFrame()
{
	for (int i = 0; i < NUM_MAIN_BUTTONS; i++)
	{
		menu::Cursor::getInstance().removeComponent(this, FRAME_BUTTONS[i].button);
		delete FRAME_BUTTONS[i].button;
	}
	delete inputStatusBar;
}

extern MenuContext *pMenuContext;

extern "C" {
void SysReset();
void SysInit();
void SysClose();
void CheckCdrom();
void LoadCdrom();
}

void Func_LoadROM()
{
	if(autoboot_warn)
	{
		// Shortcut for Reset
		if(menu::MessageBox::getInstance().askMessage("Are you sure?")) {
			SysClose();
			SysInit ();
			CheckCdrom();
  			SysReset();
			LoadCdrom();
			menu::MessageBox::getInstance().setMessage("Game has been reset.");
		}
		return;
	}
	pMenuContext->setActiveFrame(MenuContext::FRAME_LOADROM,FileBrowserFrame::FILEBROWSER_LOADISO);
}

extern BOOL hasLoadedISO;

void Func_CurrentROM()
{
	if(!hasLoadedISO)
	{
		menu::MessageBox::getInstance().setMessage("Please load an ISO first");
		return;
	}

	pMenuContext->setActiveFrame(MenuContext::FRAME_CURRENTROM);
}

void Func_Settings()
{
	menu::Gui::getInstance().menuLogo->setLocation(580.0, 410.0, -50.0);
	pMenuContext->setActiveFrame(MenuContext::FRAME_SETTINGS,SettingsFrame::SUBMENU_GENERAL);
}

void Func_Credits()
{
#if 0
	char CreditsInfo[512] = "";
#ifdef HW_RVL
	int iosversion = IOS_GetVersion();
	sprintf(CreditsInfo,"WiiSX Beta 4.1 - IOS %i\n", iosversion);
#else
	sprintf(CreditsInfo,"CubeSX Beta 4.1\n");
#endif
	strcat(CreditsInfo,"\n");
	strcat(CreditsInfo,"Wii64 Team:\n");
	strcat(CreditsInfo,"emu_kidid / sepp256 / tehpola\n");
	strcat(CreditsInfo,"\n");
	strcat(CreditsInfo,"Extra thanks to:\n");
	strcat(CreditsInfo,"    drmr - for menu graphics\n");
	strcat(CreditsInfo,"PCSX/-df/-ReARMed teams\n");
	strcat(CreditsInfo,"pcercuei - for lightrec/motivation\n");
	strcat(CreditsInfo,"WinterMute/shagkur - devkitPro/libOGC\n");
#ifdef HW_RVL
	strcat(CreditsInfo,"Team Twiizers - for Wii homebrew\n");
#endif

	menu::MessageBox::getInstance().setMessage(CreditsInfo);
#endif
}

extern char shutdown;

void Func_ExitToLoader()
{
	if(menu::MessageBox::getInstance().askMessage("Are you sure you want to exit?"))
		shutdown = 2;
}

extern "C" {
void cpu_init();
void cpu_deinit();
}

extern "C" {
extern int SaveMcd(int mcd, fileBrowser_file *savepath);
void pauseAudio(void);  void pauseInput(void);
void resumeAudio(void); void resumeInput(void);
void go(void); 
}

#include "../libgui/IPLFont.h"

extern char menuActive;
extern "C" unsigned int usleep(unsigned int us);

void Func_PlayGame()
{
	if(!hasLoadedISO)
	{
		menu::MessageBox::getInstance().setMessage("Load a game first.");
		return;
	}
	
	//Wait until 'A' button released before play/resume game
	menu::Cursor::getInstance().setFreezeAction(true);
	menu::Focus::getInstance().setFreezeAction(true);
	int buttonHeld = 1;
	while(buttonHeld)
	{
		buttonHeld = 0;
		menu::Gui::getInstance().draw();
		for (int i=0; i<4; i++)
		{
			if(PAD_ButtonsHeld(i) & PAD_BUTTON_A) buttonHeld++;
#ifdef HW_RVL
			WPADData* wiiPad = WPAD_Data(i);
			if(wiiPad->err == WPAD_ERR_NONE && wiiPad->btns_h & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)) buttonHeld++;
#endif
		}
	}
	menu::Cursor::getInstance().setFreezeAction(false);
	menu::Focus::getInstance().setFreezeAction(false);

	menu::Gui::getInstance().gfx->clearEFB((GXColor){0, 0, 0, 0xFF}, 0x000000);
#ifdef HW_RVL
	pause_netinit_thread();
#endif
	resumeAudio();
	resumeInput();
	menuActive = 0;
#ifdef DEBUGON
	_break();
#endif
	menu::Gui::getInstance().gfx->setInGameVMode();
	go();
#ifdef DEBUGON
	_break();
#endif
	menuActive = 1;
	pauseInput();
	pauseAudio();

//area scaler
#if 1
  //set texels back
  for (int i = GX_TEXCOORD0; i < GX_MAXCOORD; i++) {
        GX_SetTexCoordScaleManually(i, GX_FALSE, 1, 1);
  }
  //Fixes IPL font
  Mtx m;
  guMtxTrans(m, 0., 0., 0.);
  GX_LoadTexMtxImm(m, GX_DTTIDENTITY, GX_MTX3x4);
#endif
  // TEST, works
  //if(videoFb) {
  //if(screenMode < SCREENMODE_16x9_PILLARBOX) {

  //menu always 480i/480p
  //if(videoMode == VIDEOMODE_DS) {
    //vmode = VIDEO_GetPreferredMode(0);
  //}

  vmode->fbWidth = 640;

    switch (videoWidth) {
	case VIDEOWIDTH_640:
		vmode->viWidth   = 640;
		vmode->viXOrigin = 40; //default
		break;
	case VIDEOWIDTH_644:
		vmode->viWidth   = 644;
		vmode->viXOrigin = 38;
		break;
	case VIDEOWIDTH_704:
		vmode->viWidth   = 704;
		vmode->viXOrigin = 8;
		break;
	case VIDEOWIDTH_720:
		vmode->viWidth   = 720;
		vmode->viXOrigin = 0;
		break;
	}
  VIDEO_Configure(vmode);
  VIDEO_Flush();

  //GX_SetViewport(0,0,vmode->fbWidth,vmode->efbHeight,0,1);
  GX_SetViewport(1.0f/24.0f,1.0f/24.0f,vmode->fbWidth,vmode->efbHeight,0,1);
  //GX_SetScissor(0,0,vmode->fbWidth,vmode->efbHeight,0,1);
  GX_SetDispCopySrc(0,0,vmode->fbWidth,vmode->efbHeight);
  GX_SetDispCopyDst(vmode->fbWidth,vmode->xfbHeight);
  GX_InvVtxCache();

  need_reset = true;
  VIDEO_WaitVSync();

#ifdef HW_RVL
  resume_netinit_thread();
#endif
	FRAME_BUTTONS[5].buttonString = FRAME_STRINGS[6];
	menu::Cursor::getInstance().clearCursorFocus();
}

void Func_SetPlayGame()
{
	FRAME_BUTTONS[5].buttonString = FRAME_STRINGS[5];
}

void Func_SetResumeGame()
{
	FRAME_BUTTONS[5].buttonString = FRAME_STRINGS[6];
}

void MainFrame::Autoboot()
{
	Func_SetPlayGame();
	setDefaultFocus(FRAME_BUTTONS[4].button);
	FRAME_BUTTONS[0].buttonString = FRAME_STRINGS[6]; // Change text
	Func_SetResumeGame(); // Switch to Resume Game
	autoboot_warn = true;
}
