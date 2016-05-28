#include "sys_local.h"

/*
 * `Cvars` (or, 'console variables') are variables which can be manipulated via the console.
 * They can be archived in configuration files or developer cheat codes for testing new features.
 * Cvars can be one of four types: string, integer, float, boolean.
 */

// Copies this cvar from another
// FIXME
Cvar::Cvar(Cvar&& other) {
}

// Creates a new string-based cvar.
Cvar::Cvar(const string& sName, const string& sDesc, int iFlags, char* startValue) :
  name(sName),
  description(sDesc),
  type(Cvar::CV_STRING),
  flags(iFlags),
  ptsChangeCallback(nullptr) {
	  s.AssignBoth(startValue);
}

// Creates a new integer-based cvar.
Cvar::Cvar(const string& sName, const string& sDesc, int iFlags, int startValue) :
name(sName),
description(sDesc),
type(Cvar::CV_INTEGER),
flags(iFlags),
ptiChangeCallback(nullptr) {
	i.AssignBoth(startValue);
}

// Creates a new floating point cvar.
Cvar::Cvar(const string& sName, const string& sDesc, int iFlags, float startValue) :
name(sName),
description(sDesc),
type(Cvar::CV_FLOAT),
flags(iFlags),
ptfChangeCallback(nullptr) {
	v.AssignBoth(startValue);
}

// Creates a new boolean-based cvar.
Cvar::Cvar(const string& sName, const string& sDesc, int iFlags, bool startValue) :
name(sName),
description(sDesc),
type(Cvar::CV_BOOLEAN),
flags(iFlags),
ptbChangeCallback(nullptr) {
	b.AssignBoth(startValue);
}

// Creates a blank cvar.
Cvar::Cvar() :
  flags(0),
  name(""),
  description(""),
  type(Cvar::CV_BOOLEAN),
  ptbChangeCallback(nullptr) {
	b.AssignBoth(false);
}

// Deletes a cvar object.
Cvar::~Cvar() {
}

// Assign a string value to a cvar.
// If the cvar is not a string type, nothing happens.
Cvar& Cvar::operator= (char* str) {
	if(type != CV_STRING) return *this;
	strncpy(s.currentVal, str, sizeof(s.currentVal));
	return *this;
}

// Assign an integer value to a cvar.
// If the cvar is not an integer type, nothing happens.
Cvar& Cvar::operator= (int val) {
	if(type != CV_INTEGER) return *this;
	i.currentVal = val;
	return *this;
}

// Assigns a floating point value to a cvar.
// If the cvar is not a floating point type, nothing happens.
Cvar& Cvar::operator= (float val) {
	if(type != CV_FLOAT) return *this;
	v.currentVal = val;
	return *this;
}

// Assigns a boolean value to a cvar.
// If the cvar is not a boolean type, nothing happens.
Cvar& Cvar::operator= (bool val) {
	if(type != CV_BOOLEAN) return *this;
	b.currentVal = val;
	return *this;
}

// Checks to see if a cvar exists.
bool Cvar::Exists(const string& sName) {
	try {
		auto it = CvarSystem::cvars.find(sName);
		if(it == CvarSystem::cvars.end())
			return false;
		return true;
	}
	catch( out_of_range e ) {
		return false;
	}
}

// Changes the value of a string-based cvar.
// This also changes the default value of the cvar.
void Cvar::AssignHardValue(char* value) { 
	s.AssignBoth(value); 
	if(ptsChangeCallback) 
		ptsChangeCallback(value); 
}

// Changes the value of an integer-based cvar.
// This also changes the default value of the cvar.
void Cvar::AssignHardValue(int value) {
	i.AssignBoth(value); 
	if(ptiChangeCallback) 
		ptiChangeCallback(value); 
}

// Changes the value of a floating-point cvar.
// This also changes the default value of the cvar.
void Cvar::AssignHardValue(float value) { 
	v.AssignBoth(value); 
	if(ptfChangeCallback) 
		ptfChangeCallback(value); 
}

// Changes the value of a boolean cvar.
// This also changes the default value of the cvar.
void Cvar::AssignHardValue(bool value) {
	b.AssignBoth(value);
	if(ptbChangeCallback)
		ptbChangeCallback(value); 
}

// Changes the value of a string-based cvar.
void Cvar::SetValue(char* value) { 
	if(type != CV_STRING) return; 
	strncpy(s.currentVal, value, sizeof(s.currentVal)); 
	if(flags & (1 << CVAR_ANNOUNCE)) 
		R_Message(PRIORITY_NOTE, "%s changed to %s\n", name.c_str(), value); 
	RunCallback();
}

// Changes the value of an integer-based cvar.
void Cvar::SetValue(int value) { 
	if(type != CV_INTEGER) return; 
	i.currentVal = value; 
	if(flags & (1 << CVAR_ANNOUNCE)) 
		R_Message(PRIORITY_NOTE, "%s changed to %i\n", name.c_str(), value); 
	RunCallback();
}

// Changes the value of a floating-point cvar.
void Cvar::SetValue(float value) { 
	if(type != CV_FLOAT) return; 
	v.currentVal = value; 
	if(flags & (1 << CVAR_ANNOUNCE)) 
		R_Message(PRIORITY_NOTE, "%s changed to %f\n", name.c_str(), value); 
	RunCallback();
}

// Changes the value of a boolean cvar.
void Cvar::SetValue(bool value) { 
	if(type != CV_BOOLEAN) return; 
	b.currentVal = value; 
	if(flags & (1 << CVAR_ANNOUNCE)) 
		R_Message(PRIORITY_NOTE, "%s changed to %s\n", name.c_str(), btoa(value)); 
	RunCallback();
}

// Adds a callback function to a cvar.
// When the value of the cvar changes, the callback function will be called with the new value as parameter.
void Cvar::AddCallback(void* function) {
	switch(type) {
		default:
		case CV_STRING:
			ptsChangeCallback = (void(*)(char*))function;
			break;
		case CV_INTEGER:
			ptiChangeCallback = (void(*)(int))function;
			break;
		case CV_FLOAT:
			ptfChangeCallback = (void(*)(float))function;
			break;
		case CV_BOOLEAN:
			ptbChangeCallback = (void(*)(bool))function;
			break;
	}
}

// Runs the callback function for the cvar.
void Cvar::RunCallback() {
	switch(type) {
		default:
		case CV_STRING:
			if(ptsChangeCallback) {
				ptsChangeCallback(s.currentVal);
			}
			break;
		case CV_INTEGER:
			if(ptiChangeCallback) {
				ptiChangeCallback(i.currentVal);
			}
			break;
		case CV_FLOAT:
			if(ptfChangeCallback) {
				ptfChangeCallback(v.currentVal);
			}
			break;
		case CV_BOOLEAN:
			if(ptbChangeCallback) {
				ptbChangeCallback(b.currentVal);
			}
			break;
	}
}