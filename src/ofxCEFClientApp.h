#ifndef __CEF3SimpleSample__ClientHandler__
#define __CEF3SimpleSample__ClientHandler__

#include <set>
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/wrapper/cef_message_router.h"

struct js_callback : public CefMessageRouterBrowserSide::Handler
{
	// Handle messages in the browser process.
	virtual bool call_func(CefRefPtr<CefV8Value> &retval, const CefRefPtr<CefV8Value> object,const CefString &name, const CefV8ValueList &arguments, CefRefPtr<CefV8Context> context) {
		return false;
	}
	virtual void register_function(CefRefPtr<CefApp> app, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) {
	}

	// interface for message router javascript binding
	virtual void register_browzer(CefRefPtr<CefBrowser> browser) {};
	virtual bool OnQuery(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		int64 query_id,
		const CefString& request,
		bool persistent,
		CefRefPtr<Callback> callback) OVERRIDE
	{return false;}
	virtual void OnQueryCanceled(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		int64 query_id) OVERRIDE
	{}
};

//--------------------------------------------------------------

//--------------------------------------------------------------
#define CLIENT_TYPE 0
class ofxCEFClientApp :
	public CefApp,
#if CLIENT_TYPE==0
	public CefRenderProcessHandler
#else
	public CefBrowserProcessHandler
#endif
{
public:
#if CLIENT_TYPE==0
	class Delegate : public virtual CefBase {
	public:
		virtual void OnRenderThreadCreated(CefRefPtr<ofxCEFClientApp> app,
			CefRefPtr<CefListValue> extra_info) {}

		virtual void OnWebKitInitialized(CefRefPtr<ofxCEFClientApp> app) {}

		virtual void OnBrowserCreated(CefRefPtr<ofxCEFClientApp> app,
			CefRefPtr<CefBrowser> browser) {}

		virtual void OnBrowserDestroyed(CefRefPtr<ofxCEFClientApp> app,
			CefRefPtr<CefBrowser> browser) {}

		virtual CefRefPtr<CefLoadHandler> GetLoadHandler(
			CefRefPtr<ofxCEFClientApp> app) {
			return NULL;
		}

		virtual bool OnBeforeNavigation(CefRefPtr<ofxCEFClientApp> app,
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefRequest> request,
			cef_navigation_type_t navigation_type,
			bool is_redirect) {
			return false;
		}

		virtual void OnContextCreated(CefRefPtr<ofxCEFClientApp> app,
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefV8Context> context) {}

		virtual void OnContextReleased(CefRefPtr<ofxCEFClientApp> app,
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefV8Context> context) {}

		virtual void OnUncaughtException(CefRefPtr<ofxCEFClientApp> app,
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefV8Context> context,
			CefRefPtr<CefV8Exception> exception,
			CefRefPtr<CefV8StackTrace> stackTrace) {}

		virtual void OnFocusedNodeChanged(CefRefPtr<ofxCEFClientApp> app,
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefDOMNode> node) {}

		// Called when a process message is received. Return true if the message was
		// handled and should not be passed on to other handlers. Delegates
		// should check for unique message names to avoid interfering with each
		// other.
		virtual bool OnProcessMessageReceived(
			CefRefPtr<ofxCEFClientApp> app,
			CefRefPtr<CefBrowser> browser,
			CefProcessId source_process,
			CefRefPtr<CefProcessMessage> message) {
			return false;
		}
	};
	typedef std::set<CefRefPtr<Delegate> > DelegateSet;
#else
	// Interface for browser delegates. All Delegates must be returned via
	// CreateDelegates. Do not perform work in the Delegate
	// constructor. See CefBrowserProcessHandler for documentation.
	class Delegate : public virtual CefBase {
	public:
		virtual void OnBeforeCommandLineProcessing(
			CefRefPtr<ofxCEFClientApp> app,
			CefRefPtr<CefCommandLine> command_line) {}

		virtual void OnContextInitialized(CefRefPtr<ofxCEFClientApp> app) {}

		virtual void OnBeforeChildProcessLaunch(
			CefRefPtr<ofxCEFClientApp> app,
			CefRefPtr<CefCommandLine> command_line) {}

		virtual void OnRenderProcessThreadCreated(
			CefRefPtr<ofxCEFClientApp> app,
			CefRefPtr<CefListValue> extra_info) {}
	};
	typedef std::set<CefRefPtr<Delegate> > DelegateSet;
#endif


public:
	js_callback* call_back;
	CefRefPtr<CefV8Context> my_context;
	CefRefPtr<CefBrowser>	my_browser;
	CefRefPtr<CefFrame>		my_frame;

	
	ofxCEFClientApp();

#if CLIENT_TYPE==0
	// CefRenderProcessHandler methods.
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE
    {
        return this;
    }
	void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE;
	void OnWebKitInitialized() OVERRIDE;
	void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) OVERRIDE;
	CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE;
	bool OnBeforeNavigation(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		NavigationType navigation_type,
		bool is_redirect) OVERRIDE;
	void OnContextCreated(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefV8Context> context) OVERRIDE;
	void OnContextReleased(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefV8Context> context) OVERRIDE;
	void OnUncaughtException(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefV8Context> context,
		CefRefPtr<CefV8Exception> exception,
		CefRefPtr<CefV8StackTrace> stackTrace) OVERRIDE;
	void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefDOMNode> node) OVERRIDE;
	bool OnProcessMessageReceived(
		CefRefPtr<CefBrowser> browser,
		CefProcessId source_process,
		CefRefPtr<CefProcessMessage> message) OVERRIDE;
#else
// CefBrowserProcessHandler methods.
	CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE {
		return this;
	}
	void OnContextInitialized() OVERRIDE;
	void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line) OVERRIDE;
	void OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE;
	CefRefPtr<CefPrintHandler> GetPrintHandler() OVERRIDE {
		return print_handler_;
	}
#endif

	// app IF
	virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line);

	DelegateSet delegates_;
	CefRefPtr<CefPrintHandler> print_handler_;

    IMPLEMENT_REFCOUNTING(ofxCEFClientApp);
};

#endif
