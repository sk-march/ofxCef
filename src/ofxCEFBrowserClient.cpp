#include "ofxCEFBrowserClient.h"
#include "ofxCEF.h"

//--------------------------------------------------------------
ofxCEFBrowserClient::ofxCEFBrowserClient(ofxCEF* parent, ofxCEFRenderHandler* renderHandler, js_callback* icall_back, const std::vector<std::wstring>& icert_exception){
	browser_count_ = 0;
    _parent = parent;
    handler = renderHandler;
	call_back = icall_back;
	cert_exception = icert_exception;
}

//--------------------------------------------------------------
CefRefPtr<CefRenderHandler> ofxCEFBrowserClient::GetRenderHandler(){
    return handler;
}

//--------------------------------------------------------------
CefRefPtr<CefLoadHandler> ofxCEFBrowserClient::GetLoadHandler(){
    return this;
}

//--------------------------------------------------------------
CefRefPtr<CefLifeSpanHandler> ofxCEFBrowserClient::GetLifeSpanHandler(){
	return this;
}

//--------------------------------------------------------------
CefRefPtr<CefRequestHandler> ofxCEFBrowserClient::GetRequestHandler() {
	return this;
}

//--------------------------------------------------------------
CefRefPtr<CefContextMenuHandler> ofxCEFBrowserClient::GetContextMenuHandler()
{
	return this;
}

//--------------------------------------------------------------
void ofxCEFBrowserClient::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame){
    _parent->onLoadStart();
}


//--------------------------------------------------------------
void ofxCEFBrowserClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode){
    _parent->onLoadEnd(httpStatusCode);
}


//--------------------------------------------------------------
void ofxCEFBrowserClient::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefContextMenuParams> params,
	CefRefPtr<CefMenuModel> model)
{
	model->Clear();
}

//--------------------------------------------------------------
void ofxCEFBrowserClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// add function
	browser_count_++;

	if (!message_router_) {
		// Create the browser-side router for query handling.
		CefMessageRouterConfig config;
		message_router_ = CefMessageRouterBrowserSide::Create(config);
		if(call_back!=NULL) message_router_->AddHandler(call_back, false);
	}

//	NotifyBrowserCreated(browser);
	CefLifeSpanHandler::OnAfterCreated(browser);
}

//--------------------------------------------------------------
void ofxCEFBrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	if (--browser_count_ == 0) {
		// Remove and delete message router handlers.
		if(call_back!=NULL) message_router_->RemoveHandler(call_back);
		message_router_ = NULL;
	}

//	NotifyBrowserClosed(browser);
	CefLifeSpanHandler::OnBeforeClose(browser);
}

//--------------------------------------------------------------
bool ofxCEFBrowserClient::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect){
	CEF_REQUIRE_UI_THREAD();

	call_back->register_browzer(browser.get());
	message_router_->OnBeforeBrowse(browser, frame);
	return false;
}

//--------------------------------------------------------------
void ofxCEFBrowserClient::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) {
	CEF_REQUIRE_UI_THREAD();

	//  message_router_->OnRenderProcessTerminated(browser);

	// Don't reload if there's no start URL, or if the crash URL was specified.
//	if (startup_url_.empty() || startup_url_ == "chrome://crash")
//		return;

	CefRefPtr<CefFrame> frame = browser->GetMainFrame();
	std::string url = frame->GetURL();

	// Don't reload if the termination occurred before any URL had successfully
	// loaded.
	if (url.empty())
		return;

//	std::string start_url = startup_url_;

	// Convert URLs to lowercase for easier comparison.
//	std::transform(url.begin(), url.end(), url.begin(), tolower);
///	std::transform(start_url.begin(), start_url.end(), start_url.begin(),
//		tolower);

	// Don't reload the URL that just resulted in termination.
//	if (url.find(start_url) == 0)
//		return;

//	frame->LoadURL(startup_url_);
}

//--------------------------------------------------------------
bool ofxCEFBrowserClient::OnCertificateError(
	CefRefPtr<CefBrowser> browser,
	cef_errorcode_t cert_error,
	const CefString& request_url,
	CefRefPtr<CefSSLInfo> ssl_info,
	CefRefPtr<CefRequestCallback> callback)
{
	std::wstring t = request_url.c_str();
	std::wstring::size_type i = t.find(L"/", 8);
	t = t.substr(8, i-8);
	for (int i = 0; i < cert_exception.size(); ++i) {
		if (cert_exception[i] == t) {
			callback->Continue(true);
			return true;
		}
	}
	return false;
}


//--------------------------------------------------------------
bool ofxCEFBrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                             CefProcessId source_process,
                                             CefRefPtr<CefProcessMessage> message){
	if (message_router_->OnProcessMessageReceived(browser, source_process,
		message)) {
		return true;
	}

	
	// Retrieve the argument list object.
    CefRefPtr<CefListValue> args = message->GetArgumentList();

    // Retrieve the argument values.
    CefString type = message->GetName();
    string name = args->GetString(0).ToString();
    
    // Forward the message argument value to the parent (instance of ofxCEF).    
    if (type == "string") {
        _parent->gotMessageFromJS(name, type, args->GetString(1).ToString());
    }
    else if (type == "double") {
        _parent->gotMessageFromJS(name, type, ofToString(args->GetDouble(1)));
    }
    else if (type == "int") {
        _parent->gotMessageFromJS(name, type, ofToString(args->GetInt(1)));
    }
    else if (type == "bool") {
        _parent->gotMessageFromJS(name, type, ofToString(args->GetBool(1)));
    }
    else {
        std::cout << "ofxCEFBrowserClient received a message of unknown type." << std::endl;
        return false;
    }

    return true;
}
