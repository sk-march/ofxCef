#include "ofxCEF.h"
#include "ofMain.h"
#include "ofAppGLFWWindow.h"
#if defined(TARGET_OSX)
#include <Cocoa/Cocoa.h>
#endif
#include "ofxCEFClientApp.h"

#ifdef __linux
#include <wrapper/cef_helpers.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#endif


#ifdef _MSC_VER
#include <cef_sandbox_win.h>
//#pragma comment(lib, "cef_sandbox.lib")
#endif

//--------------------------------------------------------------
//--------------------------------------------------------------
#if defined(TARGET_OSX)
@interface NSNotificationManager: NSObject
{
	ofxCEF *observer;
notificationHandler callback;
}

-(void)onChangeScreen:(id)sender;

@end

@implementation NSNotificationManager

- (id)initWithObserver:(ofxCEF *)obs method : (notificationHandler)m;
{
	self = [super init];
	if (self) {
		observer = obs;
		callback = m;

		NSWindow * cocoaWindow = (NSWindow *)((ofAppGLFWWindow *)ofGetWindowPtr())->getCocoaWindow();
		[[NSNotificationCenter defaultCenter] addObserver:self
			selector : @selector(onChangeScreen : ) name:NSWindowDidChangeScreenNotification
					   object : cocoaWindow];
	}
	return self;
}

-(void)onChangeScreen:(id)sender
{
	(observer->*callback)();
}

@end
#endif

//--------------------------------------------------------------
//--------------------------------------------------------------
#if defined(TARGET_WIN32)
HINSTANCE hInst;   // current instance
#endif




