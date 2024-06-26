#pragma once
#include "skse64/PluginAPI.h"
#include "skse64/GameReferences.h"

namespace HiggsPluginAPI
{

	// Returns an IHiggsInterface001 object compatible with the API shown below
	// This should only be called after SKSE sends kMessage_PostLoad to your plugin
	struct IHiggsInterface001;
	IHiggsInterface001 * GetHiggsInterface001(const PluginHandle & pluginHandle, SKSEMessagingInterface * messagingInterface);

	// This object provides access to HIGGS's mod support API
	struct IHiggsInterface001
	{
		// Gets the HIGGS build number
		virtual unsigned int GetBuildNumber() = 0;

		// Callbacks for when an object is pulled, grabbed, or dropped/thrown
		typedef void(*PulledCallback)(bool isLeft, TESObjectREFR *pulledRefr);
		virtual void AddPulledCallback(PulledCallback callback) = 0;

		typedef void(*GrabbedCallback)(bool isLeft, TESObjectREFR *grabbedRefr);
		virtual void AddGrabbedCallback(GrabbedCallback callback) = 0;

		typedef void(*DroppedCallback)(bool isLeft, TESObjectREFR *droppedRefr);
		virtual void AddDroppedCallback(DroppedCallback callback) = 0;

		// Callbacks for when an object is stashed (dropped over the shoulder) or consumed (dropped at the mouth)
		// Only the base form is returned in these cases as the object will no longer exist
		typedef void(*StashedCallback)(bool isLeft, TESForm *stashedForm);
		virtual void AddStashedCallback(StashedCallback callback) = 0;

		typedef void(*ConsumedCallback)(bool isLeft, TESForm *consumedForm);
		virtual void AddConsumedCallback(ConsumedCallback callback) = 0;

		// Callback for when the hands or held objects collide with something
		typedef void(*CollisionCallback)(bool isLeft, float mass, float separatingVelocity);
		virtual void AddCollisionCallback(CollisionCallback callback) = 0;

		// Grab the given object reference. It must have collision, and the given hand must be in a ready state as returned by GetGrabbedObject.
		virtual void GrabObject(TESObjectREFR *object, bool isLeft) = 0;

		// Get the currently held object reference. Note that some references can have multiple physics objects.
		virtual TESObjectREFR * GetGrabbedObject(bool isLeft) = 0;

		// Returns whether the given hand is in a state that can grab an object
		// (no currently held object, not pulling anything or have an object locked in for pulling (i.e. trigger/grip held on a selected object)
		virtual bool CanGrabObject(bool isLeft) = 0;

		// Disable and enable grabbing, selecting, pulling, etc. for each hand. Every disable should be accompanied by a later enable.
		// Multiple mods can disable at once, and the hand is only re-enabled once all mods have have called enable.
		virtual void DisableHand(bool isLeft) = 0;
		virtual void EnableHand(bool isLeft) = 0;
		virtual bool IsDisabled(bool isLeft) = 0;


		// Whether both hands are holding a weapon
		virtual bool IsTwoHanding() = 0;
	};
}

extern HiggsPluginAPI::IHiggsInterface001 * g_higgsInterface;
