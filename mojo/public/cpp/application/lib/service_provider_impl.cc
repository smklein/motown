// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/application/service_provider_impl.h"

#include "mojo/public/cpp/application/service_connector.h"
#include "mojo/public/cpp/environment/logging.h"

namespace mojo {

ServiceProviderImpl::ServiceProviderImpl()
    : binding_(this), fallback_service_provider_(nullptr) {
}

ServiceProviderImpl::ServiceProviderImpl(
    const ConnectionContext& connection_context,
    InterfaceRequest<ServiceProvider> service_provider_request)
    : connection_context_(connection_context),
      binding_(this, service_provider_request.Pass()),
      fallback_service_provider_(nullptr) {}

ServiceProviderImpl::~ServiceProviderImpl() {}

void ServiceProviderImpl::Bind(
    const ConnectionContext& connection_context,
    InterfaceRequest<ServiceProvider> service_provider_request) {
  connection_context_ = connection_context;
  binding_.Bind(service_provider_request.Pass());
}

void ServiceProviderImpl::Close() {
  if (binding_.is_bound()) {
    binding_.Close();
    connection_context_ = ConnectionContext();
  }
}

void ServiceProviderImpl::ConnectToService(
    const String& service_name,
    ScopedMessagePipeHandle client_handle) {
  bool service_found = service_connector_registry_.ConnectToService(
      connection_context_, service_name, &client_handle);
  if (!service_found && fallback_service_provider_) {
    fallback_service_provider_->ConnectToService(service_name,
                                                 client_handle.Pass());
  }
}

}  // namespace mojo