void initofxCEF(int argc, char** argv, js_callback_handler* call_back)
{
#if defined(TARGET_OSX) 
	CefMainArgs main_args(argc, argv);
#elif defined(__linux) 
	CefMainArgs main_args(argc, argv);
#elif defined(TARGET_WIN32)
	CefMainArgs main_args(::GetModuleHandle(NULL));
#endif

	CefRefPtr<ofxCEFClientApp> app(new ofxCEFClientApp);
	if(call_back==NULL) app->call_back = &app->def_call_back;
	else app->call_back = call_back;

	int exit_code = CefExecuteProcess(main_args, app.get(), NULL);
	if (exit_code >= 0) {
		//return exit_code;
	}


#if defined(__linux)
	CefScopedArgArray scoped_arg_array(argc, argv);
	char** argv_copy = scoped_arg_array.array();
	gtk_init(&argc, &argv_copy);
	gtk_gl_init(&argc, &argv_copy);
#endif

	CefSettings settings;
	settings.background_color = 0xFFFF00FF;
	settings.single_process = true;// false;
	settings.windowless_rendering_enabled = true;
	settings.command_line_args_disabled = true;
	//settings.multi_threaded_message_loop = true;

	void* sandbox_info_ = NULL;// cef_sandbox_info_create();
	CefInitialize(main_args, settings, app.get(), sandbox_info_);
	//	cef_sandbox_info_destroy(sandbox_info_);
	/*
	#ifdef _MSC_VER
	CefAddWebPluginDirectory("C:\\Windows\\SysWOW64\\Macromed\\Flash");
	CefAddWebPluginDirectory("C:\\Users\\tko\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Extensions");
	//	CefAddWebPluginDirectory("%HOMEPATH%/AppData/Local/Google/Chrome/User Data/Default/Extensions");
	#elif defined(TARGET_OSX)
	CefAddWebPluginDirectory("~/Library/Application Support/Google/Chrome/Default/Extensions");
	#elif defined(__linux)
	//	CefAddWebPluginDirectory("~/.config/chromium/Default/Extensions");
	CefAddWebPluginDirectory("~/.config/google-chrome/Default/Extensions/");
	#endif
	CefRefreshWebPlugins();
	*/
	return;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void ofxCEF::setup(uint32_t w, uint32_t h) {

	CefWindowInfo windowInfo;
	renderHandler = new ofxCEFRenderHandler();

#if defined(TARGET_OSX) 
	NSWindow * cocoaWindow = (NSWindow *)((ofAppGLFWWindow *)ofGetWindowPtr())->getCocoaWindow();
	[cocoaWindow setReleasedWhenClosed : NO];

	NSView * view = [cocoaWindow contentView];
	windowInfo.SetAsWindowless(view, true);

	if (renderHandler->bIsRetinaDisplay) {
		ofSetWindowPosition(0, 0);
		ofSetWindowShape(ofGetWidth(), ofGetHeight());
	}
#elif defined(__linux)
	//	GtkWidget* window = NULL;
	//	GtkWidget* window = gtk_widget_get_ancestor(
	//		GTK_WIDGET(parent_handle), GTK_TYPE_WINDOW);
	//	::Window xwindow = GDK_WINDOW_XID(gtk_widget_get_window(window));
	windowInfo.SetAsWindowless(0L, true);
	windowInfo.width = w;
	windowInfo.height = h;
	windowInfo.window = 0L;
#elif defined(TARGET_WIN32)
	// Create the native window with a border so it's easier to visually identify
	// OSR windows.
	//	HWND hWnd = ::CreateWindow(L"STATIC", L"ofxCEF",
	//		WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_SIZEBOX,
	//		0, 0, w, h,
	//		NULL, 0, ::GetModuleHandle(NULL), 0);
	HWND hWnd = NULL;

	// HWND hWnd = NULL;// ofGetWin32Window();
	windowInfo.SetAsWindowless(hWnd, true);
	windowInfo.width = w;
	windowInfo.height = h;
	windowInfo.window = hWnd;
#endif

	windowInfo.transparent_painting_enabled = STATE_ENABLED;

	CefBrowserSettings settings;
	settings.web_security = STATE_DISABLED;
	settings.webgl = STATE_ENABLED;
	settings.windowless_frame_rate = 60;
	settings.plugins = STATE_ENABLED;

	client = new ofxCEFBrowserClient(this, renderHandler);
	browser = CefBrowserHost::CreateBrowserSync(windowInfo, client.get(), "", settings, NULL);

#if defined(TARGET_OSX) 
	if (renderHandler->bIsRetinaDisplay) {
		reshape(ofGetWidth() * 2, ofGetHeight() * 2);
	}
#endif

	enableEvents();
}

//--------------------------------------------------------------
ofxCEF::ofxCEF() {
}

//--------------------------------------------------------------
ofxCEF::~ofxCEF() {
}

void ofxCEF::exit() {
	//TODO Check if we need to do some calls to OnBeforeClose 
	disableEvents();
	renderHandler->bIsShuttingDown = true;
	browser->GetHost()->CloseBrowser(false);

	// The following call to CefShutdown make the app crash on OS X. Still not working on Windows neither.
	//CefShutdown();
}


//--------------------------------------------------------------
void ofxCEF::enableEvents() {
	ofAddListener(ofEvents().mousePressed, this, &ofxCEF::mousePressed);
	ofAddListener(ofEvents().mouseMoved, this, &ofxCEF::mouseMoved);
	ofAddListener(ofEvents().mouseDragged, this, &ofxCEF::mouseDragged);
	ofAddListener(ofEvents().mouseReleased, this, &ofxCEF::mouseReleased);

	ofAddListener(ofEvents().keyPressed, this, &ofxCEF::keyPressed);
	ofAddListener(ofEvents().keyReleased, this, &ofxCEF::keyReleased);

	ofAddListener(ofEvents().windowResized, this, &ofxCEF::windowResized);
}

//--------------------------------------------------------------
void ofxCEF::disableEvents() {
	ofRemoveListener(ofEvents().mousePressed, this, &ofxCEF::mousePressed);
	ofRemoveListener(ofEvents().mouseMoved, this, &ofxCEF::mouseMoved);
	ofRemoveListener(ofEvents().mouseDragged, this, &ofxCEF::mouseDragged);
	ofRemoveListener(ofEvents().mouseReleased, this, &ofxCEF::mouseReleased);

	ofRemoveListener(ofEvents().keyPressed, this, &ofxCEF::keyPressed);
	ofRemoveListener(ofEvents().keyReleased, this, &ofxCEF::keyReleased);

	ofRemoveListener(ofEvents().windowResized, this, &ofxCEF::windowResized);
}

//--------------------------------------------------------------
void ofxCEF::load(const char* url) {
	if (!renderHandler->initialized) {
		renderHandler->init();
	}

	browser->GetMainFrame()->LoadURL(url);
}

void ofxCEF::load(const wchar_t* url) {
	if (!renderHandler->initialized) {
		renderHandler->init();
	}

	browser->GetMainFrame()->LoadURL(url);
}

//--------------------------------------------------------------
void ofxCEF::update() {
	GLint swapbytes, lsbfirst, rowlength, skiprows, skippixels, alignment;
	/* Save current pixel store state. */

	glGetIntegerv(GL_UNPACK_SWAP_BYTES, &swapbytes);
	glGetIntegerv(GL_UNPACK_LSB_FIRST, &lsbfirst);
	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowlength);
	glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skiprows);
	glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skippixels);
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

	CefDoMessageLoopWork();
	//CefRunMessageLoop();

	/* Restore current pixel store state. */
	glPixelStorei(GL_UNPACK_SWAP_BYTES, swapbytes);
	glPixelStorei(GL_UNPACK_LSB_FIRST, lsbfirst);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowlength);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, skiprows);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, skippixels);
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
}

