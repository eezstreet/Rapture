#include "assettool.h"
#include <commdlg.h>

static char openFileNameBuffer[32768];
const char* OpenFileDialog(const char* filter, int flags) {
	int ofnFlags = 0;

	if (flags & OFFLAG_MULTISELECT) {
		ofnFlags |= OFN_ALLOWMULTISELECT;
	}
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = openFileNameBuffer;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(openFileNameBuffer);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | ofnFlags;

	GetOpenFileName(&ofn);

	return openFileNameBuffer;
}

static char saveFileNameBuffer[32768];
const char* SaveAsDialog(const char* filter, const char* extension) {
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = saveFileNameBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = extension;

	GetSaveFileName(&ofn);
	return saveFileNameBuffer;
}

void DisplayMessageBox(const char* title, const char* text, int type) {
	int flags = 0;
	switch (type) {
		case MESSAGEBOX_INFO:
			flags = MB_ICONINFORMATION;
			break;
		default:
		case MESSAGEBOX_WARNING:
			flags = MB_ICONWARNING;
			break;
		case MESSAGEBOX_ERROR:
			flags = MB_ICONERROR;
			break;
	}
	MessageBox(nullptr, text, title, MB_OK | flags | MB_TASKMODAL | MB_TOPMOST);
}