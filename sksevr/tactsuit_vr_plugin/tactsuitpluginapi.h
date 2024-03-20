#pragma once
#include "tactsuitinterface001.h"
#include "skse64/PluginAPI.h"
#include "TactsuitVR.h"

// TACTSUIT build numbers are made up as follows: V1.1.0
#define TACTSUIT_BUILD_NUMBER 110

namespace tactsuitPluginApi {
	// Handles skse mod messages requesting to fetch API functions from TACTSUIT
	void modMessageHandler(SKSEMessagingInterface::Message * message);

	// This object provides access to Tactsuit's mod support API version 1
	class TactsuitInterface001 : ITactsuitInterface001
	{
	public:
		// Gets the build number
		virtual unsigned int getBuildNumber();

		//Plays haptic effect once. effectFileName without extension.
		virtual void PlayHapticEffect(std::string effectFileName, float locationAngle, float locationHeight, float intensity, bool waitToPlay);

		//Stops haptic effect. effectFileName without extension.
		virtual void StopHapticEffect(std::string effectFileName);
	};
}
