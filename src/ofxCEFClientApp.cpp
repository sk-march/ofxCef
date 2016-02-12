#include "ofxCEFClientApp.h"

#include "ofxCEFV8ExtensionHandler.h"
#include "base/cef_logging.h"

//--------------------------------------------------------------
ofxCEFClientApp::ofxCEFClientApp(){
//	CreateDelegates(delegates_);
}

#if CLIENT_TYPE==0
void ofxCEFClientApp::OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info)
{
	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end(); ++it)
		(*it)->OnRenderThreadCreated(this, extra_info);
}
//--------------------------------------------------------------
void ofxCEFClientApp::OnWebKitInitialized()
{    
	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end(); ++it)
		(*it)->OnWebKitInitialized(this);
/*    std::string app_code =
        "var app;"
        "if (!app)"
        "    app = {};"
        "(function() {"
        "    app.sendMessageWithTypeToOF = function(type, name, value) {"
        "        native function sendMessageWithTypeToOF();"
        "        return sendMessageWithTypeToOF(type, name, value);"
        "    };"
        "    app.sendMessageToOF = function(name, value) {"
        "        native function sendMessageToOF();"
        "        return sendMessageToOF(name, value);"
        "    };"
        "})();;";

    CefRegisterExtension( "v8/app", app_code, new ofxCEFV8ExtensionHandler(this) );
*/
}
//--------------------------------------------------------------
void ofxCEFClientApp::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
{
	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end(); ++it)
		(*it)->OnBrowserCreated(this, browser);
}
//--------------------------------------------------------------
void ofxCEFClientApp::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser)
{
	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end(); ++it)
		(*it)->OnBrowserDestroyed(this, browser);
}
//--------------------------------------------------------------
CefRefPtr<CefLoadHandler> ofxCEFClientApp::GetLoadHandler()
{
	CefRefPtr<CefLoadHandler> load_handler;
	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end() && !load_handler.get(); ++it)
		load_handler = (*it)->GetLoadHandler(this);

	return load_handler;
}
//--------------------------------------------------------------
bool ofxCEFClientApp::OnBeforeNavigation(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, NavigationType navigation_type, bool is_redirect)
{
	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end(); ++it) {
		if ((*it)->OnBeforeNavigation(this, browser, frame, request,
			navigation_type, is_redirect)) {
			return true;
		}
	}

	return false;
}
//--------------------------------------------------------------
void ofxCEFClientApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
	// add function
	my_browser = browser;
	my_frame = frame;
	my_context = context;
	call_back->register_function(this, browser, frame, context);

	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end(); ++it)
		(*it)->OnContextCreated(this, browser, frame, context);
}
//--------------------------------------------------------------
void ofxCEFClientApp::OnContextReleased(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefV8Context> context) {
	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end(); ++it)
		(*it)->OnContextReleased(this, browser, frame, context);
}
//--------------------------------------------------------------
void ofxCEFClientApp::OnUncaughtException(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Exception> exception, CefRefPtr<CefV8StackTrace> stackTrace)
{
	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end(); ++it) {
		(*it)->OnUncaughtException(this, browser, frame, context, exception,
			stackTrace);
	}
}
//--------------------------------------------------------------
void ofxCEFClientApp::OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefDOMNode> node)
{
	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end(); ++it)
		(*it)->OnFocusedNodeChanged(this, browser, frame, node);
}
//--------------------------------------------------------------
bool ofxCEFClientApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	DCHECK_EQ(source_process, PID_BROWSER);

	bool handled = false;

	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end() && !handled; ++it) {
		handled = (*it)->OnProcessMessageReceived(this, browser, source_process,
			message);
	}

	return handled;
}
#else
void ofxCEFClientApp::OnContextInitialized() {
	// Register cookieable schemes with the global cookie manager.
	CefRefPtr<CefCookieManager> manager =
		CefCookieManager::GetGlobalManager(NULL);
	DCHECK(manager.get());
	std::vector<CefString> cookieable_schemes_;
	manager->SetSupportedSchemes(cookieable_schemes_, NULL);

	print_handler_ = NULL;

	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end(); ++it)
		(*it)->OnContextInitialized(this);
}

void ofxCEFClientApp::OnBeforeChildProcessLaunch(
	CefRefPtr<CefCommandLine> command_line) {
	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end(); ++it)
		(*it)->OnBeforeChildProcessLaunch(this, command_line);
}

void ofxCEFClientApp::OnRenderProcessThreadCreated(
	CefRefPtr<CefListValue> extra_info) {
	DelegateSet::iterator it = delegates_.begin();
	for (; it != delegates_.end(); ++it)
		(*it)->OnRenderProcessThreadCreated(this, extra_info);
}


#endif
//--------------------------------------------------------------
void ofxCEFClientApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) {
#if defined(TARGET_OSX)


#elif defined(TARGET_WIN32)
	CefString osr(L"-off-screen-rendering-enabled");
	command_line->AppendSwitch(osr);

	CefString d3d11(L"-disable-d3d11");
	command_line->AppendSwitch(d3d11);

	CefString frameScheduling(L"-enable-begin-frame-scheduling");
	command_line->AppendSwitch(frameScheduling);
#endif
	CefString systemFlash(L"--enable-system-flash");
	command_line->AppendSwitch(systemFlash);

	//CefString singleProcess(L"-single-process");
	//command_line->AppendSwitch(singleProcess);

	//CefString igProxy(L"-no-proxy-server");
	//command_line->AppendSwitch(igProxy);

	//CefString addFile(L"-allow-file-access-from-files");
	//command_line->AppendSwitch(addFile);

	//CefString touchEventsDisabled(L"-touch-events=disabled");
	//command_line->AppendSwitch(touchEventsDisabled);

	//CefString screenCap(L"-allow-http-screen-capture");
	//command_line->AppendSwitch(screenCap);

	//CefString optimizedUI(L"-touch-optimized-ui=disabled");
	//command_line->AppendSwitch(optimizedUI);

	// CefString touchSimulated(L"-simulate-touch-screen-with-mouse");
	// command_line->AppendSwitch(touchSimulated);

}
