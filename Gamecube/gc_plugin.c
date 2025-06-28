/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2002  Pcsx Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <zlib.h>
#include <libpcsxcore/misc.h>
#include <libpcsxcore/psxcommon.h>
#include <libpcsxcore/psxcounters.h>
#include "fileBrowser/fileBrowser.h"
#include "DEBUG.h"
#include "libgui/IPLFontC.h"
#include "wiiSXconfig.h"

#include <pcsx_rearmed/frontend/plugin_lib.h>

#include <libpcsxcore/spu.h>
#include <libpcsxcore/gpu.h>
#ifndef __GX__
#include "NoPic.h"
#endif //!__GX__

#include <ogc/machine/processor.h>

void OnFile_Exit(){}

unsigned long gpuDisp;

int StatesC = 0;
extern int UseGui;
int cdOpenCase = 0;
int ShowPic=0;

void gpuShowPic() {
	/*char Text[255];
	gzFile f;

	if (!ShowPic) {
		unsigned char *pMem;

		pMem = (unsigned char *) malloc(128*96*3);
		if (pMem == NULL) return;
		sprintf(Text, "sstates/%10.10s.%3.3d", CdromLabel, StatesC);

		GPU_freeze(2, (GPUFreeze_t *)&StatesC);

		f = gzopen(Text, "rb");
		if (f != NULL) {
			gzseek(f, 32, SEEK_SET); // skip header
			gzread(f, pMem, 128*96*3);
			gzclose(f);
		} else {
			memcpy(pMem, NoPic_Image.pixel_data, 128*96*3);
			DrawNumBorPic(pMem, StatesC+1);
		}
		GPU_showScreenPic(pMem);

		free(pMem);
		ShowPic = 1;
	} else { GPU_showScreenPic(NULL); ShowPic = 0; }*/
}

void PADhandleKey(int key) {
/*	char Text[255];
	int ret;

	switch (key) {
		case 0: break;
		case XK_F1:
			sprintf(Text, "sstates/%10.10s.%3.3d", CdromLabel, StatesC);
			GPU_freeze(2, (GPUFreeze_t *)&StatesC);
			ret = SaveState(Text);
			if (ret == 0)
				 sprintf(Text, _("*PCSX*: Saved State %d"), StatesC+1);
			else sprintf(Text, _("*PCSX*: Error Saving State %d"), StatesC+1);
			GPU_displayText(Text);
			if (ShowPic) { ShowPic = 0; gpuShowPic(); }
			break;
		case XK_F2:
			if (StatesC < 4) StatesC++;
			else StatesC = 0;
			GPU_freeze(2, (GPUFreeze_t *)&StatesC);
			if (ShowPic) { ShowPic = 0; gpuShowPic(); }
			break;
		case XK_F3:			
			sprintf (Text, "sstates/%10.10s.%3.3d", CdromLabel, StatesC);
			ret = LoadState(Text);
			if (ret == 0)
				 sprintf(Text, _("*PCSX*: Loaded State %d"), StatesC+1);
			else sprintf(Text, _("*PCSX*: Error Loading State %d"), StatesC+1);
			GPU_displayText(Text);
			break;
		case XK_F4:
			gpuShowPic();
			break;
		case XK_F5:
			Config.Sio ^= 0x1;
			if (Config.Sio)
				 sprintf(Text, _("*PCSX*: Sio Irq Always Enabled"));
			else sprintf(Text, _("*PCSX*: Sio Irq Not Always Enabled"));
			GPU_displayText(Text);
			break;
		case XK_F6:
			Config.Mdec ^= 0x1;
			if (Config.Mdec)
				 sprintf(Text, _("*PCSX*: Black&White Mdecs Only Enabled"));
			else sprintf(Text, _("*PCSX*: Black&White Mdecs Only Disabled"));
			GPU_displayText(Text);
			break;
		case XK_F7:
			Config.Xa ^= 0x1;
			if (Config.Xa == 0)
				 sprintf (Text, _("*PCSX*: Xa Enabled"));
			else sprintf (Text, _("*PCSX*: Xa Disabled"));
			GPU_displayText(Text);
			break;
		case XK_F8:
			GPU_makeSnapshot();
			break;
		case XK_F9:
			cdOpenCase = 1;
			break;
		case XK_F10:
			cdOpenCase = 0;
			break;
		case XK_Escape:
			ClosePlugins();
			UpdateMenuSlots();
			if (!UseGui) OnFile_Exit();
			RunGui();
			break;
		default:
			GPU_keypressed(key);
			if (Config.UseNet) NET_keypressed(key);
	}*/
}

long PAD1__open(void) {
	return PAD1_open(&gpuDisp);
}

long PAD2__open(void) {
	return PAD2_open(&gpuDisp);
}

