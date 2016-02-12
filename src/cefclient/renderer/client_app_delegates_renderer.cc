// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "./client_app_renderer.h"
#include "./client_renderer.h"
#define __DISABLE_TEST__ 1
#if __DISABLE_TEST__ != 1
#include "./performance_test.h"
#endif

namespace client {

// static
void ClientAppRenderer::CreateDelegates(DelegateSet& delegates) {
  renderer::CreateDelegates(delegates);
#if __DISABLE_TEST__ != 1
  performance_test::CreateDelegates(delegates);
#endif
}

}  // namespace client
