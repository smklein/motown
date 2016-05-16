// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_MEDIA_FACTORY_FACTORY_SERVICE_H_
#define MOJO_SERVICES_MEDIA_FACTORY_FACTORY_SERVICE_H_

#include <unordered_set>

#include "mojo/common/binding_set.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/services/media/control/interfaces/media_factory.mojom.h"

namespace mojo {
namespace media {

class MediaFactoryService : public ApplicationDelegate,
                            public InterfaceFactory<MediaFactory>,
                            public MediaFactory {
 public:
  // Provides common behavior for all objects created by the factory service.
  class ProductBase : public std::enable_shared_from_this<ProductBase> {
   public:
    virtual ~ProductBase();

   protected:
    ProductBase(MediaFactoryService* owner);

    // Returns the ApplicationImpl.
    ApplicationImpl* app() {
      DCHECK(owner_->app_);
      return owner_->app_;
    }

    // Tells the factory service to release this product.
    void ReleaseFromOwner() {
      size_t erased = owner_->products_.erase(shared_from_this());
      DCHECK(erased);
    }

   private:
    MediaFactoryService* owner_;
  };

  template <typename Interface>
  class Product : public ProductBase {
   public:
    virtual ~Product() {}

   protected:
    Product(Interface* impl,
            InterfaceRequest<Interface> request,
            MediaFactoryService* owner)
        : ProductBase(owner), binding_(impl, request.Pass()) {
      DCHECK(impl);
      binding_.set_connection_error_handler([this]() { ReleaseFromOwner(); });
    }

    // Closes the binding and calls ReleaseFromOwner.
    void UnbindAndReleaseFromOwner() {
      if (binding_.is_bound()) {
        binding_.Close();
      }

      ReleaseFromOwner();
    }

   private:
    Binding<Interface> binding_;
  };

  MediaFactoryService();

  ~MediaFactoryService() override;

  // ApplicationDelegate implementation.
  void Initialize(ApplicationImpl* app) override;

  bool ConfigureIncomingConnection(ApplicationConnection* connection) override;

  // InterfaceFactory<MediaFactory> implementation.
  void Create(const ConnectionContext& connection_context,
              InterfaceRequest<MediaFactory> request) override;

  // MediaFactory implementation.
  void CreatePlayer(InterfaceHandle<SeekingReader> reader,
                    InterfaceRequest<MediaPlayer> player) override;

  void CreateSource(InterfaceHandle<SeekingReader> reader,
                    Array<MediaTypeSetPtr> allowed_media_types,
                    InterfaceRequest<MediaSource> source) override;

  void CreateSink(const String& destination_url,
                  MediaTypePtr media_type,
                  InterfaceRequest<MediaSink> sink) override;

  void CreateDemux(InterfaceHandle<SeekingReader> reader,
                   InterfaceRequest<MediaDemux> demux) override;

  void CreateDecoder(MediaTypePtr input_media_type,
                     InterfaceRequest<MediaTypeConverter> decoder) override;

  void CreateNetworkReader(const String& url,
                           InterfaceRequest<SeekingReader> reader) override;

 private:
  BindingSet<MediaFactory> bindings_;
  ApplicationImpl* app_;
  std::unordered_set<std::shared_ptr<ProductBase>> products_;
};

// For use by products when handling mojo requests.
// Checks the condition, and, if it's false, unbinds, releases from the owner
// and calls return. Doesn't support stream arguments.
// TODO(dalesat): Support stream arguments.
#define RCHECK(condition)                                         \
  if (!(condition)) {                                             \
    LOG(ERROR) << "request precondition failed: " #condition "."; \
    UnbindAndReleaseFromOwner();                                  \
    return;                                                       \
  }

}  // namespace media
}  // namespace mojo

#endif  // MOJO_SERVICES_MEDIA_FACTORY_FACTORY_SERVICE_H_
