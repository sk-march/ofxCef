#include "cef_app.h"

#include "ofxCEFBrowserClient.h"
#include "ofxCEFClientApp.h"
#include "ofxCEFRenderHandler.h"

// use cef_binary_3.2454.1344.g2782fb8_windows32
// @ https://cefbuilds.com/
// download and run "cmake ."

#ifdef _DEBUG
#pragma comment(lib, "../../../addons/ofxCef/libs/CEF/win32/cef/Debug/libcef.lib")
#pragma comment(lib, "../../../addons/ofxCef/libs/CEF/win32/cef/libcef_dll/Debug/libcef_dll_wrapper.lib")
//#pragma comment(lib, "../../../addons/ofxCef/libs/CEF/win32/lib/libcef_d.lib")
//#pragma comment(lib, "../../../addons/ofxCef/libs/CEF/win32/lib/libcef_dll_wrapper_d.lib")
#else
#pragma comment(lib, "../../../addons/ofxCef/libs/CEF/win32/cef/Release/libcef.lib")
#pragma comment(lib, "../../../addons/ofxCef/libs/CEF/win32/cef/libcef_dll/Release/libcef_dll_wrapper.lib")
//#pragma comment(lib, "../../../addons/ofxCef/libs/CEF/win32/lib/libcef.lib")
//#pragma comment(lib, "../../../addons/ofxCef/libs/CEF/win32/lib/libcef_dll_wrapper.lib")
#endif

#ifndef CEFGUI_H
#define CEFGUI_H

//--------------------------------------------------------------
class ofxCEFMessageArgs : public ofEventArgs{
public:
    string name;
    string type;
    string value;
};

//--------------------------------------------------------------
class ofxCEFEventArgs : public ofEventArgs{
public:
    enum Type{
        onLoadStart,
        onLoadEnd
    } type;
    int httpStatusCode;
};

//--------------------------------------------------------------
void initofxCEF(int argc, char** argv, js_callback_handler* call_back);

//--------------------------------------------------------------
class ofxCEF
{
public:
    ofxCEF();
    ~ofxCEF();
    
	void exit();

    void load(const char*);
    void update();
    void draw(void);
    void reshape(int, int);
    
    void setup(uint32_t w=0, uint32_t h=0);
    
	void focus();
    void mouseWheel(int v, int h);
	void mousePressed(int code= OF_MOUSE_BUTTON_LEFT);
	void mouseReleased(int code = OF_MOUSE_BUTTON_LEFT);
	void mouseMoved(int x, int y);

	void keyPutChar(char text);
	void keyPutChar(wchar_t text);
	void keyPressed(int code);
	void keyReleased(int code);
	void keyWinMessageLoop(UINT msg, WPARAM wParam, LPARAM lParam);

	void mousePressed(ofMouseEventArgs &e);
    void mouseReleased(ofMouseEventArgs &e);
    void mouseMoved(ofMouseEventArgs &e);
    void mouseDragged(ofMouseEventArgs &e);
    
	void keyPressed(ofKeyEventArgs &e);
    void keyReleased(ofKeyEventArgs &e);
    
    void windowResized(ofResizeEventArgs &e);
    
    void enableEvents();
    void disableEvents();
    
    void executeJS(const char*);

    void notificationHandler();
    
    void onLoadStart();
    void onLoadEnd(int httpStatusCode);
    
    void gotMessageFromJS(string name, string type, string value);

    ofEvent<ofxCEFMessageArgs> messageFromJS;
    ofEvent<ofxCEFEventArgs> eventFromCEF;

 // private:
    int mouseX, mouseY;

    CefRefPtr<CefBrowser> browser;
    CefRefPtr<ofxCEFBrowserClient> client;

	CefRefPtr<ofxCEFRenderHandler> renderHandler;

};

typedef void (ofxCEF::*notificationHandler)();

#endif