//--------------------------------------------------------------
void ofxCEF::draw(void) {

	//    cout << "ofxCEF::draw "<< endl;
	//    CefDoMessageLoopWork();

	// Alpha blending style. Texture values have premultiplied alpha.
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	// Enable alpha blending.
	glEnable(GL_BLEND);

	//cout << ofGetWidth() << " - " << ofGetHeight() << endl;
	float width = ofGetWidth();
	float height = ofGetHeight();

	ofMesh temp;
	temp.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
	temp.addVertex(ofPoint(0, 0));
	temp.addTexCoord(ofPoint(0, 0));
	temp.addVertex(ofPoint(width, 0));
	temp.addTexCoord(ofPoint(1, 0));
	temp.addVertex(ofPoint(0, height));
	temp.addTexCoord(ofPoint(0, 1));
	temp.addVertex(ofPoint(width, height));
	temp.addTexCoord(ofPoint(1, 1));
	ofPushMatrix();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, renderHandler->texture_id_);
	temp.draw();
	ofPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	ofEnableAlphaBlending();
	renderHandler->bIsDirty = false;
}

//--------------------------------------------------------------
void ofxCEF::onLoadStart() {
	ofxCEFEventArgs evt;
	evt.type = ofxCEFEventArgs::onLoadStart;
	evt.httpStatusCode = -1;

	ofNotifyEvent(eventFromCEF, evt, this);
}

//--------------------------------------------------------------
void ofxCEF::onLoadEnd(int httpStatusCode) {
	ofxCEFEventArgs evt;
	evt.type = ofxCEFEventArgs::onLoadEnd;
	evt.httpStatusCode = httpStatusCode;

	ofNotifyEvent(eventFromCEF, evt, this);
}

//--------------------------------------------------------------
void ofxCEF::gotMessageFromJS(string name, string type, string value) {
	ofxCEFMessageArgs msg;
	msg.type = type;
	msg.name = name;
	msg.value = value;

	ofNotifyEvent(messageFromJS, msg, this);
}

