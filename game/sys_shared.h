#pragma once
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <Rpc.h>
#endif
#include <stdlib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include <queue>
#include <sstream>
#include <algorithm>
#include <map>
#include <regex>
#include <mutex>
#include <chrono>
#include <thread>
#include <dirent.h>
#include <functional>
#include <iomanip>
#include <time.h>
#include <assert.h>
#include <RaptureAsset.h>
using namespace std;

#ifdef _WIN32
#pragma warning(disable: 4996) // X is possibly unsafe
#pragma warning(error: 4190) // 'X' has C-linkage specified but returns UDT Y
#endif

#define MAX_HANDLE_STRING	64

extern const string keycodeNames[];
vector<string>& split(const string& str, const char delim, vector<string>& elems);
vector<wstring>& split(const wstring& str, const wchar_t delim);
bool atob(const string& str);
bool atob(const char* str);
string trim(const string& str, const string& trim = " \t");
string hexstring(const int address);
void towstring(const string& in, wstring& out);
bool checkExtension (const string &fullString, const string &ending);
void stringreplace(string& fullString, const string& sequence, const string& replace);
const char* btoa(bool b);

// Export types
class File;
class Resource;
class Image;
class Font;
class Menu;
class Material;
class Cvar;
class AnimationManager;
class Texture;

enum dispatchPriorities_e {
	PRIORITY_NONE,
	PRIORITY_NOTE,
	PRIORITY_DEBUG,
	PRIORITY_MESSAGE,
	PRIORITY_WARNING,
	PRIORITY_ERROR,
	PRIORITY_ERRFATAL,
	PRIORITY_MAX
};

enum cvarFlags_e {
	CVAR_ARCHIVE,
	CVAR_ROM,
	CVAR_ANNOUNCE,
	CVAR_SERVERINFO,		// Networked from server -> client
	CVAR_CLIENTINFO,		// Networked from client -> server
};

/*
==============================================================
NETWORKING INFORMATION
==============================================================
*/
enum packetType_e {
	PACKET_PING,			// Server <-> (Any)		--	Check if service is alive	
	PACKET_PONG,			// Server <-> (Any)		--	Response to ping packet
	PACKET_DROP,			// Server <-> (Any)		--	Connection timed out
	PACKET_CLIENTATTEMPT,	// Server <-  Client	--	Attempt to join a server
	PACKET_CLIENTACCEPT,	// Server  -> Client	--	Tell the client they are accepted into the server
	PACKET_CLIENTDENIED,	// Server  -> Client	--	Tell the client they are denied into the server
	PACKET_INFOREQUEST,		// Server <-  3rdParty	--	Ask for some info from a server
	PACKET_INFOREQUESTED,	// Server  -> 3rdParty	--	Send the info that was requested from this server
	PACKET_SENDCHAT,		// Server <-  Client	--	Client spoke
	PACKET_RECVCHAT,		// Server  -> Client	--	Client is heard
};

struct Packet {
	enum PacketDirection_e {
		PD_ServerClient,	// Server -> Client
		PD_ClientServer		// Client -> Server
	};

	struct PacketHeader {
		packetType_e		type;			// Type of this packet
		int8_t				clientNum;		// The other client number (host is always client 0)
		PacketDirection_e	direction;		// Direction this packet is going (indicates how clientNum is interpretted)
		uint64_t			sendTime;		// Time this packet was sent
		size_t				packetSize;		// Size of this packet (minus the header)
	};

	PacketHeader packetHead;
	void* packetData;
};

namespace ClientPacket {
	struct ClientAttemptPacket {
		uint8_t	netProtocol;
	};

	struct ClientAcceptPacket {
		uint8_t clientNum;
	};

	struct ClientDeniedPacket {
		char why[140];
	};
}

/*
====================================================
SAVEGAME STRUCTURE
====================================================
*/
struct Rapture_TimeDate {
	uint32_t	year;		// Year
	uint8_t		month;		// Month (0-11)
	uint8_t		day;		// Day (1-31)
	uint8_t		hour;		// Hours (0-23)
	uint8_t		minute;		// Minute (0-59)
	uint8_t		second;		// Second (0-59)
};

