/***************************************************************************
 *  GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
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
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ***************************************************************************/

/*****************************************************************************/
#include <odroid_go.h>
#include <Arduino.h>
TaskHandle_t VidTask;
#include <sys/time.h>
extern "C" {
#include "gbagi.h"
#include "screen.h"
#include "text.h"
#include "picture.h"
#include "views.h"
#include "input.h"
#include "system.h"
#include "status.h"
#include "wingui.h"
/*****************************************************************************/

#if 1
	U16 *vidPtr;
	U8 pictureBuf[PIC_SIZE];
#else
	U16 *vidPtr;
	U8 pictureBuf[PIC_SIZE] _EWRAM_;
#endif
_RECT port;
BOOL SHOW_VERSION;
/*****************************************************************************/
const int scrCoords[160][3] = {
	{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},{  3,  4,  1},{  3,  4,  1},{  3,  4,  1},{  3,  4,  1},
	{  6,  8,  2},{  6,  8,  2},{  6,  8,  2},{  6,  8,  2},{  9, 12,  3},{  9, 12,  3},{  9, 12,  3},{  9, 12,  3},
	{ 12, 16,  4},{ 12, 16,  4},{ 12, 16,  4},{ 12, 16,  4},{ 15, 20,  5},{ 15, 20,  5},{ 15, 20,  5},{ 15, 20,  5},
	{ 18, 24,  6},{ 18, 24,  6},{ 18, 24,  6},{ 18, 24,  6},{ 21, 28,  7},{ 21, 28,  7},{ 21, 28,  7},{ 21, 28,  7},
	{ 24, 32,  8},{ 24, 32,  8},{ 24, 32,  8},{ 24, 32,  8},{ 27, 36,  9},{ 27, 36,  9},{ 27, 36,  9},{ 27, 36,  9},
	{ 30, 40, 10},{ 30, 40, 10},{ 30, 40, 10},{ 30, 40, 10},{ 33, 44, 11},{ 33, 44, 11},{ 33, 44, 11},{ 33, 44, 11},
	{ 36, 48, 12},{ 36, 48, 12},{ 36, 48, 12},{ 36, 48, 12},{ 39, 52, 13},{ 39, 52, 13},{ 39, 52, 13},{ 39, 52, 13},
	{ 42, 56, 14},{ 42, 56, 14},{ 42, 56, 14},{ 42, 56, 14},{ 45, 60, 15},{ 45, 60, 15},{ 45, 60, 15},{ 45, 60, 15},
	{ 48, 64, 16},{ 48, 64, 16},{ 48, 64, 16},{ 48, 64, 16},{ 51, 68, 17},{ 51, 68, 17},{ 51, 68, 17},{ 51, 68, 17},
	{ 54, 72, 18},{ 54, 72, 18},{ 54, 72, 18},{ 54, 72, 18},{ 57, 76, 19},{ 57, 76, 19},{ 57, 76, 19},{ 57, 76, 19},
	{ 60, 80, 20},{ 60, 80, 20},{ 60, 80, 20},{ 60, 80, 20},{ 63, 84, 21},{ 63, 84, 21},{ 63, 84, 21},{ 63, 84, 21},
	{ 66, 88, 22},{ 66, 88, 22},{ 66, 88, 22},{ 66, 88, 22},{ 69, 92, 23},{ 69, 92, 23},{ 69, 92, 23},{ 69, 92, 23},
	{ 72, 96, 24},{ 72, 96, 24},{ 72, 96, 24},{ 72, 96, 24},{ 75,100, 25},{ 75,100, 25},{ 75,100, 25},{ 75,100, 25},
	{ 78,104, 26},{ 78,104, 26},{ 78,104, 26},{ 78,104, 26},{ 81,108, 27},{ 81,108, 27},{ 81,108, 27},{ 81,108, 27},
	{ 84,112, 28},{ 84,112, 28},{ 84,112, 28},{ 84,112, 28},{ 87,116, 29},{ 87,116, 29},{ 87,116, 29},{ 87,116, 29},
	{ 90,120, 30},{ 90,120, 30},{ 90,120, 30},{ 90,120, 30},{ 93,124, 31},{ 93,124, 31},{ 93,124, 31},{ 93,124, 31},
	{ 96,128, 32},{ 96,128, 32},{ 96,128, 32},{ 96,128, 32},{ 99,132, 33},{ 99,132, 33},{ 99,132, 33},{ 99,132, 33},
	{102,136, 34},{102,136, 34},{102,136, 34},{102,136, 34},{105,140, 35},{105,140, 35},{105,140, 35},{105,140, 35},
	{108,144, 36},{108,144, 36},{108,144, 36},{108,144, 36},{111,148, 37},{111,148, 37},{111,148, 37},{111,148, 37},
	{114,152, 38},{114,152, 38},{114,152, 38},{114,152, 38},{117,156, 39},{117,156, 39},{117,156, 39},{117,156, 39},
};
#define N2B(h,l)\
	((((h)<<4)&0xF0)|((l)&0x0F))
#define B2W(l,h)\
	((((h)<<8)&0xFF00)|((l)&0x00FF))
#define ODC(x)\
	(((x)&0xFF00) << 16 | (((x) & 0x0F00) << 8)|(((x)&0x000F) << 8))
/*****************************************************************************/
bool screenupdate = false;

typedef struct updatecoords {
	U8 *p;
	U16 *s;
	int w;
	int h;
};

struct updatecoords region;

static void videothread(void * parameter) {
	for(;;) {
		struct timeval now;
		long int lasttime;
		static long int next_update = 0;
		if (!screenupdate) {
			vTaskDelay(1);
			next_update = 0;
			continue;
		}
		uint16_t delay_interval = 1;
		gettimeofday(&now, NULL);
		lasttime = now.tv_sec * 1000000 + now.tv_usec;
		if (lasttime > next_update) {
			next_update = lasttime + delay_interval;
		}
		screenupdate = false;
	}
}

void display_setup() {
	//pictureBuf = (U8 *)heap_caps_malloc(PIC_SIZE, MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
	vidPtr = (U16 *)heap_caps_malloc(SCREEN_SIZE, MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
	//memset(pictureBuf, 0, PIC_SIZE);
	memset(vidPtr, 0, SCREEN_SIZE);
    xTaskCreatePinnedToCore(videothread, "VIDEO", 10000, NULL, 2,
	&VidTask, xPortGetCoreID() == 1 ? 0 : 1);
}

void RotatePicBuf()
{
	U8 *p=pictureBuf,*pend=pictureBuf+PIC_SIZE;
	while(p<pend)
		*p++ = (*p>>4)|(*p<<4);
}
#define VCOUNT (*(volatile U16*)0x04000006)       
#ifdef _WINDOWS
	#define vsync() ;
#else
	#define vsync() while (VCOUNT != 160);
#endif
void RenderShake(int offs,int amt)
{
	int i,x,y,yy;
	if(offs) {
    	vsync();
		DrawScreenBlock(
    		pictureBuf + (amt*PIC_WIDTH) + scrCoords[amt][1],
    		vidPtr,
       	 	40-scrCoords[amt][2],160-amt
    	);   
    	vsync();
        yy=0;
        for(y=0;y<160;y++) {
        	for(x=120-scrCoords[amt][0];x<120;x++)
             	vidPtr[yy+x]=0;
            yy+=120;
        }
        yy=(160-amt)*120;
        for(y=0;y<amt;y++) {
        	for(x=0;x<120;x++)
             	vidPtr[yy+x]=0;
            yy+=120;
        }
    } else {    
    	vsync();
		DrawScreenBlock(
    		pictureBuf,
    		vidPtr + (amt*VID_WIDE) + scrCoords[amt][0],
       	 	40-scrCoords[amt][2],160-amt
    	);
        yy=0;
        for(y=0;y<160;y++) {
        	for(x=scrCoords[amt][0];x>=0;x--)
             	vidPtr[yy+x]=0;
            yy+=120;
        }      
    	vsync();
        yy=0;
        for(y=0;y<amt;y++) {
        	for(x=0;x<120;x++)
             	vidPtr[yy+x]=0;
            yy+=120;
        }
    }
}
/*****************************************************************************/
void ShakeScreen(int count)
{
	int i,j;
    while(count--) {
		for(i=0;i<2;i++) {
			for(j=4;j<=8;j+=8) {
				RenderShake(i,j);
#ifdef _WINDOWS
				Delay(20);
#endif
            }
        }
    }      
	RedrawScreen();
}
/*****************************************************************************/
void RedrawScreen()
{
	RenderUpdate(0,0,PIC_MAXX,PIC_MAXY);
	WriteStatusLine();
}
/*****************************************************************************/
void UpdateGfx()
{
	UpdateVObj();
}
/*****************************************************************************/
void PlotPix(int x, int y, U8 c1)
{// slow! Used as little as possible
	U16 *p;
	if((x+=port.left)>=port.left&&x<=port.right&&
       (y+=port.top)>=port.top&&y<=port.bottom) {
		p=&vidPtr[(y*VID_WIDE)+(x>>1)];
    	if(x&1) {
        	*p &= 0x00FF;
            *p |= c1<<8;
        } else {
        	*p &= 0xFF00;
            *p |= c1;
        }
	U16 pixclr = GO.lcd.color565((*(p) & 0x7c00) >> 3,((*(p) & 0x03e0) >> 2 ),(*(p) & 0x001f) << 3);
	GO.lcd.drawPixel(x, y, pixclr);
    }
}
/*****************************************************************************/
void HilightPix(int x, int y)
{
	U16 *p;
	if((x+=port.left)>=port.left&&x<=port.right&&
       (y+=port.top)>=port.top&&y<=port.bottom) {
		p=&vidPtr[(y*VID_WIDE)+(x>>1)];
    	if(x&1) {
            *p |= 0xF000;
        } else {
            *p |= 0x00F0;
        }
	U16 pixclr = GO.lcd.color565((*(p) & 0x7c00) >> 3,((*(p) & 0x03e0) >> 2 ),(*(p) & 0x001f) << 3);
	GO.lcd.drawPixel(x, y, pixclr);
    }
}
/*****************************************************************************/
void LolightPix(int x, int y)
{
	U16 *p;
	if((x+=port.left)>=port.left&&x<=port.right&&
       (y+=port.top)>=port.top&&y<=port.bottom) {
		p=&vidPtr[(y*VID_WIDE)+(x>>1)];
    	if(x&1) {
            *p &= 0x0FFF;
            *p |= 0x7000;
        } else {
            *p &= 0xFF0F;
            *p |= 0x0070;
        }
	U16 pixclr = GO.lcd.color565((*(p) & 0x7c00) >> 3,((*(p) & 0x03e0) >> 2 ),(*(p) & 0x001f) << 3);
	GO.lcd.drawPixel(x, y, pixclr);
    }
}
/*****************************************************************************/
void ShadowPix(int x, int y)
{
	U16 *p;
	if((x+=port.left)>=port.left&&x<=port.right&&
       (y+=port.top)>=port.top&&y<=port.bottom) {
		p=&vidPtr[(y*VID_WIDE)+(x>>1)];
    	if(x&1) {
            *p &= 0x8FFF;
            //*p ^= 0x8000
            ;
        } else {
            *p &= 0xFF8F;
            //*p ^= 0x0080;
        }
	U16 pixclr = GO.lcd.color565((*(p) & 0x7c00) >> 3,((*(p) & 0x03e0) >> 2 ),(*(p) & 0x001f) << 3);
	GO.lcd.drawPixel(x, y, pixclr);
    }
}
/*****************************************************************************/
void ShadowBoxX(int x1, int y1, int x2, int y2)
{
	U16 *p;
    int l1,l;

    if(x1<port.left)
    	x1=port.left;
    else if(x1>port.right)
    	x1=port.right;
    if(x2<port.left)
    	x2=port.left;
    else if(x2>port.right)
    	x2=port.right;

    if(y1<port.top)
    	y1=port.top;
    else if(y1>port.bottom)
    	y1=port.bottom;
    if(y2<port.top)
    	y2=port.top;
    else if(y2>port.bottom)
    	y2=port.bottom;
	l1 = ((x2-x1+2)>>1);
	p=&vidPtr[(y1*VID_WIDE)+(x1>>1)];
    while(y1<=y2) {
        for(l=l1;l;l--) {
            *p++ &= 0x8F8F;
        }
        p+=VID_WIDE-l1;
    	y1++;
    }
}
/*****************************************************************************/
void PlotPixLO(int x, int y, U8 c1)
{
	U16 *p;
	if((x+=port.left)>=port.left&&x<=port.right&&
       (y+=port.top)>=port.top&&y<=port.bottom) {
		p=&vidPtr[(y*VID_WIDE)+(x>>1)];
    	if(x&1) {
        	*p &= 0xF0FF;
            *p |= c1<<8;
        } else {
        	*p &= 0xFFF0;
            *p |= c1;
        }
	U16 pixclr = GO.lcd.color565((*(p) & 0x7c00) >> 3,((*(p) & 0x03e0) >> 2 ),(*(p) & 0x001f) << 3);
	GO.lcd.drawPixel(x, y, pixclr);
    }
}
/*****************************************************************************/
void PlotPixHI(int x, int y, U8 c1)
{
	U16 *p;
	if((x+=port.left)>=port.left&&x<=port.right&&
       (y+=port.top)>=port.top&&y<=port.bottom) {
		p=&vidPtr[(y*VID_WIDE)+(x>>1)];
    	if(x&1) {
        	*p &= 0x0FFF;
            *p |= c1<<12;
        } else {
        	*p &= 0xFF0F;
            *p |= c1<<4;
        }
	U16 pixclr = GO.lcd.color565((*(p) & 0x7c00) >> 3,((*(p) & 0x03e0) >> 2 ),(*(p) & 0x001f) << 3);
	GO.lcd.drawPixel(x, y, pixclr);
    }
}
/*****************************************************************************/
void PicPlotPix(U8 x, U8 y, U8 c)
{
    if(x<PIC_WIDTH&&y<PIC_HEIGHT)
    	pictureBuf[y*PIC_WIDTH+x] = c;
	#if 0
	U16 pixclr = GO.lcd.color565(((c << 8) & 0x7c00) >> 3,(((c << 4) & 0x03e0) >> 2 ),((c) & 0x001f) << 3);
	GO.lcd.drawPixel(x, y, pixclr);
	#endif
}
/*****************************************************************************/
void ShowPic()
{
	if(PIC_VISIBLE)
    	RenderUpdate(0,0,PIC_MAXX,PIC_MAXY);
    if(SHOW_VERSION) {
		DrawStringAbs(11,132,"GBAGI v"BUILD_VERSION"",0xEF);
		DrawStringAbs(9,141,"("BUILD_DATE")",0xEF);
		DrawStringAbs(4,150,"By Brian Provinciano :: www.bripro.com",0xEF);
	}
}
/*****************************************************************************/
void DrawScreenBlock(U8 *p,U16 *s,int w,int h)
{
	U8 c1,c2,c3,c4;
    int x;
    int y = 0;
    U8 *p2;
    U16 *s2;
    int yoffset = ((p - pictureBuf) / PIC_WIDTH);
    int xoffset = (((p - pictureBuf) % PIC_WIDTH) * 2);

    while(y <= h) {
        	s2=s;
        	p2=p;
			for(x=0;x <= w;x++) {
//#define VIEWPRI
#ifdef VIEWPRI
            	c1 = *p2++>>4;
            	c2 = *p2++>>4;
            	c3 = *p2++>>4;
            	c4 = *p2++>>4;
#else
            	c1 = *p2++ & 0xf;
            	c2 = *p2++ & 0xf;
            	c3 = *p2++ & 0xf;
            	c4 = *p2++ & 0xf;
#endif
            	*s2++ = (c1 << 12) | (c1 << 8) | (c1 << 4) | c1; //B2W(N2B(c2,c2),N2B(c3,c3));
            	*s2++ = (c2 << 12) | (c2 << 8) | (c2 << 4) | c2; //B2W(N2B(c2,c2),N2B(c3,c3));
            	*s2++ = (c3 << 12) | (c3 << 8) | (c3 << 4) | c3; //B2W(N2B(c2,c2),N2B(c3,c3));
            	*s2++ = (c4 << 12) | (c4 << 8) | (c4 << 4) | c4; //B2W(N2B(c2,c2),N2B(c3,c3));
			U16 pixclr = GO.lcd.color565((*(s2 - 4) & 0x7c00) >> 7,((*(s2 - 4) & 0x03e0) >> 2 ),(*(s2 - 4) & 0x001f) << 3);
			GO.lcd.drawPixel(xoffset + x * 8 + 0, yoffset + y, pixclr);
			GO.lcd.drawPixel(xoffset + x * 8 + 1, yoffset + y, pixclr);
			pixclr = GO.lcd.color565((*(s2 - 3) & 0x7c00) >> 7,((*(s2 - 3) & 0x03e0) >> 2 ),(*(s2 - 3) & 0x001f) << 3);
			GO.lcd.drawPixel(xoffset + x * 8 + 2, yoffset + y, pixclr);
			GO.lcd.drawPixel(xoffset + x * 8 + 3, yoffset + y, pixclr);
			pixclr = GO.lcd.color565((*(s2 - 2) & 0x7c00) >> 7,((*(s2 - 2) & 0x03e0) >> 2 ),(*(s2 - 2) & 0x001f) << 3);
			GO.lcd.drawPixel(xoffset + x * 8 + 4, yoffset + y, pixclr);
			GO.lcd.drawPixel(xoffset + x * 8 + 5, yoffset + y, pixclr);
			pixclr = GO.lcd.color565((*(s2 - 1) & 0x7c00) >> 7,((*(s2 - 1) & 0x03e0) >> 2 ),(*(s2 - 1) & 0x001f) << 3);
			GO.lcd.drawPixel(xoffset + x * 8 + 6, yoffset + y, pixclr);
			GO.lcd.drawPixel(xoffset + x * 8 + 7, yoffset + y, pixclr);
        	}
        	p+=PIC_WIDTH;
        	s+=VID_WIDE;
        	y++;
    }
}
/*****************************************************************************/
void RenderUpdate(int x1, int y1, int x2, int y2)
{

	U8 *p;
	U16 *s;
	int w,h,aX1,aX2,aY1,aY2,maxY, yd,ye,i;

	if((x1>=PIC_WIDTH)||x2<0)
		return;
	if(x1<0) x1=0;

	if(STATUS_VISIBLE) {
		maxY = PIC_MAXY-16;
        yd = Y_ADJUST_CL+8;
        ye=8;
	} else {
		maxY = PIC_MAXY-8;
        yd = Y_ADJUST_CL;    
        ye=0;
	}
		if((y2-=yd)<0)
			return;
		if((y1-=yd)<0)
			y1=0;
		if(y1>maxY)
			return;
        if(y2>maxY)
        	y2=maxY;

	w = (scrCoords[(x2>=PIC_WIDTH)?PIC_MAXX:x2][2]-scrCoords[x1][2])+1;
		h=y2-y1+1;
    	p = pictureBuf + ((y1+yd)*PIC_WIDTH) + scrCoords[x1][1];
    	s = vidPtr + ((y1+(ye))*VID_WIDE) + scrCoords[x1][0];
		DrawScreenBlock(p,s,w,h);
}
/*****************************************************************************/
void ClearLine(int row, U8 c)
{
	int h=CHAR_HEIGHT,w;
	U16 wC;
    U16 *p=vidPtr+(row*VID_WIDE*CHAR_HEIGHT);
	if(row>GBA_MAXROW)
    	return;
    wC = B2W(c,c);
	while(h) {
		for(w=VID_WIDE;w;w--)
			*p++ = wC;
        h--;
    }
}
/*****************************************************************************/
void ClearTextRect(int x1, int y1, int x2, int y2, U8 c)
{
	int x,size,w,mr;
	U16 wC;
    U16 *p;
    y1 = ((y1-1)*CHAR_HEIGHT)-Y_ADJUST_CL;
    y2 = ((y2)*CHAR_HEIGHT)-Y_ADJUST_CL;
    if(y2<0||y1>=SCREEN_HEIGHT-Y_ADJUST_CL) return;
    if(y1<0)
    	y1=0;
    if(y2>SCREEN_HEIGHT-Y_ADJUST_CL+8)
    	y2=SCREEN_HEIGHT-Y_ADJUST_CL+8;

	size = ((x2+1-x1)*CHAR_WIDTH)>>1;

    p=vidPtr+(y1*VID_WIDE)+((x1*CHAR_WIDTH)>>1);
    wC = B2W(N2B(c,c),N2B(c,c));
	while(y1<y2) {
		for(w=size;w;w--)
			*p++ = wC;
        p+=(SCREEN_WIDTH>>1)-size;
        y1++;
    }
}
/*****************************************************************************/
void ClearScreen(U8 c)
{
	U16 wC = B2W(c,c),x;
    U16 *p=vidPtr;

	for(x=GBASCR_SIZEOF;x;x--)
		*p++ = wC;
}
/*****************************************************************************/
void RectFill(int x1, int y1, int x2, int y2, U8 c)
{
	int x,size,diff;
	U16 wC;
    U16 *p=vidPtr+(y1*VID_WIDE+(x1));

    if(y1>SCREEN_HEIGHT)
    	y1=SCREEN_HEIGHT;
    if(y2>SCREEN_HEIGHT)
    	y2=SCREEN_HEIGHT;

	size = (x2-x1);
	diff = VID_WIDE-size;
    wC = B2W(N2B(c,c),N2B(c,c));
	while(y1<y2) {
		for(x=size;x;x--) {
			*p++ = wC;
			GO.lcd.drawPixel(x, y1, wC << 8 | wC);
        }
        y1++;
        p+=diff;
    }
}
/*****************************************************************************/
void RectFillX(int x1, int y1, int x2, int y2, U8 c)
{

	U16 *p;
    int l1,l;
    U16 wC = B2W(c,c);

    if(x2<0||y2<0)
    	return;

    x1+= port.left;
    x2+= port.left;
    y1+= port.top;
    y2+= port.top;
    if(y1>port.bottom||x1>port.right)
    	return;
    if(x1<port.left)
    	x1=port.left;
    if(x2>port.right)
    	x2=port.right;
    if(y1<port.top)
    	y1=port.top;
    if(y2>port.bottom)
    	y2=port.bottom;

	l1 = ((x2-x1+2)>>1);
	p=&vidPtr[(y1*VID_WIDE)+(x1>>1)];
    while(y1<=y2) {
        for(l=l1;l;l--)
            *p++ = wC;
	for (int k= x1; k <= x2; k++)
		GO.lcd.drawPixel(k, y1, wC << 8 | wC);
        p+=VID_WIDE-l1;
    	y1++;
    }
}
/*****************************************************************************/
void RectFillXHl(int x1, int y1, int x2, int y2, U8 c)
{

	U16 *p;
    int l1,l;

    if(x2<0||y2<0)
    	return;

    x1+= port.left;
    x2+= port.left;
    y1+= port.top;
    y2+= port.top;
    if(y1>port.bottom||x1>port.right)
    	return;
    if(x1<port.left)
    	x1=port.left;
    if(x2>port.right)
    	x2=port.right;
    if(y1<port.top)
    	y1=port.top;
    if(y2>port.bottom)
    	y2=port.bottom;

	l1 = ((x2-x1+2)>>1);
	p=&vidPtr[(y1*VID_WIDE)+(x1>>1)];
    while(y1<=y2) {
    	//if(y1&1)
        //	for(l=l1;l;l--)
        //		*p++ |= 0x7878;
        //else
        	for(l=l1;l;l--)
        		*p++ |= 0xF8F8;
	for (int k= x1; k <= x2; k++)
		GO.lcd.drawPixel(k, y1, 0xF8F8);

        p+=VID_WIDE-l1;
    	y1++;
    }
}
/*****************************************************************************/
void TransBox(int x1, int y1, int x2, int y2)
{
	int x,size,diff;
    U16 *p=vidPtr+(y1*VID_WIDE+(x1));

    if(y1>SCREEN_HEIGHT)
    	y1=SCREEN_HEIGHT;
    if(y2>SCREEN_HEIGHT)
    	y2=SCREEN_HEIGHT;

	size = (x2-x1);
	diff = VID_WIDE-size;
	while(y1<y2) {
		for(x=size;x;x--)
			*p++ |= 0xF0F0;
        y1++;
        p+=diff;
    }
}
/*****************************************************************************/
void ShadowBox(int x1, int y1, int x2, int y2)
{
	int x,size,diff;
    U16 *p=vidPtr+(y1*VID_WIDE+(x1));

    if(y1>SCREEN_HEIGHT)
    	y1=SCREEN_HEIGHT;
    if(y2>SCREEN_HEIGHT)
    	y2=SCREEN_HEIGHT;

	size = (x2-x1);
	diff = VID_WIDE-size;
	while(y1<y2) {
		for(x=size;x;x--)
			*p++ &= 0x0F0F;
        y1++;
        p+=diff;
    }
}
/*****************************************************************************/
void BitwiseBox(int x1, int y1, int x2, int y2, U8 yand, U8 yor)
{
	int x,size,diff,o,a;
    U16 *p=vidPtr+(y1*VID_WIDE+(x1));

    if(y1>SCREEN_HEIGHT)
    	y1=SCREEN_HEIGHT;
    if(y2>SCREEN_HEIGHT)
    	y2=SCREEN_HEIGHT;

	size = (x2-x1);
	diff = VID_WIDE-size;
    o = B2W(yor,yor);
    a = B2W(yand,yand);
	while(y1<y2) {
		for(x=size;x;x--) {
			*p   &= a;
			*p++ |= o;
		}
        y1++;
        p+=diff;
    }
}
/******************************************************************************/
void BoxNBorder(int x1, int y1, int x2, int y2, U8 c)
{
#ifdef FANCY_WINDOWS
    gDrawShadow(  x1+2,  y1+2,  x2,  y2  );
    x2-=2;
    y2-=2;
	RectFillX(x1+2,y1+2,x2-2,y2-2, clWINDOW);
		gROutline(  x1,  y1,  x2,  y2, clBORDER,  clBORDER);
		gOutline(++x1,++y1,--x2,--y2, cl3DLIGHT, cl3DSHADOW);

			gOutline(++x1,++y1,--x2,--y2, cl3DLIGHTDK, cl3DSHADOWDK);

#else
	x1>>=1;
	x2>>=1;
	ShadowBox(x1+2,y1+4,x2+1,y2+1);
	RectFill(x1,y1,x2-1,y2-2,c&0xF);

	RectFill(x1+1,y1+1,x1+2,y2-3,c>>4);
	RectFill(x2-3,y1+1,x2-2,y2-3,c>>4);
	RectFill(x1+1,y1+1,x2-3,y1+2,c>>4);
	RectFill(x1+1,y2-4,x2-3,y2-3,c>>4);
#endif
}
/*****************************************************************************/
}

