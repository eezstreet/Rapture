#pragma once
#include "tr_local.h"
#include "sys_local.h"

#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
#include <Awesomium/BitmapSurface.h>

extern Awesomium::WebCore *wc;
extern Awesomium::WebView* currentFocus; // If this is non-null, then we only pipe input to that object, otherwise we do this for all renderables

void AddRenderable(Awesomium::WebView* wv);
void RemoveRenderable(Awesomium::WebView* wv);