#include "gc_input/controller.h"
#include "wiiSXconfig.h"
extern int stop;
//extern void GPUcursor(int iPlayer,int x,int y);
extern virtualControllers_t virtualControllers[4];
// Use to invoke func on the mapped controller with args
#define DO_CONTROL(Control,func,args...) \
	virtualControllers[Control].control->func( \
		virtualControllers[Control].number, ## args)

void plat_trigger_vibrate(int pad, int low, int high) {
	if(virtualControllers[pad].inUse)
		DO_CONTROL(pad, rumble, low);	// TODO vary this based on small/large motor.
}

// Lightgun vars
typedef struct XY_POS
{
 long x;
 long y;
} XyPos_t;
XyPos_t ptCursorPoint[8];
unsigned short usCursorActive=0;


void setCursor(int iPlayer,int x,int y)
{
 if(iPlayer<0) return;
 if(iPlayer>7) return;

 usCursorActive|=(1<<iPlayer);
 ptCursorPoint[iPlayer].x=x;
 ptCursorPoint[iPlayer].y=y;
}

long PAD1__readPort1(PadDataS *pad) {
	int pad_index = pad->requestPadIndex;
	//SysPrintf("Read port 1 called with %i pad index, in use %s\r\n", pad_index, virtualControllers[pad_index].inUse ? "YES":"NO");
	if(!virtualControllers[pad_index].inUse) return 0;

	static BUTTONS PAD_Data;
	if(virtualControllers[pad_index].inUse && DO_CONTROL(pad_index, GetKeys, (BUTTONS*)&PAD_Data, virtualControllers[pad_index].config, in_type[pad_index]))
			stop = 1;
	pad->controllerType = in_type[pad_index];
	pad->buttonStatus = PAD_Data.btns.All;

	pad->portMultitap = multitap1;

	usCursorActive=0;
	if (in_type[pad_index] == PSE_PAD_TYPE_ANALOGJOY || in_type[pad_index] == PSE_PAD_TYPE_ANALOGPAD || in_type[pad_index] == PSE_PAD_TYPE_NEGCON || in_type[pad_index] == PSE_PAD_TYPE_GUNCON || in_type[pad_index] == PSE_PAD_TYPE_GUN)
	{
		pad->leftJoyX = PAD_Data.leftStickX;
		pad->leftJoyY = PAD_Data.leftStickY;
		pad->rightJoyX = PAD_Data.rightStickX;
		pad->rightJoyY = PAD_Data.rightStickY;

		pad->absoluteX = PAD_Data.gunX;
		pad->absoluteY = PAD_Data.gunY;
		if(in_type[pad_index] == PSE_PAD_TYPE_GUN) {
			usCursorActive=1;
			setCursor(pad_index, pad->absoluteX, pad->absoluteY);
		}
	}
	
	if (in_type[pad_index] == PSE_PAD_TYPE_MOUSE)
	{
		pad->moveX = PAD_Data.mouseX;
		pad->moveY = PAD_Data.mouseY;
	}

	return 0;
}

long PAD2__readPort2(PadDataS *pad) {
	int pad_index = pad->requestPadIndex;
	//SysPrintf("Read port 2 called with %i pad index, in use %s\r\n", pad_index, virtualControllers[pad_index].inUse ? "YES":"NO");
	
	// If multi-tap is enabled, this comes up as 4 (port 1 will do 0,1,2,3).
	if(pad_index >=4) return 0;
	
	if(!virtualControllers[pad_index].inUse) return 0;
	
	static BUTTONS PAD_Data2;
	if(virtualControllers[pad_index].inUse && DO_CONTROL(pad_index, GetKeys, (BUTTONS*)&PAD_Data2, virtualControllers[pad_index].config, in_type[pad_index]))
		stop = 1;

	pad->controllerType = in_type[pad_index];
	pad->buttonStatus = PAD_Data2.btns.All;

	//if (multitap2 == 1)
	//	pad->portMultitap = 2;
	//else
		pad->portMultitap = 0;
	
	usCursorActive=0;
	if (in_type[pad_index] == PSE_PAD_TYPE_ANALOGJOY || in_type[pad_index] == PSE_PAD_TYPE_ANALOGPAD || in_type[pad_index] == PSE_PAD_TYPE_NEGCON || in_type[pad_index] == PSE_PAD_TYPE_GUNCON || in_type[pad_index] == PSE_PAD_TYPE_GUN)
	{
		pad->leftJoyX = PAD_Data2.leftStickX;
		pad->leftJoyY = PAD_Data2.leftStickY;
		pad->rightJoyX = PAD_Data2.rightStickX;
		pad->rightJoyY = PAD_Data2.rightStickY;

		pad->absoluteX = PAD_Data2.gunX;
		pad->absoluteY = PAD_Data2.gunY;
		if(in_type[pad_index] == PSE_PAD_TYPE_GUN) {
			usCursorActive=1;
			setCursor(pad_index, pad->absoluteX, pad->absoluteY);
		}
	}

	if (in_type[pad_index] == PSE_PAD_TYPE_MOUSE)
	{
		pad->moveX = PAD_Data2.mouseX;
		pad->moveY = PAD_Data2.mouseY;
	}

	return 0;
}

void OnFile_Exit();

void SignalExit(int sig) {
	ClosePlugins();
	OnFile_Exit();
}

void SPUirq(int);

#define PARSEPATH(dst, src) \
	ptr = src + strlen(src); \
	while (*ptr != '\\' && ptr != src) ptr--; \
	if (ptr != src) { \
		strcpy(dst, ptr+1); \
	}

int _OpenPlugins() {
	int ret;

/*	signal(SIGINT, SignalExit);
	signal(SIGPIPE, SignalExit);*/
	ret = CDR_open();
	if (ret < 0) { SysPrintf("Error Opening CDR Plugin\n"); return -1; }
	ret = SPU_open();
	if (ret < 0) { SysPrintf("Error Opening SPU Plugin\n"); return -1; }
	SPU_registerCallback(SPUirq);
	SPU_registerScheduleCb(SPUschedule);
	ret = GPU_open(&gpuDisp, "PCSX", NULL);
	if (ret < 0) { SysPrintf("Error Opening GPU Plugin\n"); return -1; }
	ret = PAD1_open(&gpuDisp);
	if (ret < 0) { SysPrintf("Error Opening PAD1 Plugin\n"); return -1; }
	ret = PAD2_open(&gpuDisp);
	if (ret < 0) { SysPrintf("Error Opening PAD2 Plugin\n"); return -1; }

	if (Config.UseNet && NetOpened == 0) {
		netInfo info;
		char path[256];

		strcpy(info.EmuName, "PCSX v1.5b3");
		strncpy(info.CdromID, CdromId, 9);
		strncpy(info.CdromLabel, CdromLabel, 9);
		info.psxMem = psxM;
		info.GPU_showScreenPic = GPU_showScreenPic;
		info.GPU_displayText = GPU_displayText;
		info.GPU_showScreenPic = GPU_showScreenPic;
		info.PAD_setSensitive = PAD1_setSensitive;
		sprintf(path, "%s%s", Config.BiosDir, Config.Bios);
		strcpy(info.BIOSpath, path);
		strcpy(info.MCD1path, Config.Mcd1);
		strcpy(info.MCD2path, Config.Mcd2);
		sprintf(path, "%s%s", Config.PluginsDir, Config.Gpu);
		strcpy(info.GPUpath, path);
		sprintf(path, "%s%s", Config.PluginsDir, Config.Spu);
		strcpy(info.SPUpath, path);
		sprintf(path, "%s%s", Config.PluginsDir, Config.Cdr);
		strcpy(info.CDRpath, path);
		NET_setInfo(&info);

		ret = NET_open(&gpuDisp);
		if (ret < 0) {
			if (ret == -2) {
				// -2 is returned when something in the info
				// changed and needs to be synced
				char *ptr;

				PARSEPATH(Config.Bios, info.BIOSpath);
				PARSEPATH(Config.Gpu,  info.GPUpath);
				PARSEPATH(Config.Spu,  info.SPUpath);
				PARSEPATH(Config.Cdr,  info.CDRpath);

				strcpy(Config.Mcd1, info.MCD1path);
				strcpy(Config.Mcd2, info.MCD2path);
				return -2;
			} else {
				Config.UseNet = 0;
			}
		} else {
			if (NET_queryPlayer() == 1) {
				if (SendPcsxInfo() == -1) Config.UseNet = 0;
			} else {
				if (RecvPcsxInfo() == -1) Config.UseNet = 0;
			}
		}
		NetOpened = 1;
	} else if (Config.UseNet) {
		NET_resume();
	}

	return 0;
}

///
// GX stuff
///
#define FB_MAX_SIZE (640 * 528 * 4)
static unsigned char	GXtexture[FB_MAX_SIZE] __attribute__((aligned(32)));
extern u32* xfb[3];	/*** Framebuffers ***/
extern char text[DEBUG_TEXT_HEIGHT][DEBUG_TEXT_WIDTH]; /*** DEBUG textbuffer ***/
extern char menuActive;
extern char screenMode;
extern GXRModeObj *vmode;
extern bool need_reset;
bool expand_width = false;
static char fpsInfo[32];
// Lightgun vars
static unsigned long crCursorColor32[8][3]={{0xff,0x00,0x00},{0x00,0xff,0x00},{0x00,0x00,0xff},{0xff,0x00,0xff},{0xff,0xff,0x00},{0x00,0xff,0xff},{0xff,0xff,0xff},{0x7f,0x7f,0x7f}};

static int vsync_enable;
static int new_frame;

enum {
	FB_BACK,
	FB_NEXT,
	FB_FRONT,
};

bool firstFrame = true;

//indirect texture matrix for sharp bilinear
static float indtexmtx[2][3] = {
    { +.5, +.0, +.0 },
    { +.0, +.5, +.0 }
};

//indirect texture
static uint16_t indtexdata[][4 * 4] ATTRIBUTE_ALIGN(32) = {
    {
        0xE0E0, 0xA0E0, 0x60E0, 0x20E0,
        0xE0A0, 0xA0A0, 0x60A0, 0x20A0,
        0xE060, 0xA060, 0x6060, 0x2060,
        0xE020, 0xA020, 0x6020, 0x2020,
    }, {
        0xC0C0, 0x40C0, 0xC0C0, 0x40C0,
        0xC040, 0x4040, 0xC040, 0x4040,
        0xC0C0, 0x40C0, 0xC0C0, 0x40C0,
        0xC040, 0x4040, 0xC040, 0x4040,
    }, {
        0x8080, 0x8080, 0x8080, 0x8080,
        0x8080, 0x8080, 0x8080, 0x8080,
        0x8080, 0x8080, 0x8080, 0x8080,
        0x8080, 0x8080, 0x8080, 0x8080,
    }
};

static GXTexObj indtexobj;

void TexelFix(void)
{
    //texel center fix
    Mtx m;
    for (int i = GX_TEXCOORD0; i < GX_MAXCOORD; i++) {
        GX_SetTexCoordScaleManually(i, GX_TRUE, 1, 1);
    }

    for (int i = 0; i < 10; i++) {
        float s = 1 << (i + 1);

        guMtxScale(m, s, s, s);
        GX_LoadTexMtxImm(m, GX_TEXMTX0 + i * 3, GX_MTX3x4);
    }

    guMtxTrans(m, 1. / 64., 1. / 64., 0);
    GX_LoadTexMtxImm(m, GX_DTTIDENTITY, GX_MTX3x4);

    for (int i = 0; i < 9; i++) {
        float x = i % 3 - 1;
        float y = i / 3 - 1;

        if (i % 2 == 0) {
            x /= 2;
            y /= 2;
        }

        x += 1. / 64.;
        y += 1. / 64.;

        guMtxTrans(m, x, y, 0);
        GX_LoadTexMtxImm(m, GX_DTTMTX1 + i * 3, GX_MTX3x4);
    }
}

void update_filter(void)
{
    //Sharp Bilinear filtering for Widescreen
    //if (CONF_GetAspectRatio() == CONF_ASPECT_16_9) {
    if (videoLinear == 2) {
    //if (videoLinear > 1) {
        GX_InitTexObj(&indtexobj, indtexdata, 4, 4, GX_TF_IA8, GX_REPEAT, GX_REPEAT, GX_TRUE);
        GX_InitTexObjLOD(&indtexobj, GX_LIN_MIP_LIN, GX_LINEAR, 0., 2., -1. / 3., GX_FALSE, GX_TRUE, GX_ANISO_4);
        GX_LoadTexObj(&indtexobj, GX_TEXMAP1);

        GX_SetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_CLEAR);
        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
        GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        GX_SetZCompLoc(GX_FALSE);

        GX_SetNumChans(0);
        GX_SetNumTexGens(2);
        GX_SetNumIndStages(1);
        GX_SetNumTevStages(1);

        GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GX_SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1);

        GX_SetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
        GX_SetIndTexMatrix(GX_ITM_0, indtexmtx, -7);

        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
        GX_SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K3_R);
        GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE, GX_CC_TEXC, GX_CC_ZERO);
        GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GX_SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF, GX_ITW_OFF, GX_FALSE, GX_FALSE, GX_ITBA_OFF);
    }
    //Nearest filtering
    else {
        GX_SetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_CLEAR);
        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
        GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        GX_SetZCompLoc(GX_FALSE);

        GX_SetNumChans(0);
        GX_SetNumTexGens(1);
        GX_SetNumIndStages(0);
        GX_SetNumTevStages(1);

        GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

        GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
    }

    //GX_SetArray(GX_VA_POS, square, 3 * sizeof(s16));
	GX_Flush();
}

