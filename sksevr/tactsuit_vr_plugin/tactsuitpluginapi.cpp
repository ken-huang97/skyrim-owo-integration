#include "tactsuitpluginapi.h"

using namespace tactsuitPluginApi;


// A message used to fetch tactsuit VR's interface
struct TactsuitMessage
{
	enum { kMessage_GetInterface = 0xF4A282CF }; // Randomly chosen by the universe
	void* (*GetApiFunction)(unsigned int revisionNumber) = nullptr;
};

// Interface classes are stored statically
TactsuitInterface001 g_interface001;

// Constructs and returns an API of the revision number requested
void* getApi(unsigned int revisionNumber)
{
	return &g_interface001;
}

// Handles skse mod messages requesting to fetch API functions from TACTSUIT
void tactsuitPluginApi::modMessageHandler(SKSEMessagingInterface::Message* message)
{
	if (message != nullptr)
	{
		if (message->type == TactsuitMessage::kMessage_GetInterface)
		{
			TactsuitMessage* tactMessage = (TactsuitMessage*)message->data;
			tactMessage->GetApiFunction = getApi;
			_MESSAGE("...Provided TactsuitVR plugin interface to %s", message->sender);
		}
	}
}

void TactsuitInterface001::PlayHapticEffect(std::string effectFileName, float locationAngle, float locationHeight, float intensity, bool waitToPlay)
{
	float intensityCalculated = intensity;
	if(Contains(effectFileName, "PlayerCatch"))
	{
		intensityCalculated *= TactsuitVR::intensityMultiplierPlayerCatch;
	}
	else if (Contains(effectFileName, "PlayerThrow"))
	{
		intensityCalculated *= TactsuitVR::intensityMultiplierPlayerThrow;
	}
	else if (Contains(effectFileName, "SpellWheelOpen"))
	{
		intensityCalculated *= TactsuitVR::intensityMultiplierSpellWheelOpen;
	}
	else if (Contains(effectFileName, "Summon"))
	{
		intensityCalculated *= TactsuitVR::intensityMultiplierTeleport;
	}
	else if (Contains(effectFileName, "Travel"))
	{
		intensityCalculated *= TactsuitVR::intensityMultiplierTravelEffect;
	}
	
	TactsuitVR::ProvideHapticFeedbackSpecificFile(locationAngle, locationHeight, effectFileName, intensityCalculated, waitToPlay);
}

// TODO
void TactsuitInterface001::StopHapticEffect(std::string effectFileName)
{
	//TurnOffKey(effectFileName.c_str());
}

// Fetches the version number
unsigned int TactsuitInterface001::getBuildNumber()
{
	return TACTSUIT_BUILD_NUMBER;
}
