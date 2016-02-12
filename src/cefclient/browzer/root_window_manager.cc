// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "./root_window_manager.h"

#include <sstream>

#define __DISABLE_TEST__  1

#include "include/base/cef_bind.h"
#include "include/base/cef_logging.h"
#include "include/wrapper/cef_helpers.h"
#include "./main_context.h"
#if __DISABLE_TEST__!=1
#include "./test_runner.h"
#endif
#include "../common/client_switches.h"

namespace client {

namespace {

class ClientRequestContextHandler : public CefRequestContextHandler {
 public:
  ClientRequestContextHandler() {}

  bool OnBeforePluginLoad(const CefString& mime_type,
                          const CefString& plugin_url,
                          const CefString& top_origin_url,
                          CefRefPtr<CefWebPluginInfo> plugin_info,
                          PluginPolicy* plugin_policy) OVERRIDE {
    // Always allow the PDF plugin to load.
    if (*plugin_policy != PLUGIN_POLICY_ALLOW &&
        mime_type == "application/pdf") {
      *plugin_policy = PLUGIN_POLICY_ALLOW;
      return true;
    }

    return false;
  }

 private:
  IMPLEMENT_REFCOUNTING(ClientRequestContextHandler);
};

}  // namespace

RootWindowManager::RootWindowManager(bool terminate_when_all_windows_closed)
    : terminate_when_all_windows_closed_(terminate_when_all_windows_closed) {
  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();
  DCHECK(command_line.get());
  request_context_per_browser_ =
      command_line->HasSwitch(switches::kRequestContextPerBrowser);
}

RootWindowManager::~RootWindowManager() {
  // All root windows should already have been destroyed.
  DCHECK(root_windows_.empty());
}

scoped_refptr<RootWindow> RootWindowManager::CreateRootWindow(
    bool with_controls,
    bool with_osr,
    const CefRect& bounds,
    const std::string& url) {

  return scoped_refptr<RootWindow>();
/*

  CefBrowserSettings settings;
  MainContext::Get()->PopulateBrowserSettings(&settings);

  scoped_refptr<RootWindow> root_window = RootWindow::Create();
  root_window->Init(this, with_controls, with_osr, bounds, settings,
                    url.empty() ? MainContext::Get()->GetMainURL() : url);

  // Store a reference to the root window on the main thread.
  OnRootWindowCreated(root_window);

  return root_window;
*/
}

scoped_refptr<RootWindow> RootWindowManager::CreateRootWindowAsPopup(
    bool with_controls,
    bool with_osr,
    const CefPopupFeatures& popupFeatures,
    CefWindowInfo& windowInfo,
    CefRefPtr<CefClient>& client,
    CefBrowserSettings& settings) {

  return scoped_refptr<RootWindow>();
/*
  MainContext::Get()->PopulateBrowserSettings(&settings);

  scoped_refptr<RootWindow> root_window = RootWindow::Create();
  root_window->InitAsPopup(this, with_controls, with_osr,
                           popupFeatures, windowInfo, client, settings);

  // Store a reference to the root window on the main thread.
  OnRootWindowCreated(root_window);

  return root_window;
*/
}

scoped_refptr<RootWindow> RootWindowManager::GetWindowForBrowser(
    int browser_id) {
  REQUIRE_MAIN_THREAD();

  RootWindowSet::const_iterator it = root_windows_.begin();
  for (; it != root_windows_.end(); ++it) {
    CefRefPtr<CefBrowser> browser = (*it)->GetBrowser();
    if (browser.get() && browser->GetIdentifier() == browser_id)
      return *it;
  }
  return NULL;
}

void RootWindowManager::CloseAllWindows(bool force) {
  if (!CURRENTLY_ON_MAIN_THREAD()) {
    // Execute this method on the main thread.
    MAIN_POST_CLOSURE(
        base::Bind(&RootWindowManager::CloseAllWindows, base::Unretained(this),
                  force));
    return;
  }

  if (root_windows_.empty())
    return;

  RootWindowSet::const_iterator it = root_windows_.begin();
  for (; it != root_windows_.end(); ++it)
    (*it)->Close(force);
}

void RootWindowManager::OnRootWindowCreated(
    scoped_refptr<RootWindow> root_window) {
  if (!CURRENTLY_ON_MAIN_THREAD()) {
    // Execute this method on the main thread.
    MAIN_POST_CLOSURE(
        base::Bind(&RootWindowManager::OnRootWindowCreated,
                   base::Unretained(this), root_window));
    return;
  }

  root_windows_.insert(root_window);
}

CefRefPtr<CefRequestContext> RootWindowManager::GetRequestContext(
    RootWindow* root_window) {
  REQUIRE_MAIN_THREAD();

  if (request_context_per_browser_) {
    // Create a new request context for each browser.
    CefRequestContextSettings settings;

    CefRefPtr<CefCommandLine> command_line =
        CefCommandLine::GetGlobalCommandLine();
    if (command_line->HasSwitch(switches::kCachePath)) {
      // If a global cache path value is specified then give each browser a
      // unique cache path.
      std::stringstream ss;
      ss << command_line->GetSwitchValue(switches::kCachePath).ToString() <<
          time(NULL);
      CefString(&settings.cache_path) = ss.str();
    }

    return CefRequestContext::CreateContext(settings,
                                            new ClientRequestContextHandler);
  }

  // All browsers will share the global request context.
  if (!shared_request_context_.get()) {
    shared_request_context_ =
        CefRequestContext::CreateContext(CefRequestContext::GetGlobalContext(),
                                         new ClientRequestContextHandler);
  }
  return shared_request_context_;
}

void RootWindowManager::OnTest(RootWindow* root_window, int test_id) {
  REQUIRE_MAIN_THREAD();

#if __DISABLE_TEST__!=1
  test_runner::RunTest(root_window->GetBrowser(), test_id);
#endif
}

void RootWindowManager::OnExit(RootWindow* root_window) {
  REQUIRE_MAIN_THREAD();

  CloseAllWindows(false);
}

void RootWindowManager::OnRootWindowDestroyed(RootWindow* root_window) {
  REQUIRE_MAIN_THREAD();

  RootWindowSet::iterator it = root_windows_.find(root_window);
  DCHECK(it != root_windows_.end());
  if (it != root_windows_.end())
    root_windows_.erase(it);

  if (terminate_when_all_windows_closed_ && root_windows_.empty()) {
    // Quit the main message loop after all windows have closed.
    MainMessageLoop::Get()->Quit();
  }
}

}  // namespace client