//--------------------------------------------------------------
void ofxCEF::notificationHandler() {

#if defined(TARGET_OSX) 
	float displayScale = [[NSScreen mainScreen] backingScaleFactor];

	cout << " ======= ofxCEF::notificationHandler =========" << endl;
	cout << "OF window size: " << ofGetWidth() << " - " << ofGetHeight() << endl;
	cout << "Changed Screen / displayScale :: " << displayScale << " ::/ frame orig : " << [NSScreen mainScreen].frame.origin.x << " - " << [NSScreen mainScreen].frame.origin.y << " ::/ size " << [NSScreen mainScreen].frame.size.width << " - " << [NSScreen mainScreen].frame.size.height << endl;

	bool scaleChanged = false;

	if (displayScale > 1.0) {
		if (!renderHandler->bIsRetinaDisplay) {
			scaleChanged = true;
		}
		renderHandler->bIsRetinaDisplay = true;

	}
	else {
		if (renderHandler->bIsRetinaDisplay) {
			scaleChanged = true;
		}

		renderHandler->bIsRetinaDisplay = false;
	}

	//    if (scaleChanged) {
	//        if (renderHandler->bIsRetinaDisplay) {
	//            reshape(ofGetWidth()*2, ofGetHeight()*2);
	//        } else{
	//            reshape(ofGetWidth(), ofGetHeight());
	//        }
	//    }
#elif defined(TARGET_WIN32)

#endif

	reshape(ofGetWidth(), ofGetHeight());
	renderHandler->init();
	browser->Reload();
}

//--------------------------------------------------------------
void ofxCEF::reshape(int w, int h) {
	cout << "Reshape: " << w << " - " << h << endl;
	renderHandler->reshape(w, h);
	browser->GetHost()->WasResized();
}

//--------------------------------------------------------------
void ofxCEF::mousePressed(ofMouseEventArgs &e) {
	int x = e.x;
	int y = e.y;

	browser->GetHost()->SendFocusEvent(true);

	if (renderHandler->bIsRetinaDisplay) {
		x /= 2;
		y /= 2;
	}

	// move
	if (mouseX != x || mouseY != y) {
		mouseX = x;
		mouseY = y;
		CefMouseEvent event;
		event.x = x;
		event.y = y;
		browser->GetHost()->SendMouseMoveEvent(event, false);
	}

	CefBrowserHost::MouseButtonType btnType;
	CefMouseEvent event;
	event.x = x;
	event.y = y;
	event.modifiers = 0;
	if (e.button == OF_MOUSE_BUTTON_LEFT) {
		event.modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
		btnType = MBT_LEFT;
	}
	else if (e.button == OF_MOUSE_BUTTON_RIGHT) {
		event.modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
		btnType = MBT_RIGHT;
	}
	else if (e.button == OF_MOUSE_BUTTON_MIDDLE) {
		event.modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
		btnType = MBT_MIDDLE;
	}
	browser->GetHost()->SendMouseClickEvent(event, btnType, false, 1);
}

//--------------------------------------------------------------
void ofxCEF::mouseReleased(ofMouseEventArgs &e) {
	int x = e.x;
	int y = e.y;

	if (renderHandler->bIsRetinaDisplay) {
		x /= 2;
		y /= 2;
	}

	CefBrowserHost::MouseButtonType btnType;
	CefMouseEvent event;
	event.x = x;
	event.y = y;
	event.modifiers = 0;
	if (e.button == OF_MOUSE_BUTTON_LEFT) {
		event.modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
		btnType = MBT_LEFT;
	}
	else if (e.button == OF_MOUSE_BUTTON_RIGHT) {
		event.modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
		btnType = MBT_RIGHT;
	}
	else if (e.button == OF_MOUSE_BUTTON_MIDDLE) {
		event.modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
		btnType = MBT_MIDDLE;
	}
	browser->GetHost()->SendMouseClickEvent(event, btnType, true, 1);

}

//--------------------------------------------------------------
void ofxCEF::mouseMoved(ofMouseEventArgs &e) {
	int x = e.x;
	int y = e.y;

	if (renderHandler->bIsRetinaDisplay) {
		x /= 2;
		y /= 2;
	}

	mouseX = x;
	mouseY = y;

	CefMouseEvent event;
	event.x = x;
	event.y = y;

	browser->GetHost()->SendMouseMoveEvent(event, false);

}

