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
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define PIN_BLUE_LED 2

extern "C" {
#include "gbagi.h"
#include "gamedata.h"
#include "text.h"
#include "errmsg.h"
#include "menu.h"
#include "system.h"
#include "picture.h"
#include "screen.h"  
#include "commands.h"  
#include "esp_attr.h"
#include "esp_partition.h"
#include "esp_heap_caps.h"
#include "esp_ota_ops.h"
/*****************************************************************************/
File tmpdir;
char fname[255];
U8 *buf;
U8 *volData;

	// *"dir"
U8 **viewDir;
U8 **picDir;
U8 **logDir;
U8 **sndDir;

	// "object"
char **objNames;
char *objNameData;	// the raw data pointed to by "objNames[256]"
U8 *objRoomsStart;

	// "words.tok"
char **words;		// a-z pointers
char *wordData;

U8 *wordFlags,**logWords;
GAMEENT *GameEnts;
//U32 totalWords;
int totalGames;

void *BASEx0X;
void *BASE80X;
/*****************************************************************************/
char s[100];
#define IDSIZE	24
const char agiid[]="GBAGI 1.0 '''BRIPRO'''\0";  // format still compatible with 1.0, so no reason to change the header

VERTYPE AGIVER;

/*****************************************************************************/
S16 wnGameSelectProc(WND *w, U16 msg, U16 wParam, U32 lParam);

WND wnGameSelect = {
	NULL,NULL,NULL,NULL,
	88,2,152,146,
	{0,0,0,0},{0,0,0,0},
	"Game Select",
	0,
	0,
	wsRESIZABLE|wsTITLE|wsSELECTABLE,
	(WNPROC)wnGameSelectProc
};
WND lbGames = {
	NULL,NULL,&wnGameSelect,NULL,
	0,0,142,106,
	{0,0,0,0},{0,0,0,0},
	"Items",
	wnLISTBOX,
	0,
	wsSELECTABLE|wsEXTFIXED,
	(WNPROC)wnGameSelectProc
};
WND sbGames;

WND bnAbout = {
	NULL,NULL,&wnGameSelect,NULL,
	26,108,40,16,
	{0,0,0,0},{0,0,0,0},
	"About",
	wnBUTTON,
	0,
	wsSELECTABLE|bsCAPTION,
	(WNPROC)wnGameSelectProc
};
WND bnSelectGame = {
	NULL,NULL,&wnGameSelect,NULL,
	66,108,76,16,
	{0,0,0,0},{0,0,0,0},
	"Select Game",
	wnBUTTON,
	0,
	wsSELECTABLE|bsCAPTION,
	(WNPROC)wnGameSelectProc
};

U32 *b;

/*****************************************************************************/
#define ROM_PATH "/roms/gbagi"

char files[10][255];
void ExecuteGameSelectDialog(void);
void ExecuteGameSelectDialog()
{
	int i;
    AddWindow(&wnGameSelect);
    AddWindow(&bnAbout);
    AddWindow(&bnSelectGame);

	WndStopUpdate(&lbGames);
    ListBoxClear(&lbGames);
    AddWindow(&lbGames);
	File result_entry;
	if ((tmpdir = SD.open(ROM_PATH, FILE_READ)) != 0) {
    	digitalWrite(PIN_BLUE_LED, HIGH);
		i = 0;
		while ((result_entry = tmpdir.openNextFile()) != 0) {
//			fprintf(stderr, "file %s\n", result_entry.name());
			snprintf(files[i], 255, "%s", result_entry.name() + strlen(ROM_PATH) + 1);
			ListBoxAdd(&lbGames,(char *)&files[i]);
			i++;
		}
    	digitalWrite(PIN_BLUE_LED, LOW);
		tmpdir.close();
	} else {
		fprintf(stderr, "directory open failed!!!\n");
		abort();
	}

	ListBoxSetScrollbar(&lbGames,&sbGames);
	ListBoxSelect(&lbGames,0);
	WndStartUpdate(&lbGames);

	WinGUIDoit();
}
/*****************************************************************************/
S16 wnGameSelectProc(WND *w, U16 msg, U16 wParam, U32 lParam)
{
	U8 *p;
	switch(msg) {
    	case wmBUTTON_CLICK:
        	if(w==&bnAbout) {
                cVersion();
                RedrawAllWindows();
                break;
            }
    	case wmLISTBOX_CLICK:
        	if(wParam==KEY_ENTER||wParam==KEY_START) {
	           	snprintf(fname, sizeof(fname), "%s/%s", 
			    ROM_PATH, (char*)((lbGames.ext.listbox.itemActive->text)));
			fprintf(stderr, "file: %s\n",fname);
             	WndDispose(&wnGameSelect);
        	}
            break;
    }
	return TRUE;
}
/*****************************************************************************/
BOOL GameDataInit()
{
	ready_to_run = FALSE;
    	pinMode(PIN_BLUE_LED, OUTPUT);
    int tw,i;
	U32 *p,*pend;
#ifdef _WINDOWS
    LISTITEM *li;
	FILE *f=fopen("E:\\agigames.gba"//myagigames.gba"//gbagi081beta-sq2.gba"//
    	,"rb");
    long l;
    U8 *buf;
    fseek(f,0,SEEK_END);
    l = ftell(f)-BASEx0X;
    fseek(f,BASEx0X,SEEK_SET);
    buf = (U8*)malloc(l);
    fread(buf,l,1,f);
    SD.close(f);
	#define _BASE (buf+0x20)
    #define bb(x) (buf+(b[x]-BASE80X))
    #define bn(x) (((U8*)b-0x20)[x])
    if(strcmp((char*)buf,agiid)!=0)
    	ErrorMessage(0,"Game data invalid! Signature check failed!");
#else
    #define bb(x) (b)[x]
    #define bn(x) (((U8*)buf)[x])
    #if 0
    const esp_partition_t* partition = esp_partition_find_first(
        66, 6, NULL);
    if (partition == NULL)
		abort();
    spi_flash_mmap_handle_t mhand;
    esp_partition_mmap(partition, 0, partition->size, SPI_FLASH_MMAP_DATA , &BASE80X, &mhand);
    #endif
#endif

    totalGames = 1;
    if(1) {
    	ClearControllers();
        cStatusLineOff();
        ShowPic();
    	ExecuteGameSelectDialog();
    }
	if ((tmpdir = SD.open(fname, FILE_READ)) != 0) {
    	digitalWrite(PIN_BLUE_LED, HIGH);
		int mysize = tmpdir.size();
		if (mysize == 0) {
    	digitalWrite(PIN_BLUE_LED, LOW);
			tmpdir.close();
			abort();
		}
		tmpdir.seek(0, SeekSet);
    buf = (U8*)heap_caps_malloc(mysize, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    int i = 0;
    while (i < mysize) {
    		buf[i++] = tmpdir.read();
	}
    fprintf(stderr, "i %d mysize %d %s\n", i, mysize,  buf);
    	digitalWrite(PIN_BLUE_LED, LOW);
    }
	#define _BASE (buf + 0x20)
    if(strcmp((char*)buf,agiid)!=0) return FALSE;
	b=(U32*)_BASE;


    GameEnts = (GAMEENT*)(b);


#if 0
    GameEnts->volData	= (U8*)(buf+((U32)GameEnts->volData));
    GameEnts->viewDir	= (U8**)(buf+((U32)GameEnts->viewDir));
    GameEnts->picDir	= (U8**)(buf+((U32)GameEnts->picDir));
    GameEnts->logDir	= (U8**)(buf+((U32)GameEnts->logDir));
    GameEnts->sndDir	= (U8**)(buf+((U32)GameEnts->sndDir));
    GameEnts->objNameData	= (char*)(buf+((U32)GameEnts->objNameData));
    GameEnts->objNames	= (char**)(buf+((U32)GameEnts->objNames));
    GameEnts->objRoomsStart	= (U8*)(buf+((U32)GameEnts->objRoomsStart));
    GameEnts->wordData	= (char*)(buf+((U32)GameEnts->wordData));
    GameEnts->words		= (char**)(buf+((U32)GameEnts->words));
    GameEnts->wordFlags = (char*)(buf+((U32)GameEnts->wordFlags));
    GameEnts->logWords	= (U8**)(buf+((U32)GameEnts->logWords));
#endif

    AGIVER.major = GameEnts->hdr[3];
    AGIVER.minor = bGetW(GameEnts->hdr+4);

    volData = (U8 *)((U32)GameEnts->volData + (U32)buf);
//    fprintf(stderr, "BASE80X: %x GM %x VOLDATA: %x VD0: %x\n",buf, GameEnts->volData, volData, volData[0]);
    viewDir = (U8 **)((U32)GameEnts->viewDir + (U32)buf);
    picDir = (U8 **)((U32)GameEnts->picDir + (U32)buf);
    logDir = (U8 **)((U32)GameEnts->logDir + (U32)buf);
    sndDir = (U8 **)((U32)GameEnts->sndDir + (U32)buf);
    objNameData = (char *)((U32)GameEnts->objNameData + (U32)buf);
    objNames = (char **)((U32)GameEnts->objNames + (U32)buf);
    objRoomsStart = (U8 *)((U32)GameEnts->objRoomsStart + (U32)buf);
    wordData = (char *)((U32)GameEnts->wordData + (U32)buf);
    words = (char **)((U32)GameEnts->words + (U32)buf);
    wordFlags = (U8 *)((U32)GameEnts->wordFlags + (U32)buf);
    logWords = (U8 **)((U32)GameEnts->logWords + (U32)buf);

#if 1
    p = (U32*)logDir;
    pend = (U32*)objNameData;
    while(p<pend) {
   		if(*p)*p = (U32)( (buf)+ (((U32)*p)) );
        p++;
    }
    p = (U32*)objNames;
    pend = (U32*)objRoomsStart-3;
    while(p<pend) {
    	if(*p)*p = (U32)(buf+((*p)));
        p++;
    }
    p = (U32*)words;
    pend = (U32*)wordFlags;
    while(p<pend) {
   		if(*p)
        	*p = (U32)( (buf)+ (((U32)*p)) );
        p++;
    }
    p = (U32*)logWords;
    tw = 256;
    while(tw) {
    	if(*p)*p = (U32)(buf+((*p)));
        p++;
        tw--;
    }
//    fprintf(stderr, "BASE80X: %x VOLDATA: %x VD0: %x\n",buf, volData, volData[0]);
#endif
	return TRUE;
}
}
/*****************************************************************************/
