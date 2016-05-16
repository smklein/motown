// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_APPLICATION_LIB_SERVICE_REGISTRY_H_
#define MOJO_PUBLIC_CPP_APPLICATION_LIB_SERVICE_REGISTRY_H_

#include <string>

#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/connection_context.h"
#include "mojo/public/cpp/application/lib/service_connector_registry.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/public/interfaces/application/service_provider.mojom.h"

namespace mojo {
namespace internal {

// A ServiceRegistry represents each half of a connection between two
// applications, allowing customization of which services are published to the
// other.
class ServiceRegistry : public ServiceProvider, public ApplicationConnection {
 public:
  ServiceRegistry();
  ServiceRegistry(const ConnectionContext& connection_context,
                  InterfaceRequest<ServiceProvider> local_services);
  ~ServiceRegistry() override;

  // ApplicationConnection overrides.
  void SetServiceConnectorForName(ServiceConnector* service_connector,
                                  const std::string& interface_name) override;
  const ConnectionContext& GetConnectionContext() const override;
  const std::string& GetConnectionURL() override;
  const std::string& GetRemoteApplicationURL() override;

  void RemoveServiceConnectorForName(const std::string& interface_name);

 private:
  // ServiceProvider method.
  void ConnectToService(const String& service_name,
                        ScopedMessagePipeHandle client_handle) override;

  ConnectionContext connection_context_;
  Binding<ServiceProvider> local_binding_;
  ServiceConnectorRegistry service_connector_registry_;

  MOJO_DISALLOW_COPY_AND_ASSIGN(ServiceRegistry);
};

}  // namespace internal
}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_APPLICATION_LIB_SERVICE_REGISTRY_H_