//--------------------------------------------------------------
void ofxCEF::mouseDragged(ofMouseEventArgs &e) {
	mouseMoved(e);
}

//--------------------------------------------------------------
void ofxCEF::focus() {
	browser->GetHost()->SendFocusEvent(true);
}

//--------------------------------------------------------------
void ofxCEF::mouseWheel(int v, int h) {
	CefMouseEvent mouse_event;
	mouse_event.x = mouseX;
	mouse_event.y = mouseY;
	mouse_event.modifiers = 0;
	browser->GetHost()->SendMouseWheelEvent(mouse_event, h, v);

}


//--------------------------------------------------------------
void ofxCEF::mousePressed(int code)
{
	int x = mouseX;
	int y = mouseY;

	browser->GetHost()->SendFocusEvent(true);

	if (renderHandler->bIsRetinaDisplay) {
		x /= 2;
		y /= 2;
	}

	CefBrowserHost::MouseButtonType btnType;
	CefMouseEvent event;
	event.x = x;
	event.y = y;
	event.modifiers = 0;
	if (code == OF_MOUSE_BUTTON_LEFT) {
		event.modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
		btnType = MBT_LEFT;
	}
	else if (code == OF_MOUSE_BUTTON_RIGHT) {
		event.modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
		btnType = MBT_RIGHT;
	}
	else if (code == OF_MOUSE_BUTTON_MIDDLE) {
		event.modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
		btnType = MBT_MIDDLE;
	}
	browser->GetHost()->SendMouseClickEvent(event, btnType, false, 1);
}

//--------------------------------------------------------------
void ofxCEF::mouseReleased(int code)
{
	int x = mouseX;
	int y = mouseY;

	if (renderHandler->bIsRetinaDisplay) {
		x /= 2;
		y /= 2;
	}

	CefBrowserHost::MouseButtonType btnType;
	CefMouseEvent event;
	event.x = x;
	event.y = y;
	event.modifiers = 0;
	if (code == OF_MOUSE_BUTTON_LEFT) {
		event.modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
		btnType = MBT_LEFT;
	}
	else if (code == OF_MOUSE_BUTTON_RIGHT) {
		event.modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
		btnType = MBT_RIGHT;
	}
	else if (code == OF_MOUSE_BUTTON_MIDDLE) {
		event.modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
		btnType = MBT_MIDDLE;
	}
	browser->GetHost()->SendMouseClickEvent(event, btnType, true, 1);
}

//--------------------------------------------------------------
void ofxCEF::mouseMoved(int x, int y)
{
	if (renderHandler->bIsRetinaDisplay) {
		x /= 2;
		y /= 2;
	}

	mouseX = x;
	mouseY = y;

	CefMouseEvent event;
	event.x = x;
	event.y = y;

	browser->GetHost()->SendMouseMoveEvent(event, false);
}