#define RAPTURE_CHARLENMAX	32
struct Rapture_CharacterMeta {	// Metadata that gets passed from character creation screen -> game
	char		charName[RAPTURE_CHARLENMAX];	// Character name
	uint8_t		charClass;						// Character class
	uint32_t	bandanaRGB;						// Bandana color
	uint8_t		charLeague;						// Character League
	uint8_t		difficulty;						// Difficulty level
	uint8_t		startingSkill;					// Starting skill
	uint8_t		multiplayer;					// Whether this is a multiplayer character
};

struct Rapture_CharacterTime {	// Time information
	Rapture_TimeDate	lastUseTime;	// Last time this character was used
	Rapture_TimeDate	creationTime;	// When this character was created
	Rapture_TimeDate	playTime;		// How long this character has been played for
};

// The actual savegame itself
struct Rapture_Savegame {
	struct Rapture_SaveHeader {
		char header[4]; // 'R' 'S' 'A' 'V'
		Rapture_CharacterMeta meta;
		Rapture_CharacterTime time;
	};
	Rapture_SaveHeader head;
};

/*
=====================================================================
API
=====================================================================
*/

// Callbacks
typedef void(*fileOpenedCallback)(File* pFile);
typedef void(*fileReadCallback)(File* pFile, void* buffer, size_t bufferSize);
typedef fileReadCallback fileWrittenCallback;
typedef fileOpenedCallback fileClosedCallback;
typedef void(*assetRequestCallback)(AssetComponent* component);
typedef void(*fontRegisteredCallback)(const char* handleName, Font* fontFile);
typedef void(__cdecl *conCmd_t)(vector<string>& args);

