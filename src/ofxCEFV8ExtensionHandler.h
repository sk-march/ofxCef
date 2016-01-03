﻿#ifndef __CEF3SimpleSample__ClientV8ExtensionHandler__
#define __CEF3SimpleSample__ClientV8ExtensionHandler__

#include "include/cef_app.h"

//--------------------------------------------------------------
struct ofxCEFV8ExtensionHandler : public CefV8Handler
{
    ofxCEFV8ExtensionHandler(CefRefPtr<CefApp> app, CefRefPtr<CefV8Context> context);

    bool Execute(const CefString &name,
                 CefRefPtr<CefV8Value> object,
                 const CefV8ValueList &arguments,
                 CefRefPtr<CefV8Value> &retval,
                 CefString &exception) OVERRIDE;

private:
    CefRefPtr<CefApp> app;
	CefRefPtr<CefV8Context> context;

    IMPLEMENT_REFCOUNTING(ofxCEFV8ExtensionHandler);

public:
};

#endif
