#include "tactsuitinterface001.h"

// A message used to fetch tactsuit VR's interface
struct TactsuitMessage
{
	enum {	kMessage_GetInterface = 0xF4A282CF }; // Randomly chosen by the universe
	void* (*GetApiFunction)(unsigned int revisionNumber) = nullptr;
};

// Stores the API after it has already been fetched
static tactsuitPluginApi::ITactsuitInterface001 * g_tactsuitInterface = nullptr;

// Fetches the interface to use from TACTSUIT
tactsuitPluginApi::ITactsuitInterface001 * tactsuitPluginApi::getTactsuitInterface001(const PluginHandle & pluginHandle, SKSEMessagingInterface * messagingInterface)
{
	// If the interface has already been fetched, return the same object
	if (g_tactsuitInterface) {
		return g_tactsuitInterface;
	}

	// Dispatch a message to get the plugin interface from TACTSUIT
	TactsuitMessage tactMessage;
	if(messagingInterface->Dispatch(pluginHandle, TactsuitMessage::kMessage_GetInterface, (void*)&tactMessage, sizeof(TactsuitMessage*), "TactsuitVRPlugin"))
	{
		_MESSAGE("tactsuit dispatch message returned true...");
	}
	else
	{
		_MESSAGE("tactsuit dispatch message returned false...");
	}
	
	if (!tactMessage.GetApiFunction)
	{
		return nullptr;
	}

	// Fetch the API for this version of the TACTSUIT interface
	g_tactsuitInterface = static_cast<ITactsuitInterface001*>(tactMessage.GetApiFunction(1));
	return g_tactsuitInterface;
}