static void gc_vout_vsync(unsigned int)
{
	u32 *tmp;

	if (new_frame) {
		tmp = xfb[FB_FRONT];
		xfb[FB_FRONT] = xfb[FB_NEXT];
		xfb[FB_NEXT] = tmp;
		new_frame = 0;

		VIDEO_SetNextFramebuffer(xfb[FB_FRONT]);
		VIDEO_Flush();
	}
}

static void gc_vout_copydone(void)
{
	u32 *tmp;

	tmp = xfb[FB_NEXT];
	xfb[FB_NEXT] = xfb[FB_BACK];
	xfb[FB_BACK] = tmp;

	new_frame = 1;
}

static void gc_vout_drawdone(void)
{
	GX_CopyDisp(xfb[FB_BACK], GX_TRUE);
	GX_SetDrawDoneCallback(gc_vout_copydone);
	GX_SetDrawDone();
}

static void gc_vout_render(void)
{
	GX_SetDrawDoneCallback(gc_vout_drawdone);
	GX_SetDrawDone();
}

static int screen_w, screen_h;

static void GX_Flip(const void *buffer, int pitch, u8 fmt,
		    int x, int y, int width, int height)
{	
	int h, w;
	static int oldwidth=0;
	static int oldheight=0;
	static int oldformat=-1;
	static GXTexObj GXtexobj;

	if((width == 0) || (height == 0))
		return;
	
	if(firstFrame)
		TexelFix();

	if(need_reset) {
		need_reset = false;
		TexelFix();
	}

	if ((oldwidth != width) || (oldheight != height) || (oldformat != fmt))
	{ //adjust texture conversion
		oldwidth = width;
		oldheight = height;
		oldformat = fmt;
		
		u8 scaler;
		scaler = videoLinear ? GX_LINEAR : GX_NEAR; //this allows bigger than 1 to use GX_LINEAR
		
		memset(GXtexture,0,sizeof(GXtexture));
		GX_InitTexObj(&GXtexobj, GXtexture, width, height, fmt, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_InitTexObjFilterMode(&GXtexobj, scaler, scaler);
	}

	if(fmt==GX_TF_RGBA8) {
		vu8 *wgPipePtr = GX_RedirectWriteGatherPipe(GXtexture);
		u8 *image = (u8 *) buffer;
		for (h = 0; h < (height >> 2); h++)
		{
			for (w = 0; w < (width >> 2); w++)
			{
				// Convert from RGB888 to GX_TF_RGBA8 texel format
				for (int texelY = 0; texelY < 4; texelY++)
				{
					for (int texelX = 0; texelX < 4; texelX++)
					{
						// AR
						int pixelAddr = (((w*4)+(texelX))*3) + (texelY*pitch);
						*wgPipePtr = 0xFF;	// A
						*wgPipePtr = image[pixelAddr+2]; // R (which is B in little endian)
					}
				}
				
				for (int texelY = 0; texelY < 4; texelY++)
				{
					for (int texelX = 0; texelX < 4; texelX++)
					{
						// GB
						int pixelAddr = (((w*4)+(texelX))*3) + (texelY*pitch);
						*wgPipePtr = image[pixelAddr+1]; // G
						*wgPipePtr = image[pixelAddr]; // B (which is R in little endian)
					}
				}
			}
			image+=pitch*4;
		}
	}
	else {
		vu32 *wgPipePtr = GX_RedirectWriteGatherPipe(GXtexture);
		u32 *src1 = (u32 *) buffer;
		u32 *src2 = (u32 *) (buffer + pitch);
		u32 *src3 = (u32 *) (buffer + (pitch * 2));
		u32 *src4 = (u32 *) (buffer + (pitch * 3));
		int rowpitch = (pitch) - (width >> 1);
		u32 alphaMask = 0x00800080;
		for (h = 0; h < height; h += 4)
		{
			for (w = 0; w < (width >> 2); w++)
			{
				// Convert from BGR555 LE data to BGR BE, we OR the first bit with "1" to send RGB5A3 mode into RGB555 on the GP (GC/Wii)
				__asm__ (
					"lwz 3, 0(%[src1])\n"
					"lwz 4, 4(%[src1])\n"
					"lwz 5, 0(%[src2])\n"
					"lwz 6, 4(%[src2])\n"
					"lwz 7, 0(%[src3])\n"
					"lwz 8, 4(%[src3])\n"
					"lwz 9, 0(%[src4])\n"
					"lwz 10, 4(%[src4])\n"
					
					"rlwinm 3, 3, 16, 0, 31\n"
					"rlwinm 4, 4, 16, 0, 31\n"
					"rlwinm 5, 5, 16, 0, 31\n"
					"rlwinm 6, 6, 16, 0, 31\n"
					"rlwinm 7, 7, 16, 0, 31\n"
					"rlwinm 8, 8, 16, 0, 31\n"
					"rlwinm 9, 9, 16, 0, 31\n"
					"rlwinm 10, 10, 16, 0, 31\n"
					
					"or 3, 3, %[alphamask]\n"
					"or 4, 4, %[alphamask]\n"
					"or 5, 5, %[alphamask]\n"
					"or 6, 6, %[alphamask]\n"
					"or 7, 7, %[alphamask]\n"
					"or 8, 8, %[alphamask]\n"
					"or 9, 9, %[alphamask]\n"
					"or 10, 10, %[alphamask]\n"					
					
					"stwbrx 3, 0, %[wgpipe]\n" 
					"stwbrx 4, 0, %[wgpipe]\n" 
					"stwbrx 5, 0, %[wgpipe]\n" 	
					"stwbrx 6, 0, %[wgpipe]\n" 
					"stwbrx 7, 0, %[wgpipe]\n" 
					"stwbrx 8, 0, %[wgpipe]\n" 					
					"stwbrx 9, 0, %[wgpipe]\n" 
					"stwbrx 10, 0, %[wgpipe]" 					
					: 
					: [wgpipe] "r" (wgPipePtr), [src1] "r" (src1), [src2] "r" (src2), [src3] "r" (src3), [src4] "r" (src4), [alphamask] "r" (alphaMask) 
					: "memory", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10");
				src1+=2;
				src2+=2;
				src3+=2;
				src4+=2;
			}

			src1 += rowpitch;
			src2 += rowpitch;
			src3 += rowpitch;
			src4 += rowpitch;
		}
	}
	GX_RestoreWriteGatherPipe();

	GX_LoadTexObj(&GXtexobj, GX_TEXMAP0);

	Mtx44	GXprojIdent;
	Mtx		GXmodelIdent;
	guMtxIdentity(GXprojIdent);
	guMtxIdentity(GXmodelIdent);
	GXprojIdent[2][2] = 0.5;
	GXprojIdent[2][3] = -0.5;
	GX_LoadProjectionMtx(GXprojIdent, GX_ORTHOGRAPHIC);
	GX_LoadPosMtxImm(GXmodelIdent,GX_PNMTX0);

	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_ENABLE,GX_ALWAYS,GX_TRUE);

	GX_InvalidateTexAll();
	//GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	//GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
	update_filter();


	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_PTNMTXIDX, GX_PNMTX0);
	GX_SetVtxDesc(GX_VA_TEX0MTXIDX, GX_IDENTITY);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	//GX_SetNumTexGens(1);
	//GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	
	//Experiment
	//GX_DrawDone();

#if 0
	float ymin = 1.0f - (float)((y + height) * 2) / (float)screen_h;
	float ymax = 1.0f - (float)y / (float)screen_h;
	
	float xmin = (float)x / (float)screen_w - 1.0f;
	float xmax = (float)((x + width) * 2) / (float)screen_w - 1.0f;

	if(screenMode == SCREENMODE_16x9_PILLARBOX) {
		xmin *= 640.0f / 848.0f;
		xmax *= 640.0f / 848.0f;
	}
	else if(screenMode == SCREENMODE_16x9) {
		xmin *= 512.0f / 640.0f;
		xmax *= 512.0f / 640.0f;
	}

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	  GX_Position2f32(xmin, ymax);
	  GX_TexCoord2f32( 0.0, 0.0);
	  GX_Position2f32(xmax, ymax);
	  GX_TexCoord2f32( width, 0.0);
	  GX_Position2f32(xmax, ymin);
	  GX_TexCoord2f32( width, height);
	  GX_Position2f32(xmin, ymin);
	  GX_TexCoord2f32( 0.0, height);
	GX_End();
#else
	float xcoord = 1.0;
	//float ycoord = 1.0;
	float ycoord = height*2 > 484 ? height/480.0 : height*2/480.0; //1.009; //1.0084 scanline ends at 1-242
	if(screenMode == SCREENMODE_16x9_PILLARBOX) xcoord = 640.0/848.0;
	else if(screenMode == SCREENMODE_16x9) xcoord = 512/640.0;

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	  GX_Position2f32(-xcoord, ycoord);
	  GX_TexCoord2f32( 0.0, 0.0);
	  GX_Position2f32( xcoord, ycoord);
	  GX_TexCoord2f32( width, 0.0);
	  GX_Position2f32( xcoord,-ycoord);
	  GX_TexCoord2f32( width, height);
	  GX_Position2f32(-xcoord,-ycoord);
	  GX_TexCoord2f32( 0.0, height);
	GX_End();
#endif

	// Show lightgun cursors
	if(usCursorActive) {
		for(int iPlayer=0;iPlayer<8;iPlayer++)                  // -> loop all possible players
		{
			if(usCursorActive&(1<<iPlayer))                   // -> player active?
			{
				// show a small semi-transparent square for the gun target
				int gx = ptCursorPoint[iPlayer].x;
				int gy = ptCursorPoint[iPlayer].y;
				// Take 0 to 1024 and scale it between -1 to 1.
				float startX = (gx < 512 ? (-1 + ((gx-8)/511.5f)) : ((gx-520)/511.5f));
				float endX = (gx < 512 ? (-1 + ((gx+8)/511.5f)) : (((gx-504))/511.5f));
				
				float startY = (gy > 512 ? (-1 * ((gy-520)/511.5f)) : (1 - ((gy-6)/511.5f)));
				float endY = (gy > 512 ? (-1 * ((gy-506)/511.5f)) : (1 - ((gy+6)/511.5f)));
				//print_gecko("startX, endX (%.2f, %.2f) startY, endY (%.2f, %.2f)\r\n", startX, endX, startY, endY);
				GX_InvVtxCache();
				GX_InvalidateTexAll();
				GX_ClearVtxDesc();
				GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
				GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
				GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,	GX_POS_XY,	GX_F32,	0);
				GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8,	0);
				// No textures
				GX_SetNumChans(1);
				GX_SetNumTexGens(0);
				GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
				GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

				GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
				  GX_Position2f32(endX, endY);
				  GX_Color4u8(crCursorColor32[iPlayer][0], crCursorColor32[iPlayer][1], crCursorColor32[iPlayer][2], 0x80);
				  GX_Position2f32( startX, endY);
				  GX_Color4u8(crCursorColor32[iPlayer][0], crCursorColor32[iPlayer][1], crCursorColor32[iPlayer][2], 0x80);
				  GX_Position2f32( startX,startY);
				  GX_Color4u8(crCursorColor32[iPlayer][0], crCursorColor32[iPlayer][1], crCursorColor32[iPlayer][2], 0x80);
				  GX_Position2f32(endX,startY);
				  GX_Color4u8(crCursorColor32[iPlayer][0], crCursorColor32[iPlayer][1], crCursorColor32[iPlayer][2], 0x80);
				GX_End();
			}
		}
	}

	//Write menu/debug text on screen
	GXColor fontColor = {150,255,150,255};
	IplFont_drawInit(fontColor);
	if(showFPSonScreen == FPS_SHOW) {
		IplFont_drawString(10,35,fpsInfo, 1.0, false);
	}

	int i = 0;
	DEBUG_update();
	for (i=0;i<DEBUG_TEXT_HEIGHT;i++)
		IplFont_drawString(10,(10*i+60),text[i], 0.5, false);

	gc_vout_render();
	
	if(firstFrame) {
		firstFrame = false;
		VIDEO_SetBlack(false);
		VIDEO_Flush();
	}
}

