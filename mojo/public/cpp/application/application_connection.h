// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_APPLICATION_APPLICATION_CONNECTION_H_
#define MOJO_PUBLIC_APPLICATION_APPLICATION_CONNECTION_H_

#include <string>

#include "mojo/public/cpp/application/lib/interface_factory_connector.h"
#include "mojo/public/interfaces/application/service_provider.mojom.h"

namespace mojo {

struct ConnectionContext;
class ServiceConnector;

// Represents a connection to another application. An instance of this class is
// passed to ApplicationDelegate's ConfigureIncomingConnection() method each
// time a connection is made to this app, and is returned by the
// ApplicationDelegate's ConnectToApplication() method when this app
// connects to another.
//
// To use, define a class that implements your specific service API (e.g.,
// FooImpl to implement a service named Foo). Then implement an
// InterfaceFactory<Foo> that binds instances of FooImpl to
// InterfaceRequest<Foo>s and register that on the connection like this:
//
//   connection->AddService(&factory);
//
// Or, if you have multiple factories implemented by the same type, explicitly
// specify the interface to register the factory for:
//
//   connection->AddService<Foo>(&my_foo_and_bar_factory_);
//   connection->AddService<Bar>(&my_foo_and_bar_factory_);
//
// The InterfaceFactory must outlive the ApplicationConnection.
//
// TODO(vtl): Don't get too attached to this class. I'm going to remove it.
class ApplicationConnection {
 public:
  virtual ~ApplicationConnection();

  // Makes Interface available as a service to the remote application.
  // |factory| will create implementations of Interface on demand.
  template <typename Interface>
  void AddService(InterfaceFactory<Interface>* factory) {
    SetServiceConnectorForName(
        new internal::InterfaceFactoryConnector<Interface>(factory),
        Interface::Name_);
  }

  virtual const ConnectionContext& GetConnectionContext() const = 0;

  // Returns the URL that was used by the source application to establish a
  // connection to the destination application.
  //
  // When ApplicationConnection is representing an incoming connection this can
  // be different than the URL the application was initially loaded from, if the
  // application handles multiple URLs. Note that this is the URL after all
  // URL rewriting and HTTP redirects have been performed.
  //
  // When ApplicationConnection is representing and outgoing connection, this
  // will be the same as the value returned by GetRemoveApplicationURL().
  virtual const std::string& GetConnectionURL() = 0;

  // Returns the URL identifying the remote application on this connection.
  virtual const std::string& GetRemoteApplicationURL() = 0;

 private:
  virtual void SetServiceConnectorForName(ServiceConnector* service_connector,
                                          const std::string& name) = 0;
};

}  // namespace mojo

#endif  // MOJO_PUBLIC_APPLICATION_APPLICATION_CONNECTION_H_
