// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/media/factory_service/factory_service.h"
#include "services/media/factory_service/media_decoder_impl.h"
#include "services/media/factory_service/media_demux_impl.h"
#include "services/media/factory_service/media_player_impl.h"
#include "services/media/factory_service/media_sink_impl.h"
#include "services/media/factory_service/media_source_impl.h"
#include "services/media/factory_service/network_reader_impl.h"

namespace mojo {
namespace media {

MediaFactoryService::ProductBase::ProductBase(MediaFactoryService* owner)
    : owner_(owner) {
  DCHECK(owner_);
}

MediaFactoryService::ProductBase::~ProductBase() {}

MediaFactoryService::MediaFactoryService() {}

MediaFactoryService::~MediaFactoryService() {}

void MediaFactoryService::Initialize(ApplicationImpl* app) {
  app_ = app;
}

bool MediaFactoryService::ConfigureIncomingConnection(
    ServiceProviderImpl* service_provider_impl) {
  service_provider_impl->AddService<MediaFactory>(
      [this](const ConnectionContext& connection_context,
             InterfaceRequest<MediaFactory> media_factory_request) {
        bindings_.AddBinding(this, media_factory_request.Pass());
      });
  return true;
}

void MediaFactoryService::CreatePlayer(InterfaceHandle<SeekingReader> reader,
                                       InterfaceRequest<MediaPlayer> player) {
  products_.insert(std::static_pointer_cast<ProductBase>(
      MediaPlayerImpl::Create(reader.Pass(), player.Pass(), this)));
}

void MediaFactoryService::CreateSource(InterfaceHandle<SeekingReader> reader,
                                       Array<MediaTypeSetPtr> media_types,
                                       InterfaceRequest<MediaSource> source) {
  products_.insert(
      std::static_pointer_cast<ProductBase>(MediaSourceImpl::Create(
          reader.Pass(), media_types, source.Pass(), this)));
}

void MediaFactoryService::CreateSink(const String& destination_url,
                                     MediaTypePtr media_type,
                                     InterfaceRequest<MediaSink> sink) {
  products_.insert(std::static_pointer_cast<ProductBase>(MediaSinkImpl::Create(
      destination_url, media_type.Pass(), sink.Pass(), this)));
}

void MediaFactoryService::CreateDemux(InterfaceHandle<SeekingReader> reader,
                                      InterfaceRequest<MediaDemux> demux) {
  products_.insert(std::static_pointer_cast<ProductBase>(
      MediaDemuxImpl::Create(reader.Pass(), demux.Pass(), this)));
}

void MediaFactoryService::CreateDecoder(
    MediaTypePtr input_media_type,
    InterfaceRequest<MediaTypeConverter> decoder) {
  products_.insert(std::static_pointer_cast<ProductBase>(
      MediaDecoderImpl::Create(input_media_type.Pass(), decoder.Pass(), this)));
}

void MediaFactoryService::CreateNetworkReader(
    const String& url,
    InterfaceRequest<SeekingReader> reader) {
  products_.insert(std::static_pointer_cast<ProductBase>(
      NetworkReaderImpl::Create(url, reader.Pass(), this)));
}

}  // namespace media
}  // namespace mojo
