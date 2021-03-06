/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <AACE/Audio/AudioManager.h>

#include "GstMediaPlayer.h"
#include "GstUtils.h"
#include "GstAudioCapture.h"
#include "GstPlayer.h"

#ifdef USE_AGL_FRAMEWORK
#include "agl/AGLAudioManager.h"
#endif

namespace aace {
namespace audio {

struct AudioManager::Impl {
#ifdef USE_AGL_FRAMEWORK
	std::unique_ptr<AGLAudioManager> mgr;
#endif
};

std::unique_ptr<AudioManager> AudioManager::create(void *platformData)
{
	GstUtils::initializeGStreamer();

	std::unique_ptr<AudioManager> impl(
		new AudioManager());
	if (!impl->init(platformData))
		return nullptr;

	return impl;
}

AudioManager::AudioManager() : m_impl(new AudioManager::Impl) {}

AudioManager::~AudioManager() = default;

bool AudioManager::init(void *platformData)
{
#ifdef USE_AGL_FRAMEWORK
	m_impl->mgr = std::unique_ptr<AGLAudioManager>(new AGLAudioManager((afb_api_t) platformData));
#endif
	return true;
}

AudioOutputChannel AudioManager::openOutputChannel(const std::string &name, const std::string &device)
{
#ifdef USE_AGL_FRAMEWORK
	// Request ALSA device
	auto roleDevice = m_impl->mgr->openAHLChannel(device);

	if (roleDevice.empty())
		return {
			nullptr,
			nullptr
		};

	std::shared_ptr<GstPlayer> player = GstPlayer::create(name, roleDevice);
#else
	std::shared_ptr<GstPlayer> player = GstPlayer::create(name, device);
#endif
	std::shared_ptr<GstMediaPlayer> mediaPlayer = GstMediaPlayer::create(name, player);

	return {
		mediaPlayer,
		mediaPlayer
	};
}

AudioInputChannel AudioManager::openInputChannel(const std::string &name, const std::string &device) {
	std::shared_ptr<GstAudioCapture> audioCapture
		= GstAudioCapture::create(name, device);

	return {
		audioCapture
	};
}

}
}