static int gc_vout_open(void) { 
	memset(GXtexture,0,sizeof(GXtexture));
	VIDEO_SetPreRetraceCallback(gc_vout_vsync);
	return 0; 
}

static void gc_vout_close(void) {}

static void gc_vout_flip(const void *vram, int stride, int bgr24,
			      int x, int y, int w, int h, int dims_changed) {
	if(vram == NULL) {
		memset(GXtexture,0,sizeof(GXtexture));
		if (menuActive) return;

		// clear the screen, and flush it
		//DEBUG_print("gc_vout_flip_null_vram",DBG_GPU1);

		//Write menu/debug text on screen
		GXColor fontColor = {150,255,150,255};
		IplFont_drawInit(fontColor);
		//if(showFPSonScreen)
		//	IplFont_drawString(10,35,szDispBuf, 1.0, false);

		int i = 0;
		DEBUG_update();
		for (i=0;i<DEBUG_TEXT_HEIGHT;i++)
			IplFont_drawString(10,(10*i+60),text[i], 0.5, false);

	   //reset swap table from GUI/DEBUG
		GX_SetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
		GX_SetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);

		gc_vout_render();
		return;
	}
	if (menuActive) return;

	//reset swap table from GUI/DEBUG
	GX_SetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_BLUE, GX_CH_GREEN, GX_CH_RED ,GX_CH_ALPHA);
	GX_SetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);

	GX_Flip(vram, stride * 2, bgr24 ? GX_TF_RGBA8 : GX_TF_RGB5A3, x, y, w, h);
}

