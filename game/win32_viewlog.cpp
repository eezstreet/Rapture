#include "win32_local.h"
#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>

Viewlog* viewlog = NULL;

static void Field_Clear(field_t *in) {
	in->cursor = 0;
	in->buffer[0] = 0;
	in->scroll = 0;
	in->widthInChars = 0;
}

HBRUSH		hbrEditBackground;
HBRUSH		hbrErrorBackground;

HWND		hWND;
HWND		hwndBuffer;

HWND		hwndButtonClear;
HWND		hwndButtonCopy;
HWND		hwndButtonQuit;

HWND		hwndErrorBox;
HWND		hwndErrorText;

HWND		hwndInputLine;

WNDPROC		SysInputLineWndProc;

static LRESULT CALLBACK ConWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	const char *cmdString;
	static bool s_timePolarity;

	switch (uMsg)
	{
	case WM_ACTIVATE:
		if ( LOWORD( wParam ) != WA_INACTIVE )
		{
			SetFocus( hwndInputLine );
		}
		break;

	case WM_CLOSE:
		if ( 0 )
		{
			setGameQuitting(true);
		}
		else
		{
			viewlog->Hide();
		}
		return 0;
	case WM_CTLCOLORSTATIC:
		if ( ( HWND ) lParam == hwndBuffer )
		{
			SetBkColor( ( HDC ) wParam, RGB( 0, 0, 0 ) );
			SetTextColor( ( HDC ) wParam, RGB( 160, 020, 020 ) );
			return ( long ) hbrEditBackground;
		}
		else if ( ( HWND ) lParam == hwndErrorBox )
		{
			if ( s_timePolarity & 1 )
			{
				SetBkColor(   ( HDC ) wParam, RGB( 0x80, 0x80, 0x80 ) );
				SetTextColor( ( HDC ) wParam, RGB( 0xff, 0x00, 0x00 ) );
			}
			else
			{
				SetBkColor(   ( HDC ) wParam, RGB( 0x80, 0x80, 0x80 ) );
				SetTextColor( ( HDC ) wParam, RGB( 0x00, 0x00, 0x00 ) );
			}
			return ( long ) hbrErrorBackground;
		}
		return FALSE;
		break;

	case WM_COMMAND:
		if ( wParam == Win32Viewlog::COPY_ID )
		{
			SendMessage( hwndBuffer, EM_SETSEL, 0, -1 );
			SendMessage( hwndBuffer, WM_COPY, 0, 0 );
		}
		else if ( Win32Viewlog::QUIT_ID )
		{
			if ( 1 )
			{
				PostQuitMessage( 0 );
			}
			else
			{
				setGameQuitting(true);
			}
		}
		else if ( wParam == Win32Viewlog::CLEAR_ID )
		{
			SendMessage( hwndBuffer, EM_SETSEL, 0, -1 );
			SendMessage( hwndBuffer, EM_REPLACESEL, FALSE, ( LPARAM ) "" );
			UpdateWindow( hwndBuffer );
		}
		break;
	case WM_CREATE:
		hbrEditBackground =  CreateSolidBrush( RGB( 0x00, 0x00, 0x00 ) );
		hbrErrorBackground = CreateSolidBrush( RGB( 0x80, 0x80, 0x80 ) );
		SetTimer( hWnd, 1, 1000, NULL );
		break;
	case WM_ERASEBKGND:
	    return DefWindowProc( hWnd, uMsg, wParam, lParam );
	case WM_TIMER:
		if ( wParam == 1 )
		{
			s_timePolarity = (bool)!s_timePolarity;
			if ( hwndErrorBox )
			{
				InvalidateRect( hwndErrorBox, NULL, FALSE );
			}
		}
		break;
    }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

