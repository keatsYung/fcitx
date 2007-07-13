/***************************************************************************
 *   Copyright (C) 2002~2005 by Yuking                                     *
 *   yuking_net@sohu.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "MainWindow.h"

#include <stdio.h>

#include "IC.h"
#include "ui.h"
#include "ime.h"
#include "tools.h"

//下面的顺序不能改变
#include "logo.xpm"
#include "fullcorner.xpm"
#include "halfcorner.xpm"
#include "chnPunc.xpm"
#include "engPunc.xpm"
#include "gbk-0.xpm"
#include "gbk-1.xpm"
#include "lx-0.xpm"
#include "lx-1.xpm"
#include "lock-0.xpm"
#include "lock-1.xpm"
#include "vk-1.xpm"
#include "ft-0.xpm"
#include "ft-1.xpm"

#include "ui.h"
#include "vk.h"

Window          mainWindow;
int             MAINWND_WIDTH = _MAINWND_WIDTH;

int             iMainWindowX = MAINWND_STARTX;
int             iMainWindowY = MAINWND_STARTY;

WINDOW_COLOR    mainWindowColor = { NULL, NULL, {0, 255 << 8, 240 << 8, 255 << 8} };
MESSAGE_COLOR   mainWindowLineColor = { NULL, {0, 220 << 8, 0, 0} };	//线条色
MESSAGE_COLOR   IMNameColor[3] = {	//输入法名称的颜色
    {NULL, {0, 170 << 8, 170 << 8, 170 << 8}},
    {NULL, {0, 150 << 8, 200 << 8, 150 << 8}},
    {NULL, {0, 0, 0, 255 << 8}}
};

Bool            _3DEffectMainWindow = False;
XImage         *pLogo = NULL;

XImage         *pCorner[2] = { NULL, NULL };
char          **CornerLogo[2] = { halfcorner_xpm, fullcorner_xpm };
XImage         *pPunc[2] = { NULL, NULL };
char          **PuncLogo[2] = { engPunc_xpm, chnPunc_xpm };
XImage         *pGBK[2] = { NULL, NULL };
char          **GBKLogo[2] = { gbk_0_xpm, gbk_1_xpm };
XImage         *pLX[2] = { NULL, NULL };
char          **LXLogo[2] = { lx_0_xpm, lx_1_xpm };
XImage         *pLock[2] = { NULL, NULL };
char          **LockLogo[2] = { lock_0_xpm, lock_1_xpm };

XImage         *pGBKT[2] = { NULL, NULL };
char          **GBKTLogo[2] = { ft_0_xpm, ft_1_xpm };

XImage         *pVK = NULL;

HIDE_MAINWINDOW hideMainWindow = HM_SHOW;

Bool            bLocked = True;
Bool            bCompactMainWindow = False;
Bool            bShowVK = True;

extern Display *dpy;
extern GC       dimGC;
extern int      i3DEffect;
extern IC      *CurrentIC;
extern Bool     bCorner;
extern Bool     bChnPunc;
extern INT8     iIMIndex;
extern Bool     bUseGBK;
extern Bool     bSP;
extern Bool     bUseLegend;
extern IM      *im;
extern CARD16   connect_id;

extern Bool     bUseGBKT;

#ifdef _USE_XFT
extern XftFont *xftMainWindowFont;
extern XftFont *xftMainWindowFontEn;
#else
extern XFontSet fontSetMainWindow;
#endif

extern VKS      vks[];
extern unsigned char iCurrentVK;
extern Bool     bVK;

Bool CreateMainWindow (void)
{
    XSetWindowAttributes attrib;
    unsigned long   attribmask;
    int             iBackPixel;

    attrib.override_redirect = True;
    attribmask = CWOverrideRedirect;

    sprintf (strMainWindowXPMBackColor, ". 	c #%2x%2x%2x", mainWindowColor.backColor.red >> 8, mainWindowColor.backColor.green >> 8, mainWindowColor.backColor.blue >> 8);

    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(mainWindowColor.backColor)))
	iBackPixel = mainWindowColor.backColor.pixel;
    else
	iBackPixel = WhitePixel (dpy, DefaultScreen (dpy));

    mainWindow = XCreateSimpleWindow (dpy, DefaultRootWindow (dpy), iMainWindowX, iMainWindowY, MAINWND_WIDTH, MAINWND_HEIGHT, 0, WhitePixel (dpy, DefaultScreen (dpy)), iBackPixel);
    if (mainWindow == (Window) NULL)
	return False;

    XChangeWindowAttributes (dpy, mainWindow, attribmask, &attrib);
    XSelectInput (dpy, mainWindow, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | EnterWindowMask | PointerMotionMask | LeaveWindowMask | VisibilityChangeMask);

    InitMainWindowColor ();

    return True;
}

void DisplayMainWindow (void)
{
    INT8            iIndex = 0;
    INT16           iPos;

#ifdef _USE_XFT
    char            strTemp[MAX_IM_NAME + 1];
    char           *p1, *p2;
    Bool            bEn;
#endif
    iIndex = IS_CLOSED;
    if (hideMainWindow == HM_SHOW || (hideMainWindow == HM_AUTO && (ConnectIDGetState (connect_id) != IS_CLOSED))) {
	XMapRaised (dpy, mainWindow);

	if (!pLogo) {
	    pLogo = XGetImage (dpy, mainWindow, 0, 0, 15, 16, AllPlanes, XYPixmap);
	    FillImageByXPMData (pLogo, logo_xpm);
	}
	if (!pVK) {
	    pVK = XGetImage (dpy, mainWindow, 0, 0, 19, 16, AllPlanes, XYPixmap);
	    FillImageByXPMData (pVK, vk_1_xpm);
	}

	XPutImage (dpy, mainWindow, mainWindowColor.backGC, pLogo, 0, 0, 2, 2, 15, 16);
	XDrawRectangle (dpy, mainWindow, mainWindowLineColor.gc, 0, 0, MAINWND_WIDTH - 1, MAINWND_HEIGHT - 1);

	iPos = 20;
	if (!bCompactMainWindow) {
	    if (!pPunc[bChnPunc]) {
		pPunc[bChnPunc] = XGetImage (dpy, mainWindow, 0, 0, 15, 16, AllPlanes, XYPixmap);
		FillImageByXPMData (pPunc[bChnPunc], PuncLogo[bChnPunc]);
	    }
	    XPutImage (dpy, mainWindow, mainWindowColor.backGC, pPunc[bChnPunc], 0, 0, iPos, 2, 15, 16);
	    iPos += 18;

	    if (!pCorner[bCorner]) {
		pCorner[bCorner] = XGetImage (dpy, mainWindow, 0, 0, 15, 16, AllPlanes, XYPixmap);
		FillImageByXPMData (pCorner[bCorner], CornerLogo[bCorner]);
	    }
	    XPutImage (dpy, mainWindow, mainWindowColor.backGC, pCorner[bCorner], 0, 0, iPos, 2, 15, 16);
	    iPos += 18;

	    if (!pGBK[bUseGBK]) {
		pGBK[bUseGBK] = XGetImage (dpy, mainWindow, 0, 0, 15, 16, AllPlanes, XYPixmap);
		FillImageByXPMData (pGBK[bUseGBK], GBKLogo[bUseGBK]);
	    }
	    XPutImage (dpy, mainWindow, mainWindowColor.backGC, pGBK[bUseGBK], 0, 0, iPos, 2, 15, 16);
	    iPos += 18;

	    if (!pLX[bUseLegend]) {
		pLX[bUseLegend] = XGetImage (dpy, mainWindow, 0, 0, 15, 16, AllPlanes, XYPixmap);
		FillImageByXPMData (pLX[bUseLegend], LXLogo[bUseLegend]);
	    }
	    XPutImage (dpy, mainWindow, mainWindowColor.backGC, pLX[bUseLegend], 0, 0, iPos, 2, 15, 16);
	    iPos += 18;

	    if (!pGBKT[bUseGBKT]) {
		pGBKT[bUseGBKT] = XGetImage (dpy, mainWindow, 0, 0, 15, 16, AllPlanes, XYPixmap);
		FillImageByXPMData (pGBKT[bUseGBKT], GBKTLogo[bUseGBKT]);
	    }
	    XPutImage (dpy, mainWindow, mainWindowColor.backGC, pGBKT[bUseGBKT], 0, 0, iPos, 2, 15, 16);
	    iPos += 18;
	}

	if (!pLock[bLocked]) {
	    pLock[bLocked] = XGetImage (dpy, mainWindow, 0, 0, 8, 16, AllPlanes, XYPixmap);
	    FillImageByXPMData (pLock[bLocked], LockLogo[bLocked]);
	}
	XPutImage (dpy, mainWindow, mainWindowColor.backGC, pLock[bLocked], 0, 0, iPos, 2, 15, 16);
	iPos += 11;

	if (bShowVK || !bCompactMainWindow) {
	    XPutImage (dpy, mainWindow, mainWindowColor.backGC, pVK, 0, 0, iPos, 2, 19, 16);
	    iPos += 23;
	}

	iIndex = ConnectIDGetState (connect_id);
	XClearArea (dpy, mainWindow, iPos, 2, MAINWND_WIDTH - iPos - 2, MAINWND_HEIGHT - 4, False);
#ifdef _USE_XFT
	iPos += 2;
	p1 = (bVK) ? vks[iCurrentVK].strName : im[iIMIndex].strName;
	while (*p1) {
	    if (isprint (*p1))
		bEn = True;
	    else
		bEn = False;
	    p2 = strTemp;
	    while (*p1) {
		*p2++ = *p1++;
		if (isprint (*p1)) {
		    if (!bEn)
			break;
		}
		else {
		    if (bEn)
			break;
		}
	    }
	    *p2 = '\0';

	    OutputString (mainWindow, (bEn) ? xftMainWindowFontEn : xftMainWindowFont, strTemp, iPos, FontHeight (xftMainWindowFont) + (MAINWND_HEIGHT - FontHeight (xftMainWindowFont)) / 2 - 1, IMNameColor[iIndex].color);
	    iPos += StringWidth (strTemp, (bEn) ? xftMainWindowFontEn : xftMainWindowFont);
	}
#else
	OutputString (mainWindow, fontSetMainWindow, (bVK) ? vks[iCurrentVK].strName : im[iIMIndex].strName, iPos, FontHeight (fontSetMainWindow) + (MAINWND_HEIGHT - FontHeight (fontSetMainWindow)) / 2 - 1, IMNameColor[iIndex].gc);
#endif

	if (_3DEffectMainWindow) {
	    Draw3DEffect (mainWindow, 1, 1, MAINWND_WIDTH - 2, MAINWND_HEIGHT - 2, _3D_UPPER);
	    if (!bCompactMainWindow) {
		Draw3DEffect (mainWindow, 1, 1, 18, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 19, 1, 18, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 37, 1, 18, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 55, 1, 18, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 73, 1, 18, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 91, 1, 11, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 102, 1, 22, 18, _3D_UPPER);
	    }
	}
	else {
	    iPos = 19;
	    XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, 18, 4, 18, MAINWND_HEIGHT - 4);
	    if (!bCompactMainWindow) {
		XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, 36, 4, 36, MAINWND_HEIGHT - 4);
		XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, 54, 4, 54, MAINWND_HEIGHT - 4);
		XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, 72, 4, 72, MAINWND_HEIGHT - 4);
		XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, 89, 4, 89, MAINWND_HEIGHT - 4);
		XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, 107, 4, 107, MAINWND_HEIGHT - 4);
		iPos = 108;
	    }
	    iPos += 11;
	    if (bShowVK || !bCompactMainWindow) {
		XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, iPos, 4, iPos, MAINWND_HEIGHT - 4);
		iPos += 21;
	    }
	    XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, iPos, 4, iPos, MAINWND_HEIGHT - 4);
	}
    }
    else
	XUnmapWindow (dpy, mainWindow);
}

void InitMainWindowColor (void)
{
    XGCValues       values;
    int             iPixel;
    int             i;

    mainWindowLineColor.gc = XCreateGC (dpy, mainWindow, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(mainWindowLineColor.color)))
	iPixel = mainWindowLineColor.color.pixel;
    else
	iPixel = WhitePixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, mainWindowLineColor.gc, iPixel);

    for (i = 0; i < 3; i++) {
	IMNameColor[i].gc = XCreateGC (dpy, mainWindow, 0, &values);
	if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(IMNameColor[i].color)))
	    iPixel = IMNameColor[i].color.pixel;
	else
	    iPixel = WhitePixel (dpy, DefaultScreen (dpy));
	XSetForeground (dpy, IMNameColor[i].gc, iPixel);
    }
}

void ChangeLock (void)
{
    bLocked = !bLocked;
    DisplayMainWindow ();

    SaveProfile ();
}
