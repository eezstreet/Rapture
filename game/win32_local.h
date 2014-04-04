#include "sys_local.h"

#define COMMAND_HISTORY 32
#define MAX_EDIT_LINE 256
#define g_console_field_width 80

void Sys_SetErrorText( const char *buf );
enum {
	COPY_ID = 1,
	QUIT_ID,
	CLEAR_ID,

	ERRORBOX_ID = 10,
	ERRORTEXT_ID,

	EDIT_ID = 100,
	INPUT_ID
};

struct field_t {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];
};

typedef struct WinConData_s {
	HWND		hWnd;
	HWND		hwndBuffer;

	HWND		hwndButtonClear;
	HWND		hwndButtonCopy;
	HWND		hwndButtonQuit;

	HWND		hwndErrorBox;
	HWND		hwndErrorText;

	HBITMAP		hbmLogo;
	HBITMAP		hbmClearBitmap;

	HBRUSH		hbrEditBackground;
	HBRUSH		hbrErrorBackground;

	HFONT		hfBufferFont;
	HFONT		hfButtonFont;

	HWND		hwndInputLine;

	char		errorString[80];

	char		consoleText[512], returnedText[512];
	int			visLevel;
	bool		quitOnClose;
	int			windowWidth, windowHeight;

	WNDPROC		SysInputLineWndProc;

	// console
	field_t		g_consoleField;
	int			nextHistoryLine;	// the last line in the history buffer, not masked
	int			historyLine;		// the line being displayed from history buffer will be <= nextHistoryLine
	field_t		historyEditLines[COMMAND_HISTORY];

} WinConData;