static LRESULT CALLBACK InputLineWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	Win32Viewlog* wView = (Win32Viewlog*)viewlog;
	switch ( uMsg )
	{
		case WM_KILLFOCUS:
			if ( ( HWND ) wParam == hWND ||
				( HWND ) wParam == hwndErrorBox )
			{
				SetFocus( hWnd );
				return 0;
			}
			break;

		case WM_CHAR:
			GetWindowText( hwndInputLine, wView->g_consoleField.buffer, sizeof( wView->g_consoleField.buffer ) );
			SendMessage( hwndInputLine, EM_GETSEL, (WPARAM) NULL, (LPARAM) &wView->g_consoleField.cursor );

			if ( wParam == VK_RETURN )
			{
				strncat( wView->consoleText, wView->g_consoleField.buffer, sizeof( wView->consoleText ) - strlen( wView->consoleText ) - 5 );
				strcat( wView->consoleText, "\n" );
				SetWindowText( hwndInputLine, "" );

				R_Printf("]%s\n", wView->g_consoleField.buffer);

				// empty lines just scroll the console without adding to history
				if ( !wView->g_consoleField.buffer[0] )
					return 0;

				// copy line to history buffer
				wView->historyEditLines[wView->nextHistoryLine % COMMAND_HISTORY] = wView->g_consoleField;
				wView->nextHistoryLine++;
				wView->historyLine = wView->nextHistoryLine;
				Field_Clear( &wView->g_consoleField );
				wView->g_consoleField.widthInChars = g_console_field_width;

				return 0;
			}
		break;
		case WM_KEYDOWN:
			// history scrolling
			if ( wParam == VK_UP )
			{// scroll up: arrow-up
				if ( wView->nextHistoryLine - wView->historyLine < COMMAND_HISTORY && wView->historyLine > 0 )
					wView->historyLine--;
				wView->g_consoleField = wView->historyEditLines[wView->historyLine % COMMAND_HISTORY];
				SetWindowText( hwndInputLine, wView->g_consoleField.buffer );
				SendMessage( hwndInputLine, EM_SETSEL, wView->g_consoleField.cursor, wView->g_consoleField.cursor );

				return 0;
			}

			if ( wParam == VK_DOWN )
			{// scroll down: arrow-down
				wView->historyLine++;
				if (wView->historyLine >= wView->nextHistoryLine) {
					wView->historyLine = wView->nextHistoryLine;
					Field_Clear( &wView->g_consoleField );
					wView->g_consoleField.widthInChars = g_console_field_width;
					SetWindowText( hwndInputLine, wView->g_consoleField.buffer );
					SendMessage( hwndInputLine, EM_SETSEL, wView->g_consoleField.cursor, wView->g_consoleField.cursor );
					return 0;
				}
				wView->g_consoleField = wView->historyEditLines[wView->historyLine % COMMAND_HISTORY];
				SetWindowText( hwndInputLine, wView->g_consoleField.buffer );
				SendMessage( hwndInputLine, EM_SETSEL, wView->g_consoleField.cursor, wView->g_consoleField.cursor );

				return 0;
			}
			break;
	}
	return CallWindowProc( SysInputLineWndProc, hWnd, uMsg, wParam, lParam );
}

//////////////////////////////
// Win32Viewlog class
//////////////////////////////

