#include "sys_local.h"

AsyncFileTask& AsyncFileTask::operator=(const AsyncFileTask& rhs) {
	this->callback = rhs.callback;
	this->data = rhs.data;
	this->dataSize = rhs.dataSize;
	this->pFile = rhs.pFile;
	this->type = rhs.type;
	return *this;
}

AsyncResourceTask& AsyncResourceTask::operator=(const AsyncResourceTask& rhs) {
	this->callback = rhs.callback;
	this->pResource = rhs.pResource;
	this->type = rhs.type;
	return *this;
}