//--------------------------------------------------------------
void ofxCEF::keyPutChar(char text)
{
#if defined(OS_LINUX)
	unsigned int native_key_code;
#endif
	int key_code = text;

	CefKeyEvent kevent;
	kevent.is_system_key = false;
	kevent.modifiers = 0;

#if defined(OS_WIN)
	BYTE VkCode = LOBYTE(VkKeyScanA(key_code));
	UINT scanCode = MapVirtualKey(VkCode, MAPVK_VK_TO_VSC);
	kevent.native_key_code = (scanCode << 16) |  // key scan code
		1;  // key repeat count
	kevent.windows_key_code = VkCode;
#elif defined(OS_MACOSX)
	osr_unittests::GetKeyEvent(
		kevent, static_cast<ui::KeyboardCode>(key_code), 0);
#elif defined(OS_LINUX)
	kevent.native_key_code = native_key_code;
	kevent.windows_key_code = key_code;
	kevent.character = kevent.unmodified_character = native_key_code;
#else
	NOTREACHED();
#endif
	kevent.type = KEYEVENT_RAWKEYDOWN;
	browser->GetHost()->SendKeyEvent(kevent);

#if defined(OS_WIN)
	kevent.windows_key_code = key_code;
#endif
	kevent.type = KEYEVENT_CHAR;
	browser->GetHost()->SendKeyEvent(kevent);

#if defined(OS_WIN)
	kevent.windows_key_code = VkCode;
	// bits 30 and 31 should be always 1 for WM_KEYUP
	kevent.native_key_code |= 0xC0000000;
#endif
	kevent.type = KEYEVENT_KEYUP;
	browser->GetHost()->SendKeyEvent(kevent);
}
void ofxCEF::keyPutChar(wchar_t text)
{
#if defined(OS_LINUX)
	unsigned int native_key_code;
#endif
	int key_code = text;

	CefKeyEvent kevent;
	kevent.is_system_key = false;
	kevent.modifiers = 0;

#if defined(OS_WIN)
	BYTE VkCode = LOBYTE(VkKeyScanA(key_code));
	UINT scanCode = MapVirtualKey(VkCode, MAPVK_VK_TO_VSC);
	kevent.native_key_code = (scanCode << 16) |  // key scan code
		1;  // key repeat count
	kevent.windows_key_code = VkCode;
#elif defined(OS_MACOSX)
	osr_unittests::GetKeyEvent(
		kevent, static_cast<ui::KeyboardCode>(key_code), 0);
#elif defined(OS_LINUX)
	kevent.native_key_code = native_key_code;
	kevent.windows_key_code = key_code;
	kevent.character = kevent.unmodified_character = native_key_code;
#else
	NOTREACHED();
#endif
	kevent.type = KEYEVENT_RAWKEYDOWN;
//	browser->GetHost()->SendKeyEvent(kevent);

#if defined(OS_WIN)
	kevent.windows_key_code = key_code;
#endif
	kevent.type = KEYEVENT_CHAR;
	kevent.character = text;
	browser->GetHost()->SendKeyEvent(kevent);

#if defined(OS_WIN)
	kevent.windows_key_code = VkCode;
	// bits 30 and 31 should be always 1 for WM_KEYUP
	kevent.native_key_code |= 0xC0000000;
#endif
	kevent.type = KEYEVENT_KEYUP;
//	browser->GetHost()->SendKeyEvent(kevent);
}


//--------------------------------------------------------------
void ofxCEF::keyPressed(int code)
{
#if defined(OS_LINUX)
	unsigned int native_key_code;
#endif
	int key_code = code;

	CefKeyEvent kevent;
	kevent.is_system_key = false;
	kevent.modifiers = 0;

#if defined(OS_WIN)
	BYTE VkCode = LOBYTE(VkKeyScanA(key_code));
	UINT scanCode = MapVirtualKey(VkCode, MAPVK_VK_TO_VSC);
	kevent.native_key_code = (scanCode << 16) |  // key scan code
		1;  // key repeat count
	kevent.windows_key_code = VkCode;
#elif defined(OS_MACOSX)
	osr_unittests::GetKeyEvent(
		kevent, static_cast<ui::KeyboardCode>(key_code), 0);
#elif defined(OS_LINUX)
	kevent.native_key_code = native_key_code;
	kevent.windows_key_code = key_code;
	kevent.character = kevent.unmodified_character = native_key_code;
#else
	NOTREACHED();
#endif
	kevent.type = KEYEVENT_RAWKEYDOWN;
	browser->GetHost()->SendKeyEvent(kevent);

#if defined(OS_WIN)
	kevent.windows_key_code = key_code;
#endif
	kevent.type = KEYEVENT_CHAR;
	kevent.character = code;
	//	browser->GetHost()->SendKeyEvent(kevent);

#if defined(OS_WIN)
	kevent.windows_key_code = VkCode;
	// bits 30 and 31 should be always 1 for WM_KEYUP
	kevent.native_key_code |= 0xC0000000;
#endif
	kevent.type = KEYEVENT_KEYUP;
	//	browser->GetHost()->SendKeyEvent(kevent);
}