extern "C" {
	struct gameImports_s {
		// Logging
		void(*printf)(int iPriority, const char* fmt, ...);
		void(*error)(const char* fmt, ...);

		// Time
		int(*GetTicks)();
		Rapture_TimeDate(*GetCurrentTimeDate)();
		void(*SubtractTimeDate)(Rapture_TimeDate a, Rapture_TimeDate b, Rapture_TimeDate* result);
		void(*AddTimeDate)(Rapture_TimeDate a, Rapture_TimeDate b, Rapture_TimeDate* result);

		// Files
		File* (*OpenFileSync)(const char* filename, const char* mode);
		bool(*ReadFileSync)(File* pFile, void* data, size_t dataSize);
		bool(*WriteFileSync)(File* pFile, void* data, size_t dataSize);
		bool(*CloseFileSync)(File* pFile);

		File*(*OpenFileAsync)(const char* fileName, const char* mode, fileOpenedCallback callback);
		void(*ReadFileAsync)(File* pFile, void* data, size_t dataSize, fileReadCallback callback);
		void(*WriteFileAsync)(File* pFile, void* data, size_t dataSize, fileWrittenCallback callback);
		void(*CloseFileAsync)(File* pFile, fileClosedCallback callback);
		bool(*FileOpened)(File* pFile);
		bool(*FileRead)(File* pFile);
		bool(*FileWritten)(File* pFile);
		bool(*FileClosed)(File* pFile);
		bool(*FileBad)(File* pFile);

		// Resources
		Resource* (*ResourceAsync)(const char* asset, const char* component, assetRequestCallback callback);
		Resource* (*ResourceAsyncURI)(const char* uri, assetRequestCallback callback);
		Resource* (*ResourceSync)(const char* asset, const char* component);
		Resource* (*ResourceSyncURI)(const char* uri);
		void	  (*FreeResource)(Resource* pResource);
		AssetComponent* (*GetAssetComponent)(Resource* pResource);
		bool(*ResourceRetrieved)(Resource* pResource);
		bool(*ResourceBad)(Resource* pResource);

		// Materials
		Material*	(*RegisterMaterial)(const char* szMaterial);
		void		(*DrawMaterial)(Material* ptMaterial, float xPct, float yPct, float wPct, float hPct);
		void		(*DrawMaterialAspectCorrection)(Material* ptMaterial, float xPct, float yPct, float wPct, float hPct);
		void		(*DrawMaterialClipped)(Material* ptMaterial, float sxPct, float syPct, float swPct, float shPct, float ixPct, float iyPct, float iwPct, float ihPct);
		void		(*DrawMaterialAbs)(Material* ptMaterial, int nX, int nY, int nW, int nH);
		void		(*DrawMaterialAbsClipped)(Material* ptMaterial, int sX, int sY, int sW, int sH, int iX, int iY, int iW, int iH);

		// Textures
		Texture*	(*RegisterStreamingTexture)(const unsigned int nW, const unsigned int nH);
		int			(*LockStreamingTexture)(Texture* ptTexture, unsigned int nX, unsigned int nY, unsigned int nW, unsigned int nH, void** pixels, int* pitch);
		void		(*UnlockStreamingTexture)(Texture* ptTexture);
		void		(*DeleteStreamingTexture)(Texture* ptTexture);
		void		(*BlendTexture)(Texture* ptTexture);

		// Text/font
		void		(*RegisterFontAsync)(const char* resourceURI, fontRegisteredCallback callback);
		void		(*RenderSolidText)(Font* font, const char* text, int x, int y, int r, int g, int b);
		void		(*RenderShadedText)(Font* font, const char* text, int x, int y, int br, int bg, int bb, int fr, int fg, int fb);
		void		(*RenderBlendedText)(Font* font, const char* text, int x, int y, int r, int g, int b);

		// UI
		Menu* (*RegisterStaticMenu)(const char* sMenuFile);
		void  (*KillStaticMenu)(Menu* menu);
		void  (*RunJavaScript)(Menu* menu, const char* sJS);
		bool  (*IsConsoleOpen)();
		void  (*AddJSCallback)(Menu* menu, const char* sCallbackName, void(*ptCallback)());
		unsigned int   (*GetJSNumArgs)(Menu* ptMenu);
		void (*GetJSStringArg)(Menu* ptMenu, unsigned int argNum, char* sBuffer, size_t numChars);
		int   (*GetJSIntArg)(Menu* ptMenu, unsigned int argNum);
		double(*GetJSDoubleArg)(Menu* ptMenu, unsigned int argNum);
		bool  (*GetJSBoolArg)(Menu* ptMenu, unsigned int argNum);

		// Network
		void(*SendServerPacket)(packetType_e packetType, int clientNum, void* packetData, size_t packetSize);
		void(*SendClientPacket)(packetType_e packetType, void* packetData, size_t packetSize);

		// Cvars
		int(*CvarIntVal)(Cvar* cvar, int* value);
		char*(*CvarStrVal)(Cvar* cvar, char* value);
		bool(*CvarBoolVal)(Cvar* cvar, bool* value);
		float(*CvarValue)(Cvar* cvar, float* value);
		Cvar* (*RegisterCvarInt)(const char* cvarName, const char* description, int flags, int startingValue);
		Cvar* (*RegisterCvarFloat)(const char* cvarName, const char* description, int flags, float startingValue);
		Cvar* (*RegisterCvarBool)(const char* cvarName, const char* description, int flags, bool bStartingValue);
		Cvar* (*RegisterCvarStr)(const char* cvarName, const char* description, int flags, char* sStartingValue);

		// Commands
		void(*AddCommand)(const char* cmdName, conCmd_t command);
		void(*RemoveCommand)(const char* cmdName);

		// Zone memory
		void* (*Zone_Alloc)(int iSize, const char* tag);
		void(*Zone_NewTag)(const char* tag);
		void(*Zone_Free)(void *memory);
		void(*Zone_FastFree)(void* memory, const char* tag);
		void(*Zone_FreeAll)(const char* tag);
		void* (*Zone_Realloc)(void* memory, size_t iNewSize);

		// Global effects
		void(*FadeFromBlack)(int time);
	};

	struct gameExports_s {
		void(*startserverfromsave)(const char* szSaveGame);
		void(*startclientfromsave)(const char* szSaveGame);
		void(*runserverframe)();
		void(*runclientframe)();
		void(*saveandexit)();
		bool(*acceptclient)(ClientPacket::ClientAttemptPacket* packet);

		void(*passmouseup)(int x, int y);
		void(*passmousedown)(int x, int y);
		void(*passmousemove)(int x, int y);
		void(*passkeypress)(int x);

		bool(*interpretPacketFromClient)(Packet* packet);
		bool(*interpretPacketFromServer)(Packet* packet);
	};
}

// sys_main.cpp
void R_Message(int iPriority, const char *fmt, ...);

// Erases an item from a vector provided that the vector contains all unique elements.
template <typename T>
void VectorErase(vector<T>& rtVector, T uniqueMember) {
	for(auto it = rtVector.begin(); it != rtVector.end(); ++it) {
		if(*it == uniqueMember) {
			rtVector.erase(it);
			break;
		}
	}
}