static void gc_vout_set_mode(int w, int h, int raw_w, int raw_h, int bpp) {
	screen_w = raw_w;
	screen_h = raw_h;
}

//static void *gc_mmap(unsigned int size) {}
//static void gc_munmap(void *ptr, unsigned int size) {}

static struct rearmed_cbs gc_rearmed_cbs = {
   .pl_vout_open     = gc_vout_open,
   .pl_vout_set_mode = gc_vout_set_mode,
   .pl_vout_flip     = gc_vout_flip,
   .pl_vout_close    = gc_vout_close,
   //.mmap             = gc_mmap,
   //.munmap           = gc_munmap,
   /* from psxcounters */
   .gpu_hcnt         = &hSyncCount,
   .gpu_frame_count  = &frame_counter,
   .gpu_state_change = gpu_state_change,

   .gpu_unai = {
	   .lighting = 1,
	   .blending = 1,
   },
};

// Frame limiting/calculation routines, taken from plugin_lib.c, adapted for Wii/GC.
extern int g_emu_resetting;
static int vsync_cnt;
static int is_pal, frame_interval, frame_interval1024;
static int vsync_usec_time;
#define MAX_LAG_FRAMES 3

#define tvdiff(tv, tv_old) \
	((tv.tv_sec - tv_old.tv_sec) * 1000000 + tv.tv_usec - tv_old.tv_usec)