//--------------------------------------------------------------
void ofxCEF::keyReleased(int code)
{
#if defined(OS_LINUX)
	unsigned int native_key_code;
#endif
	int key_code = code;

	CefKeyEvent kevent;
	kevent.is_system_key = false;
	kevent.modifiers = 0;

#if defined(OS_WIN)
	BYTE VkCode = LOBYTE(VkKeyScanA(key_code));
	UINT scanCode = MapVirtualKey(VkCode, MAPVK_VK_TO_VSC);
	kevent.native_key_code = (scanCode << 16) |  // key scan code
		1;  // key repeat count
	kevent.windows_key_code = VkCode;
#elif defined(OS_MACOSX)
	osr_unittests::GetKeyEvent(
		kevent, static_cast<ui::KeyboardCode>(key_code), 0);
#elif defined(OS_LINUX)
	kevent.native_key_code = native_key_code;
	kevent.windows_key_code = key_code;
	kevent.character = kevent.unmodified_character = native_key_code;
#else
	NOTREACHED();
#endif
	kevent.type = KEYEVENT_RAWKEYDOWN;
	//	browser->GetHost()->SendKeyEvent(kevent);

#if defined(OS_WIN)
	kevent.windows_key_code = key_code;
#endif
	kevent.type = KEYEVENT_CHAR;
	//	browser->GetHost()->SendKeyEvent(kevent);

#if defined(OS_WIN)
	kevent.windows_key_code = VkCode;
	// bits 30 and 31 should be always 1 for WM_KEYUP
	kevent.native_key_code |= 0xC0000000;
#endif
	kevent.type = KEYEVENT_KEYUP;
	browser->GetHost()->SendKeyEvent(kevent);
}


//--------------------------------------------------------------
void ofxCEF::keyPressed(ofKeyEventArgs &e) {
	//cout << "KEY:: " << e.key << " - KEYCODE:: " <<  e.keycode << " - SCANCODE::" << e.scancode << endl;

	CefKeyEvent event;

	if (e.key == OF_KEY_LEFT || e.key == OF_KEY_UP
		|| e.key == OF_KEY_RIGHT || e.key == OF_KEY_DOWN
		|| e.key == OF_KEY_BACKSPACE || e.key == OF_KEY_DEL) {

		event.windows_key_code = e.key;
		event.native_key_code = e.scancode;
		event.type = KEYEVENT_KEYDOWN;

	}
	else {
		event.windows_key_code = e.key;
		event.native_key_code = e.scancode;
		event.character = (char)e.key;
		event.type = KEYEVENT_CHAR;

	}

	browser->GetHost()->SendKeyEvent(event);
}

//--------------------------------------------------------------
void ofxCEF::keyReleased(ofKeyEventArgs &e) {
	CefKeyEvent event;

	if (e.key == OF_KEY_LEFT || e.key == OF_KEY_UP
		|| e.key == OF_KEY_RIGHT || e.key == OF_KEY_DOWN
		|| e.key == OF_KEY_BACKSPACE || e.key == OF_KEY_DEL) {

		// Hack - Need to do this otherwise we loose an event.
		event.windows_key_code = e.key;
		event.native_key_code = e.scancode;
		event.character = (char)e.key;
		event.type = KEYEVENT_CHAR;
		browser->GetHost()->SendKeyEvent(event);

	}
	else {
		event.windows_key_code = e.key;
		event.native_key_code = e.scancode;
		event.type = KEYEVENT_KEYUP;
		browser->GetHost()->SendKeyEvent(event);
	}
}

