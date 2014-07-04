#pragma once
#include "Entity.h"
#include "Local.h"

class Container : public Entity {
protected:
	bool bIsOpen;
public:
	virtual void Open() = 0;
};