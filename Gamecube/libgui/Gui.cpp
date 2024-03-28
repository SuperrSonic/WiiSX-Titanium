/**
 * WiiSX - Gui.cpp
 * Copyright (C) 2009, 2010 sepp256
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

#include "Gui.h"
#include "IPLFont.h"
#include "InputManager.h"
#include "CursorManager.h"
#include "FocusManager.h"
#include "MessageBox.h"
#include "LoadingBar.h"
#include "GuiResources.h"

extern "C" {
#include "../gc_input/controller.h"
#ifdef WII
#include <di/di.h>
#include <ogc/machine/processor.h>

#include <stdio.h>
#include <string.h>
#include <ogcsys.h>
#include <malloc.h>
#include <gctypes.h>

static int Playlog_Exit(void);

#define ALIGN32(x) (((x) + 31) & ~31)
#define SECONDS_TO_2000 946684800LL
#define TICKS_PER_SECOND 60750000LL

//! Should be 32 byte aligned
static const char PLAYRECPATH[] ATTRIBUTE_ALIGN(32) = "/title/00000001/00000002/data/play_rec.dat";

typedef struct _PlayRec
{
	u32 checksum;
	union
	{
		u32 data[31];
		struct
		{
			u16 name[42];
			u64 ticks_boot;
			u64 ticks_last;
			char title_id[6];
			char unknown[18];
		} ATTRIBUTE_PACKED;
	};
} PlayRec;

static u64 getWiiTime(void)
{
	time_t uTime = time(NULL);
	return TICKS_PER_SECOND * (uTime - SECONDS_TO_2000);
}

int Playlog_Exit(void)
{
	u64 stime = 0;
	s32 res = -1;
	u32 sum = 0;
	u8 i;

	//Open play_rec.dat
	s32 fd = IOS_Open(PLAYRECPATH, IPC_OPEN_RW);
	if(fd < 0)
		return fd;

	PlayRec * playrec_buf = (PlayRec*) memalign(32, ALIGN32(sizeof(PlayRec)));
	if(!playrec_buf)
		goto cleanup;

	//Read play_rec.dat
	if(IOS_Read(fd, playrec_buf, sizeof(PlayRec)) != sizeof(PlayRec))
		goto cleanup;

	if(IOS_Seek(fd, 0, 0) < 0)
		goto cleanup;

	// update exit time
	stime = getWiiTime();
	playrec_buf->ticks_last = stime;
	
	//Calculate and update checksum
	for(i = 0; i < 31; i++)
		sum += playrec_buf->data[i];

	playrec_buf->checksum = sum;

	if(IOS_Write(fd, playrec_buf, sizeof(PlayRec)) != sizeof(PlayRec))
		goto cleanup;

	res = 0;

cleanup:
	free(playrec_buf);
	IOS_Close(fd);
	return res;
}
#endif
}



extern bool Autoboot;

extern char shutdown;
#ifdef WII
bool isWiiU = (*(vu16*)0xCD8005A0 == 0xCAFE) && CONF_GetAspectRatio();
#endif

namespace menu {

Gui::Gui()
	: fade(9)
{
	menuLogo = new Logo();
	menuLogo->setLocation(570.0, 70.0, -150.0);
	menuLogo->setVisible(true);
}

Gui::~Gui()
{
	delete menuLogo;
	delete gfx;
}

void Gui::setVmode(GXRModeObj *vmode)
{
	gfx = new Graphics(vmode);
	IplFont::getInstance().setVmode(vmode);
}

void Gui::addFrame(Frame *frame)
{
	frameList.push_back(frame);
}

void Gui::removeFrame(Frame *frame)
{
	frameList.erase(std::remove(frameList.begin(),frameList.end(),frame),frameList.end());
}

void Gui::draw()
{
//	printf("Gui draw\n");
	Input::getInstance().refreshInput();
	Cursor::getInstance().updateCursor();
	Focus::getInstance().updateFocus();
	if(padAutoAssign) auto_assign_controllers();
	//Update time??

#ifdef WII
	// Adjust Wii U aspect ratio via DMCU
	static char oldScreenMode = -1;
	if (isWiiU && (oldScreenMode != screenMode)) {
		write32(0xd8006a0, screenMode ? 0x30000004 : 0x30000002);
		mask32(0xd8006a8, 0, 2);
		oldScreenMode = screenMode;
	}
#endif

	//Get graphics framework and pass to Frame draw fns?
	gfx->drawInit();
	drawBackground();
	FrameList::const_iterator iteration;
	for (iteration = frameList.begin(); iteration != frameList.end(); iteration++)
	{
		(*iteration)->drawChildren(*gfx);
	}
//	menuLogo->drawComponent(*gfx);
	menuLogo->draw(*gfx);
	if (MessageBox::getInstance().getActive()) MessageBox::getInstance().drawMessageBox(*gfx);
	if (LoadingBar::getInstance().getActive()) LoadingBar::getInstance().drawLoadingBar(*gfx);
	Cursor::getInstance().drawCursor(*gfx);

	if(shutdown)
	{
		Cursor::getInstance().setFreezeAction(true);
		Focus::getInstance().setFreezeAction(true);
		gfx->enableBlending(true);
		gfx->setTEV(GX_PASSCLR);
		gfx->setDepth(-10.0f);
		gfx->newModelView();
		gfx->loadModelView();
		gfx->loadOrthographic();

		gfx->setColor((GXColor){0, 0, 0, fade});
		if(screenMode)	gfx->fillRect(-104, 0, 848, 480);
		else			gfx->fillRect(0, 0, 640, 480);
		
		if(fade == 255)
		{
			VIDEO_SetBlack(true);
			VIDEO_Flush();
#ifdef WII
			//Update message board time
			//if(Autoboot) // always false at this point
			// TODO: fix this - if(writePlaylog)
				Playlog_Exit();
#endif
		 	VIDEO_WaitVSync();
#ifdef WII
			// If this is a Wii U, restore the original aspect ratio
			if(isWiiU) {
				write32(0xd8006a0, 0x30000004);
				mask32(0xd8006a8, 0, 2);
			}
#endif
			if(shutdown==1)	//Power off System
				SYS_ResetSystem(SYS_POWEROFF, 0, 0);
			else			//Return to Loader
			{
#ifdef WII
				DI_Close();
#endif
				void (*rld)() = (void (*)()) 0x80001800;
#ifdef HW_DOL
				#define PSOSDLOADID 0x7c6000a6
				// try to reload
				if(*(volatile unsigned int*)0x80001800 == PSOSDLOADID) {
					rld();
				}
				else {
					*(volatile unsigned int*)0xCC003024 = 0;  //reboot
			  }
#else
				if(*(volatile unsigned int*)0x80001804 == 0x53545542 &&
					*(volatile unsigned int*)0x80001808 == 0x48415858)
					rld();
				else
					SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
#endif
			}
		}

		char increment = 8;
		fade = fade +increment > 255 ? 255 : fade + increment;
	}

	gfx->swapBuffers();
}

void Gui::drawBackground()
{
	//Draw Menu Backdrop
	Resources::getInstance().getImage(Resources::IMAGE_MENU_BACKGROUND)->activateImage(GX_TEXMAP0);
//	gfx->setTEV(GX_REPLACE);
	GXColor muxCol = (GXColor){0,35,70,255};
	GX_SetTevColor(GX_TEVREG0,muxCol);
	GX_SetTevColorIn(GX_TEVSTAGE0,GX_CC_C0,GX_CC_ZERO,GX_CC_TEXC,GX_CC_TEXC);
	GX_SetTevColorOp(GX_TEVSTAGE0,GX_TEV_ADD,GX_TB_ZERO,GX_CS_SCALE_1,GX_TRUE,GX_TEVPREV);
	gfx->enableBlending(false);
	if(screenMode)	gfx->drawImage(0, -104, 0, 848, 480, 0, 1, 0, 1);
	else			gfx->drawImage(0, 0, 0, 640, 480, (848.0-640.0)/2/848.0, 1.0 - (848.0-640.0)/2/848.0, 0, 1);
	gfx->setTEV(GX_PASSCLR);
}

} //namespace menu 