/* called on every vsync */
void pl_frame_limit(void)
{
	static struct timeval tv_old, tv_expect;
	static int vsync_cnt_prev, drc_active_vsyncs;
	struct timeval now;
	int diff, usadj;

	if (g_emu_resetting)
		return;

	vsync_cnt++;
	gettimeofday(&now, 0);

	if (now.tv_sec != tv_old.tv_sec) {
		diff = tvdiff(now, tv_old);
		gc_rearmed_cbs.vsps_cur = 0.0f;
		if (0 < diff && diff < 2000000)
			gc_rearmed_cbs.vsps_cur = 1000000.0f * (vsync_cnt - vsync_cnt_prev) / diff;
		vsync_cnt_prev = vsync_cnt;

		gc_rearmed_cbs.flips_per_sec = gc_rearmed_cbs.flip_cnt;
		sprintf(fpsInfo, "FPS %.2f", gc_rearmed_cbs.vsps_cur);

		gc_rearmed_cbs.flip_cnt = 0;

		tv_old = now;
		//new_dynarec_print_stats();
	}

	// tv_expect uses usec*1024 units instead of usecs for better accuracy
	tv_expect.tv_usec += frame_interval1024;
	if (tv_expect.tv_usec >= (1000000 << 10)) {
		tv_expect.tv_usec -= (1000000 << 10);
		tv_expect.tv_sec++;
	}
	diff = (tv_expect.tv_sec - now.tv_sec) * 1000000 + (tv_expect.tv_usec >> 10) - now.tv_usec;

	if (diff > MAX_LAG_FRAMES * frame_interval || diff < -MAX_LAG_FRAMES * frame_interval) {
		//printf("pl_frame_limit reset, diff=%d, iv %d\n", diff, frame_interval);
		tv_expect = now;
		diff = 0;
		// try to align with vsync
		usadj = vsync_usec_time;
		while (usadj < tv_expect.tv_usec - frame_interval)
			usadj += frame_interval;
		tv_expect.tv_usec = usadj << 10;
	}

	if (frameLimit == FRAMELIMIT_AUTO && diff > frame_interval) {
		if (vsync_enable)
			VIDEO_WaitVSync();
		else
			usleep(diff - frame_interval);
	}

	if (gc_rearmed_cbs.frameskip) {
		if (diff < -frame_interval)
			gc_rearmed_cbs.fskip_advice = 1;
		else if (diff >= 0)
			gc_rearmed_cbs.fskip_advice = 0;
	}
}