/*
** Sys_CreateConsole
*/
Win32Viewlog::Win32Viewlog()
{
	HDC hDC;
	WNDCLASS wc;
	RECT rect;
	int nHeight;
	int swidth, sheight;
	int DEDSTYLE = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX;
	
	memset( &wc, 0, sizeof( wc ) );

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC) ConWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = NULL;
	//wc.hIcon         = LoadIcon( g_wv.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH__ *)COLOR_INACTIVEBORDER;
	wc.lpszMenuName  = 0;
	wc.lpszClassName = "Rapture Viewlog";

	if ( !RegisterClass (&wc) )	{
		return;
	}

	rect.left = 0;
	rect.right = 600;
	rect.top = 0;
	rect.bottom = 450;
	AdjustWindowRect( &rect, DEDSTYLE, FALSE );

	hDC = GetDC( GetDesktopWindow() );
	swidth = GetDeviceCaps( hDC, HORZRES );
	sheight = GetDeviceCaps( hDC, VERTRES );
	ReleaseDC( GetDesktopWindow(), hDC );

	windowWidth = rect.right - rect.left + 1;
	windowHeight = rect.bottom - rect.top + 1;

	hWND = CreateWindowEx( 0,
							   "Rapture Viewlog",
							   "Rapture",
							   DEDSTYLE,
							   ( swidth - 600 ) / 2, ( sheight - 450 ) / 2 , rect.right - rect.left + 1, rect.bottom - rect.top + 1,
							   NULL,
							   NULL,
							   NULL,
							   NULL );

	if ( hWND == NULL )
	{
		return;
	}

	//
	// create fonts
	//
	hDC = GetDC( hWND );
	nHeight = -MulDiv( 8, GetDeviceCaps( hDC, LOGPIXELSY), 72);

	hfBufferFont = CreateFont( nHeight,
									  0,
									  0,
									  0,
									  FW_LIGHT,
									  0,
									  0,
									  0,
									  DEFAULT_CHARSET,
									  OUT_DEFAULT_PRECIS,
									  CLIP_DEFAULT_PRECIS,
									  DEFAULT_QUALITY,
									  FF_MODERN | FIXED_PITCH,
									  "Courier New" );

	ReleaseDC( hWND, hDC );

	//
	// create the input line
	//
	hwndInputLine = CreateWindow( "edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
												ES_LEFT | ES_AUTOHSCROLL | WS_TABSTOP,
												6, 400, windowWidth-20, 20,
												hWND,
												( HMENU ) INPUT_ID,	// child window ID
												NULL, NULL );

	//
	// create the buttons
	//
	hwndButtonCopy = CreateWindow( "button", "Copy", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
												5, 425, 72, 24,
												hWND,
												( HMENU ) COPY_ID,	// child window ID
												NULL, NULL );

	hwndButtonClear = CreateWindow( "button", "Clear", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
												82, 425, 72, 24,
												hWND,
												( HMENU ) CLEAR_ID,	// child window ID
												NULL, NULL );

	hwndButtonQuit = CreateWindow( "button", "Quit", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
												windowWidth-92, 425, 72, 24,
												hWND,
												( HMENU ) QUIT_ID,	// child window ID
												NULL, NULL );


	//
	// create the scrollbuffer
	//
	hwndBuffer = CreateWindow( "edit", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
												ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_TABSTOP,
												6, 40, windowWidth-20, 354,
												hWND,
												( HMENU ) EDIT_ID,	// child window ID
												NULL, NULL );
	SendMessage( hwndBuffer, WM_SETFONT, ( WPARAM ) hfBufferFont, 0 );

	SysInputLineWndProc = ( WNDPROC ) SetWindowLongPtr( hwndInputLine, GWLP_WNDPROC, ( long ) InputLineWndProc );
	SendMessage( hwndInputLine, WM_SETFONT, ( WPARAM ) hfBufferFont, 0 );
	SendMessage( hwndBuffer, EM_LIMITTEXT, ( WPARAM ) 0x7fff, 0 );

	ShowWindow( hWND, SW_SHOWDEFAULT);
	UpdateWindow( hWND );
	SetForegroundWindow( hWND );
	SetFocus( hwndInputLine );

	Field_Clear( &g_consoleField );
	g_consoleField.widthInChars = g_console_field_width;
	for ( int i = 0 ; i < COMMAND_HISTORY ; i++ ) {
		Field_Clear( &historyEditLines[i] );
		historyEditLines[i].widthInChars = g_console_field_width;
	}
}

