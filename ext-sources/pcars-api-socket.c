Skip to content
Sign up Sign in
This repository  
Explore
Features
Enterprise
Blog
 Watch 1  Star 0  Fork 2 mdalda/pcars-api-socket
 branch: master  pcars-api-socket/main.cpp
mdalda on 8 Dec 2014 Merge branch 'master' of https://github.com/mdalda/pcars-api-socket
1 contributor
RawBlameHistory     384 lines (318 sloc)  9.508 kB
/*
 * pCARS API Socket
 * Copyright (C) 2014 - Manuel Dalda Calle
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 *
 */


#include <windows.h>
#include <process.h>
#include <conio.h>
#include <winsock.h>
#include <stdio.h>
#include <string>
#include "resource.h"
#include "sharedmemory.h"

#pragma comment(lib, "ws2_32.lib")

const double version = 0.1; // Useless for now
const int port = 23614;
SharedMemory* sharedData;

UINT WM_TASKBARCREATED = 0 ;
NOTIFYICONDATA g_notifyIconData ;

HWND g_hwnd ;
HMENU g_menu ;
HANDLE hFileHandle = 0;
BOOL close_app = FALSE;
SOCKET s;
WSADATA w;

#define ID_TIMER_MEMORY_MAPPED			2000
#define ID_TRAY_APP_ICON                5000
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM  3000
#define WM_TRAYICON ( WM_USER + 1 )

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
void InitNotifyIconData();

#define MAP_OBJECT_NAME "$pcars$"

unsigned __stdcall ClientSession(void *data)
{
	SOCKET s = (SOCKET)data;
	int bytesSent;
	int bytesRecv = SOCKET_ERROR;
	char sendbuf[4096]="";
	char recvbuf[4]="";

	sprintf_s(sendbuf, "pCARS API Server v%d\r\n", version);
	bytesSent = send( s, sendbuf, strlen(sendbuf), 0);

	if (bytesSent == SOCKET_ERROR)
	{
		goto goout;
	}

	while (1)
	{
		int optVal = 0;
		int optLen = sizeof(int);		

		ZeroMemory (recvbuf, sizeof(recvbuf));
		bytesRecv = recv( s, recvbuf,  32, 0);
		if (strncmp(recvbuf, "json", 4) == 0) {
			if (sharedData == NULL) {
				sprintf_s(sendbuf, 
					"{\r\n"					
					"    \"pCARS\": {\r\n"
					"        \"State\": 0\r\n"
					"    }\r\n"
					"}\r\n"
					, 0 );

			} else {

				if (sharedData->mRaceState != RACESTATE_INVALID) {
					sprintf_s(sendbuf, 
						"{\r\n"					
						"    \"pCARS\": {\r\n"
						"        \"State\": %d,\r\n"
						"            \"Me\": {\r\n"
						"                \"OilTempCelsius\" : %f,\r\n"
						"                \"OilPressureKPa\" : %f\r\n"
						"                \"WaterTempCelsius\" : %f\r\n"
						"                \"WaterPressureKPa\" : %f\r\n"
						"                \"FuelPressureKPa\" : %f\r\n"
						"                \"FuelLevel\" : %f\r\n"
						"                \"FuelCapacity\" : %f\r\n"
						"                \"Speed\" : %f\r\n"
						"                \"Rpm\" : %f\r\n"
						"                \"MaxRPM\" : %f\r\n"
						"                \"Brake\" : %f\r\n"
						"                \"Throttle\" : %f\r\n"
						"                \"Clutch\" : %f\r\n"
						"                \"Steering\" : %f\r\n"
						"                \"Gear\" : %i\r\n"
						"                \"NumGears\" : %i\r\n"
						"            }\r\n"
						"    }\r\n"
						"}\r\n"
						, 
						sharedData->mRaceState,
						sharedData->mOilTempCelsius,
						sharedData->mOilPressureKPa,
						sharedData->mWaterTempCelsius,
						sharedData->mWaterPressureKPa,
						sharedData->mFuelPressureKPa,
						sharedData->mFuelLevel,
						sharedData->mFuelCapacity,
						sharedData->mSpeed,
						sharedData->mRpm,
						sharedData->mMaxRPM,
						sharedData->mBrake,
						sharedData->mThrottle,
						sharedData->mClutch,
						sharedData->mSteering,
						sharedData->mGear,
						sharedData->mNumGears 
						);

				} else {
					sprintf_s(sendbuf, 
						"{\r\n"					
						"    \"pCARS\": {\r\n"
						"        \"State\": 0\r\n"
						"    }\r\n"
						"}\r\n"
						, 0 );				
				}

			}

			bytesSent = send( s, sendbuf, strlen(sendbuf), 0);

			if (bytesSent == SOCKET_ERROR)
			{
				goto goout;
			}
		}
		if (bytesRecv == SOCKET_ERROR) {
			goto goout; // Probably client disconnects
		}
		Sleep(50);
	}

goout:
	closesocket(s);
	return 0;
}