void pl_timing_prepare(int is_pal_)
{
	gc_rearmed_cbs.fskip_advice = 0;
	gc_rearmed_cbs.flips_per_sec = 0;
	gc_rearmed_cbs.cpu_usage = 0;

	is_pal = is_pal_;
	frame_interval = is_pal ? 20000 : 16667;
	frame_interval1024 = is_pal ? 20000*1024 : 17066667;

	// used by P.E.Op.S. frameskip code
	gc_rearmed_cbs.gpu_peops.fFrameRateHz = is_pal ? 50.0f : 59.94f;
	gc_rearmed_cbs.gpu_peops.dwFrameRateTicks =
		(100000*100 / (unsigned long)(gc_rearmed_cbs.gpu_peops.fFrameRateHz*100));

	vsync_enable = !is_pal && frameLimit == FRAMELIMIT_AUTO;
}

void plugin_call_rearmed_cbs(void)
{
	extern void *hGPUDriver;
	void (*rearmed_set_cbs)(const struct rearmed_cbs *cbs);

	rearmed_set_cbs = SysLoadSym(hGPUDriver, "GPUrearmedCallbacks");
	if (rearmed_set_cbs != NULL)
		rearmed_set_cbs(&gc_rearmed_cbs);
}

void go(void) {
	Config.PsxOut = 0;
	stop = 0;
	
	// Enable a multi-tap in port 1 if we have more than 2 mapped controllers.
	int controllersConnected = 0;
	for(int i = 0; i < 4; i++) {
		controllersConnected += ((in_type[i] && virtualControllers[i].inUse) ? 1 : 0);
	}
	multitap1 = controllersConnected > 2 ? 1 : 0;
	SysPrintf("Multi-tap port status: %s\r\n", multitap1 ? "connected" : "disconnected");

	/* Apply settings from menu */
	gc_rearmed_cbs.gpu_unai.dithering = !!useDithering;
	gc_rearmed_cbs.gpu_peops.iUseDither = useDithering;
	gc_rearmed_cbs.gpu_peopsgl.bDrawDither = useDithering;
	gc_rearmed_cbs.frameskip = frameSkip;

	plugin_call_rearmed_cbs();
	psxCpu->Execute();
	
	// remove this callback to avoid any issues when returning to the menu.
	GX_SetDrawDoneCallback(NULL);
}

