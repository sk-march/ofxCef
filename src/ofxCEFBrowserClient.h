#include "cef_client.h"

#include "ofxCEFRenderHandler.h"

#ifndef BROWSER_CLIENT_H
#define BROWSER_CLIENT_H

class ofxCEF;

//--------------------------------------------------------------
class ofxCEFBrowserClient : public CefClient, public CefLoadHandler
{
public:
    ofxCEFBrowserClient(ofxCEF* parent, ofxCEFRenderHandler* renderHandler);

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler();
    
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE;

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                              CefProcessId source_process,
                              CefRefPtr<CefProcessMessage> message) OVERRIDE;
    
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame);
    

    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           int httpStatusCode);

	inline ofxCEF* GetCEF() { return _parent; }

private:
    CefRefPtr<CefRenderHandler> handler;
    ofxCEF* _parent;
    
    IMPLEMENT_REFCOUNTING(ofxCEFBrowserClient);

};

#endif
