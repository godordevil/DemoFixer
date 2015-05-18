/*
===========================================================================
Copyright (C) 2011 GodOrDevil

This file is part of DemoFixer source code.

DemoFixer source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

DemoFixer source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "stdafx.h"
#include "main.h"
#include <iostream>
#include <windows.h>
#include <iomanip>
#include <stdio.h>
#include "version.h"

using namespace std;

extern int serverMessageSequence;
extern HANDLE hConsole; 

int  FS_Write( const void *buffer, int len, FILE	*f ) {
	int		block, remaining;
	int		written;
	byte	*buf;
	int		tries;

	buf = (byte *)buffer;

	remaining = len;
	tries = 0;
	while (remaining) {
		block = remaining;
		written = fwrite (buf, 1, block, f);
		if (written == 0) {
			if (!tries) {
				tries = 1;
			} else {
				cout << "FS_Write: 0 bytes written" << endl;
				return 0;
			}
		}

		if (written == -1) {
			cout << "FS_Write: -1 bytes written" << endl;
			return 0;
		}

		remaining -= written;
		buf += written;
	}
	fflush( f );
	return len;
}

void GetTitleLine()
{	
	//system("cls"); // clear the console... nah
	SetConsoleTextAttribute(hConsole, 12);  
	cout << "\n         DEMO";
	SetConsoleTextAttribute(hConsole, 15);  
	cout << "FIXER ";
	SetConsoleTextAttribute(hConsole, 14); 
	cout << APP_VER;
	SetConsoleTextAttribute(hConsole, 15);
	cout << "\n----------------------------------------------" << endl;

	SetConsoleTextAttribute(hConsole, 14); 
	cout << "       Game: ";
	SetConsoleTextAttribute(hConsole, 7);  
	cout << "Soldier of Fortune 2 v1.00" << endl;

	SetConsoleTextAttribute(hConsole, 14); 
	cout << "  Developer: ";
	SetConsoleTextAttribute(hConsole, 12);  
	cout << "God";
	SetConsoleTextAttribute(hConsole, 15);  
	cout << "Or";
	SetConsoleTextAttribute(hConsole, 12);  
	cout << "Devil" << endl;
	SetConsoleTextAttribute(hConsole, 14); 
	cout << "    Website: ";
	SetConsoleTextAttribute(hConsole, 7);  
	cout << "www.github.com/godordevil";

	SetConsoleTextAttribute(hConsole, 15); 
	cout << "\n----------------------------------------------\n" << endl;	
}
void GetTagLine(char *s)
{
	sprintf(s,"%s%s", TAGLINE, "^,- ^7FRAG^CNET^7.NET");
} 
void WriteMessage ( sizebuf_t *msg, int seq ) 
{
	int			len, swlen;
	int			headerBytes;

	headerBytes = msg->readcount;

	// write the packet sequence
	len = seq;
	swlen = LittleLong( len );
	FS_Write (&swlen, 4, demo.saveFile);

	// skip the packet sequencing information
	len = msg->cursize - headerBytes;
	swlen = LittleLong(len);
	FS_Write (&swlen, 4, demo.saveFile);
	FS_Write ( msg->data + headerBytes, len, demo.saveFile);

}
void WriteConfig ( void )
{
	byte		bufData[MAX_MSGLEN];
	sizebuf_t	buf;
	int			i;
	int			len;
	entityState_t	*ent;
	entityState_t	nullstate;
	char			*s;
	extern int serverMessageSequence;
	extern int serverMessageLen;
	extern int reliableSequence;

	MSG_Init (&buf, bufData, sizeof(bufData));
	MSG_Bitstream(&buf);

	MSG_WriteLong( &buf, reliableSequence ); //needed
	MSG_WriteByte (&buf, svc_gamestate);
	MSG_WriteLong (&buf, ds.firstserverCommandSequence ); //needed

	// configstrings
	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) 
	{
		if(i == 67)
		{
			GetTagLine(s);	
		}
		else
		{	
			if ( !ds.gameState.stringOffsets[i] )
				continue;
			s = ds.gameState.stringData + ds.gameState.stringOffsets[i];
		}
		MSG_WriteByte (&buf, svc_configstring);
		MSG_WriteShort (&buf, i);
		MSG_WriteBigString (&buf, s);
	}

	// baselines
	Com_Memset (&nullstate, 0, sizeof(nullstate));
	for ( i = 0; i < MAX_GENTITIES ; i++ )
	{
		ent = &ds.entityBaselines[i];
		if ( !ent->number ) 
			continue;
		MSG_WriteByte (&buf, svc_baseline);		
		MSG_WriteDeltaEntity (&buf, &nullstate, ent, qtrue );
	}
	MSG_WriteByte( &buf, svc_EOF );
	
	// write the client num
	MSG_WriteLong(&buf, ds.clientNum);
	MSG_WriteLong(&buf, ds.checksumFeed );
	
#if _GOLD
	MSG_WriteShort(&buf, 0 );
#endif
	// no clue what this really is, but it makes 1.00 work (required for gold aswell)
	MSG_WriteLong(&buf, 0 );

	// finished writing the client packet
	MSG_WriteByte( &buf, svc_EOF );

	// write it to the demo file
	len = LittleLong( serverMessageSequence );
	FS_Write (&len, 4, demo.saveFile);

	len = LittleLong (buf.cursize);
	FS_Write (&len, 4, demo.saveFile);	
	FS_Write (buf.data, buf.cursize, demo.saveFile);	
}