int OpenPlugins() {
	int ret;

	plugin_call_rearmed_cbs();
	while ((ret = _OpenPlugins()) == -2) {
		ReleasePlugins();
		if (LoadPlugins() == -1) return -1;
	}
	return ret;	
}

void ClosePlugins() {
	int ret;

	ret = CDR_close();
	if (ret < 0) { SysPrintf("Error Closing CDR Plugin\n"); return; }
	ret = SPU_close();
	if (ret < 0) { SysPrintf("Error Closing SPU Plugin\n"); return; }
	ret = PAD1_close();
	if (ret < 0) { SysPrintf("Error Closing PAD1 Plugin\n"); return; }
	ret = PAD2_close();
	if (ret < 0) { SysPrintf("Error Closing PAD2 Plugin\n"); return; }
	ret = GPU_close();
	if (ret < 0) { SysPrintf("Error Closing GPU Plugin\n"); return; }

	if (Config.UseNet) {
		NET_pause();
	}
}

void ResetPlugins() {
	int ret;

	CDR_shutdown();
	GPU_shutdown();
	SPU_shutdown();
	PAD1_shutdown();
	PAD2_shutdown();
	if (Config.UseNet) NET_shutdown(); 

	ret = CDR_init();
	if (ret < 0) { SysPrintf("CDRinit error: %d\n", ret); return; }
	ret = GPU_init();
	if (ret < 0) { SysPrintf("GPUinit error: %d\n", ret); return; }
	ret = SPU_init();
	if (ret < 0) { SysPrintf("SPUinit error: %d\n", ret); return; }
	ret = PAD1_init(1);
	if (ret < 0) { SysPrintf("PAD1init error: %d\n", ret); return; }
	ret = PAD2_init(2);
	if (ret < 0) { SysPrintf("PAD2init error: %d\n", ret); return; }
	if (Config.UseNet) {
		ret = NET_init();
		if (ret < 0) { SysPrintf("NETinit error: %d\n", ret); return; }
	}

	NetOpened = 0;
}
