#include "cef_client.h"

#include "ofxCEFRenderHandler.h"
#include "include/cef_context_menu_handler.h"
#include "include/cef_request_handler.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_message_router.h"
#include "ofxCEFClientApp.h"


#ifndef BROWSER_CLIENT_H
#define BROWSER_CLIENT_H

class ofxCEF;

//--------------------------------------------------------------
class ofxCEFBrowserClient :
	public CefClient, 
	public CefLoadHandler, 
	public CefLifeSpanHandler, 
	public CefRequestHandler, 
	public CefContextMenuHandler

{
public:
    ofxCEFBrowserClient(ofxCEF* parent, ofxCEFRenderHandler* renderHandler, js_callback* icall_back, const std::vector<std::wstring>& cert_exception);

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE;
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE;
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE;
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE;
	virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE;



    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                              CefProcessId source_process,
                              CefRefPtr<CefProcessMessage> message) OVERRIDE;
    
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame);
    

    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           int httpStatusCode);

	inline ofxCEF* GetCEF() { return _parent; }

	// CefContextMenuHandler methods
	virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefContextMenuParams> params,
		CefRefPtr<CefMenuModel> model) OVERRIDE;

	// javascript binding----------------
	int browser_count_;
	CefRefPtr<CefMessageRouterBrowserSide> message_router_;
	// CefLifeSpanHandler methods
	void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

	// CefRequestHandler methods
	bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		bool is_redirect) OVERRIDE;

	void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
		TerminationStatus status) OVERRIDE;

	virtual bool OnCertificateError(
		CefRefPtr<CefBrowser> browser,
		cef_errorcode_t cert_error,
		const CefString& request_url,
		CefRefPtr<CefSSLInfo> ssl_info,
		CefRefPtr<CefRequestCallback> callback) OVERRIDE;


private:
	std::vector<std::wstring> cert_exception;
	CefRefPtr<CefRenderHandler> handler;
    ofxCEF* _parent;
	js_callback* call_back;

    IMPLEMENT_REFCOUNTING(ofxCEFBrowserClient);

};

#endif
