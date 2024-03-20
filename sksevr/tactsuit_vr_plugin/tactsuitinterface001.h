#pragma once
#include "skse64/PluginAPI.h"

namespace tactsuitPluginApi {
	// Returns an ITactsuitInterface001 object compatible with the API shown below
	// This should only be called after SKSE sends kMessage_PostLoad to your plugin
	class ITactsuitInterface001;
	ITactsuitInterface001* getTactsuitInterface001(const PluginHandle & pluginHandle, SKSEMessagingInterface * messagingInterface);

	// This object provides access to Tactsuit VR's mod support API
	class ITactsuitInterface001
	{
	public:
		// Gets the build number
		virtual unsigned int getBuildNumber() = 0;

		//Plays haptic effect once. effectFileName without extension.
		virtual void PlayHapticEffect(std::string effectFileName, float locationAngle, float locationHeight, float intensity, bool waitToPlay) = 0;

		//Stops haptic effect. effectFileName without extension.
		virtual void StopHapticEffect(std::string effectFileName) = 0;
	};
}

extern tactsuitPluginApi::ITactsuitInterface001* g_tactsuitInterface;
