#include "tactsuitpluginapi.h"
#include "TactsuitVR.h"

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
	_MESSAGE("SKSE mod integrations not working due to debug/release library combinination. Defaulting to SpellWheelOpen");

	return;
	std::string effectFileName2 = "SpellWheelOpenLeft_1";

	float intensityCalculated = intensity;
	if(Contains(effectFileName2, "PlayerCatch"))
	{
		intensityCalculated *= TactsuitVR::intensityMultiplierPlayerCatch;
	}
	else if (Contains(effectFileName2, "PlayerThrow"))
	{
		intensityCalculated *= TactsuitVR::intensityMultiplierPlayerThrow;
	}
	else if (Contains(effectFileName2, "SpellWheelOpen"))
	{
		intensityCalculated *= TactsuitVR::intensityMultiplierSpellWheelOpen;
	}
	else if (Contains(effectFileName2, "Summon"))
	{
		intensityCalculated *= TactsuitVR::intensityMultiplierTeleport;
	}
	else if (Contains(effectFileName2, "Travel"))
	{
		intensityCalculated *= TactsuitVR::intensityMultiplierTravelEffect;
	}
	
	TactsuitVR::ProvideHapticFeedbackSpecificFile(locationAngle, locationHeight, effectFileName2, intensityCalculated, waitToPlay);
}

void TactsuitInterface001::StopHapticEffect(std::string effectFileName)
{
	_MESSAGE("PlayHapticEffect received SKSE %s", effectFileName.c_str());
	TactsuitVR::PauseHapticFeedBack();
}

// Fetches the version number
unsigned int TactsuitInterface001::getBuildNumber()
{
	return TACTSUIT_BUILD_NUMBER;
}
