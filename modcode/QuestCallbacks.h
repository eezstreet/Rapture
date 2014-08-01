#pragma once
#include "Local.h"

typedef void(*Q_EnterAreaCallback)(const string& level);
typedef void(*Q_StateChangeCallback)(const int toState);
typedef void(*Q_StateAwayCallback)(const int fromState);