/*
** Sys_DestroyConsole
*/
Win32Viewlog::~Win32Viewlog()
{
	if ( hWND )
	{
		DeleteObject(hbrEditBackground);
		DeleteObject(hbrErrorBackground);
		DeleteObject(hfBufferFont);

		ShowWindow( hWND, SW_HIDE );
		CloseWindow( hWND );
		DestroyWindow( hWND );
		hWND = 0;
	}
}

void Win32Viewlog::Show() {
	ShowWindow( hWND, SW_SHOWNORMAL );
	SendMessage( hwndBuffer, EM_LINESCROLL, 0, 0xffff );
}

void Win32Viewlog::Hide() {
	ShowWindow( hWND, SW_HIDE );
}

void Win32Viewlog::SetErrorText(const string& text) {
	strncpy(errorString, text.c_str(), sizeof(errorString));

	if ( !hwndErrorBox )
	{
		hwndErrorBox = CreateWindow( "static", NULL, WS_CHILD | WS_VISIBLE | SS_SUNKEN,
													6, 5, windowWidth-20, 30,
													hWND,
													( HMENU ) ERRORBOX_ID,	// child window ID
													NULL, NULL );
		SendMessage( hwndErrorBox, WM_SETFONT, ( WPARAM ) hfBufferFont, 0 );
		SetWindowText( hwndErrorBox, errorString );

		DestroyWindow( hwndInputLine );
		hwndInputLine = NULL;
	}
};

void Win32Viewlog::AppendText(const string& text) {
	#define CONSOLE_BUFFER_SIZE		16384
	if ( !hWND ) {
		return;
	}
	char buffer[CONSOLE_BUFFER_SIZE*4];
	char *b = buffer;
	const char *msg;
	int bufLen;
	int i = 0;
	static unsigned long s_totalChars;

	//
	// if the message is REALLY long, use just the last portion of it
	//
	if ( text.length() > CONSOLE_BUFFER_SIZE - 1 )
	{
		msg = text.c_str() + text.length() - CONSOLE_BUFFER_SIZE + 1;
	}
	else
	{
		msg = text.c_str();
	}

	//
	// copy into an intermediate buffer
	//
	while ( msg[i] && ( ( b - buffer ) < sizeof( buffer ) - 1 ) )
	{
		if ( msg[i] == '\n' && msg[i+1] == '\r' )
		{
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
			i++;
		}
		else if ( msg[i] == '\r' )
		{
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		}
		else if ( msg[i] == '\n' )
		{
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		}
		else
		{
			*b= msg[i];
			b++;
		}
		i++;
	}
	*b = 0;
	bufLen = b - buffer;

	s_totalChars += bufLen;

	//
	// replace selection instead of appending if we're overflowing
	//
	if ( s_totalChars > 0x7fff )
	{
		SendMessage( hwndBuffer, EM_SETSEL, 0, -1 );
		s_totalChars = bufLen;
	}

	//
	// put this text into the windows console
	//
	SendMessage( hwndBuffer, EM_LINESCROLL, 0, 0xffff );
	SendMessage( hwndBuffer, EM_SCROLLCARET, 0, 0 );
	SendMessage( hwndBuffer, EM_REPLACESEL, 0, (LPARAM) buffer );
};

void Win32Viewlog::TestViewlogShow() {
	cvViewlog = Cvar::Get<bool>("viewlog", "Displays the viewlog during normal ingame use", Cvar::CVAR_ARCHIVE, false);
	if(!cvViewlog->Bool()) {
		Hide();
	}
};

/*
** Sys_PassToViewlog
*/
void Sys_PassToViewlog(const char* text) {
	OutputDebugString(text);
	Win32Viewlog* wView = (Win32Viewlog*)viewlog;
	wView->AppendText(text);
}

/*
** Sys_InitViewlog
*/
void Sys_InitViewlog() {
	ShowWindow( GetConsoleWindow(), SW_HIDE );
	viewlog = new Win32Viewlog();
	viewlog->Show();
}