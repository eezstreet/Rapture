#include "assettool.h"

#pragma warning(disable:4996)

void TryGuessMimeType(char*& mimeType, const char* extension) {
	if (!stricmp(extension, ".pdf")) {
		strcpy(mimeType, "application/pdf");
	}
	else if (!stricmp(extension, ".js")) {
		strcpy(mimeType, "application/javascript");
	}
	else if (!stricmp(extension, ".mpeg")) {
		strcpy(mimeType, "audio/mpeg");
	}
	else if (!stricmp(extension, ".ogg")) {
		strcpy(mimeType, "audio/ogg");
	}
	else if (!stricmp(extension, ".bmp")) {
		strcpy(mimeType, "image/bmp");
	}
	else if (!stricmp(extension, ".gif")) {
		strcpy(mimeType, "image/gif");
	}
	else if (!stricmp(extension, ".jpeg") || !stricmp(extension, ".jpg")) {
		strcpy(mimeType, "image/jpeg");
	}
	else if (!stricmp(extension, ".png")) {
		strcpy(mimeType, "image/png");
	}
	else if (!stricmp(extension, ".css")) {
		strcpy(mimeType, "text/css");
	}
	else if (!stricmp(extension, ".csv")) {
		strcpy(mimeType, "text/csv");
	}
	else if (!stricmp(extension, ".html") || !stricmp(extension, ".htm")) {
		strcpy(mimeType, "text/html");
	}
	else if (!stricmp(extension, ".md")) {
		strcpy(mimeType, "text/markdown");
	}
	else if (!stricmp(extension, ".txt")) {
		strcpy(mimeType, "text/plain");
	}
	else if (!stricmp(extension, ".rtf")) {
		strcpy(mimeType, "text/rtf");
	}
	else if (!stricmp(extension, ".xml")) {
		strcpy(mimeType, "text/xml");
	}
}