unsigned __stdcall ListenOnPort(void *)
{
	int iError = WSAStartup (0x0202, &w);

	if (iError)
	{
		char e[100];
		sprintf_s(e, sizeof(e), "Port %i in use by other application", port);
		MessageBox(NULL, e, "Error", 16);
		exit(0);
		return false; //Error opening port
	}

	if (w.wVersion != 0x0202)
	{
		WSACleanup ();
		MessageBox(NULL, "How do you can run pCARS in this Windows Version?", "Error", 16);
		exit(0);
		return false; // Don't have Winsock2?
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	addr.sin_addr.s_addr = htonl (INADDR_ANY);

	s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (s == INVALID_SOCKET)
	{
		MessageBox(NULL, "Unknown error creating the port listener", "Error", 16);
		exit(0);
		return false; // Unknown error opening socket
	}

	if (bind(s, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		MessageBox(NULL, "Error binding the port listener", "Error", 16);
		exit(0);
		return false; // Binding error
	}

	listen(s, SOMAXCONN);

	SOCKET client_s;
	while ((client_s = accept(s, NULL, NULL))) {
		unsigned threadID;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &ClientSession, (void*)client_s, 0, &threadID); // New thread for new socket
	}

	return 0;
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char*, int nShowCmd)
{
	unsigned threadID;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &ListenOnPort, 0, 0, &threadID); // Listener socket in other thread

	TCHAR className[] = TEXT( "tray icon class" );
	WM_TASKBARCREATED = RegisterWindowMessageA("TaskbarCreated") ;

#pragma region get window up
	WNDCLASSEX wnd = { 0 };

	wnd.hInstance = hInstance;
	wnd.lpszClassName = className;
	wnd.lpfnWndProc = WndProc;
	wnd.style = CS_HREDRAW | CS_VREDRAW ;
	wnd.cbSize = sizeof (WNDCLASSEX);

	wnd.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wnd.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	wnd.hCursor = LoadCursor (NULL, IDC_ARROW);
	wnd.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE ;

	if (!RegisterClassEx(&wnd))
	{
		FatalAppExit( 0, TEXT("Couldn't register window class!") );
	}

	g_hwnd = CreateWindowEx (0, className, TEXT( "pCARS API Socket" ),WS_OVERLAPPEDWINDOW,CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, NULL, NULL, hInstance, NULL);





	// Create System Tray Icon
	LoadIcon (GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));

	memset( &g_notifyIconData, 0, sizeof( NOTIFYICONDATA ) ) ;
	g_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	g_notifyIconData.hWnd = g_hwnd;
	g_notifyIconData.uID = ID_TRAY_APP_ICON;
	g_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE  | NIF_TIP;
	g_notifyIconData.uCallbackMessage = WM_TRAYICON;
	g_notifyIconData.hIcon = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32 ,0);
	strcpy_s(g_notifyIconData.szTip, sizeof(g_notifyIconData.szTip), TEXT("pCARS API Socket"));

	//ShowWindow (g_hwnd, nShowCmd); // Debug or Settings
	Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);

	SetTimer(g_hwnd, ID_TIMER_MEMORY_MAPPED, 1000, (TIMERPROC) NULL);  // Timer to open the memory-mapped file

	MSG msg ;
	while (GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if( !IsWindowVisible( g_hwnd ) )
	{
		Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);
	}

	return msg.wParam;

}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( message==WM_TASKBARCREATED && !IsWindowVisible( g_hwnd ) )
	{
		return 0;
	}

	switch (message)
	{
	case WM_CREATE:

		g_menu = CreatePopupMenu();
		AppendMenu(g_menu, MF_STRING, ID_TRAY_EXIT_CONTEXT_MENU_ITEM,  TEXT( "Exit" ) );

		break;
	case WM_TIMER:

		switch(wParam)
		{

		case ID_TIMER_MEMORY_MAPPED:

			if (hFileHandle == NULL) {
				// Open the memory-mapped file
				hFileHandle = OpenFileMapping( PAGE_READONLY, FALSE, MAP_OBJECT_NAME );
				// Get the data structure
				sharedData = (SharedMemory*)MapViewOfFile( hFileHandle, PAGE_READONLY, 0, 0, sizeof(SharedMemory) );
				if (sharedData == NULL)
				{
					CloseHandle( hFileHandle );
					return 1;
				}

				// Ensure we're sync'd to the correct data version
				if ( sharedData->mVersion != SHARED_MEMORY_VERSION )
				{
					MessageBox(NULL, "Data version mismatch", "Error", 16);
					exit(0);

					return 1;
				}
			}
			break;
		}

		break;

	case WM_SYSCOMMAND:
		switch( wParam & 0xfff0 )
		{
		case SC_MINIMIZE:
		case SC_CLOSE:
			return 0 ;
			break;
		}
		break;

	case WM_TRAYICON:
		{
			switch(wParam)
			{
			case ID_TRAY_APP_ICON:
				break;
			}

			if ((lParam == WM_RBUTTONDOWN) || (lParam == WM_LBUTTONUP))
			{
				POINT curPoint ;
				GetCursorPos( &curPoint ) ;
				SetForegroundWindow(hwnd);
				UINT clicked = TrackPopupMenu(g_menu,TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x,curPoint.y,0,hwnd,NULL);
				if (clicked == ID_TRAY_EXIT_CONTEXT_MENU_ITEM)
				{
					PostQuitMessage( 0 ) ;
				}
			}
		}
		break;

	case WM_NCHITTEST:
		{
			UINT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if(uHitTest == HTCLIENT)
				return HTCAPTION;
			else
				return uHitTest;
		}

	case WM_CLOSE:
		UnmapViewOfFile( sharedData );
		CloseHandle( hFileHandle );
		return 0;
		break;

	case WM_DESTROY:
		PostQuitMessage (0);
		break;
	}
	return DefWindowProc( hwnd, message, wParam, lParam ) ;
}
Status API Training Shop Blog About
© 2015 GitHub, Inc. Terms Privacy Security Contact