void ofxCEF::keyWinMessageLoop(UINT message, WPARAM wParam, LPARAM lParam)
{
	struct local_tool {
		static bool IsKeyDown(WPARAM wparam) {
			return (GetKeyState(wparam) & 0x8000) != 0;
		}
		static int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam) {
			int modifiers = 0;
			if (IsKeyDown(VK_SHIFT))
				modifiers |= EVENTFLAG_SHIFT_DOWN;
			if (IsKeyDown(VK_CONTROL))
				modifiers |= EVENTFLAG_CONTROL_DOWN;
			if (IsKeyDown(VK_MENU))
				modifiers |= EVENTFLAG_ALT_DOWN;

			// Low bit set from GetKeyState indicates "toggled".
			if (::GetKeyState(VK_NUMLOCK) & 1)
				modifiers |= EVENTFLAG_NUM_LOCK_ON;
			if (::GetKeyState(VK_CAPITAL) & 1)
				modifiers |= EVENTFLAG_CAPS_LOCK_ON;

			switch (wparam) {
			case VK_RETURN:
				if ((lparam >> 16) & KF_EXTENDED)
					modifiers |= EVENTFLAG_IS_KEY_PAD;
				break;
			case VK_INSERT:
			case VK_DELETE:
			case VK_HOME:
			case VK_END:
			case VK_PRIOR:
			case VK_NEXT:
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
				if (!((lparam >> 16) & KF_EXTENDED))
					modifiers |= EVENTFLAG_IS_KEY_PAD;
				break;
			case VK_NUMLOCK:
			case VK_NUMPAD0:
			case VK_NUMPAD1:
			case VK_NUMPAD2:
			case VK_NUMPAD3:
			case VK_NUMPAD4:
			case VK_NUMPAD5:
			case VK_NUMPAD6:
			case VK_NUMPAD7:
			case VK_NUMPAD8:
			case VK_NUMPAD9:
			case VK_DIVIDE:
			case VK_MULTIPLY:
			case VK_SUBTRACT:
			case VK_ADD:
			case VK_DECIMAL:
			case VK_CLEAR:
				modifiers |= EVENTFLAG_IS_KEY_PAD;
				break;
			case VK_SHIFT:
				if (IsKeyDown(VK_LSHIFT))
					modifiers |= EVENTFLAG_IS_LEFT;
				else if (IsKeyDown(VK_RSHIFT))
					modifiers |= EVENTFLAG_IS_RIGHT;
				break;
			case VK_CONTROL:
				if (IsKeyDown(VK_LCONTROL))
					modifiers |= EVENTFLAG_IS_LEFT;
				else if (IsKeyDown(VK_RCONTROL))
					modifiers |= EVENTFLAG_IS_RIGHT;
				break;
			case VK_MENU:
				if (IsKeyDown(VK_LMENU))
					modifiers |= EVENTFLAG_IS_LEFT;
				else if (IsKeyDown(VK_RMENU))
					modifiers |= EVENTFLAG_IS_RIGHT;
				break;
			case VK_LWIN:
				modifiers |= EVENTFLAG_IS_LEFT;
				break;
			case VK_RWIN:
				modifiers |= EVENTFLAG_IS_RIGHT;
				break;
			}
			return modifiers;
		}
	};

	CefKeyEvent event;
	event.windows_key_code = wParam;
	event.native_key_code = lParam;
	event.is_system_key = message == WM_SYSCHAR ||
		message == WM_SYSKEYDOWN ||
		message == WM_SYSKEYUP;

	if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
		event.type = KEYEVENT_RAWKEYDOWN;
	else if (message == WM_KEYUP || message == WM_SYSKEYUP)
		event.type = KEYEVENT_KEYUP;
	else
		event.type = KEYEVENT_CHAR;
	event.modifiers = local_tool::GetCefKeyboardModifiers(wParam, lParam);

	browser->GetHost()->SendKeyEvent(event);
}


//--------------------------------------------------------------
void ofxCEF::windowResized(ofResizeEventArgs &e) {
	reshape(e.width, e.height);
	renderHandler->init();
	//cefgui->browser->Reload();
}


//--------------------------------------------------------------
void ofxCEF::executeJS(const char* command) {
	CefRefPtr<CefFrame> frame = browser->GetMainFrame();
	frame->ExecuteJavaScript(command, frame->GetURL(), 0);

	// TODO limit frequency of texture updating
	CefRect rect;
	renderHandler->GetViewRect(browser, rect);
	browser->GetHost()->Invalidate(PET_VIEW);
}
