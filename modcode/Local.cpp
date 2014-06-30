#include "Local.h"

// swap endianness
void eswap(unsigned short &x) {
	x = (x>>8) | (x<<8);
}

void eswap(unsigned int &x) {
	x = (x>>24) | ((x<<8) & 0x00FF0000) | ((x>>8) & 0x0000FF00) | (x<<24);
}

void eswap(unsigned long &x) {
	
}

string genuuid() {
#ifdef _WIN32
	UUID uuid;
	UuidCreate(&uuid);

	unsigned char* str;
	UuidToStringA(&uuid, &str);
	string s((char*)str);
	
	RpcStringFree(&str);
#else
	string s = "Currently not coded for Linux";
#endif

	return s;
}