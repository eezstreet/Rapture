#include "sys_local.h"

#define COMMAND_HISTORY 32
#define MAX_EDIT_LINE 256
#define g_console_field_width 80

struct field_t {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];
};

class Win32Viewlog : public Viewlog {
public:
	Win32Viewlog();
	~Win32Viewlog();

	void AppendText(const string& message);
	void SetErrorText(const string& message);
	void Show();
	void Hide();
	void TestViewlogShow();

	static enum {
		COPY_ID = 1,
		QUIT_ID,
		CLEAR_ID,

		ERRORBOX_ID = 10,
		ERRORTEXT_ID,

		EDIT_ID = 100,
		INPUT_ID
	};
private:
	HBITMAP		hbmLogo;
	HBITMAP		hbmClearBitmap;

	HFONT		hfBufferFont;
	HFONT		hfButtonFont;

	int			windowWidth, windowHeight;

public:
	// console
	field_t		g_consoleField;
	int			nextHistoryLine;	// the last line in the history buffer, not masked
	int			historyLine;		// the line being displayed from history buffer will be <= nextHistoryLine
	field_t		historyEditLines[COMMAND_HISTORY];

	char		errorString[80];
	char		consoleText[512], returnedText[512];

	bool		quitOnClose;
};