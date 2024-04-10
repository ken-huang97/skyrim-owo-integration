#include "Engine.h"

#include <deque>


namespace TactsuitVR
{
	UInt32 funcAddress = 0x0127B460;

	RelocAddr<uintptr_t*> addressStart = funcAddress + 0x38D;
	RelocAddr<uintptr_t*> addressEnd = funcAddress + 0x3BF;

	SKSETrampolineInterface* g_trampolineInterface = nullptr;

	bool locationalDamageInstalled = false;
	//from lfrazer's locational damage

	RelocAddr<_OnProjectileHitFunction> OnProjectileHitFunction(ONPROJECTILEHIT_INNERFUNCTION);
	RelocAddr<uintptr_t> OnProjectileHitHookLocation(ONPROJECTILEHIT_HOOKLOCATION);

	UInt64 directAddressX;
	
	GetVelocityOriginalFunctionProjectile g_originalGetVelocityFunctionProjectile = nullptr;

	const uintptr_t ArrowProjectileVtbl_Offset = 0x016F93A8;

	RelocPtr<GetVelocityOriginalFunctionProjectile> GetVelocityOriginalFunctionProjectile_vtbl(ArrowProjectileVtbl_Offset + 0xAC * 8);

	RelocAddr<_FindCollidableNode> FindCollidableNode(0xE01FE0);
	RelocAddr <sub240690> sub_240690(0x240690);
	RelocAddr <_subAF4960> sub_AF4960(0xAF4960);
	RelocAddr <_sub685270> sub_685270(0x685270);

	static uintptr_t g_moduleBase = 0;

	// Fetches the character movement state from the exe //Prog's code
	MovementState vlibGetMovementState() {
		g_moduleBase = uintptr_t(GetModuleHandle(NULL));

		if (!g_moduleBase)
			return MovementState::Unknown;
		ActorProcessManager* pManager = (*g_thePlayer)->processManager;
		if (!pManager)
			return MovementState::Unknown;
		uintptr_t bhkCharProxyController = sub_685270(pManager);
		if (!bhkCharProxyController)
			return MovementState::Unknown;
		uintptr_t hkpCharacterContext = bhkCharProxyController + 0x1E0;
		uintptr_t hkpCharacterStateManager = *(uintptr_t*)(bhkCharProxyController + 0x1F8);
		if (!hkpCharacterStateManager)
			return MovementState::Unknown;
		uintptr_t movementState = sub_AF4960(hkpCharacterStateManager, *(unsigned int*)(hkpCharacterContext + 0x20));
		if (!movementState)
			return MovementState::Unknown;
		uintptr_t stateObject = (*(uintptr_t*)movementState) - g_moduleBase;
		if (stateObject == 0x1838CA8)
			return MovementState::Grounded;
		else if (stateObject == 0x1838B40)
			return MovementState::Jumping;
		else if (stateObject == 0x1838AA0)
			return MovementState::InAir;
		return MovementState::Unknown;
	}

	UInt32 weaponThrowVRModIndex = 999;

	bool sraIsInstalled = false;

	std::mutex shooterProjectileMutex;
	std::unordered_map<UInt32, NiPoint3> shooterLastProjectilePositionList;
	
	PlayerCharacter::Node hmdNode = PlayerCharacter::kNode_HmdNode;
	PlayerCharacter::Node leftHandNode = PlayerCharacter::kNode_LeftHandBone;
	PlayerCharacter::Node rightHandNode = PlayerCharacter::kNode_RightHandBone;
	BSFixedString leftForeArmName("NPC L Forearm [LLar]");
	BSFixedString rightForeArmName("NPC R Forearm [RLar]");
	BSFixedString spine0Name("NPC Spine [Spn0]");

	PlayerCharacter::Node leftCavicleNode = PlayerCharacter::kNode_LeftCavicle;
	PlayerCharacter::Node rightCavicleNode = PlayerCharacter::kNode_RightCavicle;
	
	std::atomic<bool> LeftHandSpellCastingOn;
	std::atomic<bool> RightHandSpellCastingOn;
	std::atomic<bool> DualSpellCastingOn;
	std::atomic<bool> DualAimedStarted;
	std::atomic<bool> LeftAimedStarted;
	std::atomic<bool> RightAimedStarted;

	std::atomic<bool> RightHandAimedConcentrationSpellCastingOn;

	std::atomic<bool> isOnCart;


	int magicreleaseburstloopcount = 3;
		
	int defaultType = 2;
	
	Sky ** g_SkyPtr = nullptr;

	//Openvr system variable
	vr::IVRSystem *ivrSystem = nullptr;

	//Thread safe variables.
	std::atomic<bool> bowPulled(false);

	std::atomic<bool> rightMagic(false);
	std::atomic<bool> leftMagic(false);

	std::atomic<bool> raining(false);

	std::vector<UInt32> notExteriorWorlds = { 0x69857, 0x1EE62, 0x20DCB, 0x1FAE2, 0x34240, 0x50015, 0x2C965, 0x29AB7, 0x4F838, 0x3A9D6, 0x243DE, 0xC97EB, 0xC350D, 0x1CDD3, 0x1CDD9, 0x21EDB, 0x1E49D, 0x2B101, 0x2A9D8, 0x20BFE };

	//Default left-right controller variables. For left handed support to be changed later if needed.
	vr::TrackedDeviceIndex_t defaultControllerIndex;
	vr::ETrackedControllerRole leftControllerRole = vr::ETrackedControllerRole::TrackedControllerRole_LeftHand;
	vr::ETrackedControllerRole rightControllerRole = vr::ETrackedControllerRole::TrackedControllerRole_RightHand;

	PapyrusVR::VRDevice leftController = PapyrusVR::VRDevice::VRDevice_LeftController;
	PapyrusVR::VRDevice rightController = PapyrusVR::VRDevice::VRDevice_RightController;

	FeedbackType PlayerBowHoldType = FeedbackType::PlayerBowHoldRight;


	PlayerCharacter::Node leftWandNode = PlayerCharacter::kNode_LeftWandNode;
	PlayerCharacter::Node rightWandNode = PlayerCharacter::kNode_RightWandNode;


	UInt32 KeywordVendorItemPotionFormId = 0x0008CDEC;
	BGSKeyword* KeywordVendorItemPotion;

	UInt32 KeywordVendorItemPoisonFormId = 0x0008CDED;
	BGSKeyword* KeywordVendorItemPoison;

	UInt32 KeywordArmorShieldFormId = 0x000965B2;
	BGSKeyword* KeywordArmorShield;

	UInt32 KeywordArmorCuirassFormId = 0x0006C0EC;
	BGSKeyword* KeywordArmorCuirass;

	UInt32 KeywordArmorHelmetFormId = 0x0006C0EE;
	BGSKeyword* KeywordArmorHelmet;

	UInt32 KeywordArmorGauntletsFormId = 0x0006C0EF;
	BGSKeyword* KeywordArmorGauntlets;

	UInt32 KeywordClothingBodyFormId = 0x000A8657;
	BGSKeyword* KeywordClothingBody;

	UInt32 KeywordClothingHeadFormId = 0x0010CD11;
	BGSKeyword* KeywordClothingHead;


	UInt32 KeywordMagicArmorSpellFormId = 0x001EA72;
	BGSKeyword* KeywordMagicArmorSpell;

	UInt8 DragonbornIndex = 0x04;
	UInt8 DawnguardIndex = 0x02;

	//Bow haptic feedback detection variables.
	int bowReleaseCount = 0;
	int bowPullCount = 0;
	int bowReversePullCount = 0;
	int lastleftPulse = 0;
	int lastrightPulse = 0;

	//Rain variables:
	float rainIntensity = 1.0f;
	float rainDensity = 10;

	double holdBreathLimit = 22.0;

	bool moving = false;
	NiPoint3 vestVelocity;

	std::atomic<bool> headIsInsideWaterGeneral;
	std::atomic<bool> playerIsInsideWaterGeneral;

	////////////////////////////////
	/*BSFixedString cubemap = BSFixedString("Data\\Textures\\cubemaps\\OutputCube.dds");

	TESWaterReflections_CTOR g_TESWaterReflectionsFunction = nullptr;

	auto TESWaterReflectionsFunctionAddress_HookLoc = RelocAddr<uintptr_t>(0x04D70BF);
	const uintptr_t TESWaterReflectionsFunctionAddress = 0x04D70BF;

	RelocPtr<TESWaterReflections_CTOR> TESWaterReflectionsFunction_vtbl(TESWaterReflectionsFunctionAddress);

	void SkyVRaanHooks()
	{
		g_TESWaterReflectionsFunction = *TESWaterReflectionsFunction_vtbl;
		g_branchTrampoline.Write5Call(TESWaterReflectionsFunctionAddress_HookLoc.GetUIntPtr(), uintptr_t(TESWaterReflectionsFunc_Hook));
	}

	UInt32* TESWaterReflectionsFunc_Hook(UInt32* Src, UInt32* a2, UInt32* a3, __int16 flags, const char* staticReflectionTexture)
	{
		return g_TESWaterReflectionsFunction(Src, a2, a3, flags, cubemap.data);
	}*/
	//////////////////////////////////
	

	AIProcessManager* AIProcessManager::GetSingleton()
	{
		static RelocPtr<AIProcessManager*> AIProcessManagersingleton(0x01F831B0); //For VR 1.4.15

		return *AIProcessManagersingleton;
	}

	BGSWaterSystemManager* BGSWaterSystemManager::GetSingleton()
	{
		static RelocPtr<BGSWaterSystemManager*> BGSWaterSystemManagersingleton(0x01F85120); //For VR 1.4.15

		return *BGSWaterSystemManagersingleton;
	}

	bool runOnce = false;

	void GameLoad()
	{
		if (runOnce == false)
		{
			runOnce = true;
			std::string weaponthrowpluginName = "WeaponThrowVR.esp";
			std::string srapluginName = "Simple Realistic Archery VR.esp";
			
			DataHandler* dataHandler = DataHandler::GetSingleton();

			if (dataHandler)
			{
				const ModInfo* modInfoWT = dataHandler->LookupLoadedModByName(weaponthrowpluginName.c_str());

				if (modInfoWT)
				{
					if (IsValidModIndex(modInfoWT->modIndex)) //If plugin is in the load order.
					{
						weaponThrowVRModIndex = modInfoWT->modIndex;
					}
				}

				const ModInfo* modInfoSRA = dataHandler->LookupLoadedModByName(srapluginName.c_str());

				if (modInfoSRA)
				{
					if (IsValidModIndex(modInfoSRA->modIndex)) //If plugin is in the load order.
					{
						sraIsInstalled = true;
					}
				}

				const ModInfo* modInfoDragonborn = dataHandler->LookupLoadedModByName("Dragonborn.esm");
				if (modInfoDragonborn)
				{
					DragonbornIndex = (modInfoDragonborn->modIndex);
				}

				const ModInfo* modInfoDawnguard = dataHandler->LookupLoadedModByName("Dawnguard.esm");
				if (modInfoDawnguard)
				{
					DawnguardIndex = (modInfoDawnguard->modIndex);
				}
			}

			TESForm* keywordForm = LookupFormByID(KeywordVendorItemPotionFormId);
			if (keywordForm)
				KeywordVendorItemPotion = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);

			keywordForm = LookupFormByID(KeywordVendorItemPoisonFormId);
			if (keywordForm)
				KeywordVendorItemPoison = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);

			keywordForm = LookupFormByID(KeywordArmorShieldFormId);
			if (keywordForm)
				KeywordArmorShield = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);

			keywordForm = LookupFormByID(KeywordArmorCuirassFormId);
			if (keywordForm)
				KeywordArmorCuirass = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);

			keywordForm = LookupFormByID(KeywordArmorHelmetFormId);
			if (keywordForm)
				KeywordArmorHelmet = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);

			keywordForm = LookupFormByID(KeywordArmorGauntletsFormId);
			if (keywordForm)
				KeywordArmorGauntlets = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);

			keywordForm = LookupFormByID(KeywordClothingBodyFormId);
			if (keywordForm)
				KeywordClothingBody = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);

			keywordForm = LookupFormByID(KeywordClothingHeadFormId);
			if (keywordForm)
				KeywordClothingHead = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);

			keywordForm = LookupFormByID(KeywordMagicArmorSpellFormId);
			if (keywordForm)
				KeywordMagicArmorSpell = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);
		}
	}

	FeedbackType GetOtherHandSpellFeedback(FeedbackType feedback)
	{
		if (feedback == PlayerSpellAlterationRight)
		{
			return PlayerSpellAlterationLeft;
		}
		else if (feedback == PlayerSpellConjurationRight)
		{
			return PlayerSpellConjurationLeft;
		}
		else if (feedback == PlayerSpellFireRight)
		{
			return PlayerSpellFireLeft;
		}
		else if (feedback == PlayerSpellIceRight)
		{
			return PlayerSpellIceLeft;
		}
		else if (feedback == PlayerSpellIllusionRight)
		{
			return PlayerSpellIllusionLeft;
		}
		else if (feedback == PlayerSpellLightRight)
		{
			return PlayerSpellLightLeft;
		}
		else if (feedback == PlayerSpellOtherRight)
		{
			return PlayerSpellOtherLeft;
		}
		else if (feedback == PlayerSpellRestorationRight)
		{
			return PlayerSpellRestorationLeft;
		}
		else if (feedback == PlayerSpellShockRight)
		{
			return PlayerSpellShockLeft;
		}
		else if (feedback == PlayerSpellWardRight)
		{
			return PlayerSpellWardLeft;
		}
		else if (feedback == PlayerSpellAlterationLeft)
		{
			return PlayerSpellAlterationRight;
		}
		else if (feedback == PlayerSpellConjurationLeft)
		{
			return PlayerSpellConjurationRight;
		}
		else if (feedback == PlayerSpellFireLeft)
		{
			return PlayerSpellFireRight;
		}
		else if (feedback == PlayerSpellIceLeft)
		{
			return PlayerSpellIceRight;
		}
		else if (feedback == PlayerSpellIllusionLeft)
		{
			return PlayerSpellIllusionRight;
		}
		else if (feedback == PlayerSpellLightLeft)
		{
			return PlayerSpellLightRight;
		}
		else if (feedback == PlayerSpellOtherLeft)
		{
			return PlayerSpellOtherRight;
		}
		else if (feedback == PlayerSpellRestorationLeft)
		{
			return PlayerSpellRestorationRight;
		}
		else if (feedback == PlayerSpellShockLeft)
		{
			return PlayerSpellShockRight;
		}
		else if (feedback == PlayerSpellWardLeft)
		{
			return PlayerSpellWardRight;
		}
		else
		{
			return PlayerSpellOtherLeft;
		}
	}

	// Functions to detect what the player is doing
	bool vlibIsWalking()
	{
		// Do not use - always returns false?  Perhaps SkyrimVR doesn't differentiate walking/running.
		return ((*g_thePlayer)->actorState.flags04 & ActorState::kState_Running);
	}
	bool vlibIsRunning()
	{
		// Note: This is true when either walking or running
		return ((*g_thePlayer)->actorState.flags04 & ActorState::kState_Walking);
	}
	bool vlibIsSprinting()
	{
		// Only true when burning stamina to run fast
		return ((*g_thePlayer)->actorState.flags04 & ActorState::kState_Sprinting);
	}


	
	//Animation stuff:
	typedef bool (*subGetAnimationVariableBool)(VMClassRegistry* registry, UInt32 stackId, TESObjectREFR* target, BSFixedString* name);
	RelocAddr<subGetAnimationVariableBool> sub_GetAnimationVariableBool(0x9CE880);

	// Animation graph hook for the player that allows specific animations to be blocked from playing
	typedef uint64_t(*IAnimationGraphManagerHolder_NotifyAnimationGraph_VFunc)(uintptr_t* iagmh, BSFixedString* animName);
	IAnimationGraphManagerHolder_NotifyAnimationGraph_VFunc g_originalNotifyAnimationGraph = nullptr; // Normally a JMP to 0x00501530
	static RelocPtr<IAnimationGraphManagerHolder_NotifyAnimationGraph_VFunc> IAnimationGraphManagerHolder_NotifyAnimationGraph_vtbl(0x016E2BF8);
	uint64_t Hooked_IAnimationGraphManagerHolder_NotifyAnimationGraph(uintptr_t* iAnimationGraphManagerHolder, BSFixedString* animationName)
	{
		//_MESSAGE("Animation Called %s", animationName->data);

		const bool isMounted = vlibIsMounted();

		if (strcmp(animationName->data, "MRh_Equipped_Event") == 0)
		{
			RightHandAimedConcentrationSpellCastingOn.store(false);

			RightHandSpellCastingOn.store(false);
			DualSpellCastingOn.store(false);
		}
		else if (strcmp(animationName->data, "MLh_Equipped_Event") == 0)
		{
			LeftHandSpellCastingOn.store(false);
			DualSpellCastingOn.store(false);
		}
		else if (strcmp(animationName->data, "DualMagic_SpellAimedConcentrationStart") == 0 || strcmp(animationName->data, "DualMagic_SpellSelfConcentrationStart") == 0 || strcmp(animationName->data, "RitualSpellAimConcentrationStart") == 0)
		{
			DualSpellCastingOn.store(true);
			SpellCastingEventDecider(false, true, false);
		}
		else if (strcmp(animationName->data, "MRh_SpellAimedConcentrationStart") == 0 || strcmp(animationName->data, "MRh_SpellSelfConcentrationStart") == 0 || strcmp(animationName->data, "MRh_SpellTelekinesisStart") == 0 || strcmp(animationName->data, "MRh_WardStart") == 0)
		{
			RightHandAimedConcentrationSpellCastingOn.store(true);
			RightHandSpellCastingOn.store(true);
			SpellCastingEventDecider(false, false, false);
		}
		else if (strcmp(animationName->data, "MLh_SpellAimedConcentrationStart") == 0 || strcmp(animationName->data, "MLh_SpellSelfConcentrationStart") == 0 || strcmp(animationName->data, "MLh_SpellTelekinesisStart") == 0 || strcmp(animationName->data, "MLh_WardStart") == 0)
		{
			LeftHandSpellCastingOn.store(true);
			SpellCastingEventDecider(false, false, true);
		}
		else if (strcmp(animationName->data, "MLh_SpellAimedStart") == 0 || strcmp(animationName->data, "MLh_SpellSelfStart") == 0)
		{
			LeftAimedStarted.store(true);
			LeftHandSpellCastingOn.store(true);
			SpellCastingEventDecider(true, false, true);
		}
		else if (strcmp(animationName->data, "MRh_SpellAimedStart") == 0 || strcmp(animationName->data, "MRh_SpellSelfStart") == 0)
		{
			RightAimedStarted.store(true);
			RightHandSpellCastingOn.store(true);
			SpellCastingEventDecider(true, false, false);
		}
		else if (strcmp(animationName->data, "DualMagic_SpellAimedStart") == 0 || strcmp(animationName->data, "DualMagic_SpellSelfStart") == 0 || strcmp(animationName->data, "RitualSpellStart") == 0)
		{
			DualAimedStarted.store(true);
			DualSpellCastingOn.store(true);
			SpellCastingEventDecider(true, true, false);
		}
		else if (strcmp(animationName->data, "RitualSpellOut") == 0)
		{
			if (DualAimedStarted.load() || DualSpellCastingOn.load())
			{
				DualAimedStarted.store(false);
				DualSpellCastingOn.store(false);
				std::thread t3(ReleaseBurst, true, false);
				t3.detach();
			}
		}
		else if (strcmp(animationName->data, "MRh_SpellRelease_Event") == 0 || strcmp(animationName->data, "attackReleaseR") == 0)
		{
			if (isMounted && RightHandAimedConcentrationSpellCastingOn.load())
			{
				//don't do anything for mounted concentration spells
			}
			else
			{
				if (RightAimedStarted.load())
				{
					RightAimedStarted.store(false);
					RightHandSpellCastingOn.store(false);
					std::thread t3(ReleaseBurst, false, false);
					t3.detach();
				}
				if (isMounted && RightHandSpellCastingOn.load())
				{
					RightAimedStarted.store(false);
					RightHandSpellCastingOn.store(false);
				}
				if (DualAimedStarted.load())
				{
					DualAimedStarted.store(false);
					DualSpellCastingOn.store(false);
					std::thread t3(ReleaseBurst, true, false);
					t3.detach();
				}
			}
		}
		else if (strcmp(animationName->data, "MLH_SpellRelease_event") == 0 || strcmp(animationName->data, "attackReleaseL") == 0)
		{
			if (LeftAimedStarted.load())
			{
				LeftAimedStarted.store(false);
				LeftHandSpellCastingOn.store(false);
				std::thread t3(ReleaseBurst, false, true);
				t3.detach();
			}
			if (isMounted && LeftHandSpellCastingOn.load())
			{
				LeftAimedStarted.store(false);
				LeftHandSpellCastingOn.store(false);
			}
			if (DualAimedStarted.load())
			{
				DualAimedStarted.store(false);
				DualSpellCastingOn.store(false);
				std::thread t3(ReleaseBurst, true, true);
				t3.detach();
			}
		}
		else if (strcmp(animationName->data, "IdleCartPrisonerCSway") == 0 || strcmp(animationName->data, "IdleCartTravelPlayerEnter") == 0) //In cart
		{
			isOnCart.store(true);
		}
		else if (strcmp(animationName->data, "IdleCartPrisonerCExit") == 0 || strcmp(animationName->data, "IdleChairExitStart") == 0) //out of cart
		{
			isOnCart.store(true);
		}
		// return 0 to block the animation from playing
		// Otherwise return...
		return   g_originalNotifyAnimationGraph(iAnimationGraphManagerHolder, animationName);
	}

	float GetCurrentWaterBreathing()
	{
		return (*g_thePlayer)->actorValueOwner.GetCurrent(57);
	}

	void DoorOpeningAnimation(NiPoint3 pos)
	{
		Sleep(1000);
		for (int i = 0; i < 22; i++)
		{
			float distance = sqrtf(distanceNoSqrt(pos, (*g_thePlayer)->pos));

			float intensity = 1.0f;

			if (distance < 300.0f)
			{
				intensity = 1.0f;
			}
			else if (distance > 1500.0f)
			{
				intensity = 0.0f;
			}
			else
			{
				intensity = 300.0f / distance;
			}
			if (intensity > 0.0001f)
			{
				ProvideHapticFeedback(randomGenerator(0, 359), 0, FeedbackType::EnvironmentRumble, intensity * intensityMultiplierEnvironmentRumble * 0.50f);
			}
			Sleep(500);
		}
	}

	typedef uint64_t(*TESObjectREFR_IAnimationGraphManagerHolder_NotifyAnimationGraph_VFunc)(uintptr_t* iagmh, BSFixedString* animName);
	TESObjectREFR_IAnimationGraphManagerHolder_NotifyAnimationGraph_VFunc g_originalTESObjectREFR_NotifyAnimationGraph = nullptr; // Normally a JMP to 0x00501530
	static RelocPtr<TESObjectREFR_IAnimationGraphManagerHolder_NotifyAnimationGraph_VFunc> TESObjectREFR_IAnimationGraphManagerHolder_NotifyAnimationGraph_vtbl(0x015E1D50);
	uint64_t Hooked_TESObjectREFR_IAnimationGraphManagerHolder_NotifyAnimationGraph(uintptr_t* iAnimationGraphManagerHolder, BSFixedString* animationName)
	{
		if (iAnimationGraphManagerHolder)
		{
			IAnimationGraphManagerHolder* holder = (IAnimationGraphManagerHolder*)(iAnimationGraphManagerHolder);

			if (holder)
			{
				TESObjectREFR* ref = DYNAMIC_CAST(holder, IAnimationGraphManagerHolder, TESObjectREFR);

				if (ref && ref->baseForm)
				{
					if (ref->baseForm->formType == kFormType_MovableStatic)
					{
						if (strcmp(animationName->data, "FXDoorOpen") == 0)
						{
							//Play rumble according to distance to the event
							if (ref->baseForm->formID == 0x52172)//Puzzle door open
							{
								std::thread t3(DoorOpeningAnimation, ref->pos);
								t3.detach();
							}
							LOG("Door Open animation %s for %x - FormType: %d", animationName->data, ref->baseForm->formID, ref->baseForm->formType);
						}
						else
						{
							if (logging > 0)
								LOG("Unprocessed MovableStatic Animation Called %s for %x - FormType: %d", animationName->data, ref->baseForm->formID, ref->baseForm->formType);
						}
					}
					else if (ref->baseForm->formType == kFormType_Activator)
					{
						TESObjectACTI* activator = DYNAMIC_CAST(ref->baseForm, TESForm, TESObjectACTI);

						if (activator != nullptr)
						{
							if (strcmp(animationName->data, "Learned") == 0 && ContainsNoCase(activator->texSwap.GetModelName(), "powerword"))
							{
								ProvideHapticFeedback(0, 0, FeedbackType::Learned, intensityMultiplierLearned);
								//if(logging > 0)
								//	LOG("Word Wall animation %s for %s formid:%x Distance:%g", animationName->data, activator->texSwap.GetModelName(), activator->formID, sqrtf(distanceNoSqrt(ref->pos, (*g_thePlayer)->pos)));
							}
							else if (strcmp(animationName->data, "Play") == 0 && ContainsNoCase(activator->texSwap.GetModelName(), "fxgreybeardshout"))
							{
								std::thread t15(LateFeedback, 0, FeedbackType::WordWall, intensityMultiplierWordWall, 900, 12, false, false);
								t15.detach();
								//if(logging > 0)
								//	LOG("Word Wall animation %s for %s formid:%x Distance:%g", animationName->data, activator->texSwap.GetModelName(), activator->formID, sqrtf(distanceNoSqrt(ref->pos, (*g_thePlayer)->pos)));
							}
							else if (strcmp(animationName->data, "Away") == 0 && ContainsNoCase(activator->texSwap.GetModelName(), "fxgreybeardshout"))
							{
								ProvideHapticFeedback(0, 0, FeedbackType::Learned, intensityMultiplierLearned);
								//if(logging > 0)
								//	LOG("Word Wall animation %s for %s formid:%x Distance:%g", animationName->data, activator->texSwap.GetModelName(), activator->formID, sqrtf(distanceNoSqrt(ref->pos, (*g_thePlayer)->pos)));
							}
							else if (strcmp(animationName->data, "Open") == 0 && ContainsNoCase(activator->texSwap.GetModelName(), "StoneBarrier"))
							{
								ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentRumble, intensityMultiplierEnvironmentRumble);								
							}
							else
							{
								if (logging > 0)
									LOG("Unprocessed TESObjectACTI Animation Called %s for %x - FormType: %d - Model: %s - Distance: %g", animationName->data, ref->baseForm->formID, ref->baseForm->formType, activator->texSwap.GetModelName(), sqrtf(distanceNoSqrt(ref->pos, (*g_thePlayer)->pos)));
							}
						}
					}
					else
					{
						if (logging > 0)
							LOG("Unprocessed TESObjectREFR Animation Called %s for %x - FormType: %d", animationName->data, ref->baseForm->formID, ref->baseForm->formType);
					}
				}
			}
		}
		return   g_originalTESObjectREFR_NotifyAnimationGraph(iAnimationGraphManagerHolder, animationName);
	}

	typedef uint64_t(*Actor_IAnimationGraphManagerHolder_NotifyAnimationGraph_VFunc)(uintptr_t* iagmh, BSFixedString* animName);
	Actor_IAnimationGraphManagerHolder_NotifyAnimationGraph_VFunc g_originalActor_NotifyAnimationGraph = nullptr; // Normally a JMP to 0x00501530
	static RelocPtr<Actor_IAnimationGraphManagerHolder_NotifyAnimationGraph_VFunc> Actor_IAnimationGraphManagerHolder_NotifyAnimationGraph_vtbl(0x016D7780);
	uint64_t Hooked_Actor_IAnimationGraphManagerHolder_NotifyAnimationGraph(uintptr_t* iAnimationGraphManagerHolder, BSFixedString* animationName)
	{
		if (iAnimationGraphManagerHolder)
		{
			IAnimationGraphManagerHolder* holder = (IAnimationGraphManagerHolder*)(iAnimationGraphManagerHolder);
			if (holder)
			{
				Actor* ref = DYNAMIC_CAST(holder, IAnimationGraphManagerHolder, Actor);
				if (ref && ref->baseForm && ref->baseForm->formID != 0x14)
				{
					if (ref->race->formID == 0x12e82)
					{
						if (Contains(animationName->data, "FlyStop"))
						{
							float distance = sqrtf(distance2dNoSqrt(ref->pos, (*g_thePlayer)->pos));
							float intensity = 1.0f;

							if (distance < 1000.0f)
							{
								intensity = 1.0f;
							}
							else if (distance > 5000.0f)
							{
								intensity = 0.0f;
							}
							else
							{
								intensity = 1000.0f / distance;
							}
							if (intensity > 0.0001f)
							{
								std::thread t15(LateFeedback, 0, FeedbackType::DragonLanding, intensityMultiplierDragonLanding * intensity, Contains(animationName->data, "Vertical") ? 1250 : 2000, 1, false, false);
								t15.detach();
								//ProvideHapticFeedback(0, 0, FeedbackType::DragonLanding, intensityMultiplierDragonLanding* intensity);
							}
							LOG("Actor Animation Called %s for %x - %s - distance:%g intensity:%g", animationName->data, ref->baseForm->formID, ref->GetFullName() != nullptr ? ref->GetFullName() : "null", distance, intensity);
						}
					}
				}
			}
		}
		return   g_originalActor_NotifyAnimationGraph(iAnimationGraphManagerHolder, animationName);
	}

	void SpellCastingEventDecider(bool fireAndForget, bool dual, bool leftHand)
	{
		auto player = DYNAMIC_CAST(LookupFormByID(0x14), TESForm, Actor);
		_MESSAGE("Spell casting event decider");

		if (player != nullptr)
		{
			if (leftHand || dual)
			{
				TESForm* leftHandObject = player->GetEquippedObject(true);
				if (player->leftHandSpell && leftHandObject != nullptr)
				{
					std::string leftSpellName = player->leftHandSpell->fullName.GetName();
					LOG("Name of the Left Spell: %s", leftSpellName.c_str());

					bool shortSpell = false;
					if (std::find(ignoredSpells.begin(), ignoredSpells.end(), leftSpellName) != ignoredSpells.end())
					{
						LOG("%s is ignored", leftSpellName.c_str());
						return;
					}
					else if (std::find(shortSpells.begin(), shortSpells.end(), leftSpellName) != shortSpells.end())
					{
						LOG("%s is a short spell", leftSpellName.c_str());
						shortSpell = true;
					}

					std::vector<unsigned short>* magicArray;

					bool staff = leftHandObject->IsWeapon();

					FeedbackType spellFeedbackType = FeedbackType::PlayerSpellOtherLeft;

					magicArray = DecideSpellType(player->leftHandSpell, staff, leftHand, spellFeedbackType);
					
					if (!magicArray || magicArray->empty())
						return;

					bool selfHealingSpell = player->leftHandSpell->data.delivery == SpellItem::Delivery::kSelf && !fireAndForget && spellFeedbackType == FeedbackType::PlayerSpellRestorationLeft;

					std::thread t2(StartMagicEffectLeft, magicArray, shortSpell, staff, spellFeedbackType, selfHealingSpell);
					t2.detach();

					
					if (dual && !rightMagic.load())
					{
						std::thread t3(StartMagicEffectRight, magicArray, shortSpell, staff, GetOtherHandSpellFeedback(spellFeedbackType), false);
						t3.detach();
					}
					LOG("Start left spell feedback.");
				}
			}

			if(!leftHand)
			{
				TESForm* rightHandObject = player->GetEquippedObject(false);
				if (player->rightHandSpell && rightHandObject != nullptr)
				{
					std::string rightSpellName = player->rightHandSpell->fullName.GetName();
					LOG("Name of the Right Spell: %s", rightSpellName.c_str());

					bool shortSpell = false;
					if (std::find(ignoredSpells.begin(), ignoredSpells.end(), rightSpellName) != ignoredSpells.end())
					{
						LOG("%s is ignored", rightSpellName.c_str());
						return;
					}
					else if (std::find(shortSpells.begin(), shortSpells.end(), rightSpellName) != shortSpells.end())
					{
						LOG("%s is a short spell", rightSpellName.c_str());
						shortSpell = true;
					}

					std::vector<unsigned short>* magicArray;

					bool staff = rightHandObject->IsWeapon();
					FeedbackType spellFeedbackType = FeedbackType::PlayerSpellOtherLeft;
										
					magicArray = DecideSpellType(player->rightHandSpell, staff, leftHand, spellFeedbackType);
					
					if (!magicArray || magicArray->empty())
						return;

					LOG("Start right spell feedback.");

					bool selfHealingSpell = !dual && player->rightHandSpell->data.delivery == SpellItem::Delivery::kSelf && !fireAndForget && spellFeedbackType == FeedbackType::PlayerSpellRestorationRight;
					
					std::thread t3(StartMagicEffectRight, magicArray, shortSpell, staff, spellFeedbackType, selfHealingSpell);
					t3.detach();
				}
			}

		}
	}

	bool TwoHandedWeapon(UInt8 weaponType)
	{
		return weaponType == TESObjectWEAP::GameData::kType_TwoHandAxe || weaponType == TESObjectWEAP::GameData::kType_2HA ||
			weaponType == TESObjectWEAP::GameData::kType_TwoHandSword || weaponType == TESObjectWEAP::GameData::kType_2HS;
	}

	EventDispatcher<TESEquipEvent>* g_TESEquipEventDispatcher;
	TESEquipEventHandler g_TESEquipEventHandler;

	EventResult TESEquipEventHandler::ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>* dispatcher)
	{
		if (evn)
		{
			if (!(*g_thePlayer) || !(*g_thePlayer)->loadedState)
			{
				return EventResult::kEvent_Continue;
			}

			if(evn->actor!=nullptr && evn->actor->formID == 0x14)
			{
				if(evn->baseObject > 0)
				{
					TESForm * form = LookupFormByID(evn->baseObject);

					if(form !=nullptr)
					{
						TESObjectARMO* armor = DYNAMIC_CAST(form, TESForm, TESObjectARMO);
						if (armor !=nullptr)
						{
							LOG("Player is %s something formId: %x", evn->equipped ? "equipping" : "unequipping", evn->baseObject);
							if (!armor->keyword.HasKeyword(KeywordArmorShield) && ((armor->bipedObject.GetSlotMask() & BGSBipedObjectForm::kPart_Body) != 0 || (armor->bipedObject.GetSlotMask() & BGSBipedObjectForm::kPart_Head) != 0 || (armor->bipedObject.GetSlotMask() & BGSBipedObjectForm::kPart_Hair) != 0 || (armor->bipedObject.GetSlotMask() & BGSBipedObjectForm::kPart_LongHair) != 0 || (armor->bipedObject.GetSlotMask() & BGSBipedObjectForm::kPart_Circlet) != 0 || (armor->bipedObject.GetSlotMask() & BGSBipedObjectForm::kPart_Forearms) != 0 || (armor->bipedObject.GetSlotMask() & BGSBipedObjectForm::kPart_Hands) != 0))
							{
								if (armor->keyword.HasKeyword(KeywordArmorHelmet))
								{
									if (evn->equipped)
									{
										std::thread t15(LateFeedback, 0, FeedbackType::EquipHelmet, intensityMultiplierEquipUnequip * 0.75f, 700, 1, false, true);
										t15.detach();
									}
									else
									{
										ProvideHapticFeedback(0, 0, FeedbackType::UnequipHelmet, intensityMultiplierEquipUnequip * 0.75f, false, true);
									}
								}
								else if (armor->keyword.HasKeyword(KeywordArmorCuirass))
								{
									if (evn->equipped)
									{
										std::thread t15(LateFeedback, 0, FeedbackType::EquipCuirass, intensityMultiplierEquipUnequip * 0.75f, 700, 1, false, true);
										t15.detach();
									}
									else
									{
										ProvideHapticFeedback(0, 0, FeedbackType::UnequipCuirass, intensityMultiplierEquipUnequip * 0.75f, false, true);
									}
								}
								else if (armor->keyword.HasKeyword(KeywordArmorGauntlets))
								{
									if (evn->equipped)
									{
										std::thread t15(LateFeedback, 0, FeedbackType::EquipGauntlets, intensityMultiplierEquipUnequip * 0.75f, 700, 1, false, true);
										t15.detach();
									}
									else
									{
										ProvideHapticFeedback(0, 0, FeedbackType::UnequipGauntlets, intensityMultiplierEquipUnequip * 0.75f, false, true);
									}
								}
								else if (armor->keyword.HasKeyword(KeywordClothingBody))
								{
									if (evn->equipped)
									{
										std::thread t15(LateFeedback, 0, FeedbackType::EquipClothing, intensityMultiplierEquipUnequip * 0.75f, 700, 1, false, true);
										t15.detach();
									}
									else
									{
										ProvideHapticFeedback(0, 0, FeedbackType::UnequipClothing, intensityMultiplierEquipUnequip * 0.75f, false, true);
									}
								}
								else if (armor->keyword.HasKeyword(KeywordClothingHead))
								{
									if (evn->equipped)
									{
										std::thread t15(LateFeedback, 0, FeedbackType::EquipHood, intensityMultiplierEquipUnequip * 0.75f, 700, 1, false, true);
										t15.detach();
									}
									else
									{
										ProvideHapticFeedback(0, 0, FeedbackType::UnequipHood, intensityMultiplierEquipUnequip * 0.75f, false, true);
									}
								}

								LOG("Player is %s something formId: %x", evn->equipped ? "equipping" : "unequipping", evn->baseObject);
							}							
							return EventResult::kEvent_Continue;
						}
						else if(evn->equipped && !isGameStopped())
						{
							TESAmmo* ammo = DYNAMIC_CAST(form, TESForm, TESAmmo);
							if(ammo != nullptr)
							{
								if (sraIsInstalled)
								{
									ProvideHapticFeedback(0, 0, leftHandedMode ? UnholsterArrowLeftShoulder : UnholsterArrowRightShoulder, intensityMultiplierArrowShoulderEquip);
								}
								else
								{
									auto& nodeList = (*g_thePlayer)->nodeList;

									NiAVObject* rightCavicle_node = nodeList[rightCavicleNode];
									NiAVObject* rightHand_node = nodeList[rightHandNode];

									if (rightCavicle_node && rightHand_node)
									{
										const float distancesqr = distanceNoSqrt(rightHand_node->m_worldTransform.pos, rightCavicle_node->m_worldTransform.pos);
										if (distancesqr < 2400.0f)
										{
											ProvideHapticFeedback(0, 0, leftHandedMode ? UnholsterArrowLeftShoulder : UnholsterArrowRightShoulder, intensityMultiplierArrowShoulderEquip);
											LOG("Player is equipping an arrow formId: %x - DistanceSqr: %g", evn->baseObject, distancesqr);
										}
									}
								}
							}							
						}						
					}
				}
			}
		}
		return EventResult::kEvent_Continue;
	}

	void CallEnvironmentRumble(int count, float intensity, int waitBetween)
	{
		for (int i = 0; i < count; i++)
		{
			ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentRumble, intensityMultiplierEnvironmentRumble * intensity , true);
			if (waitBetween > 0)
			{
				Sleep(waitBetween);
			}
		}
	}

	EventDispatcher<TESQuestStageEvent>* g_TESQuestStageEventDispatcher;
	TESQuestStageEventHandler g_TESQuestStageEventHandler;

	EventResult TESQuestStageEventHandler::ReceiveEvent(TESQuestStageEvent* evn, EventDispatcher<TESQuestStageEvent>* dispatcher)
	{
		if (evn)
		{
			// LOG_ERR("Quest Stage Event: FormId: %x - Stage: %d", evn->formId, evn->stage);

			/////High Hrothgar Greybeards speaking to you
			//Quest Stage Event : FormId: 242ba - Stage : 280
			//Quest Stage Event : FormId: 242ba - Stage : 282
			//Quest Stage Event : FormId: 242ba - Stage : 284
			if (evn->formId == 0x242ba && (evn->stage == 280 || evn->stage == 284))
			{
				std::thread t15(CallEnvironmentRumble, 12, 5.0f, 400);
				t15.detach();
			}
			else if (evn->formId == 0x242ba && (evn->stage == 282))
			{
				std::thread t15(CallEnvironmentRumble, 24, 5.0f, 400);
				t15.detach();
			}

			////Prophet Forebear's holdout rising stuff
			else if (evn->formId == (DawnguardIndex << 24 | 0x004c3d) && evn->stage == 65)
			{
				std::thread t15(CallEnvironmentRumble, 3, 1.0f, 400);
				t15.detach();
			}
		}

		return EventResult::kEvent_Continue;
	}

	
	//HitEvent variables
	EventDispatcher<TESHitEvent>* g_HitEventDispatcher;
	HitEventHandler g_HitEventHandler;

	EventResult HitEventHandler::ReceiveEvent(TESHitEvent * evn, EventDispatcher<TESHitEvent> * dispatcher)
	{
		if (evn->target == (*g_thePlayer)) //Attacks against player
		{
			TESForm* sourceForm = LookupFormByID(evn->sourceFormID);
			TESObjectWEAP* sourceWeapon = nullptr;
			if (sourceForm != nullptr && sourceForm->IsWeapon())
			{
				sourceWeapon = DYNAMIC_CAST(sourceForm, TESForm, TESObjectWEAP);
			}
			
			if (evn->flags & evn->kFlag_Blocked) //If blocked
			{
				if (sourceWeapon != nullptr && !(sourceWeapon->gameData.type == TESObjectWEAP::GameData::kType_Bow || sourceWeapon->gameData.type == TESObjectWEAP::GameData::kType_Bow2 || sourceWeapon->gameData.type == TESObjectWEAP::GameData::kType_CrossBow || sourceWeapon->gameData.type == TESObjectWEAP::GameData::kType_CBow))
				{
					TESForm* leftEquippedObject = (*g_thePlayer)->GetEquippedObject(true);

					if (leftEquippedObject != nullptr)
					{
						if (!leftEquippedObject->IsWeapon())
						{
							//Play blocked vibration on hands
							LOG("Play blocked vibration on left arm");
							ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::PlayerBlockRight : FeedbackType::PlayerBlockLeft, intensityMultiplierPlayerBlock);
						}
						else
						{
							LOG("Play blocked vibration on right arm");
							ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::PlayerBlockLeft : FeedbackType::PlayerBlockRight, intensityMultiplierPlayerBlock);
						}
					}
					else
					{
						LOG("Play blocked vibration on right arm");
						ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::PlayerBlockLeft : FeedbackType::PlayerBlockRight, intensityMultiplierPlayerBlock);
					}
				}
			}
			else
			{
				if (evn->caster)
				{
					Actor* actor = DYNAMIC_CAST(evn->caster, TESObjectREFR, Actor);

					if (actor && actor->loadedState && actor->loadedState->node && actor != (*g_thePlayer) && (*g_thePlayer)->loadedState->node)
					{		
						bool HostileActor = CALL_MEMBER_FN((*g_thePlayer), IsHostileToActor)(actor);

						float heading = 0.0;
						float attitude = 0.0;
						float bank = 0.0;

						NiPoint3 position = actor->loadedState->node->m_worldTransform.pos;

						NiMatrix33 playerrotation = (*g_thePlayer)->loadedState->node->m_worldTransform.rot;
						playerrotation.GetEulerAngles(&heading, &attitude, &bank);
						NiPoint3 playerposition = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;

						float playerHeading = heading * (180 / MATH_PI);

						float playerAttitude = attitude * (180 / MATH_PI);

						float calcPlayerHeading = 0.0;

						if (playerHeading == -180)
						{
							calcPlayerHeading = 180 - playerAttitude;
						}
						else
						{
							if (playerAttitude < 0)
							{
								calcPlayerHeading = 360 + playerAttitude;
							}
							else
							{
								calcPlayerHeading = playerAttitude;
							}
						}
						float angle = (std::atan2(position.y - playerposition.y, position.x - playerposition.x) - std::atan2(1, 0)) * (180 / MATH_PI);
						if (angle < 0) {
							angle = angle + 360;
						}

						float headingAngle = angle - calcPlayerHeading;
						if (headingAngle < 0)
							headingAngle += 360;
						else if (headingAngle > 360)
							headingAngle -= 360;

						int heightrand = randomGenerator(0, 1000);
						float heightOffset = ((float)heightrand / 1000.0f) - 0.5f;
						
						TESForm* sourceForm = LookupFormByID(evn->sourceFormID);

						TESObjectWEAP* sourceWeapon = nullptr;
						if (sourceForm != nullptr)
						{
							if (sourceForm->IsWeapon())
							{
								sourceWeapon = DYNAMIC_CAST(sourceForm, TESForm, TESObjectWEAP);
							}

							//LOG("SourceForm formId:%x formType:%d weaponType:%d ranged: %s", sourceForm->formID, sourceForm->formType, sourceWeapon!=nullptr ? sourceWeapon->type() : -1,
							//	(sourceWeapon != nullptr && (sourceWeapon->gameData.type == TESObjectWEAP::GameData::kType_Bow || sourceWeapon->gameData.type == TESObjectWEAP::GameData::kType_Bow2 || sourceWeapon->gameData.type == TESObjectWEAP::GameData::kType_CrossBow || sourceWeapon->gameData.type == TESObjectWEAP::GameData::kType_CBow)) ? "yes" : "no");

							if (sourceWeapon != nullptr && (sourceWeapon->gameData.type == TESObjectWEAP::GameData::kType_Bow || sourceWeapon->gameData.type == TESObjectWEAP::GameData::kType_Bow2 || sourceWeapon->gameData.type == TESObjectWEAP::GameData::kType_CrossBow || sourceWeapon->gameData.type == TESObjectWEAP::GameData::kType_CBow))
							{
								return EventResult::kEvent_Continue;
							}
						}

						LOG("heightOffset: %g", heightOffset);

						SpellItem* spell = DYNAMIC_CAST(sourceForm, TESForm, SpellItem);
						//TESShout* shout = DYNAMIC_CAST(sourceForm, TESForm, TESShout);

						TESForm* rightEquippedObject = actor->GetEquippedObject(false);
						TESForm* leftEquippedObject = actor->GetEquippedObject(true);

						TESObjectWEAP* rightWeapon = nullptr;
						if (rightEquippedObject)
						{
							if (rightEquippedObject->IsWeapon())
							{
								rightWeapon = DYNAMIC_CAST(rightEquippedObject, TESForm, TESObjectWEAP);
							}
						}
						TESObjectWEAP* leftWeapon= nullptr;
						if (leftEquippedObject)
						{
							if (leftEquippedObject->IsWeapon())
							{
								leftWeapon = DYNAMIC_CAST(leftEquippedObject, TESForm, TESObjectWEAP);
							}
						}

						LOG("SourceForm:%x SourceType:%d - LeftEquippedObject:%x RightEquippedObject:%x", evn->sourceFormID, sourceForm != nullptr ? sourceForm->formType : -1, leftEquippedObject != nullptr ? leftEquippedObject->formID : 0, rightEquippedObject != nullptr ? rightEquippedObject->formID : 0);

						if (sourceForm->formType == kFormType_Spell && spell && spell->data.spellType != SpellItem::SpellType::kVoicePower && /*evn->projectileFormID && (actor->leftHandSpell || actor->rightHandSpell) &&*/ ((actor->leftHandSpell && actor->leftHandSpell == spell) || (actor->rightHandSpell && actor->rightHandSpell == spell)))
						{
							//play spell vibration on vest handled in projectile hook							
							return EventResult::kEvent_Continue;
						}
						else if ((sourceForm->formType == kFormType_Spell && spell && spell->data.spellType == SpellItem::SpellType::kVoicePower))
						{
							//play shout vibration on vest
							LOG("shout vibration on vest formId: %x - Name: %s", sourceForm->formID, spell ? spell->fullName.GetName() : "null");
							FeedbackType shoutFeedback = FeedbackType::Shout;

							if(sourceForm->formType == kFormType_Spell && spell && spell->fullName.GetName())
							{
								shoutFeedback = DecideShoutFeedbackType(spell->fullName.GetName());
							}
							
							ProvideHapticFeedback(headingAngle, 0, shoutFeedback, intensityMultiplierShout * 2.0f, false);
							/*std::thread t15(LateFeedback, 0, shoutFeedback, intensityMultiplierShout * 2.0f, 350, 3);
							t15.detach();*/
							return EventResult::kEvent_Continue;
						}
						else if(HostileActor && sourceForm->formType == kFormType_Explosion)
						{
							BGSExplosion* explosion = DYNAMIC_CAST(sourceForm, TESForm, BGSExplosion);

							if (explosion != nullptr) //Caused by an explosion
							{
								LOG("Explosion effect on player. FormID: %x ", explosion->formID);																

								if (explosion->formID == 0x72675 || explosion->formID == 0x104D46 || explosion->formID == 0x105CFF)
								{
									ProvideHapticFeedback(headingAngle, 0, FeedbackType::GiantStomp, intensityMultiplierUnarmedGiant * ((evn->flags & evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
								}
								else if (explosion->enchantment.enchantment != nullptr && explosion->enchantment.enchantment->effectItemList.count > 0)
								{
									bool hostileEffect = false;
									for (UInt32 i = 0; i < explosion->enchantment.enchantment->effectItemList.count; i++)
									{
										if (((explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Hostile) != 0))
										{
											hostileEffect = true;
											
											if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistFire)
											{
												ProvideHapticFeedback(headingAngle, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
												break;
											}
											else if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistFrost)
											{
												ProvideHapticFeedback(headingAngle, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
												break;
											}
											else if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistShock)
											{
												ProvideHapticFeedback(headingAngle, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
												break;
											}
											else if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kPoisonResist)
											{
												ProvideHapticFeedback(headingAngle, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
												break;
											}
										}
									}
									if (hostileEffect)
									{
										ProvideHapticFeedback(headingAngle, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
									}
								}
								else
								{
									if (explosion->data.impactDataSet != nullptr)
									{
										if (explosion->data.impactDataSet->formID == 0x26113
											|| explosion->data.impactDataSet->formID == 0x1c2af
											|| explosion->data.impactDataSet->formID == 0x647bd
											|| explosion->data.impactDataSet->formID == 0x20de1
											|| explosion->data.impactDataSet->formID == 0x6051e
											|| explosion->data.impactDataSet->formID == 0x8f3f0) //fire
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
											ProvideHapticFeedback(headingAngle, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
										}
										else if (explosion->data.impactDataSet->formID == 0x18a2e
											|| explosion->data.impactDataSet->formID == 0xd6c64
											|| explosion->data.impactDataSet->formID == 0x32da7
											|| explosion->data.impactDataSet->formID == 0x49b64
											|| explosion->data.impactDataSet->formID == 0xd7863
											|| explosion->data.impactDataSet->formID == 0x3af15
											|| explosion->data.impactDataSet->formID == 0x8f3f6) //frost
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
											ProvideHapticFeedback(headingAngle, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
										}
										else if (explosion->data.impactDataSet->formID == 0x38b05
											|| explosion->data.impactDataSet->formID == 0x59ed9
											|| explosion->data.impactDataSet->formID == 0x25dab
											|| explosion->data.impactDataSet->formID == 0x592d0
											|| explosion->data.impactDataSet->formID == 0x665b9) //shock
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
											ProvideHapticFeedback(headingAngle, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
										}
										else if (explosion->data.impactDataSet->formID == 0xfff4b
											|| explosion->data.impactDataSet->formID == 0x46008) //Poison
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
											ProvideHapticFeedback(headingAngle, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
										}
									}
									else if((explosion->data.sound1!=nullptr && explosion->data.sound1->formID == (DragonbornIndex << 24 | 0x0325DC))
										|| (explosion->data.sound2 != nullptr && explosion->data.sound2->formID == (DragonbornIndex << 24 | 0x0325DC)))
									{
										ProvideHapticFeedback(headingAngle, 0, FeedbackType::TentacleAttack, intensityMultiplierExplosion);
									}
								}
								return EventResult::kEvent_Continue;
							}
						}
						else if(sourceForm->formType == kFormType_Hazard)
						{
							BGSHazard* hazard = DYNAMIC_CAST(sourceForm, TESForm, BGSHazard);

							if (hazard != nullptr) //Caused by a hazard
							{
								LOG("Caster is a hazard. FormID: %x ", hazard->formID);

								if (hazard->data.spell != nullptr && hazard->data.spell->effectItemList.count > 0)
								{
									for (UInt32 i = 0; i < hazard->data.spell->effectItemList.count; i++)
									{
										if (hazard->data.spell->effectItemList[i]->mgef != nullptr)
										{
											if (((hazard->data.spell->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Hostile) != 0))
											{
												//kResistPoison = 40,
												//kResistFire = 41,
												//kResistShock = 42,
												//kResistFrost = 43,
												if (hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue == 41) //Fire
												{
													ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
													break;
												}
												else if (hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue == 42) //Shock
												{
													ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
													break;
												}
												else if (hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue == 43) //Frost
												{
													ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
													break;
												}
												else if (hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue == 40) //Poison
												{
													ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
													break;
												}
											}
										}
									}
								}
								else
								{
									if (hazard->data.impactDataSet != nullptr)
									{
										if (hazard->data.impactDataSet->formID == 0x26113
											|| hazard->data.impactDataSet->formID == 0x1c2af
											|| hazard->data.impactDataSet->formID == 0x647bd
											|| hazard->data.impactDataSet->formID == 0x20de1
											|| hazard->data.impactDataSet->formID == 0x6051e
											|| hazard->data.impactDataSet->formID == 0x8f3f0) //fire
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
										}
										else if (hazard->data.impactDataSet->formID == 0x18a2e
											|| hazard->data.impactDataSet->formID == 0xd6c64
											|| hazard->data.impactDataSet->formID == 0x32da7
											|| hazard->data.impactDataSet->formID == 0x49b64
											|| hazard->data.impactDataSet->formID == 0xd7863
											|| hazard->data.impactDataSet->formID == 0x3af15
											|| hazard->data.impactDataSet->formID == 0x8f3f6) //frost
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
										}
										else if (hazard->data.impactDataSet->formID == 0x38b05
											|| hazard->data.impactDataSet->formID == 0x59ed9
											|| hazard->data.impactDataSet->formID == 0x25dab
											|| hazard->data.impactDataSet->formID == 0x592d0
											|| hazard->data.impactDataSet->formID == 0x665b9) //shock
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
										}
										else if (hazard->data.impactDataSet->formID == 0xfff4b
											|| hazard->data.impactDataSet->formID == 0x46008) //Poison
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
										}
									}
								}
								return EventResult::kEvent_Continue;
							}
						}
						else if(sourceForm->formType == kFormType_Enchantment)
						{
							EnchantmentItem* enchantmentItem = DYNAMIC_CAST(sourceForm, TESForm, EnchantmentItem);

							LOG("Caster is an enchantmentItem. FormID: %x ", enchantmentItem->formID);

							if (enchantmentItem != nullptr)
							{
								for (UInt32 i = 0; i < enchantmentItem->effectItemList.count; i++)
								{
									if (enchantmentItem->effectItemList[i]->mgef != nullptr)
									{
										if (((enchantmentItem->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Hostile) != 0))
										{
											//kResistFire = 41,
											//kResistShock = 42,
											//kResistFrost = 43,
											if (enchantmentItem->effectItemList[i]->mgef->properties.primaryValue == 41) //Fire
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
												break;
											}
											else if (enchantmentItem->effectItemList[i]->mgef->properties.primaryValue == 42) //Shock
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
												break;
											}
											else if (enchantmentItem->effectItemList[i]->mgef->properties.primaryValue == 43) //Frost
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
												break;
											}
											else if (enchantmentItem->effectItemList[i]->mgef->properties.primaryValue == 40) //Poison
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
												break;
											}
										}
									}
								}
								return EventResult::kEvent_Continue;
							}
						}

						if (leftEquippedObject != nullptr)
						{
							if (leftWeapon != nullptr)
							{
								if (leftWeapon->gameData.type == TESObjectWEAP::GameData::kType_Bow || leftWeapon->gameData.type == TESObjectWEAP::GameData::kType_Bow2)
								{									
									return EventResult::kEvent_Continue;
								}
							}
						}

						TESObjectWEAP* attackWeapon = nullptr;
						bool rightAttack = true;
						if (sourceWeapon != nullptr)
						{
							attackWeapon = sourceWeapon;
							if (!(rightWeapon != nullptr && sourceWeapon == rightWeapon) && leftWeapon != nullptr && sourceWeapon == leftWeapon)
							{
								rightAttack = false;
							}
						}
						else if(rightEquippedObject!=nullptr && rightWeapon != nullptr)
						{
							attackWeapon = rightWeapon;
						}
						else if(leftEquippedObject!=nullptr && leftWeapon != nullptr)
						{
							attackWeapon = leftWeapon;
							rightAttack = false;
						}

						if (evn->sourceFormID != 0x1f4 && attackWeapon != nullptr)
						{
							if(attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_CrossBow || attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_CBow)
							{
								return EventResult::kEvent_Continue;
							}
							else if (attackWeapon->gameData.type != TESObjectWEAP::GameData::kType_Bow && attackWeapon->gameData.type != TESObjectWEAP::GameData::kType_Bow2)
							{
								if (actor !=nullptr && actor->race!=nullptr && actor->race->bodyPartData !=nullptr && actor->race->bodyPartData->formID == 0x517ab) //Giant
								{
									if(evn->sourceFormID == 0x461DA || evn->sourceFormID == 0xC334F || evn->sourceFormID == 0xCDEC9)
									{
										int attackLocation = CheckAttackLocation((*g_thePlayer), actor, rightAttack);
										ProvideHapticFeedback(headingAngle, 0, attackLocation > 0 ? FeedbackType::GiantClubLeft : FeedbackType::GiantClubRight, intensityMultiplierUnarmedGiant * ((evn->flags & evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
									}
								}
								else
								{
									//melee
									if (evn->flags & evn->kFlag_PowerAttack) //If power attack
									{
										//play power attack vibration on vest
										LOG("power attack vibration on vest");
										ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleePowerAttack, intensityMultiplierMeleePowerAttack);
									}
									else
									{
										if (evn->flags & evn->kFlag_Bash)
										{
											LOG("melee bash vibration on vest");
											ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeBash, intensityMultiplierMeleeBash);
										}
										else
										{
											int attackLocation = CheckAttackLocation((*g_thePlayer), actor, rightAttack);

											//play melee vibration on vest
											LOG("melee vibration on vest from:%d", attackLocation);

											if (attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_OneHandAxe || attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_1HA)
											{
												LOG("One Hand Axe");
												if (attackLocation > 0)
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeAxeLeft, intensityMultiplierMeleeAxe);
												else
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeAxeRight, intensityMultiplierMeleeAxe);
											}
											else if (attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_OneHandDagger || attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_1HD)
											{
												LOG("One Hand Dagger");
												if (attackLocation > 0)
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeDaggerLeft, intensityMultiplierMeleeDagger);
												else
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeDaggerRight, intensityMultiplierMeleeDagger);
											}
											else if (attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_OneHandMace || attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_1HM)
											{
												LOG("One Hand Mace");
												if (attackLocation > 0)
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeMaceLeft, intensityMultiplierMeleeMace);
												else
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeMaceRight, intensityMultiplierMeleeMace);
											}
											else if (attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_OneHandSword || attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_1HS)
											{
												LOG("One Hand Sword");
												if (attackLocation > 0)
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeSwordLeft, intensityMultiplierMeleeSword);
												else
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeSwordRight, intensityMultiplierMeleeSword);
											}
											else if (attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_TwoHandAxe || attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_2HA)
											{
												LOG("Two Hand Axe");
												if (attackLocation > 0)
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::Melee2HAxeLeft, intensityMultiplierMelee2HAxe);
												else
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::Melee2HAxeRight, intensityMultiplierMelee2HAxe);
											}
											else if (attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_TwoHandSword || attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_2HS)
											{
												LOG("Two Hand Sword");
												if (attackLocation > 0)
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::Melee2HSwordLeft, intensityMultiplierMelee2HSword);
												else
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::Melee2HSwordRight, intensityMultiplierMelee2HSword);
											}
											else if (attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_HandToHandMelee || attackWeapon->gameData.type == TESObjectWEAP::GameData::kType_H2H)
											{
												LOG("Hand to Hand");
												ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeFist, intensityMultiplierMeleeFist);
											}
											else
											{
												LOG("Some unknown weapon.");
												if (attackLocation > 0)
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeSwordLeft, intensityMultiplierMeleeSword);
												else
													ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeSwordRight, intensityMultiplierMeleeSword);
											}

										}
									}

									if ((evn->flags & evn->kFlag_PowerAttack) ? (randomGenerator(0, 1) == 1) : (randomGenerator(0, 10) == 10))
									{
										ProvideHapticFeedback(headingAngle, 0, FeedbackType::MeleeHead, intensityMultiplierMeleeHead);
									}
								}
								return EventResult::kEvent_Continue;
							}
						}

						//else
						LOG("Unarmed effect from formId: %x BodypartData:%x", (actor != nullptr && actor->baseForm!=nullptr) ? actor->baseForm->formID : 0, (actor != nullptr && actor->race !=nullptr && actor->race->bodyPartData!=nullptr) ? actor->race->bodyPartData->formID : 0);

						float heightdiff = (position.z - playerposition.z)/50.0f;

						//We can support many mod added creatures too this way
						if(actor!=nullptr && actor->race!=nullptr && actor->race->bodyPartData!=nullptr)
						{
							for (int j = 0; j < 5; j++)
							{
								if (actor->race->bodyPartData->part[j] !=nullptr && actor->race->bodyPartData->part[j]->partName.data != nullptr && strcmp(actor->race->bodyPartData->part[j]->partName.data, "Head")==0)
								{
									BGSBodyPartData::Data* data = actor->race->bodyPartData->part[j];

									if (data != nullptr)
									{
										if (data->targetName.data[0])
										{
											NiAVObject* object = (NiAVObject*)actor->GetNiNode();
											if (object)
											{
												object = object->GetObjectByName(&data->targetName.data);
												if (object)
												{
													heightdiff = (object->m_worldTransform.pos.z - playerposition.z) / 80.0f;
												}
											}
										}
									}
								} 
							}
							
							if (actor->race->bodyPartData->formID == 0x1d) //default
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedDefault, intensityMultiplierUnarmedDefault* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if(actor->race->bodyPartData->formID == 0x13492
								|| actor->race->bodyPartData->formID == (DragonbornIndex << 24 | 0x01bc4a)) //Dragon
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedDragon, intensityMultiplierUnarmedDragon* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x17929) //Frostbite Spider
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedFrostbiteSpider, intensityMultiplierUnarmedFrostbiteSpider* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x20e26) //Sabre Cat
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedSabreCat, intensityMultiplierUnarmedSabreCat* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x264ef) //Skeever
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedSkeever, intensityMultiplierUnarmedSkeever* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x40c6a) //Slaughterfish
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedSlaughterfish, intensityMultiplierUnarmedSlaughterfish* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x42529) //Wisp
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedWisp, intensityMultiplierUnarmedWisp* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x43592) //dragon priest
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedDragonPriest, intensityMultiplierUnarmedDragonPriest* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x48c13) //draugr
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedDraugr, intensityMultiplierUnarmedDraugr* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x4fbf5) //Wolf
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedWolf, intensityMultiplierUnarmedWolf* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x517ab) //Giant
							{
								if(evn->sourceFormID == 0x72675 || evn->sourceFormID == 0x104D46 || evn->sourceFormID == 0x105CFF)
								{
									ProvideHapticFeedback(headingAngle, 0, FeedbackType::GiantStomp, intensityMultiplierUnarmedGiant* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
								}
								else
								{
									ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedGiant, intensityMultiplierUnarmedGiant * ((evn->flags & evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
								}
							}
							else if (actor->race->bodyPartData->formID == 0x538f9) //Ice Wraith
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedIceWraith, intensityMultiplierUnarmedIceWraith* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x59060) //Chaurus
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedChaurus, intensityMultiplierUnarmedChaurus* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x59255) //Mammoth
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedMammoth, intensityMultiplierUnarmedMammoth* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x5b2e9) //Frost Atronach
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedFrostAtronach, intensityMultiplierUnarmedFrostAtronach* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x5dda2) //Falmer
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedFalmer, intensityMultiplierUnarmedFalmer* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x60716) //Horse
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedHorse, intensityMultiplierUnarmedHorse * ((evn->flags & evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x6b7c9) //Storm Atronach
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedStormAtronach, intensityMultiplierUnarmedStormAtronach * ((evn->flags & evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x76b30) //Elk
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedElk, intensityMultiplierUnarmedElk * ((evn->flags & evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x7874d) //Dwarven Sphere Centurion
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedDwarvenSphere, intensityMultiplierUnarmedDwarvenSphere* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x800ec) //Dwarven Steam Centurion
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedDwarvenSteam, intensityMultiplierUnarmedDwarvenSteam* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x81c7a) //Dwarven Spider
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedDwarvenSpider, intensityMultiplierUnarmedDwarvenSpider* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x868fc) //Bear
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedBear, intensityMultiplierUnarmedBear* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x8691c) //Flame Atronach
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedFlameAtronach, intensityMultiplierUnarmedFlameAtronach* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x86f43) //Witchlight
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedWitchlight, intensityMultiplierUnarmedWitchlight * ((evn->flags & evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x8ca6b) //Horker
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedHorker, intensityMultiplierUnarmedHorker* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x92c93) //Troll
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedTroll, intensityMultiplierUnarmedTroll* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0x97242) //Hagraven
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedHagraven, intensityMultiplierUnarmedHagraven* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0xa0420) //Spriggan
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedSpriggan, intensityMultiplierUnarmedSpriggan* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0xba549) //Mudcrab
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedMudcrab, intensityMultiplierUnarmedMudcrab* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == 0xcdd85) //Werewolf
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedWerewolf, intensityMultiplierUnarmedWerewolf* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == (DawnguardIndex << 24 | 0x005205)) //ChaurusFlyer
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedChaurusFlyer, intensityMultiplierUnarmedChaurusFlyer* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == (DawnguardIndex << 24 | 0x00a2c7) || actor->race->formID == (DawnguardIndex << 24 | 0x00283A)) //Gargoyle or Vampire Lord
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedGargoyle, intensityMultiplierUnarmedGargoyle* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == (DragonbornIndex << 24 | 0x017f53)) //Riekling
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedRiekling, intensityMultiplierUnarmedRiekling* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == (DragonbornIndex << 24 | 0x019ad2)) //Scrib
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedScrib, intensityMultiplierUnarmedScrib* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == (DragonbornIndex << 24 | 0x01dcbc)) //Seeker
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedSeeker, intensityMultiplierUnarmedSeeker* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == (DragonbornIndex << 24 | 0x01e2a3)) //Mounted Riekling
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedMountedRiekling, intensityMultiplierUnarmedMountedRiekling* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == (DragonbornIndex << 24 | 0x01feb9)) //Netch
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedNetch, intensityMultiplierUnarmedNetch* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else if (actor->race->bodyPartData->formID == (DragonbornIndex << 24 | 0x028537)) //Benthic Lurker
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::UnarmedBenthicLurker, intensityMultiplierUnarmedBenthicLurker* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							else
							{
								ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::Bite, intensityMultiplierBite* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));								
							}

							if(heightdiff > 0.495f)
							{
								ProvideHapticFeedback(headingAngle, 0, FeedbackType::UnarmedHead, intensityMultiplierUnarmedHead* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
							}
							
							return EventResult::kEvent_Continue;
						}

						ProvideHapticFeedback(headingAngle, heightdiff, FeedbackType::Bite, intensityMultiplierBite* ((evn->flags& evn->kFlag_PowerAttack) ? 2.0f : 1.0f));
						return EventResult::kEvent_Continue;
					}
					else
					{
						BGSExplosion* explosion = DYNAMIC_CAST(evn->caster, TESObjectREFR, BGSExplosion);
						BGSHazard* hazard = DYNAMIC_CAST(evn->caster, TESObjectREFR, BGSHazard);
						EnchantmentItem* enchantmentItem = DYNAMIC_CAST(evn->caster, TESObjectREFR, EnchantmentItem);

						if(explosion!=nullptr) //Caused by an explosion
						{
							LOG("Caster is an explosion. FormID: %x ", explosion->formID);
							if(explosion->data.damage > 0)
							{
								if (explosion->enchantment.enchantment != nullptr && explosion->enchantment.enchantment->effectItemList.count > 0)
								{
									bool hostileEffect = false;
									for (UInt32 i = 0; i < explosion->enchantment.enchantment->effectItemList.count; i++)
									{
										if (((explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Hostile) != 0))
										{
											hostileEffect = true;
											
											if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistFire)
											{
												ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
												break;
											}
											else if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistFrost)
											{
												ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
												break;
											}
											else if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistShock)
											{
												ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
												break;
											}
											else if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kPoisonResist)
											{
												ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
												break;
											}
										}
									}
									if (hostileEffect)
									{
										ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
									}
								}
								else
								{
									if (explosion->data.impactDataSet != nullptr)
									{
										if (explosion->data.impactDataSet->formID == 0x26113
											|| explosion->data.impactDataSet->formID == 0x1c2af
											|| explosion->data.impactDataSet->formID == 0x647bd
											|| explosion->data.impactDataSet->formID == 0x20de1
											|| explosion->data.impactDataSet->formID == 0x6051e
											|| explosion->data.impactDataSet->formID == 0x8f3f0) //fire
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
											ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
										}
										else if (explosion->data.impactDataSet->formID == 0x18a2e
											|| explosion->data.impactDataSet->formID == 0xd6c64
											|| explosion->data.impactDataSet->formID == 0x32da7
											|| explosion->data.impactDataSet->formID == 0x49b64
											|| explosion->data.impactDataSet->formID == 0xd7863
											|| explosion->data.impactDataSet->formID == 0x3af15
											|| explosion->data.impactDataSet->formID == 0x8f3f6) //frost
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
											ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
										}
										else if (explosion->data.impactDataSet->formID == 0x38b05
											|| explosion->data.impactDataSet->formID == 0x59ed9
											|| explosion->data.impactDataSet->formID == 0x25dab
											|| explosion->data.impactDataSet->formID == 0x592d0
											|| explosion->data.impactDataSet->formID == 0x665b9) //shock
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
											ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
										}
										else if (explosion->data.impactDataSet->formID == 0xfff4b
											|| explosion->data.impactDataSet->formID == 0x46008) //Poison
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
											ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
										}
									}
								}
							}
						}
						else if (hazard != nullptr) //Caused by a hazard
						{
							LOG("Caster is a hazard. FormID: %x ", hazard->formID);

							if(hazard->data.spell!=nullptr && hazard->data.spell->effectItemList.count> 0)
							{
								for (UInt32 i = 0; i < hazard->data.spell->effectItemList.count; i++)
								{
									if (hazard->data.spell->effectItemList[i]->mgef != nullptr)
									{
										if (((hazard->data.spell->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Hostile) != 0))
										{
											//kResistPoison = 40,
											//kResistFire = 41,
											//kResistShock = 42,
											//kResistFrost = 43,
											if (hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue == 41) //Fire
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
												break;
											}
											else if (hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue == 42) //Shock
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
												break;
											}
											else if (hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue == 43) //Frost
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
												break;
											}
											else if (hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue == 40) //Poison
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
												break;
											}
										}
									}
								}
							}
							else
							{
								if (hazard->data.impactDataSet != nullptr)
								{
									if (hazard->data.impactDataSet->formID == 0x26113
										|| hazard->data.impactDataSet->formID == 0x1c2af
										|| hazard->data.impactDataSet->formID == 0x647bd
										|| hazard->data.impactDataSet->formID == 0x20de1
										|| hazard->data.impactDataSet->formID == 0x6051e
										|| hazard->data.impactDataSet->formID == 0x8f3f0) //fire
									{
										ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
									}
									else if (hazard->data.impactDataSet->formID == 0x18a2e
										|| hazard->data.impactDataSet->formID == 0xd6c64
										|| hazard->data.impactDataSet->formID == 0x32da7
										|| hazard->data.impactDataSet->formID == 0x49b64
										|| hazard->data.impactDataSet->formID == 0xd7863
										|| hazard->data.impactDataSet->formID == 0x3af15
										|| hazard->data.impactDataSet->formID == 0x8f3f6) //frost
									{
										ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
									}
									else if (hazard->data.impactDataSet->formID == 0x38b05
										|| hazard->data.impactDataSet->formID == 0x59ed9
										|| hazard->data.impactDataSet->formID == 0x25dab
										|| hazard->data.impactDataSet->formID == 0x592d0
										|| hazard->data.impactDataSet->formID == 0x665b9) //shock
									{
										ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
									}
									else if (hazard->data.impactDataSet->formID == 0xfff4b
										|| hazard->data.impactDataSet->formID == 0x46008) //Poison
									{
										ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
									}
								}
							}
						}
						else if (enchantmentItem != nullptr) //Caused by an enchantmentItem
						{
							LOG("Caster is an enchantmentItem. FormID: %x ", enchantmentItem->formID);
							
							if (enchantmentItem != nullptr)
							{
								for (UInt32 i = 0; i < enchantmentItem->effectItemList.count; i++)
								{
									if (enchantmentItem->effectItemList[i]->mgef != nullptr)
									{
										if (((enchantmentItem->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Hostile) != 0))
										{
											//kResistFire = 41,
											//kResistShock = 42,
											//kResistFrost = 43,
											if (enchantmentItem->effectItemList[i]->mgef->properties.primaryValue == 41) //Fire
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
												break;
											}
											else if (enchantmentItem->effectItemList[i]->mgef->properties.primaryValue == 42) //Shock
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
												break;
											}
											else if (enchantmentItem->effectItemList[i]->mgef->properties.primaryValue == 43) //Frost
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
												break;
											}
											else if (enchantmentItem->effectItemList[i]->mgef->properties.primaryValue == 40) //Poison
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
												break;
											}
										}
									}
								}
							}
						}
						else if(!actor)
						{						
							LOG("Caster is not another npc. FormID: %x", evn->caster->formID);
							if (randomGenerator(0, 10) == 10)
							{
								ProvideHapticFeedback(0, 0, FeedbackType::DefaultHead, intensityMultiplierDefaultHead);
							}
							ProvideHapticFeedback(0, 0, FeedbackType::Default, intensityMultiplierDefault);
						}
						return EventResult::kEvent_Continue;
					}
					return EventResult::kEvent_Continue;
				}
				else
				{
					TESForm* sourceForm = nullptr;
					if (evn->sourceFormID != 0)
					{
						sourceForm = LookupFormByID(evn->sourceFormID);
					}

					if (sourceForm != nullptr)
					{
						BGSExplosion* explosion = DYNAMIC_CAST(sourceForm, TESForm, BGSExplosion);
						BGSHazard* hazard = DYNAMIC_CAST(sourceForm, TESForm, BGSHazard);
						EnchantmentItem* enchantmentItem = DYNAMIC_CAST(sourceForm, TESForm, EnchantmentItem);

						if (explosion != nullptr) //Caused by an explosion
						{
							LOG("Source is an explosion. FormID: %x", explosion->formID);
							if (explosion->data.damage > 0)
							{
								if (explosion->enchantment.enchantment != nullptr && explosion->enchantment.enchantment->effectItemList.count > 0)
								{
									bool hostileEffect = false;
									for (UInt32 i = 0; i < explosion->enchantment.enchantment->effectItemList.count; i++)
									{
										if (((explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Hostile) != 0))
										{
											hostileEffect = true;
											if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.school >= 18 && explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.school <= 22)
											{
												if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.school == EffectSetting::Properties::ActorValue::kDestruction)
												{
													if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistFire)
													{
														ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
														ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
														break;
													}
													else if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistFrost)
													{
														ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
														ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
														break;
													}
													else if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistShock)
													{
														ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
														ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
														break;
													}
												}
												break;
											}
											else
											{
												if (explosion->enchantment.enchantment->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kPoisonResist)
												{
													ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
													ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
													break;
												}
											}
										}
									}
									if (hostileEffect)
									{
										ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
									}
									else
									{
										return EventResult::kEvent_Continue;
									}
								}
								else if(explosion->data.impactDataSet!=nullptr)
								{
									if (explosion->data.impactDataSet->formID == 0x26113
										|| explosion->data.impactDataSet->formID == 0x1c2af
										|| explosion->data.impactDataSet->formID == 0x647bd
										|| explosion->data.impactDataSet->formID == 0x20de1
										|| explosion->data.impactDataSet->formID == 0x6051e
										|| explosion->data.impactDataSet->formID == 0x8f3f0) //fire
									{
										ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
										ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
									}
									else if (explosion->data.impactDataSet->formID == 0x18a2e
										|| explosion->data.impactDataSet->formID == 0xd6c64
										|| explosion->data.impactDataSet->formID == 0x32da7
										|| explosion->data.impactDataSet->formID == 0x49b64
										|| explosion->data.impactDataSet->formID == 0xd7863
										|| explosion->data.impactDataSet->formID == 0x3af15
										|| explosion->data.impactDataSet->formID == 0x8f3f6) //frost
									{
										ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
										ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
									}
									else if (explosion->data.impactDataSet->formID == 0x38b05
										|| explosion->data.impactDataSet->formID == 0x59ed9
										|| explosion->data.impactDataSet->formID == 0x25dab
										|| explosion->data.impactDataSet->formID == 0x592d0
										|| explosion->data.impactDataSet->formID == 0x665b9) //shock
									{
										ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
										ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
									}
									else if (explosion->data.impactDataSet->formID == 0xfff4b
										|| explosion->data.impactDataSet->formID == 0x46008) //Poison
									{
										ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
										ProvideHapticFeedback(0, 0, FeedbackType::Explosion, intensityMultiplierExplosion);
									}
									else
									{
										return EventResult::kEvent_Continue;
									}
								}
								else
								{
									return EventResult::kEvent_Continue;
								}
							}
							else
							{
								return EventResult::kEvent_Continue;
							}
						}
						else if (hazard != nullptr) //Caused by a hazard
						{
							LOG("Source is a hazard. FormID: %x", hazard->formID);

							if (hazard->data.spell != nullptr && hazard->data.spell->effectItemList.count > 0)
							{
								for (UInt32 i = 0; i < hazard->data.spell->effectItemList.count; i++)
								{
									if (hazard->data.spell->effectItemList[i]->mgef != nullptr)
									{
										if (((hazard->data.spell->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Hostile) != 0)
											&& ((hazard->data.spell->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Detrimental) != 0))
										{
											//kResistPoison = 40,
											//kResistFire = 41,
											//kResistShock = 42,
											//kResistFrost = 43,
											LOG("Hazard %x - effect: PrimaryValue: %d", hazard->formID, hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue);
											if (hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue == 41) //Fire
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
												break;
											}
											else if (hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue == 42) //Shock
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
												break;
											}
											else if (hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue == 43) //Frost
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
												break;
											}
											else if (hazard->data.spell->effectItemList[i]->mgef->properties.primaryValue == 40) //Poison
											{
												ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
												break;
											}
										}
									}
								}
								return EventResult::kEvent_Continue;
							}
							else if (hazard->data.impactDataSet != nullptr)
							{
								if (hazard->data.impactDataSet->formID == 0x26113
									|| hazard->data.impactDataSet->formID == 0x1c2af
									|| hazard->data.impactDataSet->formID == 0x647bd
									|| hazard->data.impactDataSet->formID == 0x20de1
									|| hazard->data.impactDataSet->formID == 0x6051e
									|| hazard->data.impactDataSet->formID == 0x8f3f0) //fire
								{
									ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
								}
								else if (hazard->data.impactDataSet->formID == 0x18a2e
									|| hazard->data.impactDataSet->formID == 0xd6c64
									|| hazard->data.impactDataSet->formID == 0x32da7
									|| hazard->data.impactDataSet->formID == 0x49b64
									|| hazard->data.impactDataSet->formID == 0xd7863
									|| hazard->data.impactDataSet->formID == 0x3af15
									|| hazard->data.impactDataSet->formID == 0x8f3f6) //frost
								{
									ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
								}
								else if (hazard->data.impactDataSet->formID == 0x38b05
									|| hazard->data.impactDataSet->formID == 0x59ed9
									|| hazard->data.impactDataSet->formID == 0x25dab
									|| hazard->data.impactDataSet->formID == 0x592d0
									|| hazard->data.impactDataSet->formID == 0x665b9) //shock
								{
									ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
								}
								else if (hazard->data.impactDataSet->formID == 0xfff4b
									|| hazard->data.impactDataSet->formID == 0x46008) //Poison
								{
									ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
								}
								else
								{
									return EventResult::kEvent_Continue;
								}
							}
							else
							{
								return EventResult::kEvent_Continue;
							}
						}
						else if (enchantmentItem != nullptr) //Caused by an enchantmentItem
						{
							LOG("Source is an enchantmentItem. FormID: %x", enchantmentItem->formID);

							for (UInt32 i = 0; i < enchantmentItem->effectItemList.count; i++)
							{
								if (enchantmentItem->effectItemList[i]->mgef != nullptr)
								{
									if (((enchantmentItem->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Hostile) != 0)
										&& ((enchantmentItem->effectItemList[i]->mgef->properties.flags & EffectSetting::Properties::kEffectType_Detrimental) != 0))
									{
										//kResistFire = 41,
										//kResistShock = 42,
										//kResistFrost = 43,
										LOG("EnchantmentItem %x - effect: PrimaryValue: %d", enchantmentItem->formID, enchantmentItem->effectItemList[i]->mgef->properties.primaryValue);
										if (enchantmentItem->effectItemList[i]->mgef->properties.primaryValue == 41) //Fire
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFire, intensityMultiplierEnvironmentalFire);
											break;
										}
										else if (enchantmentItem->effectItemList[i]->mgef->properties.primaryValue == 42) //Shock
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalElectric, intensityMultiplierEnvironmentalShock);
											break;
										}
										else if (enchantmentItem->effectItemList[i]->mgef->properties.primaryValue == 43) //Frost
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalFrost, intensityMultiplierEnvironmentalFrost);
											break;
										}
										else if (enchantmentItem->effectItemList[i]->mgef->properties.primaryValue == 40) //Poison
										{
											ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentalPoison, intensityMultiplierEnvironmentalPoison);
											break;
										}
									}
								}
							}
							return EventResult::kEvent_Continue;
						}
						else
						{
							LOG("player attacked by other stuff. FormId: %x FormType: %d", sourceForm->formID, sourceForm->formType);
							if (!IsPlayingFeedback(FeedbackType::Explosion))
							{
								if (randomGenerator(0, 10) == 10)
								{
									ProvideHapticFeedback(0, 0, FeedbackType::DefaultHead, intensityMultiplierDefaultHead);
								}
								ProvideHapticFeedback(0, 0, FeedbackType::Default, intensityMultiplierDefault);
							}
							return EventResult::kEvent_Continue;
						}
					}
					else
					{
						LOG("player attacked by other stuff");
						if (randomGenerator(0, 10) == 10)
						{
							ProvideHapticFeedback(0, 0, FeedbackType::DefaultHead, intensityMultiplierDefaultHead);
						}
						ProvideHapticFeedback(0, 0, FeedbackType::Default, intensityMultiplierDefault);
						return EventResult::kEvent_Continue;
					}
				}
			}
		}
		else if (evn->caster == (*g_thePlayer))
		{
			//Actor *actor = DYNAMIC_CAST(evn->target, TESObjectREFR, Actor);

			if (evn->target != nullptr)
			{
				TESForm* sourceForm = LookupFormByID(evn->sourceFormID);

				if ((sourceForm && sourceForm->IsWeapon()) || evn->sourceFormID == 0x1f4)
				{
					if (GetModIndex(evn->sourceFormID) == weaponThrowVRModIndex && GetBaseFormID(evn->sourceFormID) != 0x9EF6B) //Ignore weaponthrowvr weapons
					{
						return EventResult::kEvent_Continue;
					}
					
					TESForm* rightEquippedObject = (*g_thePlayer)->GetEquippedObject(false);
					TESForm* leftEquippedObject = (*g_thePlayer)->GetEquippedObject(true);

					if (rightEquippedObject)
						LOG_INFO("rightEquippedObject: %08x", rightEquippedObject->formID);
					if (leftEquippedObject)
						LOG_INFO("leftEquippedObject: %08x", leftEquippedObject->formID);

					//LOG("sourceForm: %08x", sourceForm->formID);

					bool unarmedLeftHandAttack = false;
					bool unarmedRightHandAttack = false;
					if (evn->sourceFormID == 0x1f4)
					{
						if (rightEquippedObject != nullptr && leftEquippedObject != nullptr)
						{
							if (GetWhichArm((*g_thePlayer), evn->target))
							{
								unarmedLeftHandAttack = true;
							}
							else
							{
								unarmedRightHandAttack = true;
							}
						}
						else if (rightEquippedObject != nullptr)
						{
							unarmedLeftHandAttack = true;
						}
						else if (leftEquippedObject != nullptr)
						{
							unarmedRightHandAttack = true;
						}
						else
						{
							unarmedRightHandAttack = true;
						}
					}

					TESObjectWEAP* rightWeapon = nullptr;
					TESObjectWEAP* leftWeapon = nullptr;
					bool leftArm = false;
					if (unarmedRightHandAttack || unarmedLeftHandAttack)
					{
						if (rightEquippedObject != nullptr)
						{
							if (rightEquippedObject->IsWeapon())
							{
								rightWeapon = DYNAMIC_CAST(rightEquippedObject, TESForm, TESObjectWEAP);
								if (rightWeapon != nullptr)
								{
									if (rightWeapon->gameData.type == TESObjectWEAP::GameData::kType_Bow || rightWeapon->gameData.type == TESObjectWEAP::GameData::kType_Bow2)
									{
										return EventResult::kEvent_Continue;
									}
									LOG_INFO("rightWeapon: %08x", rightWeapon->formID);
								}
							}
						}
						if (leftEquippedObject != nullptr)
						{
							if (leftEquippedObject->IsWeapon())
							{
								leftWeapon = DYNAMIC_CAST(leftEquippedObject, TESForm, TESObjectWEAP);
								if (leftWeapon != nullptr)
								{
									if (leftWeapon->gameData.type == TESObjectWEAP::GameData::kType_Bow || leftWeapon->gameData.type == TESObjectWEAP::GameData::kType_Bow2)
									{
										return EventResult::kEvent_Continue;
									}
									LOG_INFO("leftWeapon: %08x", leftWeapon->formID);
								}
							}
						}

						if (rightEquippedObject == leftEquippedObject && !(rightWeapon != nullptr && TwoHandedWeapon(rightWeapon->type()))) // Determine which hand when they are same
						{
							//Get which arm is closer to the npc
							if (GetWhichArm((*g_thePlayer), evn->target))
								leftArm = true;
						}
					}
					if ((sourceForm == rightEquippedObject && !leftArm) || unarmedRightHandAttack)
					{
						if ((rightEquippedObject != nullptr) || unarmedRightHandAttack)
						{
							if (((rightWeapon != nullptr && rightWeapon->gameData.type != TESObjectWEAP::GameData::kType_Bow && rightWeapon->gameData.type != TESObjectWEAP::GameData::kType_Bow2) || unarmedRightHandAttack) || unarmedRightHandAttack)
							{
								LOG_INFO("attacked using right hand...");
								//melee
								//TODO: Add different effects for different weapon types here
								if (evn->flags & evn->kFlag_PowerAttack) //If power attack
								{
									LOG("power attack vibration on right arm");
									ProvideHapticFeedback(0, 0, FeedbackType::PlayerPowerAttackRight, intensityMultiplierPlayerPowerAttack);
								}
								else
								{
									if (evn->flags & evn->kFlag_Bash)
									{
										LOG("melee bash vibration on right arm");
										ProvideHapticFeedback(0, 0, FeedbackType::PlayerBashRight, intensityMultiplierPlayerBash);
									}
									else
									{
										LOG("melee vibration on right arm");
										ProvideHapticFeedback(0, 0, FeedbackType::PlayerAttackRight, intensityMultiplierPlayerAttack);
									}
								}
								return EventResult::kEvent_Continue;
							
							}
						}
					}
					else if ((sourceForm == leftEquippedObject) || unarmedLeftHandAttack)
					{
						if ((leftEquippedObject != nullptr) || unarmedLeftHandAttack)
						{
							if ((leftWeapon != nullptr && leftWeapon->gameData.type != TESObjectWEAP::GameData::kType_Bow && leftWeapon->gameData.type != TESObjectWEAP::GameData::kType_Bow2) || unarmedLeftHandAttack)
							{
								LOG_INFO("...using left hand...");
								//melee
								//TODO: Add different effects for different weapon types here
								if (evn->flags & evn->kFlag_PowerAttack) //If power attack
								{
									LOG("power attack vibration on left arm");
									ProvideHapticFeedback(0, 0, FeedbackType::PlayerPowerAttackLeft, intensityMultiplierPlayerPowerAttack);
								}
								else
								{
									if (evn->flags & evn->kFlag_Bash)
									{
										LOG("melee bash vibration on left arm");
										ProvideHapticFeedback(0, 0, FeedbackType::PlayerBashLeft, intensityMultiplierPlayerBash);
									}
									else
									{
										LOG("melee vibration on left arm");
										ProvideHapticFeedback(0, 0, FeedbackType::PlayerAttackLeft, intensityMultiplierPlayerAttack);
									}
								}
								return EventResult::kEvent_Continue;							
							}
						}
					}
				}
				else if (sourceForm && sourceForm->IsArmor())
				{
					LOG("Player attacked with shield.");
					ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::PlayerBashRight : FeedbackType::PlayerBashLeft, intensityMultiplierPlayerBash);					
				}
				else
				{
					if (evn->target == (*g_thePlayer))
					{
						LOG("Player did something to himself.");
						//ProvideHapticFeedback(0, 0, FeedbackType::MagicRestoration, intensityMultiplierMagic);
					}
				}
			}
		}

		return EventResult::kEvent_Continue;
	}

	FeedbackType GetLeftArmVersionOfMagic(FeedbackType feedback)
	{
		if (feedback == FeedbackType::MagicFire)
		{
			return FeedbackType::MagicLeftArmFire;
		}
		else if (feedback == FeedbackType::MagicShock)
		{
			return FeedbackType::MagicLeftArmShock;
		}
		else if (feedback == FeedbackType::MagicIce)
		{
			return FeedbackType::MagicLeftArmIce;
		}
		else if (feedback == FeedbackType::MagicRestoration)
		{
			return FeedbackType::MagicLeftArmRestoration;
		}
		else if (feedback == FeedbackType::MagicAlteration)
		{
			return FeedbackType::MagicLeftArmAlteration;
		}
		else if (feedback == FeedbackType::MagicIllusion)
		{
			return FeedbackType::MagicLeftArmIllusion;
		}
		else
		{
			return FeedbackType::MagicLeftArmIllusion;
		}
	}

	FeedbackType GetRightArmVersionOfMagic(FeedbackType feedback)
	{
		if (feedback == FeedbackType::MagicFire)
		{
			return FeedbackType::MagicRightArmFire;
		}
		else if (feedback == FeedbackType::MagicShock)
		{
			return FeedbackType::MagicRightArmShock;
		}
		else if (feedback == FeedbackType::MagicIce)
		{
			return FeedbackType::MagicRightArmIce;
		}
		else if (feedback == FeedbackType::MagicRestoration)
		{
			return FeedbackType::MagicRightArmRestoration;
		}
		else if (feedback == FeedbackType::MagicAlteration)
		{
			return FeedbackType::MagicRightArmAlteration;
		}
		else if (feedback == FeedbackType::MagicIllusion)
		{
			return FeedbackType::MagicRightArmIllusion;
		}
		else
		{
			return FeedbackType::MagicRightArmIllusion;
		}
	}

	FeedbackType GetHeadVersionOfMagic(FeedbackType feedback)
	{
		if(feedback == FeedbackType::MagicFire)
		{
			return FeedbackType::MagicHeadFire;
		}
		else if (feedback == FeedbackType::MagicShock)
		{
			return FeedbackType::MagicHeadShock;
		}
		else if (feedback == FeedbackType::MagicIce)
		{
			return FeedbackType::MagicHeadIce;
		}
		else if (feedback == FeedbackType::MagicRestoration)
		{
			return FeedbackType::MagicHeadRestoration;
		}
		else if (feedback == FeedbackType::MagicAlteration)
		{
			return FeedbackType::MagicHeadAlteration;
		}
		else if (feedback == FeedbackType::MagicIllusion)
		{
			return FeedbackType::MagicHeadIllusion;
		}
		else
		{
			return FeedbackType::MagicHeadIllusion;
		}
	}
	
	///Decides which type of magic feedback type it is
	FeedbackType DecideMagicFeedbackType(SpellItem * spell)
	{
		FeedbackType feedback = spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousOther : MagicOther;

		if (spell->effectItemList.count > 0)
		{
			for (UInt32 i = 0; i < spell->effectItemList.count; i++)
			{
				if (spell->effectItemList[i]->mgef != nullptr)
				{
					//kAlteration = 18,
					//kConjuration = 19,
					//kDestruction = 20,
					//kIllusion = 21,
					//kRestoration = 22,
					if (spell->effectItemList[i]->mgef->properties.school >= 18 && spell->effectItemList[i]->mgef->properties.school <= 22)
					{
						if (spell->effectItemList[i]->mgef->properties.school == EffectSetting::Properties::ActorValue::kAlteration)
						{
							return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousAlteration : FeedbackType::MagicAlteration;
						}
						else if (spell->effectItemList[i]->mgef->properties.school == EffectSetting::Properties::ActorValue::kIllusion)
						{
							return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousIllusion : FeedbackType::MagicIllusion;
						}
						else if (spell->effectItemList[i]->mgef->properties.school == EffectSetting::Properties::ActorValue::kRestoration)
						{
							return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousRestoration : FeedbackType::MagicRestoration;
						}
						else if (spell->effectItemList[i]->mgef->properties.school == EffectSetting::Properties::ActorValue::kDestruction)
						{
							if (spell->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistFire)
							{
								return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousFire : FeedbackType::MagicFire;
							}
							else if (spell->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistFrost)
							{
								return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousIce : FeedbackType::MagicIce;
							}
							else if (spell->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistShock)
							{
								return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousShock : FeedbackType::MagicShock;
							}
							else if (spell->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kPoisonResist)
							{
								return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousPoison : FeedbackType::MagicPoison;
							}
						}
						break;
					}
					else
					{
						if (spell->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kPoisonResist)
						{
							return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousPoison : FeedbackType::MagicPoison;
						}
						else if (spell->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistFire)
						{
							return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousFire : FeedbackType::MagicFire;
						}
						else if (spell->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistFrost)
						{
							return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousIce : FeedbackType::MagicIce;
						}
						else if (spell->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistShock)
						{
							return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousShock : FeedbackType::MagicShock;
						}
					}
				}
			}
		}

		if (feedback == MagicOther && spell->dispObj.worldStatic)
		{
			std::string str = spell->dispObj.worldStatic->texSwap.GetModelName();

			std::transform(str.begin(), str.end(), str.begin(), ::tolower);

			if (Contains(str, "fire") || Contains(str, "destr") || Contains(str, "flame") || Contains(str, "burn"))
			{
				return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousFire : FeedbackType::MagicFire;
			}
			else if (Contains(str, "shock") || Contains(str, "lightning") || Contains(str, "bolt") || Contains(str, "spark") || Contains(str, "elect"))
			{
				return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousShock : FeedbackType::MagicShock;
			}
			else if (Contains(str, "ice") || Contains(str, "frost") || Contains(str, "freez"))
			{
				return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousIce : FeedbackType::MagicIce;
			}
			else if (Contains(str, "restoration") || Contains(str, "heal") || Contains(str, "turnundead") || Contains(str, "drain") || Contains(str, "creeping"))
			{
				return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousRestoration : FeedbackType::MagicRestoration;
			}
			else if (Contains(str, "alter") || Contains(str, "detect") || Contains(str, "telekinesis") || Contains(str, "paralyz"))
			{
				return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousAlteration : FeedbackType::MagicAlteration;
			}
			else if (Contains(str, "illusion") || Contains(str, "invisib") || Contains(str, "vanish"))
			{
				return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousIllusion : FeedbackType::MagicIllusion;
			}
			else
			{
				return spell->data.castingType == SpellItem::CastingType::kConcentration ? FeedbackType::MagicContinuousOther : FeedbackType::MagicOther;
			}
			//Restoration: Restoration, Heal, TurnUndead
			//Fire: Destruction, Fire
			//Ice: Ice
			//Shock: Shock
			//Alteration: Alteration, Detect, Telekinesis, Paralyze
			//Illusion: Illusion, Invisibility
		}
		LOG("Couldn't find the magic effect applied");
		return feedback;
	}

	FeedbackType DecideShoutFeedbackType(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);

		if (Contains(str, "fire") || Contains(str, "destr") || Contains(str, "flame") || Contains(str, "burn"))
		{
			return FeedbackType::ShoutFire;
		}
		else if (Contains(str, "shock") || Contains(str, "lightning") || Contains(str, "bolt") || Contains(str, "spark") || Contains(str, "elect"))
		{
			return FeedbackType::ShoutLightning;
		}
		else if (Contains(str, "ice") || Contains(str, "frost") || Contains(str, "freez"))
		{
			return FeedbackType::ShoutFrost;
		}
		else if (Contains(str, "steam") || Contains(str, "vapor"))
		{
			return FeedbackType::ShoutSteam;
		}
		else
		{
			return FeedbackType::Shout;
		}
	}

	int CheckAttackLocation(Actor* player, Actor* actor, bool right) //return positive for left 0 for middle negative for right
	{
		if (actor && actor->loadedState && actor->loadedState->node)
		{
			BSFixedString hand(right ? "NPC R Hand [RHnd]" : "NPC L Hand [LHnd]");

			NiNode* rootNode = actor->loadedState->node->GetAsNiNode();

			if (rootNode)
			{
				NiAVObject* handnode = rootNode->GetObjectByName(&hand.data);

				if (handnode)
				{
					float heading = 0.0;
					float attitude = 0.0;
					float bank = 0.0;

					NiPoint3 position = actor->loadedState->node->m_worldTransform.pos;
					NiPoint3 handposition = handnode->m_worldTransform.pos;

					NiMatrix33 playerrotation = player->loadedState->node->m_worldTransform.rot;
					playerrotation.GetEulerAngles(&heading, &attitude, &bank);
					NiPoint3 playerposition = player->loadedState->node->m_worldTransform.pos;

					float playerHeading = heading * (180 / MATH_PI);

					float playerAttitude = attitude * (180 / MATH_PI);

					float calcPlayerHeading = 0.0;

					if (playerHeading == -180)
					{
						calcPlayerHeading = 180 - playerAttitude;
					}
					else
					{
						if (playerAttitude < 0)
						{
							calcPlayerHeading = 360 + playerAttitude;
						}
						else
						{
							calcPlayerHeading = playerAttitude;
						}
					}
					float angle = (std::atan2(position.y - playerposition.y, position.x - playerposition.x) - std::atan2(1, 0)) * (180 / MATH_PI);
					if (angle < 0) {
						angle = angle + 360;
					}

					float headingAngle = angle - calcPlayerHeading;
					if (headingAngle < 0)
						headingAngle += 360;
					else if (headingAngle > 360)
						headingAngle -= 360;

					float handangle = (std::atan2(handposition.y - playerposition.y, handposition.x - playerposition.x) - std::atan2(1, 0)) * (180 / MATH_PI);
					if (handangle < 0) {
						handangle = handangle + 360;
					}

					float handheadingAngle = handangle - calcPlayerHeading;
					if (handheadingAngle < 0)
						handheadingAngle += 360;
					else if (handheadingAngle > 360)
						handheadingAngle -= 360;


					float difference = handheadingAngle - headingAngle;

					if (difference > 180) //right
					{
						return -1;
					}
					else if (difference < -180) //left
					{
						return 1;
					}
					else
					{
						return difference;
					}

				}
			}
		}
		return -1;
	}

	bool GetWhichArm(Actor* player, TESObjectREFR* target) //return true for left arm
	{
		if (player && player->loadedState && player->loadedState->node)
		{
			auto& nodeList = (*g_thePlayer)->nodeList;
			if (!nodeList)
				return false;

			NiAVObject* leftnode = nodeList[leftWandNode];
			NiAVObject* rightnode = nodeList[rightWandNode];

			float leftDistance = 99999.0f;
			float rightDistance = 99999.0f;
			if (leftnode)
			{
				leftDistance = distanceNoSqrt(leftnode->m_worldTransform.pos, target->pos);
			}
			if (rightnode)
			{
				rightDistance = distanceNoSqrt(rightnode->m_worldTransform.pos, target->pos);
			}
			LOG("Left Arm Distance: %g - Right Arm Distance: %g", leftDistance, rightDistance);
			if (leftDistance < rightDistance)
				return true;
			else
				return false;
		}
		return false;
	}

	

	EventDispatcher<SKSEActionEvent>* g_skseActionEventDispatcher;

	MYSKSEActionEvent mySKSEActionEvent;

	EventResult MYSKSEActionEvent::ReceiveEvent(SKSEActionEvent * evn, EventDispatcher<SKSEActionEvent> * dispatcher)
	{
		_MESSAGE("SKSE Action Event: Slot:%d Type:%d Name: %s SourceFormId: %x", evn->slot, evn->type, evn->sourceForm != nullptr ? evn->sourceForm->GetName() : "No Name", evn->sourceForm != nullptr ? evn->sourceForm->formID : 0);
		if (evn->actor == (*g_thePlayer))
		{
//			_MESSAGE("SKSE Action Event: Slot:%d Type:%d Name: %s SourceFormId: %x", evn->slot, evn->type, evn->sourceForm != nullptr ? evn->sourceForm->GetName() : "No Name", evn->sourceForm != nullptr ? evn->sourceForm->formID : 0);


			if(evn->type == evn->kType_BowRelease)
			{
				if(evn->sourceForm != nullptr && evn->sourceForm->formType == kFormType_Weapon)
				{
					TESObjectWEAP* weapon = DYNAMIC_CAST(evn->sourceForm, TESForm, TESObjectWEAP);

					if (weapon && (weapon->type() == TESObjectWEAP::GameData::kType_CBow || weapon->type() == TESObjectWEAP::GameData::kType_CrossBow))
					{
						//Crossbow fire effect
						ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::PlayerCrossbowFireLeft : FeedbackType::PlayerCrossbowFireRight, intensityMultiplierPlayerCrossbowFire);
						ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::PlayerCrossbowKickbackLeft : FeedbackType::PlayerCrossbowKickbackRight, intensityMultiplierPlayerCrossbowKickback);
					}
				}
			}
			else if (evn->slot == evn->kSlot_Voice && (evn->type == evn->kType_VoiceCast || evn->type == evn->kType_SpellCast || evn->type == evn->kType_SpellFire))
			{
				TESForm* sourceForm = evn->sourceForm;

				SpellItem* spell = DYNAMIC_CAST(sourceForm, TESForm, SpellItem);

				if (spell)
				{
					_MESSAGE("Voice Spell event %s", spell->fullName.GetName());
					if (spell->data.spellType == SpellItem::SpellType::kVoicePower)
					{
						if (strcmp(spell->fullName.GetName(), "Bind") == 0)
						{
							ProvideHapticFeedback(0, 0, FeedbackType::PlayerShoutBindVest, intensityMultiplierPlayerShoutBind);
							std::thread t15(LateFeedback, 0, FeedbackType::PlayerShoutBindHands, intensityMultiplierPlayerShoutBind, 250, 1, false, false);
							t15.detach();
						}
						else
						{
							ProvideHapticFeedback(0, 0, FeedbackType::PlayerShout, intensityMultiplierPlayerShout);
							if (strcmp(spell->fullName.GetName(), "Sprint") == 0)
							{
								std::thread t15(LateFeedback, 0, FeedbackType::TravelEffect, intensityMultiplierTravelEffect, 800, 1, false, false);
								t15.detach();
							}
						}
					}
					else if (spell->data.spellType == SpellItem::SpellType::kPower || spell->data.spellType == SpellItem::SpellType::kLesserPower)
					{
						ProvideHapticFeedback(0, 0, FeedbackType::PlayerShoutBindHands, intensityMultiplierPlayerPower, false, true);
					}
				}
				else
				{
					TESShout* shout = DYNAMIC_CAST(sourceForm, TESForm, TESShout);
					if (shout)
					{
						_MESSAGE("Voice shout event %s", shout->fullName.GetName());
						if (strcmp(shout->fullName.GetName(), "Bind") == 0)
						{
							ProvideHapticFeedback(0, 0, FeedbackType::PlayerShoutBindVest, intensityMultiplierPlayerShoutBind);
							std::thread t15(LateFeedback, 0, FeedbackType::PlayerShoutBindHands, intensityMultiplierPlayerShoutBind, 250, 1, false, false);
							t15.detach();
						}
						else
						{
							ProvideHapticFeedback(0, 0, FeedbackType::PlayerShout, intensityMultiplierPlayerShout);
							if (strcmp(shout->fullName.GetName(), "Sprint") == 0)
							{
								std::thread t15(LateFeedback, 0, FeedbackType::TravelEffect, intensityMultiplierTravelEffect, 800, 1, false, false);
								t15.detach();
							}
						}
					}
				}
			} 
			else if (evn->type == evn->kType_SpellCast || evn->type == evn->kType_SpellFire) {

				TESForm* sourceForm = evn->sourceForm;

				SpellItem* spell = DYNAMIC_CAST(sourceForm, TESForm, SpellItem);

				if (spell)
				{
					_MESSAGE("Spell 2 %s", spell->fullName.GetName());
					std::thread t15(CheckAndPlayMagicArmorSpellEffect, spell);
					t15.detach();

				}
			}
		}		

		return EventResult::kEvent_Continue;
	}

	/*void HealEffectFeedback()
	{
		int angleOffset = randomGenerator(0, 359);
		ProvideHapticFeedback(angleOffset, 0, FeedbackType::MagicRestoration, intensityMultiplierMagic);
		Sleep(healsleepduration);
		angleOffset = randomGenerator(0, 359);
		ProvideHapticFeedback(angleOffset, 0, FeedbackType::MagicRestoration, intensityMultiplierMagic);
	}*/
	
	float lowhealthpercentage = 0.2f;
	float verylowhealthpercentage = 0.1f;

	float GetCurrentHealth()
	{
		return (*g_thePlayer)->actorValueOwner.GetCurrent(24);
	}

	float GetMaxHealth()
	{
		return (*g_thePlayer)->actorValueOwner.GetMaximum(24);
	}

	void Heartbeat()
	{
		float playerHealth = 100.0;
		float playerHealthMaximum = 100.0;
		while (true)
		{
			if (!(*g_thePlayer) || !(*g_thePlayer)->loadedState)
			{
				Sleep(3000);
				continue;
			}

			playerHealthMaximum = GetMaxHealth();

			playerHealth = GetCurrentHealth();

			if (playerHealth / playerHealthMaximum < verylowhealthpercentage)
			{
				LOG("Very Low Health Effect");
				for (int i = 0; i < 7; i++)
				{
					ProvideHapticFeedback(0, 0, FeedbackType::HeartBeatFast, intensityMultiplierHeartBeatFast, true);
					Sleep(428);
				}
			}
			else if (playerHealth / playerHealthMaximum < lowhealthpercentage)
			{
				LOG("Low Health Effect");
				for (int i = 0; i < 4; i++)
				{
					ProvideHapticFeedback(0, 0, FeedbackType::HeartBeat, intensityMultiplierHeartBeat, true);
					Sleep(750);
				}
			}
			else
			{
				Sleep(3000);
			}
		}
	}

	int outerRadius = 1200000;
	int innerRadius = 90000;

	void WordWallCheck()
	{
		TESObjectCELL* curCell = nullptr;
		TESObjectREFR* wordwallreference;
		bool wordlearned = false;
		while (true)
		{
			Actor* player = DYNAMIC_CAST(LookupFormByID(0x14), TESForm, Actor);

			if (!player || !player->loadedState)
			{
				Sleep(5000);
				continue;
			}

			TESObjectCELL* cell = player->parentCell;
			if (!cell)
			{
				Sleep(5000);
				continue;
			}

			NiAVObject* playerObj = player->loadedState->node;

			if (cell != curCell) 
			{// Cell change
				//LOG("Cell Change");
				curCell = cell;

				wordwallreference = nullptr;
				wordlearned = false;

				for (int i = 0; i < cell->refData.maxSize; i++) {
					auto ref = cell->refData.refArray[i];
					if (ref.unk08 != NULL && ref.ref) {

						if (isItWordWall(ref.ref))
						{
							wordwallreference = ref.ref;
							break;
						}
					}
				}
			}

			if (wordwallreference != nullptr && !wordlearned)
			{
				float distancePwr2 = distanceNoSqrt(playerObj->m_worldTransform.pos, wordwallreference->pos);

				if (distancePwr2 >= outerRadius) //No effect
				{
					Sleep(2000);
					continue;
				}
				else if (distancePwr2 < innerRadius)
				{
					wordlearned = true;
					LOG("Just a little word wall effect, then stop %g", distancePwr2);
					for (int i = 0; i < 3; i++)
					{
						float intensity = (float)outerRadius / distancePwr2;
						ProvideHapticFeedback(0, 0, FeedbackType::WordWall, intensity*intensityMultiplierWordWall);
						Sleep(900);
					}
					continue;
				}
				else if (distancePwr2 < outerRadius)
				{
					float intensity = (float)outerRadius / distancePwr2;
					LOG("Word Wall Effect intensity:%g distance^2:%g", intensity, distancePwr2);
					ProvideHapticFeedback(0, 0, FeedbackType::WordWall, intensity*intensityMultiplierWordWall);
					Sleep(900);
					continue;
				}

			}
			else
			{
				Sleep(5000);
				continue;
			}
		}
	}

	bool isItWordWall(TESObjectREFR* ref)
	{
		TESForm* baseForm = ref->baseForm;

		if (baseForm)
		{
			BSFixedString bs = papyrusForm::GetWorldModelPath(baseForm);
			
			if (bs.data)
			{
				std::string modelPath = bs.data;
				std::transform(modelPath.begin(), modelPath.end(), modelPath.begin(), ::tolower);
				if (Contains(modelPath, "powerword"))
					return true;
			}		
		}
		return false;
	}

	int GetClassification(TESWeather * weather)
	{
		const auto flags = weather->data.flags; //*((byte *)weather + 0x66F);

		if (flags & 1)
			return 0;
		if (flags & 2)
			return 1;
		if (flags & 4)
			return 2;
		if (flags & 8)
			return 3;

		return -1;
	}

	void GetCurrentWeatherClassification(int &classification)
	{
		const auto skyPtr = *g_SkyPtr;
		if (skyPtr && skyPtr->currentWeather)
		{
			classification = GetClassification(skyPtr->currentWeather);
		}
		else
		{
			classification = -1;
		}
	}

	void GetOutgoingWeatherClassification(int &classification)
	{
		const auto skyPtr = *g_SkyPtr;
		if (skyPtr && skyPtr->outgoingWeather)
		{
			classification = GetClassification(skyPtr->outgoingWeather);
		}
		else
		{
			classification = -1;
		}
	}

	void WeatherCheck()
	{
		//Address found by: Find "const Sky::`vftable'", then go to the first subroutine that uses it, then right click "Jump to xref to operand", go to the first one(the subroutine), address is the first qword you see after that call sub.
		 //for skyrimvr 1.4.15 skse 2.0.10
				
		TESObjectCELL* cell = nullptr;

		Actor* player = nullptr;
		bool isItRaining = false;		

		int32_t curWeatherClassification = -1;
		int32_t outWeatherClassification = -1;

		g_SkyPtr = RelocPtr<Sky *>(0x2FC62C8);



		while (true)
		{
			player = DYNAMIC_CAST(LookupFormByID(0x14), TESForm, Actor);

			if (!player || !player->loadedState)
			{
				//LOG("player null. Waiting for 5seconds");
				raining.store(false);
				Sleep(5000);
				continue;
			}
			
			cell = player->parentCell;

			if (!cell)
			{
				raining.store(false);
				continue;
			}

			if (!(cell->unk120)) //Interior cell
			{
				//LOG("In interior cell. Waiting for 5 seconds");
				raining.store(false);
				Sleep(5000);
				continue;
			}
			else
			{
				if (std::find(notExteriorWorlds.begin(), notExteriorWorlds.end(), cell->unk120->formID) != notExteriorWorlds.end())
				{
					//LOG("In interior cell world. Waiting for 5 seconds");
					raining.store(false);
					Sleep(5000);
					continue;
				}
			}

			isItRaining = false;

			const auto skyPtr = *g_SkyPtr;
			if (skyPtr != nullptr)
			{
				GetCurrentWeatherClassification(curWeatherClassification);
				GetOutgoingWeatherClassification(outWeatherClassification);

				//LOG("Outgoing weather: %x (%d) - Current weather: %x (%d) Transition: %g - Next weather: %x", skyPtr->outgoingWeather ? skyPtr->outgoingWeather->formID : 0, outWeatherClassification, skyPtr->currentWeather ? skyPtr->currentWeather->formID : 0, curWeatherClassification, skyPtr->transition, skyPtr->nextWeather ? skyPtr->nextWeather->formID : 0);
				if (curWeatherClassification == 2 || outWeatherClassification == 2)
				{
					isItRaining = true;

					float curWeatherIntensity = 1.0f;
					float outWeatherIntensity = 1.0f;

					float curWeatherDensity = 10;
					float outWeatherDensity = 10;

					if (curWeatherClassification == 2)
					{
						if (skyPtr->currentWeather)
						{
							BGSShaderParticleGeometryData* particles = skyPtr->currentWeather->precipitationData;
							if (particles)
							{
								if (particles->unk20.count >= 11)
								{
									curWeatherIntensity = (*((float *)&(particles->unk20.entries[2])))*(*((float *)&(particles->unk20.entries[3])));
									curWeatherDensity = *((float *)&(particles->unk20.entries[11]));

									if (curWeatherIntensity == 0)
										curWeatherIntensity = 1.0f;
									if (curWeatherDensity == 0)
										curWeatherDensity = 10;
								}
								else if (particles->texture.str.data)
								{
									std::string particleTexture = particles->texture.str.data;
									std::transform(particleTexture.begin(), particleTexture.end(), particleTexture.begin(), ::tolower);
								}
							}
						}
					}
					else
					{
						curWeatherIntensity = 0.0f;
						curWeatherDensity = 0;
					}
					if (outWeatherClassification == 2)
					{
						if (skyPtr->outgoingWeather)
						{
							BGSShaderParticleGeometryData* particles = skyPtr->outgoingWeather->precipitationData;
							if (particles)
							{
								if (particles->unk20.count >= 11)
								{
									outWeatherIntensity = (*((float *)&(particles->unk20.entries[2])))*(*((float *)&(particles->unk20.entries[3])));
									outWeatherDensity = *((float *)&(particles->unk20.entries[11]));

									if (outWeatherIntensity == 0)
										outWeatherIntensity = 1.0f;
									if (outWeatherDensity == 0)
										outWeatherDensity = 10;
								}
								else if (particles->texture.str.data)
								{
									std::string particleTexture = particles->texture.str.data;
									std::transform(particleTexture.begin(), particleTexture.end(), particleTexture.begin(), ::tolower);
								}
							}
						}
					}
					else
					{
						outWeatherIntensity = 0.0f;
						outWeatherDensity = 0;
					}

					if (curWeatherClassification == 2)
					{
						if (outWeatherClassification == 2)
						{
							rainIntensity = ((curWeatherIntensity - outWeatherIntensity)*skyPtr->transition) + outWeatherIntensity;
							rainDensity = ((curWeatherDensity - outWeatherDensity)*skyPtr->transition) + outWeatherDensity;
						}
						else
						{
							float transMultiplier = skyPtr->transition;
							if (transMultiplier < 0.4f)
								transMultiplier = 0.0f;

							rainIntensity = curWeatherIntensity*skyPtr->transition;
							rainDensity = curWeatherDensity*skyPtr->transition;
						}
					}
					else
					{
						if (outWeatherClassification == 2)
						{
							float transMultiplier = (1.0f - skyPtr->transition);
							if (transMultiplier < 0.5f)
								transMultiplier = 0.0f;

							rainIntensity = outWeatherIntensity*transMultiplier;
							rainDensity = outWeatherDensity*transMultiplier;
						}
						else
						{
							rainIntensity = 0;
							rainDensity = 0;
						}
					}
				}
				if (rainIntensity <= 0.1f || rainDensity <= 1)
				{
					isItRaining = false;
				}
				
				if (isItRaining)
				{
					//LOG("Transition: %g", skyPtr->transition);
					if (raining.load() == false)
					{
						LOG("Enabling rain effects...");
						raining.store(true);
						std::thread t12(RaindropVestEffect);
						t12.detach();

						std::thread t13(RaindropArmEffect, false);
						t13.detach();

						std::thread t14(RaindropArmEffect, true);
						t14.detach();

						std::thread t15(RaindropHeadEffect);
						t15.detach();
					}
				}
				else
				{
					raining.store(false);
				}

				//Wind Detection
				if(skyPtr->currentWeather && !headIsInsideWaterGeneral.load())
				{					
					//LOG("Wind Speed: %d WindDir:%g WindDirRange:%g VisualEffect:%s", skyPtr->currentWeather->data.windSpeed, skyPtr->currentWeather->data.windDirection*180.0f/256.0f, skyPtr->currentWeather->data.windDirectionRange * 360.0f / 256.0f, (skyPtr->currentWeather->referenceEffect!=nullptr && skyPtr->currentWeather->data.visualEffectBegin > 0) ? "ON" : "OFF");
					//Wind Speed is the only thing that changes. Wind direction and range are same all the time :( Visual effect is changed correctly.

					const bool visualEffectOn = (skyPtr->currentWeather->referenceEffect != nullptr && skyPtr->currentWeather->data.visualEffectBegin > 0);
					if((intensityMultiplierWindEffect > TOLERANCE && skyPtr->currentWeather->data.windSpeed > WindEffectMinSpeed) || (intensityMultiplierBlizzardEffect > TOLERANCE && visualEffectOn && skyPtr->currentWeather->data.windSpeed > WindEffectMinSpeed)) //Apply wind effect
					{
						NiPoint3 forward(0, 100.0f, 0);

						const float range = (randomGeneratorLowMoreProbable(0, 5,6,50, 10)/10.0f);
						const float theta = (((skyPtr->currentWeather->data.windDirection) * 180.0f) / 256.0f) - 90.0f + randomGenerator(-range, range);
						forward = rotate(forward, NiPoint3(0, 0, 1.0f), theta / 57.295776f);

						float heading = 0.0;
						float attitude = 0.0;
						float bank = 0.0;

						NiMatrix33 playerrotation = (*g_thePlayer)->loadedState->node->m_worldTransform.rot;
						playerrotation.GetEulerAngles(&heading, &attitude, &bank);
						NiPoint3 playerposition = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;

						NiPoint3 position = (forward) * -10.0f + playerposition;

						float playerHeading = heading * (180 / MATH_PI);

						float playerAttitude = attitude * (180 / MATH_PI);

						float calcPlayerHeading = 0.0;

						if (playerHeading == -180)
						{
							calcPlayerHeading = 180 - playerAttitude;
						}
						else
						{
							if (playerAttitude < 0)
							{
								calcPlayerHeading = 360 + playerAttitude;
							}
							else
							{
								calcPlayerHeading = playerAttitude;
							}
						}
						float angle = (std::atan2(position.y - playerposition.y, position.x - playerposition.x) - std::atan2(1, 0)) * (180 / MATH_PI);
						if (angle < 0) {
							angle = angle + 360;
						}

						float headingAngle = angle - calcPlayerHeading;
						if (headingAngle < 0)
							headingAngle += 360;
						else if (headingAngle > 360)
							headingAngle -= 360;

						float intensityAdd = 0.0f;

						const float angleBetween = angleBetweenVectors(forward, vestVelocity);
						const float angleBetweenReverse = angleBetweenVectors(forward*-1.0f, vestVelocity);
						if(angleBetween <= 45.0f)
						{
							intensityAdd = ((45.0f - angleBetween) / 45.0f) * 0.33f;
						}
						if (angleBetweenReverse <= 45.0f)
						{
							intensityAdd = ((45.0f - angleBetween) / 45.0f) * -0.33f;
						}
						
						if(intensityMultiplierBlizzardEffect > TOLERANCE && visualEffectOn && skyPtr->currentWeather->data.windSpeed > WindEffectMinSpeed)
						{
							ProvideHapticFeedback(headingAngle, 0, FeedbackType::Wind, ((randomGenerator(5, 10) / 10.0f) +intensityAdd)*intensityMultiplierBlizzardEffect*((float)skyPtr->currentWeather->data.windSpeed/(float)WindEffectMinSpeed), false);
						}
						else
						{
							ProvideHapticFeedback(headingAngle, 0, FeedbackType::Wind, ((randomGenerator(5,10)/10.0f) + intensityAdd)*intensityMultiplierWindEffect*((float)skyPtr->currentWeather->data.windSpeed / (float)WindEffectMinSpeed), false);
						}
					}
				}				
				Sleep(500);
			}
			else
			{
				//LOG("Sky is null. waiting for 5 seconds.");
				raining.store(false);
				Sleep(5000);
			}
		}
	}

	void RaindropVestEffect()
	{
		float minsleepduration = 100.0f;
		int minsleepDurationInt = 100;
		int sleepDuration = 100;

		while (raining.load() == true)
		{
			if (rainDensity > 0 && rainIntensity > 0 && !headIsInsideWaterGeneral.load())
			{
				minsleepduration = (float)rainvestsleepduration / rainDensity;
				minsleepDurationInt = static_cast<int>(minsleepduration);

				sleepDuration = randomGenerator(minsleepDurationInt, minsleepDurationInt * 2);

				const int index = randomGeneratorLowMoreProbable(0, 7, 8, 15, 4);
				const int pos = randomGenerator(0, 1);
				const int durationOffset = randomGenerator(0, 30) - 15;
				ProvideDotFeedback(pos == 1 ? OWOGame::MusclesGroup::Front() : OWOGame::MusclesGroup::Back(), index, 30 * rainIntensity * intensityMultiplierRaindropVest, raineffectduration + durationOffset);
			}
			Sleep(sleepDuration);
		}
	}

	void RaindropArmEffect(bool left)
	{
		float minsleepduration = 100.0;
		int minsleepDurationInt = 100;
		int sleepDuration = 100;

		while (raining.load() == true)
		{
			if (rainDensity > 0 && rainIntensity > 0 && !headIsInsideWaterGeneral.load())
			{
				minsleepduration = (float)rainarmsleepduration / rainDensity;
				minsleepDurationInt = static_cast<int>(minsleepduration);

				sleepDuration = randomGenerator(minsleepDurationInt, minsleepDurationInt * 2);

				const int index = randomGenerator(0, 5);

				const int durationOffset = randomGenerator(0, 30) - 15;
				// ProvideDotFeedback(left ? bhaptics::ForearmL : bhaptics::ForearmR, index, 30 * rainIntensity * intensityMultiplierRaindropArm, raineffectduration + durationOffset);
			}

			Sleep(sleepDuration);
		}
	}

	void RaindropHeadEffect()
	{
		float minsleepduration = 100.0;
		int minsleepDurationInt = 100;
		int sleepDuration = 1;

		while (raining.load() == true)
		{
			if (rainDensity > 0 && rainIntensity > 0 && !headIsInsideWaterGeneral.load())
			{
				minsleepduration = (float)rainarmsleepduration / rainDensity;
				minsleepDurationInt = static_cast<int>(minsleepduration);

				sleepDuration = randomGenerator(minsleepDurationInt, minsleepDurationInt * 2);

				const int index = randomGenerator(0, 5);
				const int durationOffset = randomGenerator(0, 30) - 15;
				// ProvideDotFeedback(bhaptics::Head, index, 30 * rainIntensity * intensityMultiplierRaindropHead, raineffectduration + durationOffset);
			}
			Sleep(sleepDuration);
		}
	}

	std::chrono::steady_clock::time_point beginHoldingBreath;
	NiPoint3 lastPlayerPos;
	
	void SwimmingEffects(BGSWaterSystemManager* waterSystemManager)
	{
		auto& nodeList = (*g_thePlayer)->nodeList;

		NiAVObject* hmd_node = nodeList[hmdNode];
		NiNode* m_nodePlayer3rdP = (*g_thePlayer)->GetNiRootNode(0);
		NiAVObject* spine0_node = nullptr;

		const float lowerZ = (hmd_node->m_worldTransform.pos.z - (*g_thePlayer)->pos.z)* 0.45f + (*g_thePlayer)->pos.z;//0.5834f + (*g_thePlayer)->pos.z;
		const float higherZ = (hmd_node->m_worldTransform.pos.z - (*g_thePlayer)->pos.z) * 0.95f + (*g_thePlayer)->pos.z;//0.9166f + (*g_thePlayer)->pos.z;

		const float SecondLineZ = (higherZ - lowerZ) * 0.2f + lowerZ;
		const float ThirdLineZ = (higherZ - lowerZ) * 0.4f + lowerZ;
		const float FourthLineZ = (higherZ - lowerZ) * 0.6f + lowerZ;
		const float FifthLineZ = (higherZ - lowerZ) * 0.8f + lowerZ;

		const NiPoint3 vestLowPos = NiPoint3((*g_thePlayer)->pos.x, (*g_thePlayer)->pos.y, lowerZ);
		//NiPoint3 vestHighPos = NiPoint3((*g_thePlayer)->pos.x, (*g_thePlayer)->pos.y, higherZ);		
		/*if (m_nodePlayer3rdP != nullptr)
		{
			spine0_node = m_nodePlayer3rdP->GetObjectByName(&spine0Name.data);

			if (spine0_node != nullptr)
			{
				vestLowPos = spine0_node->m_worldTransform.pos + NiPoint3(0,0,-40.0f);
			}
		}*/

		vestVelocity = hmd_node->m_worldTransform.pos - lastPlayerPos;
		
		lastPlayerPos = hmd_node->m_worldTransform.pos;
		
		if (waterSystemManager)
		{
			bool headIsInsideWater = false;

			bool playerIsInsideWater = false;

			bool enterOnce = true;
			
			for (UInt32 i = 0; i < waterSystemManager->waterObjects.count; i++)
			{
				TESWaterObject* waterObject;
				waterSystemManager->waterObjects.GetNthItem(i, waterObject);
				if (waterObject)
				{
					for (UInt32 j = 0; j < waterObject->multiBounds.count; j++)
					{
						BSMultiBoundAABB* bound;
						waterObject->multiBounds.GetNthItem(j, bound);

						if (bound)
						{
							if (bound->size.z <= 10.0f)
							{	//avoid sloped water
								const NiPoint3 boundMin = bound->center - bound->size;
								const NiPoint3 boundMax = bound->center + bound->size;

								if (vestLowPos.x > boundMin.x && vestLowPos.x < boundMax.x
									&& vestLowPos.y > boundMin.y && vestLowPos.y < boundMax.y
									&& vestLowPos.z-100.0f < boundMax.z)
								{
									playerIsInsideWater = true;
								}

								if (enterOnce && vestLowPos.x > boundMin.x && vestLowPos.x < boundMax.x
									&& vestLowPos.y > boundMin.y && vestLowPos.y < boundMax.y
									&& vestLowPos.z < boundMax.z)
								{
									
									if(((*g_thePlayer)->flags2 & Actor::kFlag_kUnderwater) == Actor::kFlag_kUnderwater && GetCurrentWaterBreathing() < TOLERANCE)
									{
										headIsInsideWater = true;
										if (headIsInsideWaterGeneral.load() == false)
										{
											LOG("Head is inside water.");
											//Start countdown
											beginHoldingBreath = std::chrono::steady_clock::now();
										}
										else
										{
											double secondsPassed = ((std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - beginHoldingBreath).count()) / 1000000.0);
											//LOG("Seconds passed with head inside water: %g / %g - WaterBreathing: %g", secondsPassed, holdBreathLimit, GetCurrentWaterBreathing());
											if (secondsPassed > holdBreathLimit-2.0)
											{
												//Play drowning effect
												ProvideHapticFeedback(0, 0, FeedbackType::DrowningEffectVest, intensityMultiplierDrowningVest, true);
												ProvideHapticFeedback(0, 0, FeedbackType::DrowningEffectHead, intensityMultiplierDrowningHead, true);
											}
										}
									}
									
									//Inside this body of water
									if (vlibIsWalking() || vlibIsRunning() || vlibIsSprinting())
									{
										float heading = 0.0;
										float attitude = 0.0;
										float bank = 0.0;

										NiMatrix33 playerrotation = (*g_thePlayer)->loadedState->node->m_worldTransform.rot;
										playerrotation.GetEulerAngles(&heading, &attitude, &bank);
										NiPoint3 playerposition = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;
										
										NiPoint3 position = vestVelocity*10.0f + playerposition;

										float playerHeading = heading * (180 / MATH_PI);

										float playerAttitude = attitude * (180 / MATH_PI);

										float calcPlayerHeading = 0.0;

										if (playerHeading == -180)
										{
											calcPlayerHeading = 180 - playerAttitude;
										}
										else
										{
											if (playerAttitude < 0)
											{
												calcPlayerHeading = 360 + playerAttitude;
											}
											else
											{
												calcPlayerHeading = playerAttitude;
											}
										}
										float angle = (std::atan2(position.y - playerposition.y, position.x - playerposition.x) - std::atan2(1, 0)) * (180 / MATH_PI);
										if (angle < 0) {
											angle = angle + 360;
										}

										float headingAngle = angle - calcPlayerHeading;
										if (headingAngle < 0)
											headingAngle += 360;
										else if (headingAngle > 360)
											headingAngle -= 360;

										float intensity = magnitude(vestVelocity)/150.0f;

										if (FifthLineZ < boundMax.z)
										{
											ProvideHapticFeedback(headingAngle, 0, FeedbackType::SwimVest100, intensity*intensityMultiplierPlayerSwimming);
											LOG("Moving inside the body of water %d - 100 percent", i);
										}
										else if (FourthLineZ < boundMax.z)
										{
											ProvideHapticFeedback(headingAngle, 0, FeedbackType::SwimVest80, intensity*intensityMultiplierPlayerSwimming);
											LOG("Moving inside the body of water %d - 80 percent", i);
										}
										else if (ThirdLineZ < boundMax.z)
										{
											ProvideHapticFeedback(headingAngle, 0, FeedbackType::SwimVest60, intensity*intensityMultiplierPlayerSwimming);
											LOG("Moving inside the body of water %d - 60 percent", i);
										}
										else if (SecondLineZ < boundMax.z)
										{
											ProvideHapticFeedback(headingAngle, 0, FeedbackType::SwimVest40, intensity*intensityMultiplierPlayerSwimming);
											LOG("Moving inside the body of water %d - 40 percent", i);
										}
										else
										{
											ProvideHapticFeedback(headingAngle, 0, FeedbackType::SwimVest20, intensity*intensityMultiplierPlayerSwimming);
											LOG("Moving inside the body of water %d - 20 percent", i);
										}
									}
									enterOnce = false;
								}
							}
						}
					}
				}
			}
			headIsInsideWaterGeneral.store(headIsInsideWater);
			playerIsInsideWaterGeneral.store(playerIsInsideWater);
			
		}
	}

	void SwimmingCheck()
	{
		BGSWaterSystemManager* waterSystemManager = BGSWaterSystemManager::GetSingleton();

		while (true)
		{
			if (!(*g_thePlayer) || !(*g_thePlayer)->loadedState)
			{
				Sleep(5000);
				continue;
			}

			if(waterSystemManager == nullptr)
			{
				waterSystemManager = BGSWaterSystemManager::GetSingleton();
			}

			if (waterSystemManager != nullptr)
			{
				SwimmingEffects(waterSystemManager);
			}
			Sleep(swimmingsleepduration);
		}
	}
	
	///Decides which type of spell it is
	std::vector<unsigned short>* DecideSpellType(SpellItem* spell, bool staff, bool left, FeedbackType &spellFeedBackType)
	{
		if (spell->effectItemList.count > 0)
		{
			for (UInt32 i = 0; i < spell->effectItemList.count; i++)
			{
				if (spell->effectItemList[i]->mgef != nullptr)
				{
					//kAlteration = 18,
					//kConjuration = 19,
					//kDestruction = 20,
					//kIllusion = 21,
					//kRestoration = 22,
					if (spell->effectItemList[i]->mgef->properties.school >= 18 && spell->effectItemList[i]->mgef->properties.school <= 22)
					{
						if (spell->effectItemList[i]->mgef->properties.school == EffectSetting::Properties::ActorValue::kAlteration)
						{
							if(spell->effectItemList[i]->mgef->properties.archetype == 12)
							{
								spellFeedBackType = left ? FeedbackType::PlayerSpellLightLeft : FeedbackType::PlayerSpellLightRight;
								LOG("lightReady spell selected");
								return &lightReadyArray1;
							}
							else
							{
								spellFeedBackType = left ? FeedbackType::PlayerSpellAlterationLeft : FeedbackType::PlayerSpellAlterationRight;
								if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
								{
									LOG("alterationReady spell selected");
									return &alterationReadyArray1;
								}
								else
								{
									LOG("alteration spell selected");
									return &alterationArray1;
								}
							}
						}
						else if (spell->effectItemList[i]->mgef->properties.school == EffectSetting::Properties::ActorValue::kIllusion)
						{
							spellFeedBackType = left ? FeedbackType::PlayerSpellIllusionLeft : FeedbackType::PlayerSpellIllusionRight;
							if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
							{
								LOG("illusionReady spell selected");
								return &illusionReadyArray1;
							}
							else
							{
								LOG("illusion spell selected");
								return &illusionArray1;
							}
						}
						else if (spell->effectItemList[i]->mgef->properties.school == EffectSetting::Properties::ActorValue::kRestoration)
						{
							if(spell->effectItemList[i]->mgef->properties.primaryValue == EffectSetting::Properties::ActorValue::kWardPower)
							{
								spellFeedBackType = left ? FeedbackType::PlayerSpellWardLeft : FeedbackType::PlayerSpellWardRight;

								LOG("ward spell selected");
								return &wardArray1;
							}
							else
							{
								spellFeedBackType = left ? FeedbackType::PlayerSpellRestorationLeft : FeedbackType::PlayerSpellRestorationRight;
								if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
								{
									LOG("restorationReady spell selected");
									return &restorationReadyArray1;
								}
								else
								{
									LOG("restoration spell selected");
									return &restorationArray1;
								}
							}
						}
						else if (spell->effectItemList[i]->mgef->properties.school == EffectSetting::Properties::ActorValue::kConjuration)
						{
							spellFeedBackType = left ? FeedbackType::PlayerSpellConjurationLeft : FeedbackType::PlayerSpellConjurationRight;

							LOG("conjurationReady spell selected");
							return &conjurationReadyArray1;
						}
						else if (spell->effectItemList[i]->mgef->properties.school == EffectSetting::Properties::ActorValue::kDestruction)
						{
							if (spell->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistFire)
							{								
								spellFeedBackType = left ? FeedbackType::PlayerSpellFireLeft : FeedbackType::PlayerSpellFireRight;
								if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
								{
									LOG("fireReady spell selected");
									return &fireReadyArray1;
								}
								else
								{
									LOG("fire spell selected");
									return &fireArray1;
								}
							}
							else if (spell->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistFrost)
							{
								spellFeedBackType = left ? FeedbackType::PlayerSpellIceLeft : FeedbackType::PlayerSpellIceRight;
								if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
								{
									LOG("iceReady spell selected");
									return &iceReadyArray1;
								}
								else
								{
									LOG("ice spell selected");
									return &iceArray1;
								}
							}
							else if (spell->effectItemList[i]->mgef->properties.resistance == EffectSetting::Properties::ActorValue::kResistShock)
							{
								spellFeedBackType = left ? FeedbackType::PlayerSpellShockLeft : FeedbackType::PlayerSpellShockRight;
								if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
								{
									LOG("shockReady spell selected");
									return &shockReadyArray1;
								}
								else
								{
									LOG("shock spell selected");
									return &shockArray1;
								}
							}
						}
						break;
					}
				}
			}
		}
		
		std::string str = "";
		if(staff)
		{
			str = spell->fullName.GetName();
		}
		else if (spell->dispObj.worldStatic)
		{
			str = spell->dispObj.worldStatic->texSwap.GetModelName();
		}
		
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		
		if (Contains(str, "fire") || Contains(str, "destr") || Contains(str, "flame") || Contains(str, "burn"))
		{
			if (left)
				spellFeedBackType = FeedbackType::PlayerSpellFireLeft;
			else
				spellFeedBackType = FeedbackType::PlayerSpellFireRight;

			if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
			{
				LOG("fireReady spell selected");
				return &fireReadyArray1;
			}
			else
			{
				LOG("fire spell selected");
				return &fireArray1;
			}
		}
		else if (Contains(str, "shock") || Contains(str, "lightning") || Contains(str, "bolt") || Contains(str, "spark") || Contains(str, "elect"))
		{
			if (left)
				spellFeedBackType = FeedbackType::PlayerSpellShockLeft;
			else
				spellFeedBackType = FeedbackType::PlayerSpellShockRight;

			if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
			{
				LOG("shockReady spell selected");
				return &shockReadyArray1;
			}
			else
			{
				LOG("shock spell selected");
				return &shockArray1;
			}
		}
		else if (Contains(str, "ice") || Contains(str, "frost") || Contains(str, "freez"))
		{
			if (left)
				spellFeedBackType = FeedbackType::PlayerSpellIceLeft;
			else
				spellFeedBackType = FeedbackType::PlayerSpellIceRight;

			if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
			{
				LOG("iceReady spell selected");
				return &iceReadyArray1;
			}
			else
			{
				LOG("ice spell selected");
				return &iceArray1;
			}
		}
		else if (Contains(str, "restoration") || Contains(str, "heal") || Contains(str, "turnundead") || Contains(str, "drain") || Contains(str, "creeping"))
		{
			if (left)
				spellFeedBackType = FeedbackType::PlayerSpellRestorationLeft;
			else
				spellFeedBackType = FeedbackType::PlayerSpellRestorationRight;

			if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
			{
				LOG("restorationReady spell selected");
				return &restorationReadyArray1;
			}
			else
			{
				LOG("restoration spell selected");
				return &restorationArray1;
			}
		}
		else if (Contains(str, "ward") || Contains(str, "protect"))
		{
			if (left)
				spellFeedBackType = FeedbackType::PlayerSpellWardLeft;
			else
				spellFeedBackType = FeedbackType::PlayerSpellWardRight;

			LOG("ward spell selected");
			return &wardArray1;
		}
		else if (Contains(str, "conjur") || Contains(str, "summon") || Contains(str, "bound") || Contains(str, "banish") || Contains(str, "reanimate"))
		{
			if (left)
				spellFeedBackType = FeedbackType::PlayerSpellConjurationLeft;
			else
				spellFeedBackType = FeedbackType::PlayerSpellConjurationRight;

			LOG("conjurationReady spell selected");
			return &conjurationReadyArray1;
		}
		else if (Contains(str, "alter") || Contains(str, "detect") || Contains(str, "telekinesis") || Contains(str, "paralyz"))
		{
			if (left)
				spellFeedBackType = FeedbackType::PlayerSpellAlterationLeft;
			else
				spellFeedBackType = FeedbackType::PlayerSpellAlterationRight;

			if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
			{
				LOG("alterationReady spell selected");
				return &alterationReadyArray1;
			}
			else
			{
				LOG("alteration spell selected");
				return &alterationArray1;
			}
		}
		else if (Contains(str, "illusion") || Contains(str, "invisib") || Contains(str, "vanish"))
		{
			if (left)
				spellFeedBackType = FeedbackType::PlayerSpellIllusionLeft;
			else
				spellFeedBackType = FeedbackType::PlayerSpellIllusionRight;

			if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
			{
				LOG("illusionReady spell selected");
				return &illusionReadyArray1;
			}
			else
			{
				LOG("illusion spell selected");
				return &illusionArray1;
			}
		}
		else if (Contains(str, "light"))
		{
			if (left)
				spellFeedBackType = FeedbackType::PlayerSpellLightLeft;
			else
				spellFeedBackType = FeedbackType::PlayerSpellLightRight;

			LOG("lightReady spell selected");
			return &lightReadyArray1;
		}
		else
		{
			if (left)
				spellFeedBackType = FeedbackType::PlayerSpellOtherLeft;
			else
				spellFeedBackType = FeedbackType::PlayerSpellOtherRight;

			if (spell->data.castingType == SpellItem::CastingType::kFireAndForget)
			{
				LOG("defaultType fire and forget spell selected");
				if (defaultType == 1)
					return &fireReadyArray1;
				else if (defaultType == 2)
					return &shockReadyArray1;
				else if (defaultType == 3)
					return &iceReadyArray1;
				else if (defaultType == 4)
					return &alterationReadyArray1;
				else if (defaultType == 5)
					return &illusionReadyArray1;
				else if (defaultType == 6)
					return &restorationReadyArray1;
			}
			else
			{
				LOG("defaultType concentration spell selected");
				if (defaultType == 1)
					return &fireArray1;
				else if (defaultType == 2)
					return &shockArray1;
				else if (defaultType == 3)
					return &iceArray1;
				else if (defaultType == 4)
					return &alterationArray1;
				else if (defaultType == 5)
					return &illusionArray1;
				else if (defaultType == 6)
					return &restorationArray1;
			}
		}
		//Restoration: Restoration, Heal, TurnUndead
		//Fire: Destruction, Fire
		//Ice: Ice
		//Shock: Shock
		//Ward: Ward
		//Conjuration: Conjuration, Summon, Bound, Banish, Reanimate
		//Alteration: Alteration, Detect, Telekinesis, Paralyze
		//Illusion: Illusion, Invisibility
		//Light: Light

		LOG("Couldn't find the spell selected");
		return &restorationArray1;
	}

	bool DecideCastType(SpellItem* spell)
	{
		VMResultArray<EffectSetting*> effects = magicItemUtils::GetMagicEffects(spell);

		for each (EffectSetting* var in effects)
		{
			if (var)
			{
				if (var->properties.castType == var->properties.kCastingType_Concentration)
					return false;
			}
		}
		return true;
	}

	bool GetDeliveryTypeSelf(SpellItem* spell)
	{
		VMResultArray<EffectSetting*> effects = magicItemUtils::GetMagicEffects(spell);

		for each (EffectSetting* var in effects)
		{
			if (var)
			{
				if (var->properties.deliveryType == var->properties.kDeliveryType_Self)
					return true;

			}
		}
		return false;
	}

	bool GetEffectTypeRecover(SpellItem* spell)
	{
		VMResultArray<EffectSetting*> effects = magicItemUtils::GetMagicEffects(spell);

		for each (EffectSetting* var in effects)
		{
			if (var)
			{
				if (var->properties.flags & var->properties.kEffectType_Recover)
					return true;
			}
		}
		return false;
	}

	UInt32 GetSchool(SpellItem* spell)
	{
		VMResultArray<EffectSetting*> effects = magicItemUtils::GetMagicEffects(spell);

		for each (EffectSetting* var in effects)
		{
			if (var)
			{
				if (var->school() > 0)
				{
					LOG("School: %08x", var->school());
					return var->school();
				}
			}
		}
		return 0;
	}

	BSFixedString isBlockingStr("IsBlocking");
	bool IsBlockingInternal(Actor* actor, TESObjectREFR* actorRef) //Atom's code
	{
		if (actor == nullptr)
			return false;

		return ((actor->actorState.flags08 >> 8) & 1) || sub_GetAnimationVariableBool((*g_skyrimVM)->GetClassRegistry(), 0, actorRef, &isBlockingStr);
	}

	UInt32 actorMagickaValue = -1;

	bool PlayerHasMagicka()
	{
		if (!(*g_thePlayer) || !(*g_thePlayer)->loadedState)
			return false;

		float magickaValue = 0.0;
		if (actorMagickaValue == -1)
		{
			BSFixedString magicka("Magicka");
			ActorValueInfo * magickaInfo = papyrusActorValueInfo::GetActorValueInfoByName(NULL, magicka);

			if (magickaInfo) {
				UInt32 actorValue = LookupActorValueByName(magickaInfo->name);
				if (actorValue < ActorValueList::kNumActorValues) {
					actorMagickaValue = actorValue;
				}
			}
		}

		if (actorMagickaValue != -1)
		{
			magickaValue = (*g_thePlayer)->actorValueOwner.GetCurrent(actorMagickaValue);
		}
		return magickaValue > 0.0;
	}

	void CheckAndPlayMagicArmorSpellEffect(SpellItem* spell)
	{
		Sleep(100);

		bool isMagicArmor = false;
		if (spell!=nullptr && spell->data.delivery == SpellItem::Delivery::kSelf && spell->effectItemList.count > 0)
		{
			for (UInt32 i = 0; i < spell->effectItemList.count; i++)
			{
				if (spell->effectItemList[i]->mgef != nullptr)
				{
					if(spell->effectItemList[i]->mgef->keywordForm.HasKeyword(KeywordMagicArmorSpell))
					{
						isMagicArmor = true;
					}
				}
			}
		}
		if(isMagicArmor)
		{
			//Play magicArmor effect
			ProvideHapticFeedback(0, 0, FeedbackType::MagicArmorSpell, intensityMultiplierMagicArmorSpell);
		}
	}

	void ReleaseBurst(bool dual, bool leftHand)
	{
		if (!ivrSystem)
			LOG("You are missing skyrimvrtools.dll!!!!!!!!");

		vr::TrackedDeviceIndex_t controller;
		_MESSAGE("Release burst %s", dual ? "dual" : (leftHand ? "left" : "right"));

		std::vector<unsigned short>* magicArray;
		auto player = DYNAMIC_CAST(LookupFormByID(0x14), TESForm, Actor);
		if (player != nullptr)
		{
			FeedbackType spellFeedbackType = leftHand ? FeedbackType::PlayerSpellOtherLeft : FeedbackType::PlayerSpellOtherRight;
			if (leftHand)
			{
				controller = ivrSystem->GetTrackedDeviceIndexForControllerRole(leftControllerRole);

				TESForm* leftHandObject = player->GetEquippedObject(true);
				if (player->leftHandSpell && leftHandObject != nullptr)
				{
					bool staff = leftHandObject->IsWeapon();
					
					magicArray = DecideSpellType(player->leftHandSpell, staff, true, spellFeedbackType);

					// TODO: Remove this? Since we are adding listening for the spell cast event
					std::thread t1(CheckAndPlayMagicArmorSpellEffect, player->leftHandSpell);
					t1.detach();
				}
			}
			else
			{
				controller = ivrSystem->GetTrackedDeviceIndexForControllerRole(rightControllerRole);

				TESForm* rightHandObject = player->GetEquippedObject(false);
				if (player->rightHandSpell && rightHandObject != nullptr)
				{
					bool staff = rightHandObject->IsWeapon();

					magicArray = DecideSpellType(player->rightHandSpell, staff, false, spellFeedbackType);
					// TODO: Remove this? Since we are adding listening for the spell cast event
					std::thread t1(CheckAndPlayMagicArmorSpellEffect, player->rightHandSpell);
					t1.detach();
				}
			}

			if (magicArray == nullptr || magicArray->size() < 5)
				magicArray = &restorationReadyArray1;

			Sleep(100);

			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < magicreleaseburstloopcount; j++)
				{
					const float intensity = ((float)magicArray->at(i) / 3999.0f) * (intensityMultiplierPlayerSpell);
					if (dual)
					{
						ProvideHapticFeedback(randomGenerator(0, 359), 0, spellFeedbackType, intensity, true);
						ProvideHapticFeedback(randomGenerator(0, 359), 0, GetOtherHandSpellFeedback(spellFeedbackType), intensity, true);
					}
					else
					{
						ProvideHapticFeedback(randomGenerator(0, 359), 0, spellFeedbackType, intensity, true);
					}
					Sleep(4);
				}
			}
		}

	}

	//Magic effect thread functions
	void StartMagicEffectRight(std::vector<unsigned short>* magicArray, bool shortSpell, bool staff, FeedbackType spellFeedbackType, bool selfHealingSpell)
	{
		if (magicArray->size() > 2545454)
			magicArray = &restorationArray1;

		if (!magicArray || magicArray->size() < 2)
			return;

		rightMagic.store(true);

		if (selfHealingSpell)
		{
			std::thread t8(StartHealingEffect, true);
			t8.detach();
		}
		TESObjectREFR* actorRef = DYNAMIC_CAST(LookupFormByID(0x14), TESForm, TESObjectREFR);
		bool gameStopped = false;
		int i = 0;
		while ((staff || PlayerHasMagicka()) && IsTriggerPressed(false) && !IsBlockingInternal(*g_thePlayer, actorRef))
		{
			if (i < magicArray->size() - 1)
			{
				i++;
				if (i % 10 == 0)
				{
					gameStopped = isGameStopped();
				}
			}
			else
			{
				i = 0;
				if (shortSpell)
					break;
			}

			if (!gameStopped)
			{
				ProvideHapticFeedback(randomGenerator(0, 359), 0, spellFeedbackType, ((float)magicArray->at(i) / 3999.0f) * (staff ? intensityMultiplierPlayerStaff : intensityMultiplierPlayerSpell), true);
			}
			Sleep(magicsleepduration);
		}
		Sleep(50);
		rightMagic.store(false);

	}

	void StartMagicEffectLeft(std::vector<unsigned short>* magicArray, bool shortSpell, bool staff, FeedbackType spellFeedbackType, bool selfHealingSpell)
	{
		if (magicArray->size() > 2545454)
			magicArray = &restorationArray1;

		if (!magicArray || magicArray->size() < 2)
			return;

		leftMagic.store(true);

		if (selfHealingSpell)
		{
			std::thread t8(StartHealingEffect, false);
			t8.detach();
		}
		TESObjectREFR* actorRef = DYNAMIC_CAST(LookupFormByID(0x14), TESForm, TESObjectREFR);
		bool gameStopped = false;
		int i = 0;
		while ((staff || PlayerHasMagicka()) && IsTriggerPressed(true) && !IsBlockingInternal(*g_thePlayer, actorRef))
		{
			if (i < magicArray->size() - 1)
			{
				i++;
				if (i % 10 == 0)
				{
					gameStopped = isGameStopped();
				}
			}
			else
			{
				i = 0;
				if (shortSpell)
					break;
			}

			if (!gameStopped)
			{
				ProvideHapticFeedback(randomGenerator(0, 359), 0, spellFeedbackType, ((float)magicArray->at(i) / 3999.0f) * (staff ? intensityMultiplierPlayerStaff : intensityMultiplierPlayerSpell), true);
			}
			Sleep(magicsleepduration);
		}
		Sleep(50);
		leftMagic.store(false);

	}

	EventDispatcher<TESContainerChangedEvent>* g_ContainerChangedEventDispatcher;
	ContainerChangedEventHandler g_ContainerChangedEventHandler;
	
	EventResult ContainerChangedEventHandler::ReceiveEvent(TESContainerChangedEvent * evn, EventDispatcher<TESContainerChangedEvent> * dispatcher)
	{
		auto player = DYNAMIC_CAST(LookupFormByID(0x14), TESForm, Actor);
		if (!player || !player->loadedState)
			return EventResult::kEvent_Continue;
		
		if (evn->fromFormId == player->formID && evn->toFormId == 0 && evn->itemFormId != 0 && evn->toReference == 0)
		{			
			TESForm * item = LookupFormByID(evn->itemFormId);
			
			if (item != nullptr)
			{
				if (item->formType == kFormType_Potion)
				{
					if (!menuTypes["Crafting Menu"])
					{
						LOG("Player consumed something. fromFormId:%08x toFormId:%08x itemFormId:%08x toref:%08x, unk14:%08x", evn->fromFormId, evn->toFormId, evn->itemFormId, evn->toReference, evn->unk14);
						AlchemyItem* potion = DYNAMIC_CAST(item, TESForm, AlchemyItem);

						if (potion != nullptr)
						{
							if (!(potion->keyword.HasKeyword(KeywordVendorItemPoison) || potion->IsPoison()))
							{
								if (!potion->IsFood()) //Potion
								{
									//drink effect first
									ProvideHapticFeedback(0, 0, FeedbackType::ConsumableDrink, intensityMultiplierDrink, true, true);

									//Heal effect
									/*std::thread t9(HealEffectWithMenuWait);
									t9.detach();*/
								}
								else
								{
									//drink/eat effect first
									ProvideHapticFeedback(0, 0, FeedbackType::ConsumableFood, intensityMultiplierEat, true, true);
								}
							}
						}
					}
				}
			}
		}

		return EventResult::kEvent_Continue;
	}
	
	/*void HealEffectWithMenuWait()
	{
		int i = 0;
		while (isGameStoppedNoDialogue() && i < 60)
		{
			i++;
			Sleep(1000);			
		}
		int angleOffset = randomGenerator(0, 359);
		ProvideHapticFeedback(angleOffset, 0, FeedbackType::MagicRestoration, intensityMultiplierMagic);
	}*/

	//uint64_t TriggerMask = 0;
	bool IsTriggerPressed(bool left)
	{
		if (DualSpellCastingOn.load())
			return true;

		if (left)
			return LeftHandSpellCastingOn.load();
		else
			return RightHandSpellCastingOn.load();
	}

	//Catches controller trigger presses. Required for both spellcasting and bow events to decide if it's time to trigger haptics.
	void OnVRButtonEventListener(PapyrusVR::VREventType eventType, PapyrusVR::EVRButtonId buttonId, PapyrusVR::VRDevice device)
	{
		if (eventType == PapyrusVR::VREventType::VREventType_Released && buttonId == PapyrusVR::EVRButtonId::k_EButton_SteamVR_Trigger && device == rightController)
		{
			bowPulled.store(false);
			bowPullCount = 0;
			bowReleaseCount = 0;
			lastleftPulse = 0;
			lastrightPulse = 0;
		}
	}

	void LatePlayDrowning()
	{
		Sleep(500);
		// TODO add back
		//if (!IsPlaying())
		//{
			ProvideHapticFeedback(0, 0, FeedbackType::DrowningEffectVest, intensityMultiplierDrowningVest, true);
		//}
	}

	bool ActorInCombat(Actor* actor)
	{
		UInt64* vtbl = *((UInt64**)actor);
		const bool combat = ((_IsInCombatNative)(vtbl[0xE5]))(actor);
		//LOG("----Actor is %s in combat", combat ? "" : "not");
		return combat;
	}

	std::atomic<UInt32> waitingForPulseDuration;

	void WaitForAnotherHapticEvent(UInt32 pulseDuration)
	{
		int count = 1;
		for (int i = 0; i < 100; i++)
		{
			Sleep(10);
			if (waitingForPulseDuration.load())
			{
				count++;
				if (count > 5)
				{
					waitingForPulseDuration.store(0);
					ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentRumble, pulseDuration == 399 ? intensityMultiplierEnvironmentRumble * 4.0f : intensityMultiplierEnvironmentRumble, true);
					Sleep(1000);
					ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentRumble, pulseDuration == 399 ? intensityMultiplierEnvironmentRumble * 4.0f : intensityMultiplierEnvironmentRumble, true);
					return;
				}
				else
				{
					waitingForPulseDuration.store(0);
				}
			}
		}
	}

	//Catches game's own haptic events
	void OnVRHapticEventListener(UInt32 axisId, UInt32 pulseDuration, PapyrusVR::VRDevice device)
	{
		if (UseControllerHapticsForEnvironmentAttack && axisId == 0 && pulseDuration == 1199 && device == leftController && !bowPulled.load())
		{
			ProvideHapticFeedback(0, 0, FeedbackType::PlayerAttackLeft, intensityMultiplierPlayerEnvironmentAttack, true);
		}
		else if (UseControllerHapticsForEnvironmentAttack && axisId == 0 && pulseDuration == 1199 && device == rightController && !bowPulled.load())
		{
			ProvideHapticFeedback(0, 0, FeedbackType::PlayerAttackRight, intensityMultiplierPlayerEnvironmentAttack, true);
		}
		else if (axisId == 0 && (pulseDuration == 239 || pulseDuration == 91 || pulseDuration == 92) && !bowPulled.load() && !vlibIsMounted())
		{
			if (device == leftController)
			{
				for (int i = 0; i < 100; i++)
				{
					if (waitingForPulseDuration.load() == pulseDuration)
					{
						LOG("Both Controller Haptic Event detected: PulseDuration: %d", pulseDuration);
						ProvideHapticFeedback(0, 0, FeedbackType::EnvironmentRumble, intensityMultiplierEnvironmentRumble, true);
						waitingForPulseDuration.store(0);
						break;
					}
				}
				waitingForPulseDuration.store(0);
			}
			else
			{
				waitingForPulseDuration.store(pulseDuration);
			}
		}	
		/*else if (axisId == 0 && (pulseDuration == 399) && !bowPulled.load()) //Causes environment rumble effect on headshot with bow
		{
			if (device == leftController)
			{
				for (int i = 0; i < 100; i++)
				{
					if (waitingForPulseDuration.load() == pulseDuration)
					{
						LOG("Both Controller Haptic Event detected: PulseDuration: %d", pulseDuration);
						waitingForPulseDuration.store(0);
						std::thread t15(WaitForAnotherHapticEvent, pulseDuration);
						t15.detach();
						break;
					}
				}
				waitingForPulseDuration.store(0);
			}
			else
			{
				waitingForPulseDuration.store(pulseDuration);
			}
		}*/
		else if (axisId == 0 && device == leftController && !bowPulled.load() && pulseDuration != 399 && !vlibIsMounted())
		{
			if (device == leftController)
			{
				for (int i = 0; i < 100; i++)
				{
					if (waitingForPulseDuration.load() == pulseDuration)
					{
						LOG("Possible unprocessed Both Controller Haptic Event detected: PulseDuration: %d", pulseDuration);
						waitingForPulseDuration.store(0);
						break;
					}
				}
			}
			else
			{
				waitingForPulseDuration.store(pulseDuration);
			}
		}
		if (*g_thePlayer)
		{
			TESForm* leftequipped = (*g_thePlayer)->GetEquippedObject(true);

			if (leftequipped)
			{
				if (leftequipped->IsWeapon())
				{
					TESObjectWEAP* leftWeapon = DYNAMIC_CAST(leftequipped, TESForm, TESObjectWEAP);
					if (leftWeapon)
					{
						if (leftWeapon->gameData.type != TESObjectWEAP::GameData::kType_Bow && leftWeapon->gameData.type != TESObjectWEAP::GameData::kType_Bow2)
						{
							return;
						}
					}
					else
						return;
				}
				else
					return;
			}
			else
				return;
		}

		if (isGameStopped() && !IsTriggerPressed(false))
		{
			bowPulled.store(false);
			bowPullCount = 0;
			bowReleaseCount = 0;
			lastleftPulse = 0;
			lastrightPulse = 0;
			LOG("Menu is open. Skipping.");
			return;
		}



		if (axisId == 0 && (pulseDuration == 599 || pulseDuration == 399 || pulseDuration == 749 || pulseDuration == 239 || pulseDuration == 91 || pulseDuration == 92 || pulseDuration == 92))
		{

		}
		else if (axisId == 0 && (pulseDuration == 519)) //Dragon shouting
		{

		}
		else if (axisId == 0 && pulseDuration == 1199)
		{

		}
		else if (axisId == 0 && pulseDuration < 3199)
		{
			//LOG("Haptic Event detected: axisId: %d PulseDuration: %d device: %s", axisId, pulseDuration, device == leftController ? "left" : (device == rightController ? "right" : "unknown"));
			if (device == rightController)
			{
				if (pulseDuration > lastrightPulse && !bowPulled.load())
				{
					lastrightPulse = pulseDuration;
					bowPullCount++;
				}
				else if (pulseDuration < lastrightPulse && bowPulled.load())
				{
					lastrightPulse = pulseDuration;
					bowReversePullCount++;
				}

				int heightrand = randomGenerator(0, 1000);
				float intensity = (float)pulseDuration / 3199.0f;
				float heightOffset = intensity - 0.5f;
				if (leftHandedMode != 0)
				{
					ProvideHapticFeedback(0, heightOffset*-1, FeedbackType::PlayerBowPullRight, intensity*intensityMultiplierPlayerBowPullRight, true);//0.2f
					//ProvideHapticFeedback(0, heightOffset, FeedbackType::PlayerBowPullLeft, intensity*intensityMultiplierPlayerBowPullLeft); // 0.4f
				}
				else
				{
					ProvideHapticFeedback(0, heightOffset*-1, FeedbackType::PlayerBowPullLeft, intensity*intensityMultiplierPlayerBowPullRight, true);//0.2f
					//ProvideHapticFeedback(0, heightOffset, FeedbackType::PlayerBowPullRight, intensity*intensityMultiplierPlayerBowPullLeft); // 0.4f
				}				
			}
			else if (device == leftController)
			{
				if (pulseDuration > lastleftPulse && !bowPulled.load())
				{
					lastleftPulse = pulseDuration;
					bowPullCount++;
				}
				else if (pulseDuration < lastleftPulse && bowPulled.load())
				{
					lastleftPulse = pulseDuration;
					bowReversePullCount++;
				}

				int heightrand = randomGenerator(0, 1000);
				float intensity = (float)pulseDuration / 3199.0f;
				float heightOffset = intensity - 0.5f;
				if (leftHandedMode != 0)
				{
					//ProvideHapticFeedback(0, heightOffset*-1, FeedbackType::PlayerBowPullRight, intensity*intensityMultiplierPlayerBowPullRight);//0.2f
					ProvideHapticFeedback(0, heightOffset, FeedbackType::PlayerBowPullLeft, intensity*intensityMultiplierPlayerBowPullLeft, true); // 0.4f
				}
				else
				{
					//ProvideHapticFeedback(0, heightOffset*-1, FeedbackType::PlayerBowPullLeft, intensity*intensityMultiplierPlayerBowPullRight);//0.2f
					ProvideHapticFeedback(0, heightOffset, FeedbackType::PlayerBowPullRight, intensity*intensityMultiplierPlayerBowPullLeft, true); // 0.4f
				}
			}
		}
		else if (axisId == 0 && pulseDuration == 3199 && device == leftController)
		{
			bowReleaseCount++;
		}

		if (bowPullCount > 10 && !bowPulled.load())
		{
			bowPullCount = 0;
			bowPulled.store(true);
			LOG("Start bow pulling feedback.");
			std::thread t1(StartBowHoldEffect);
			t1.detach();
		}
		if (bowReversePullCount >= 3)
		{
			bowPullCount = 0;
			bowReversePullCount = 0;
			bowPulled.store(false);
			lastrightPulse = 0;
			lastleftPulse = 0;
		}
		if (bowReleaseCount >= 2)
		{
			bowPulled.store(false);
			bowReleaseCount = 0;
			bowReversePullCount = 0;
			lastrightPulse = 0;
			lastleftPulse = 0;
		}
	}
	
	//Threaded function to trigger arrow holding effect on the bow.
	void StartBowHoldEffect()
	{
		while (bowPulled.load())
		{
			ProvideHapticFeedback(0, 0, PlayerBowHoldType, intensityMultiplierPlayerBowHold, true);//0.3f);
			
			Sleep(bowsleepduration);
		}
		LOG("End of bow pulling feedback.");
	}

	//Initializes openvr system. Required for haptic triggers.
	void InitSystem()
	{
		if ((ivrSystem = vr::VRSystem()))
			LOG("VR System is alive.");
		else
			LOG("No VR System found.");
	}

	void LeftHandedModeChange()
	{
		const int value = vlibGetSetting("bLeftHandedMode:VRInput");
		if (value != leftHandedMode)
		{
			leftHandedMode = value;

			if (leftHandedMode != 0)
			{
				leftControllerRole = vr::ETrackedControllerRole::TrackedControllerRole_RightHand;
				rightControllerRole = vr::ETrackedControllerRole::TrackedControllerRole_LeftHand;

				leftController = PapyrusVR::VRDevice::VRDevice_RightController;
				rightController = PapyrusVR::VRDevice::VRDevice_LeftController;
				PlayerBowHoldType = FeedbackType::PlayerBowHoldLeft;
			}
			else
			{
				leftControllerRole = vr::ETrackedControllerRole::TrackedControllerRole_LeftHand;
				rightControllerRole = vr::ETrackedControllerRole::TrackedControllerRole_RightHand;

				leftController = PapyrusVR::VRDevice::VRDevice_LeftController;
				rightController = PapyrusVR::VRDevice::VRDevice_RightController;
				PlayerBowHoldType = FeedbackType::PlayerBowHoldRight;
			}
			LOG("Left Handed Mode is %s.", leftHandedMode ? "ON" : "OFF");
		}
	}

	void PlayHapticEffect(StaticFunctionTag* base, BSFixedString effect, float locationAngle, float locationHeight, float intensity, bool waitToPlay)
	{
		_MESSAGE("PlayHapticEffect papyrus triggered");
		std::string feedbackFileName = effect.data + '\0';
		ProvideHapticFeedbackSpecificFile(locationAngle, locationHeight, feedbackFileName, intensity, waitToPlay);
	}

	void VRIKHolster(StaticFunctionTag* base, UInt32 slotNumber, bool holster)
	{
		if (holster)
		{
			if (slotNumber == 1)// "Left Hip"
				ProvideHapticFeedback(0, 0, HipHolsterStoreLeft, intensityMultiplierHolster);
			else if (slotNumber == 2)// "Right Hip"
				ProvideHapticFeedback(0, 0, HipHolsterStoreRight, intensityMultiplierHolster);
			else if (slotNumber == 7)// "Left Upper Arm"
				ProvideHapticFeedback(0, 0, SleeveHolsterStoreLeft, intensityMultiplierHolster);
			else if (slotNumber == 8)// "Right Upper Arm"
				ProvideHapticFeedback(0, 0, SleeveHolsterStoreRight, intensityMultiplierHolster);
			else if (slotNumber == 9)// "Left Forearm"
				ProvideHapticFeedback(0, 0, SleeveHolsterStoreLeft, intensityMultiplierHolster);
			else if (slotNumber == 10)// "Right Forearm"
				ProvideHapticFeedback(0, 0, SleeveHolsterStoreRight, intensityMultiplierHolster);
			else if (slotNumber == 11)// "Left Clavicle"
				ProvideHapticFeedback(0, 0, BackpackStoreLeft, intensityMultiplierHolster);
			else if (slotNumber == 12)// "Right Clavicle"
				ProvideHapticFeedback(0, 0, BackpackStoreRight, intensityMultiplierHolster);
			else if (slotNumber == 13)// "Stomach"
				ProvideHapticFeedback(0, 0, StomachStore, intensityMultiplierHolster);
			else if (slotNumber == 14)// "Chest"
				ProvideHapticFeedback(0, 0, ChestStore, intensityMultiplierHolster);
		}
		else
		{
			if (slotNumber == 1)// "Left Hip"
				ProvideHapticFeedback(0, 0, HipHolsterRemoveLeft, intensityMultiplierHolster);
			else if (slotNumber == 2)// "Right Hip"
				ProvideHapticFeedback(0, 0, HipHolsterRemoveRight, intensityMultiplierHolster);
			else if (slotNumber == 7)// "Left Upper Arm"
				ProvideHapticFeedback(0, 0, SleeveHolsterRemoveLeft, intensityMultiplierHolster);
			else if (slotNumber == 8)// "Right Upper Arm"
				ProvideHapticFeedback(0, 0, SleeveHolsterRemoveRight, intensityMultiplierHolster);
			else if (slotNumber == 9)// "Left Forearm"
				ProvideHapticFeedback(0, 0, SleeveHolsterRemoveLeft, intensityMultiplierHolster);
			else if (slotNumber == 10)// "Right Forearm"
				ProvideHapticFeedback(0, 0, SleeveHolsterRemoveRight, intensityMultiplierHolster);
			else if (slotNumber == 11)// "Left Clavicle"
				ProvideHapticFeedback(0, 0, BackpackRemoveLeft, intensityMultiplierHolster);
			else if (slotNumber == 12)// "Right Clavicle"
				ProvideHapticFeedback(0, 0, BackpackRemoveRight, intensityMultiplierHolster);
			else if (slotNumber == 13)// "Stomach"
				ProvideHapticFeedback(0, 0, StomachRemove, intensityMultiplierHolster);
			else if (slotNumber == 14)// "Chest"
				ProvideHapticFeedback(0, 0, ChestRemove, intensityMultiplierHolster);

		}		
	}

	void StopHapticEffect(StaticFunctionTag* base, BSFixedString effect)
	{
		//TurnOffKey(effect.data);
		PauseHapticFeedBack();
	}

	void StartHealingEffect(bool rightSpell)
	{
		_MESSAGE("Start Healing Effect");

		while (((rightSpell && rightMagic.load()) || (!rightSpell && leftMagic.load())) && PlayerHasMagicka())
		{
			ProvideHapticFeedback(randomGenerator(0, 359), 0, FeedbackType::MagicRestoration, intensityMultiplierMagic, false);

			Sleep(2000);
		}
	}

	void ProcessVisualEffects(ReferenceEffect * referenceEffect, BGSArtObject* artObject)
	{
		if (referenceEffect != nullptr && artObject != nullptr)
		{
			std::string model = artObject->texSwap.GetModelName();

			trim(model);

			if (model.length() > 0)
			{
				std::transform(model.begin(), model.end(), model.begin(), ::tolower);

				FeedbackType feedback = NoFeedback;
				float intensity = 1.0f;
				bool once = false;
				bool heal = false;

				if (!ContainsNoCase(model, "po3") && !ContainsNoCase(model, "vibrant") && !ContainsNoCase(model, "conduit") && !ContainsNoCase(model, "cste_") && !ContainsNoCase(model, "weapon") && !ContainsNoCase(model, "axe") && !ContainsNoCase(model, "sword") && !ContainsNoCase(model, "dagger") && !ContainsNoCase(model, "mace"))
				{
					//Play appropriate haptic effects
					//_MESSAGE("Processing Visual Effect: %s", model.c_str());
					if (Contains(model, "dlc1snowelftelekinesishandl"))
					{
						feedback = leftHandedMode ? FeedbackType::PlayerSpellWardRight : FeedbackType::PlayerSpellWardLeft;
						intensity = intensityMultiplierMagic;
					}
					else if (Contains(model, "dlc1snowelftelekinesishandr"))
					{
						feedback = leftHandedMode ? FeedbackType::PlayerSpellWardLeft : FeedbackType::PlayerSpellWardRight;
						intensity = intensityMultiplierMagic;
					}
					else if (Contains(model, "healtargetfx") || Contains(model, "healcontargetfx"))
					{
						feedback = FeedbackType::MagicRestoration;
						intensity = intensityMultiplierMagic;
						heal = true;
					}
					else if (Contains(model, "greybeardpowerabsorb"))
					{
						feedback = FeedbackType::GreybeardPowerAbsorb;
						intensity = intensityMultiplierDragonSoul;
					}
					else if (Contains(model, "powerabsorb"))
					{
						feedback = FeedbackType::DragonSoul;
						intensity = intensityMultiplierDragonSoul;
					}
					else if (Contains(model, "travel"))
					{
						feedback = FeedbackType::TravelEffect;
						intensity = intensityMultiplierTravelEffect;
						once = true;
					}
					else if (Contains(model, "summon"))
					{
						feedback = FeedbackType::Teleport;
						intensity = intensityMultiplierTeleport;
						once = true;
					}
					else if ((!Contains(model, "campfire") && !Contains(model, "cookfire") && Contains(model, "fire")) || Contains(model, "burn") || Contains(model, "flam"))
					{
						if (Contains(model, "cloak") || Contains(model, "castbodyfx"))
						{
							feedback = FeedbackType::EnvironmentalFireCloak;
							intensity = intensityMultiplierEnvironmentalFire * 0.5f;
						}
						else
						{
							feedback = FeedbackType::EnvironmentalFire;
							intensity = intensityMultiplierEnvironmentalFire;
						}
					}
					else if (Contains(model, "frost") || Contains(model, "ice") || Contains(model, "freez"))
					{
						if (Contains(model, "cloak") || Contains(model, "castbodyfx"))
						{
							feedback = FeedbackType::EnvironmentalFrostCloak;
							intensity = intensityMultiplierEnvironmentalFrost * 0.5f;
						}
						else
						{
							feedback = FeedbackType::EnvironmentalFrost;
							intensity = intensityMultiplierEnvironmentalFrost;
						}
					}
					else if (Contains(model, "shock") || Contains(model, "electri") || Contains(model, "lightning"))
					{
						if (Contains(model, "cloak") || Contains(model, "castbodyfx"))
						{
							feedback = FeedbackType::EnvironmentalElectricCloak;
							intensity = intensityMultiplierEnvironmentalShock * 0.5f;
						}
						else
						{
							feedback = FeedbackType::EnvironmentalElectric;
							intensity = intensityMultiplierEnvironmentalShock;
						}
					}
					else if (Contains(model, "poison"))
					{
						feedback = FeedbackType::EnvironmentalPoison;
						intensity = intensityMultiplierEnvironmentalPoison;
					}
					/*else if (Contains(model, "greybeardpowerabsorb"))
					{
						feedback = FeedbackType::DragonSoul;
						intensity = intensityMultiplierDragonSoul;
					}*/
				}

				if (feedback != NoFeedback)
				{
					ProvideHapticFeedback(0, 0, feedback, intensity, false, true);
					if (!once)
					{
						Sleep(heal ? 1000 : 500);

						for (int i = 0; i < 2; i++)
						{
							if (referenceEffect != nullptr && referenceEffect->finished == false)
							{
								ProvideHapticFeedback(0, 0, feedback, intensity, heal ? false : true, true);

								Sleep(heal ? 1000 : 500);
							}
							else
							{
								break;
							}
						}
					}
				}
			}
		}
	}

	std::unordered_set<UInt32> viewedEffectsList;
	
	

	void CheckEffects()
	{
		AIProcessManager* processMan = AIProcessManager::GetSingleton();
		
		UInt32 lastMagicEffectCount = 0;
		
		TESObjectREFR* playerRefr = nullptr;
		TESObjectCELL* cell = nullptr;
		TESObjectCELL* previousCell = nullptr;

		UInt32 playerHandle = 0;

		bool once = false;

		std::deque<NiPoint3> lastPositions;
		int sleepCount = 0;
		float LastGroundedZ = 0.0f;

		while (true)
		{
			if (once == false && menuTypes["Main Menu"])
			{
				PerformHooks();
				once = true;
			}
			
			if ((*g_thePlayer) == nullptr || (*g_thePlayer)->loadedState == nullptr || (*g_thePlayer)->loadedState->node==nullptr || isGameStoppedNoDialogue())
			{
				Sleep(1000);
				continue;
			}
			
			if(playerRefr==nullptr)
			{
				playerRefr = DYNAMIC_CAST((*g_thePlayer), Actor, TESObjectREFR);
			}
			if(playerHandle == 0)
			{
				playerHandle = GetOrCreateRefrHandle(playerRefr);
			}

			if (processMan == nullptr)
			{
				processMan = AIProcessManager::GetSingleton();
			}

			if (LastGroundedZ - (*g_thePlayer)->loadedState->node->m_worldTransform.pos.z > FallEffectMinHeight)
			{			
				if (!loadingMenuJustClosed.load())
				{
					std::thread t15(LateFeedback, 0, FeedbackType::FallEffect, intensityMultiplierFallEffect * ((LastGroundedZ - (*g_thePlayer)->loadedState->node->m_worldTransform.pos.z) / FallEffectMinHeight), 1, 100, true, false);
					t15.detach();
				}
			}

			float movingSpeed = 25.0f;
			if (lastPositions.size() >= 5)
			{
				NiPoint3 pos = lastPositions.at(0);
				
				bool same = true;
				for (unsigned int i = 1; i < lastPositions.size(); i++)
				{
					if (!(lastPositions.at(i).x == pos.x && lastPositions.at(i).y == pos.y))
					{
						movingSpeed = sqrtf(distance2dNoSqrt(lastPositions.at(i), pos));						
						same = false;
						break;
					}
				}
				if (same)
				{
					moving = (false);
				}
				else
				{
					moving = (true);
				}
			}
			else
			{
				moving = (true);
			}
			lastPositions.emplace_back((*g_thePlayer)->loadedState->node->m_worldTransform.pos);
			const MovementState state = vlibGetMovementState();
			if ((state != MovementState::InAir && state != MovementState::Jumping))
			{
				LastGroundedZ = (*g_thePlayer)->loadedState->node->m_worldTransform.pos.z;
				//LOG("Last Grounded Z: %g", LastGroundedZ);
			}
			if (lastPositions.size() > 5)
			{
				lastPositions.pop_front();
			}
			if((vlibIsMounted() || vlibIsOnCart()) && !playerIsInsideWaterGeneral.load())
			{
				if(moving && movingSpeed>9.0f)
				{
					//Moving mounted
					if(sleepCount <= 0)
					{
						if(!isSprinting())
						{
							ProvideHapticFeedback(0, 0, FeedbackType::HorseRidingSlow, intensityMultiplierHorseRidingSlow * (movingSpeed / 20.0f), true, false);
							sleepCount = ((horseridingsleepduration) / 100) / (movingSpeed / 35.0f);
						}
						else
						{
							ProvideHapticFeedback(0, 0, FeedbackType::HorseRiding, intensityMultiplierHorseRiding * (movingSpeed / 40.0f), true, false);
							sleepCount = ((horseridingsleepduration + randomGenerator(0, 150)) / 100) / (movingSpeed / 35.0f);
						}
					}
					else
					{
						sleepCount--;
					}
				}
			}

			if (processMan != nullptr && processVisualEffects!=0)
			{
				if (processMan->referenceEffects.count > 0)
				{
					if (processMan->referenceEffects.count != lastMagicEffectCount)
					{
						//There is a new referenceEffect
						processMan->referenceEffectsLock.Lock();

						for (UInt32 i = 0; i < processMan->referenceEffects.count; i++)
						{
							if (processMan->referenceEffects.entries[i] != nullptr)
							{
								if (viewedEffectsList.find(processMan->referenceEffects.entries[i]->effectID) == viewedEffectsList.end())
								{
									ReferenceEffect* refEffect = DYNAMIC_CAST(processMan->referenceEffects.entries[i], BSTempEffect, ReferenceEffect);
									if (refEffect != nullptr/* && !refEffect->finished*/)
									{
										ModelReferenceEffect* modelRef = DYNAMIC_CAST(processMan->referenceEffects.entries[i], BSTempEffect, ModelReferenceEffect);

										if (modelRef != nullptr && modelRef->artObject != nullptr && modelRef->artObject->data.artType == BGSArtObject::ArtType::kMagicHitEffect)
										{
											if (playerHandle == refEffect->aimAtTargetRefHandle || playerHandle == refEffect->targetRefHandle)
											{
												std::thread t3(ProcessVisualEffects, refEffect, modelRef->artObject);
												t3.detach();
												LOG("ON PLAYER: Currently active magic effectId: %x ArtObjectFormID: %x Model: %s - TargetRef:%x AimAtTargetRef:%x PlayerHandle:%x", processMan->referenceEffects.entries[i]->effectID, modelRef->artObject->formID, modelRef->artObject->texSwap.GetModelName(), refEffect->targetRefHandle, refEffect->aimAtTargetRefHandle, playerHandle);												
											}
											else
											{
												if(ContainsNoCase(modelRef->artObject->texSwap.GetModelName(), "greybeardpowerabsorb"))
												{
													std::thread t3(ProcessVisualEffects, refEffect, modelRef->artObject);
													t3.detach();
													LOG("Environment: currently active magic effectId: %x ArtObjectFormID: %x Model: %s - TargetRef:%x AimAtTargetRef:%x", processMan->referenceEffects.entries[i]->effectID, modelRef->artObject->formID, modelRef->artObject->texSwap.GetModelName(), refEffect->targetRefHandle, refEffect->aimAtTargetRefHandle);
												}
											}
											viewedEffectsList.insert(processMan->referenceEffects.entries[i]->effectID);
										}
									}
								}
							}
						}
						lastMagicEffectCount = processMan->referenceEffects.count;
						processMan->referenceEffectsLock.Release();
					}
				}
			}
			Sleep(100);
		}
	}

	bool RegisterFuncs(VMClassRegistry* registry)
	{

		InitSystem();

		LeftHandedModeChange();

		FillFeedbackList();

		CreateSystem();

		SafeWriteJump(addressStart.GetUIntPtr(), addressEnd.GetUIntPtr());

		g_originalNotifyAnimationGraph = *IAnimationGraphManagerHolder_NotifyAnimationGraph_vtbl;
		SafeWrite64(IAnimationGraphManagerHolder_NotifyAnimationGraph_vtbl.GetUIntPtr(), uintptr_t(Hooked_IAnimationGraphManagerHolder_NotifyAnimationGraph));

		g_originalTESObjectREFR_NotifyAnimationGraph = *TESObjectREFR_IAnimationGraphManagerHolder_NotifyAnimationGraph_vtbl;
		SafeWrite64(TESObjectREFR_IAnimationGraphManagerHolder_NotifyAnimationGraph_vtbl.GetUIntPtr(), uintptr_t(Hooked_TESObjectREFR_IAnimationGraphManagerHolder_NotifyAnimationGraph));

		g_originalActor_NotifyAnimationGraph = *Actor_IAnimationGraphManagerHolder_NotifyAnimationGraph_vtbl;
		SafeWrite64(Actor_IAnimationGraphManagerHolder_NotifyAnimationGraph_vtbl.GetUIntPtr(), uintptr_t(Hooked_Actor_IAnimationGraphManagerHolder_NotifyAnimationGraph));

		const double swimBreathBase = vlibGetGameSetting("fActorSwimBreathBase");
		const double swimBreathMult = 0.2;//vlibGetGameSetting("fActorSwimBreathMult");
		if (swimBreathBase != -1 && swimBreathMult != 1)
		{
			holdBreathLimit = swimBreathBase + (swimBreathMult * 60.0);
			//LOG("Base:%g Mult:%g BreathLimit:%g", swimBreathBase, swimBreathMult, holdBreathLimit);
		}

		std::thread t3(CheckEffects);
		t3.detach();

		std::thread t4(Heartbeat);
		t4.detach();

		std::thread t5(WordWallCheck);
		t5.detach();

		std::thread t7(WeatherCheck);
		t7.detach();

		std::thread t8(SwimmingCheck);
		t8.detach();

		registry->RegisterFunction(
			new NativeFunction5<StaticFunctionTag, void, BSFixedString, float, float, float, bool>("PlayHapticEffect", "TactsuitVR_PluginScript", PlayHapticEffect, registry));

		registry->RegisterFunction(
			new NativeFunction1<StaticFunctionTag, void, BSFixedString>("StopHapticEffect", "TactsuitVR_PluginScript", StopHapticEffect, registry));

		registry->RegisterFunction(
			new NativeFunction2<StaticFunctionTag, void, UInt32, bool>("BHapticsHolster", "VRIKTactsuitVR_PluginScript", VRIKHolster, registry));
				
		LOG("TactsuitVR registerFunction");
		return true;
	}

	//Spell frequency patterns
	std::vector<unsigned short> fireArray1 = { 3300,3288,3301,3498,3355,3549,3316,3388,3416,3386,3306,3470,3575,3472,3466,3401,3545,3488,3485,3520,3453,3544,3396,3486,3490,3350,3535,3523,3680,3544,3563,3470,3393,3422,3373,3392,3495,3464 };

	std::vector<unsigned short> fireReadyArray1 = { 2420,2411,2344,2511,2249,2338,2237,2609,2386,2855,2483,2138,2224,2124,2074,2773,2432,2095,2789,2226,2008,2320,2552,2131,2065,2290,2169,2632,2645,1734,2213,2344,2380,2313,2225,2218,2023,2437,2228 };

	std::vector<unsigned short> iceArray1 = { 3776,1790,1,2126,2549,1517,2180,3220,1512,2115,2807,3143,2444,1129,2204,2010,2583,3030,2576,2038,2145,2040,2871,2673,1701,933,3819,2357,2423,1627,962,1983,3026,3183,3446,2736,2738,2711,3333,2707,2385,1556,3320,2156,3102,3999,2453 };

	std::vector<unsigned short> iceReadyArray1 = { 1846,1224,352,666,1094,285,980,3999,2044,2571,2149,1889,2520,876,882,912,551,3721,2569,1634,2248,2811,1658,1430,1623,1563,3103,1285,1320,2251,985,639,1,277,461,395,1978,2281 };

	std::vector<unsigned short> shockArray1 = { 1686,2466,1930,1104,3007,993,2348,2019,1007,2802,1240,1388,1951,1,2869,2773,2448,2419,1755,2664,1902,1569,2255,339,1741,637,1834,186,1265,3463,2886,3117,3999,1708,1608,2166,466,1650,1803,3027,2245,3142,3570,1194,635,1623,2307,1165,2014,2457,1769,1267,2584,1708,2028,935,1019,1515,1016,2177,1105,1816 };

	std::vector<unsigned short> shockReadyArray1 = { 3335,2960,2035,3256,2215,1123,1163,1586,2483,1789,1281,659,2612,2536,733,608,1328,3999,2465,1399,1,191,1917,3618,2896,1255,2109,1882,2220,1432,1190,2037,1971,2147,2943,2285,1497,1292,3298,2093,1452,1320,1406,635,1753,2541,1650,1246,1768,2127,2609,1106,1039,2925,3108,1215 };

	std::vector<unsigned short> alterationArray1 = { 626,1,2596,2452,2219,1159,1357,1693,1794,681,3600,2014,2654,3275,1311,2434,2939,2786,2476,3373,3999,3469,2106,1993,2972,3432,3079,3871,3143,3865,3731,2158,1577,1697,1689,1660,2262,3754,1542,2434,517,1160,1571,2765,3933,2969,2609 };

	std::vector<unsigned short> alterationReadyArray1 = { 1920,1647,1222,1009,1271,2019,2903,2826,2221,3743,3653,3953,3160,1897,1080,2802,3616,2417,1283,1479,204,454,1490,1145,1074,1,1528,1484,2629,3589,3709,3999,3042,3472,2048,3026,2430,1832,1619,1285,549,941,1119,1828,2131,1460,1371 };

	std::vector<unsigned short> conjurationReadyArray1 = { 2340,2190,2205,2398,2268,2479,2404,2172,2681,3363,2178,1548,1448,993,1053,1028,413,1020,868,2305,2835,1774,1,496,1454,1822,1102,401,2612,2038,2145,1791,1432,2271,2566,2237,3228,3913,2468,2741,2494,3200,3999,2769,3685,1951,2446,2088,1887,2592,2024,893,1205,1924 };

	std::vector<unsigned short> illusionArray1 = { 614,1723,2521,2472,1030,2784,3364,3214,3513,3275,1903,2570,1355,1460,1144,509,324,1401,1067,2649,1854,2503,3655,3057,2174,1128,1186,1527,2115,1247,711,1,762,1449,1762,2015,2087,1740,2562,2314,1428,274,1497,38,1057,1280,1220,1350,2624,2720,1523,1778,1736,2435,2354,2407,2289,1423,946,581,412,1748,2537,2874,2913,3974,3302,2867,3099,2300,1658,1506,1660,2215,2793,3095,2754,2401,2063,2252,3127,2548,3154,2082,3398,2712,3999,2390,2072,1013,2890,1000,1213,1446,1210,2226,2398,1635,1757,2442,2099,2315,2705,1888,1619,1830,1157,419,649,2004 };

	std::vector<unsigned short> illusionReadyArray1 = { 1862,2443,2174,2731,2429,3123,3209,2886,3143,2424,2480,2513,2466,881,2836,3596,3743,3999,3404,2426,2798,2819,2906,2463,2950,2311,1815,1401,1458,876,23,202,1,829,1234,157,792,629,764,2186,1540,670,417,1384,2402,2139,2743,2112 };

	std::vector<unsigned short> lightReadyArray1 = { 115,606,630,362,419,798,459,735,375,517,600,272,103,386,764,20,64,265,700,349,1,365,739,409,725,367,478 };

	std::vector<unsigned short> restorationArray1 = { 1493,1386,705,1047,1489,2025,2369,2910,3154,2062,1473,1637,2107,1180,1123,1625,2121,2824,3633,3113,3117,2916,2563,2070,1893,2064,1382,1386,2533,3286,3345,3503,3373,2527,1578,562,322,504,1,1111,2147,2436,2225,2727,2813,3310,2720,1544,899,894,533,1020,1301,2360,3412,3466,3305,2942,1798,1459,1102,965,1282,879,1888,2657,2589,3644,2774,2657,2080,1685,1398,2103,1266,1647,2384,2908,3656,3692,3169,2977,2266,1903,1478,1299,2323,2657,3146,3312,3999,3322,3497,3344 };

	std::vector<unsigned short> restorationReadyArray1 = { 1033,925,935,1084,862,736,686,725,757,681,818,943,1039,895,1038,947,909,776,903,836,816,917,807,689,680,684,668,628,762,580,812,706,760,837,781,770 };

	std::vector<unsigned short> wardArray1 = { 2941,3326,3999,3304,2417,1876,1688,2357,2716,2242,1955,848,2107,1715,58,267,1703,2224,1221,1380,2417,3461,3825,2938,1555,1634,2803,2411,1012,653,1000,1272,3364,3072,1378,496,881,2179,3455,2207,417,1,2034,3741 };

	std::vector<unsigned short> emptyArray = {};

	//HIGGS Integration
	void higgspull(bool isLeft, TESObjectREFR* pulledRefr)
	{
		LOG("HIGGS Pull effect on %s hand", isLeft ? "left" : "right");
		
		ProvideHapticFeedback(0, 0, isLeft ? HiggsPullLeft : HiggsPullRight, intensityMultiplierHiggsPull);
	}
		
	void higgsstash(bool isLeft, TESForm* stashedForm)
	{
		LOG("HIGGS shoulder stash effect on %s shoulder", isLeft ? "left" : "right");
		
		ProvideHapticFeedback(0, 0, isLeft ? BackpackStoreLeft : BackpackStoreRight, intensityMultiplierHolster);
	}
	
	void higgscollide(bool isLeft, float mass, float separatingVelocity)
	{
		LOG("HIGGS hand collide effect on %s hand %g mass with %g velocity", isLeft ? "left" : "right", mass, separatingVelocity);

		ProvideHapticFeedback(0, 0, isLeft ? PlayerEnvironmentHitLeft : PlayerEnvironmentHitRight, intensityMultiplierPlayerEnvironmentHit * (mass * (separatingVelocity / 5000.0f) / 4));
	}

	void higgsconsume(bool isLeft, TESForm* consumedForm)
	{
		if (consumedForm != nullptr)
		{
			if (consumedForm->formType == kFormType_Potion)
			{
				LOG("Player consumed something with higgs. itemFormId:%x", consumedForm->formID);
				
				AlchemyItem* potion = DYNAMIC_CAST(consumedForm, TESForm, AlchemyItem);

				if (potion != nullptr)
				{
					if (!(potion->keyword.HasKeyword(KeywordVendorItemPoison) || potion->IsPoison()))
					{
						if (!potion->IsFood()) //Potion
						{
							ProvideHapticFeedback(0, 0, FeedbackType::ConsumableDrink, intensityMultiplierDrink, true, false);
						}
						else
						{
							ProvideHapticFeedback(0, 0, FeedbackType::ConsumableFood, intensityMultiplierEat, true, false);
						}
					}
				}
			}
		}
	}

	void RangedProjectileFeedback(NiPoint3 actorPos, NiPoint3 impactPosition, ProjectileHitLocation hitLocation)
	{
		auto& nodeList = (*g_thePlayer)->nodeList;

		const NiPoint3 playerPos = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;

		NiAVObject* hmd_node = nodeList[hmdNode];

		if (hmd_node)
		{
			float shooterImpactDistance =distance(actorPos, impactPosition);
			
			NiPoint3 pos = useShooterPosition ? actorPos : InterpolateBetweenVectors(impactPosition, actorPos, 30.f / shooterImpactDistance);
			
			float heading = 0.0;
			float attitude = 0.0;
			float bank = 0.0;

			NiMatrix33 playerrotation = (*g_thePlayer)->loadedState->node->m_worldTransform.rot;
			playerrotation.GetEulerAngles(&heading, &attitude, &bank);
			NiPoint3 playerposition = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;

			float playerHeading = heading * (180 / MATH_PI);

			float playerAttitude = attitude * (180 / MATH_PI);

			float calcPlayerHeading = 0.0;

			if (playerHeading == -180)
			{
				calcPlayerHeading = 180 - playerAttitude;
			}
			else
			{
				if (playerAttitude < 0)
				{
					calcPlayerHeading = 360 + playerAttitude;
				}
				else
				{
					calcPlayerHeading = playerAttitude;
				}
			}
			float angle = (std::atan2(pos.y - playerposition.y, pos.x - playerposition.x) - std::atan2(1, 0)) * (180 / MATH_PI);
			if (angle < 0) {
				angle = angle + 360;
			}

			float headingAngle = angle - calcPlayerHeading;
			if (headingAngle < 0)
				headingAngle += 360;
			else if (headingAngle > 360)
				headingAngle -= 360;

			//Player got hit with a projectile
			LOG("Player got hit with a projectile. Heading:%g Projectile X:%g Y:%g Z:%g PlayerHMD Z: %g PlayerBase Z: %g", headingAngle, pos.x, pos.y, impactPosition.z, hmd_node->m_worldTransform.pos.z, playerPos.z);

			//70-120(hmd_node->m_worldTransform.pos.z)

			NiAVObject* leftHand_node = nodeList[leftHandNode];
			NiAVObject* rightHand_node = nodeList[rightHandNode];
			NiNode* m_nodePlayer3rdP = (*g_thePlayer)->GetNiRootNode(0);
			NiAVObject* leftForeArm_node = nullptr;
			NiAVObject * rightForeArm_node = nullptr;
			if (m_nodePlayer3rdP != nullptr)
			{
				leftForeArm_node = m_nodePlayer3rdP->GetObjectByName(&leftForeArmName.data);
				rightForeArm_node = m_nodePlayer3rdP->GetObjectByName(&rightForeArmName.data);
			}

			//TODO
			//bool leftSleeveActive = IsDevicePlaying(bhaptics::PositionType::ForearmL) || IsDevicePlaying(bhaptics::PositionType::Left);
			//bool rightSleeveActive = IsDevicePlaying(bhaptics::PositionType::ForearmR) || IsDevicePlaying(bhaptics::PositionType::Right);
			bool leftSleeveActive = false;
			bool rightSleeveActive = false;

			if (hitLocation == Head)
			{
				//if (IsDevicePlaying(bhaptics::PositionType::Head))
				//{
				//	ProvideHapticFeedback(headingAngle, 0, FeedbackType::RangedHead, intensityMultiplierRangedHead);
				//}
				//else
				//{
					ProvideHapticFeedback(headingAngle, 0.5f, FeedbackType::Ranged, intensityMultiplierRanged);
				/*}*/
			}
			else if(hitLocation == LeftArmItem)
			{
				ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::PlayerBlockRight : FeedbackType::PlayerBlockLeft, intensityMultiplierPlayerBlock);
			}
			else if (hitLocation == RightArmItem)
			{
				ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::PlayerBlockLeft : FeedbackType::PlayerBlockRight, intensityMultiplierPlayerBlock);
			}
			else if(leftSleeveActive && hitLocation == LeftArm)
			{
				ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::RangedRightArm : FeedbackType::RangedLeftArm, intensityMultiplierRangedArm);
			}
			else if (rightSleeveActive && hitLocation == RightArm)
			{
				ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::RangedLeftArm : FeedbackType::RangedRightArm, intensityMultiplierRangedArm);
			}
			else
			{
				if (hitLocation != Body && leftSleeveActive && leftHand_node != nullptr && distance(impactPosition, leftHand_node->m_worldTransform.pos) <= 20.0f)
				{
					ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::RangedRightArm : FeedbackType::RangedLeftArm, intensityMultiplierRangedArm);
				}
				else if (hitLocation != Body && rightSleeveActive && rightHand_node != nullptr && distance(impactPosition, rightHand_node->m_worldTransform.pos) <= 20.0f)
				{
					ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::RangedLeftArm : FeedbackType::RangedRightArm, intensityMultiplierRangedArm);
				}
				else if (hitLocation != Body && leftSleeveActive && leftForeArm_node != nullptr && distance(impactPosition, leftForeArm_node->m_worldTransform.pos) <= 20.0f)
				{
					ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::RangedRightArm : FeedbackType::RangedLeftArm, intensityMultiplierRangedArm);
				}
				else if (hitLocation != Body && rightSleeveActive && rightForeArm_node != nullptr && distance(impactPosition, rightForeArm_node->m_worldTransform.pos) <= 20.0f)
				{
					ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::RangedLeftArm : FeedbackType::RangedRightArm, intensityMultiplierRangedArm);
				}
				else
				{
					const float lowerZ = (hmd_node->m_worldTransform.pos.z - playerPos.z) * 0.5834f + playerPos.z;
					const float higherZ = (hmd_node->m_worldTransform.pos.z - playerPos.z) * 0.9166f + playerPos.z;
					if (impactPosition.z < lowerZ)
					{
						ProvideHapticFeedback(headingAngle, -0.5f, FeedbackType::Ranged, intensityMultiplierRanged);
					}
					else if (lowerZ <= impactPosition.z && impactPosition.z < higherZ)
					{
						ProvideHapticFeedback(headingAngle, ((impactPosition.z - lowerZ) / (higherZ - lowerZ)) - 0.5f, FeedbackType::Ranged, intensityMultiplierRanged);
					}
					else //headshot
					{
					/*	if (IsDevicePlaying(bhaptics::PositionType::Head))
						{
							ProvideHapticFeedback(headingAngle, 0, FeedbackType::RangedHead, intensityMultiplierRangedHead);
						}
						else
						{*/
							ProvideHapticFeedback(headingAngle, 0.5f, FeedbackType::Ranged, intensityMultiplierRanged);
						/*}*/
					}
				}
			}
		}
	}

	void SpellProjectileFeedback(SpellItem* spell, NiPoint3 actorPos, NiPoint3 impactPosition, ProjectileHitLocation hitLocation)
	{
		auto& nodeList = (*g_thePlayer)->nodeList;

		const NiPoint3 playerPos = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;

		NiAVObject* hmd_node = nodeList[hmdNode];

		if (hmd_node)
		{
			//float shooterImpactDistance = distance(actorPos, impactPosition);

			NiPoint3 pos = useShooterPosition ? actorPos : impactPosition;// InterpolateBetweenVectors(impactPosition, actorPos, 30.f / shooterImpactDistance);

			float heading = 0.0;
			float attitude = 0.0;
			float bank = 0.0;

			NiMatrix33 playerrotation = (*g_thePlayer)->loadedState->node->m_worldTransform.rot;
			playerrotation.GetEulerAngles(&heading, &attitude, &bank);
			NiPoint3 playerposition = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;

			float playerHeading = heading * (180 / MATH_PI);

			float playerAttitude = attitude * (180 / MATH_PI);

			float calcPlayerHeading = 0.0;

			if (playerHeading == -180)
			{
				calcPlayerHeading = 180 - playerAttitude;
			}
			else
			{
				if (playerAttitude < 0)
				{
					calcPlayerHeading = 360 + playerAttitude;
				}
				else
				{
					calcPlayerHeading = playerAttitude;
				}
			}
			float angle = (std::atan2(pos.y - playerposition.y, pos.x - playerposition.x) - std::atan2(1, 0)) * (180 / MATH_PI);
			if (angle < 0) {
				angle = angle + 360;
			}

			float headingAngle = angle - calcPlayerHeading;
			if (headingAngle < 0)
				headingAngle += 360;
			else if (headingAngle > 360)
				headingAngle -= 360;

			//Player got hit with a projectile
			LOG("Player got hit with a spell projectile. FormId: %x. Heading:%g Projectile X:%g Y:%g Z:%g PlayerHMD Z: %g PlayerBase Z: %g", spell->formID, headingAngle, pos.x, pos.y, impactPosition.z, hmd_node->m_worldTransform.pos.z, playerPos.z);

			//70-120(hmd_node->m_worldTransform.pos.z)

			FeedbackType magicFeedback = FeedbackType::MagicOther;
			magicFeedback = DecideMagicFeedbackType(spell);

			NiAVObject* leftHand_node = nodeList[leftHandNode];
			NiAVObject* rightHand_node = nodeList[rightHandNode];
			NiNode* m_nodePlayer3rdP = (*g_thePlayer)->GetNiRootNode(0);
			NiAVObject* leftForeArm_node = nullptr;
			NiAVObject* rightForeArm_node = nullptr;
			if (m_nodePlayer3rdP != nullptr)
			{
				leftForeArm_node = m_nodePlayer3rdP->GetObjectByName(&leftForeArmName.data);
				rightForeArm_node = m_nodePlayer3rdP->GetObjectByName(&rightForeArmName.data);
			}

			//bool leftSleeveActive = IsDevicePlaying(bhaptics::PositionType::ForearmL) || IsDevicePlaying(bhaptics::PositionType::Left);
			//bool rightSleeveActive = IsDevicePlaying(bhaptics::PositionType::ForearmR) || IsDevicePlaying(bhaptics::PositionType::Right);
			bool leftSleeveActive = false;
			bool rightSleeveActive = false;

			if (hitLocation == Head)
			{
/*				if (IsDevicePlaying(bhaptics::PositionType::Head))
				{
					FeedbackType headVersion = GetHeadVersionOfMagic(magicFeedback);
					ProvideHapticFeedback(headingAngle, 0, headVersion, intensityMultiplierMagicHead, false);
				}	*/			

				ProvideHapticFeedback(headingAngle, 0.5f, magicFeedback, intensityMultiplierMagic, false);
			}
			else if (hitLocation == LeftArmItem)
			{
				ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::PlayerBlockRight : FeedbackType::PlayerBlockLeft, intensityMultiplierPlayerBlock);
			}
			else if (hitLocation == RightArmItem)
			{
				ProvideHapticFeedback(0, 0, leftHandedMode ? FeedbackType::PlayerBlockLeft : FeedbackType::PlayerBlockRight, intensityMultiplierPlayerBlock);
			}
			else if (leftSleeveActive && hitLocation == LeftArm)
			{
				ProvideHapticFeedback(0, 0, leftHandedMode ? GetRightArmVersionOfMagic(magicFeedback) : GetLeftArmVersionOfMagic(magicFeedback), intensityMultiplierMagicArm, false);
			}
			else if (rightSleeveActive && hitLocation == RightArm)
			{
				ProvideHapticFeedback(0, 0, leftHandedMode ? GetLeftArmVersionOfMagic(magicFeedback) : GetRightArmVersionOfMagic(magicFeedback), intensityMultiplierMagicArm, false);
			}
			else
			{
				if (hitLocation != Body && leftSleeveActive && leftHand_node != nullptr && distance(impactPosition, leftHand_node->m_worldTransform.pos) <= 20.0f)
				{
					ProvideHapticFeedback(0, 0, leftHandedMode ? GetRightArmVersionOfMagic(magicFeedback) : GetLeftArmVersionOfMagic(magicFeedback), intensityMultiplierMagicArm, false);
				}
				else if (hitLocation != Body && rightSleeveActive && rightHand_node != nullptr && distance(impactPosition, rightHand_node->m_worldTransform.pos) <= 20.0f)
				{
					ProvideHapticFeedback(0, 0, leftHandedMode ? GetLeftArmVersionOfMagic(magicFeedback) : GetRightArmVersionOfMagic(magicFeedback), intensityMultiplierMagicArm, false);
				}
				else if (hitLocation != Body && leftSleeveActive && leftForeArm_node != nullptr && distance(impactPosition, leftForeArm_node->m_worldTransform.pos) <= 20.0f)
				{
					ProvideHapticFeedback(0, 0, leftHandedMode ? GetRightArmVersionOfMagic(magicFeedback) : GetLeftArmVersionOfMagic(magicFeedback), intensityMultiplierMagicArm, false);
				}
				else if (hitLocation != Body && rightSleeveActive && rightForeArm_node != nullptr && distance(impactPosition, rightForeArm_node->m_worldTransform.pos) <= 20.0f)
				{
					ProvideHapticFeedback(0, 0, leftHandedMode ? GetLeftArmVersionOfMagic(magicFeedback) : GetRightArmVersionOfMagic(magicFeedback), intensityMultiplierMagicArm, false);
				}
				else
				{
					const float lowerZ = (hmd_node->m_worldTransform.pos.z - playerPos.z) * 0.5834f + playerPos.z;
					const float higherZ = (hmd_node->m_worldTransform.pos.z - playerPos.z) * 0.9166f + playerPos.z;
					if (impactPosition.z < lowerZ)
					{
						ProvideHapticFeedback(headingAngle, -0.5f, magicFeedback, intensityMultiplierMagic, false);
					}
					else if (lowerZ <= impactPosition.z && impactPosition.z < higherZ)
					{
						ProvideHapticFeedback(headingAngle, ((impactPosition.z - lowerZ) / (higherZ - lowerZ)) - 0.5f, magicFeedback, intensityMultiplierMagic, false);
					}
					else //headshot
					{
						//if (IsDevicePlaying(bhaptics::PositionType::Head))
						//{
						//	FeedbackType headVersion = GetHeadVersionOfMagic(magicFeedback);
						//	ProvideHapticFeedback(headingAngle, 0, headVersion, intensityMultiplierMagicHead, false);
						//}

						ProvideHapticFeedback(headingAngle, 0.5f, magicFeedback, intensityMultiplierMagic, false);
					}
				}
			}
		}
	}

	void UpdateProjectile(Projectile* proj, float delta)
	{
		if (proj)
		{
			if (proj && proj->GetNiNode()) 
			{
				if(proj->weaponSource != nullptr && proj->weaponSource->gameData.type == TESObjectWEAP::GameData::kType_Bow || proj->weaponSource->gameData.type == TESObjectWEAP::GameData::kType_Bow2 || proj->weaponSource->gameData.type == TESObjectWEAP::GameData::kType_CrossBow || proj->weaponSource->gameData.type == TESObjectWEAP::GameData::kType_CBow)
				{
					TESObjectREFR* playerRefr = DYNAMIC_CAST((*g_thePlayer), Actor, TESObjectREFR);
					const UInt32 playerHandle = GetOrCreateRefrHandle(playerRefr);
					if (proj->shooter > 0 && proj->shooter != playerHandle)
					{
						NiPointer<TESObjectREFR> shooterRefr = nullptr;						
						LookupREFRByHandle(proj->shooter, shooterRefr);
						if (shooterRefr != nullptr)
						{
							Actor* shooterActor = DYNAMIC_CAST(shooterRefr, TESObjectREFR, Actor);

							if (shooterActor != nullptr && shooterActor->loadedState != nullptr && shooterActor->loadedState->node != nullptr && CALL_MEMBER_FN((*g_thePlayer), IsHostileToActor)(shooterActor))
							{
								shooterProjectileMutex.lock();
								shooterLastProjectilePositionList[proj->shooter] = proj->pos;
								shooterProjectileMutex.unlock();
							}
						}
					}
				}
			}
		}
	}

	void GetVelocity_HookProjectile(Projectile* ref, float delta)
	{
		UpdateProjectile(ref, delta);
		g_originalGetVelocityFunctionProjectile(ref, delta);
	}

	// Fetches information about a node
	std::string getNodeDesc(NiAVObject* node) {
		// Use periods to indicate the depth in the scene graph
		if (!node) {
			return "Missing node";
		}

		// Include the node's type followed by its name
		std::string text;
		if (node->GetRTTI() && node->GetRTTI()->name)
			text = text + node->GetRTTI()->name + " ";
		else
			text = text + "UnknownType ";
		if (node->m_name)
			text = text + node->m_name;
		else
			text = text + "Unnamed";

		// Include any extra data
		if (node->m_extraDataLen > 0)
			text = text + " {";
		for (int i = 0; i < node->m_extraDataLen; ++i) {
			// Fetch extra data, writing "NULL" for missing elements
			NiExtraData* extraData = node->m_extraData[i];
			if (i > 0)
				text = text + ", ";
			if (!extraData) {
				text = text + "Missing Data";
				continue;
			}
			else if (!extraData->m_pcName) {
				text = text + "(null)=";
			}
			else {
				text = text + extraData->m_pcName + "=";
			}

			// Cast the extra data to all supported types
			NiStringExtraData* stringData = DYNAMIC_CAST(extraData, NiExtraData, NiStringExtraData);
			NiStringsExtraData* stringsData = DYNAMIC_CAST(extraData, NiExtraData, NiStringsExtraData);
			NiBooleanExtraData* boolData = DYNAMIC_CAST(extraData, NiExtraData, NiBooleanExtraData);
			NiIntegerExtraData* intData = DYNAMIC_CAST(extraData, NiExtraData, NiIntegerExtraData);
			NiIntegersExtraData* intsData = DYNAMIC_CAST(extraData, NiExtraData, NiIntegersExtraData);
			NiFloatExtraData* floatData = DYNAMIC_CAST(extraData, NiExtraData, NiFloatExtraData);
			NiFloatsExtraData* floatsData = DYNAMIC_CAST(extraData, NiExtraData, NiFloatsExtraData);
			NiVectorExtraData* vectorData = DYNAMIC_CAST(extraData, NiExtraData, NiVectorExtraData);

			// Include NiStringExtraData
			if (stringData) {
				text = text + "\"" + stringData->m_pString + "\"";

				// Include NiStringsExtraData as an array
			}
			else if (stringsData) {
				text = text + "[";
				if (!stringsData->m_data) {
					text = text + "(null string data)";
				}
				else {
					for (int j = 0; j < stringsData->m_size; ++j) {
						if (j > 0)
							text = text + ", ";
						text = text + "\"" + stringsData->m_data[j] + "\"";
					}
				}
				text = text + "]";

				// Include NiBooleanExtraData
			}
			else if (boolData) {
				text = text + (boolData->m_data ? "True" : "False");

				// Include NiIntegerExtraData
			}
			else if (intData) {
				text = text + std::to_string(intData->m_data);

				// Include NiIntegersExtraData as an array
			}
			else if (intsData) {
				text = text + "[";
				if (!intsData->m_data) {
					text = text + "(null int data)";
				}
				else {
					for (int j = 0; j < intsData->m_size; ++j) {
						if (j > 0)
							text = text + ", ";
						text = text + std::to_string(intsData->m_data[j]);
					}
				}
				text = text + "]";

				// Include NiFloatExtraData
			}
			else if (floatData) {
				text = text + std::to_string(floatData->m_data);

				// Include NiFloatsExtraData as an array
			}
			else if (floatsData) {
				text = text + "[";
				if (!floatsData->m_data) {
					text = text + "(null float data)";
				}
				else {
					for (int j = 0; j < floatsData->m_size; ++j) {
						if (j > 0)
							text = text + ", ";
						text = text + std::to_string(floatsData->m_data[j]);
					}
				}
				text = text + "]";

				// Include NiVectorExtraData
			}
			else if (vectorData) {
				text = text + "<" + std::to_string(vectorData->m_vector[0]) +
					std::to_string(vectorData->m_vector[1]) +
					std::to_string(vectorData->m_vector[2]) +
					std::to_string(vectorData->m_vector[3]) + +">";

			}
			else {
				text = text + extraData->GetRTTI()->name;
			}

		}
		if (node->m_extraDataLen > 0)
			text = text + "}";
		return text;
	}

	// Writes information about a node to the log file
	void logNode(int depth, NiAVObject* node) {
		auto text = std::string(depth, '.') + getNodeDesc(node);
		_MESSAGE("%s\n", text.c_str());
	}

	// Lists all children of a bone to the log file, filtering by RTTI type name	
	void logChildren(NiAVObject* bone, int depth, int maxDepth, const char* filter) {
		if (!bone) return;
		if (filter != 0 && filter[0]) {
			if (strcmp(bone->GetRTTI()->name, filter) != 0) {
				return;
			}
		}
		//auto blanks = std::string(depth, '.');
		//log("%sNode name = %s, RTTI = %s\n", blanks.c_str(), bone->m_name, bone->GetRTTI()->name);
		logNode(depth, bone);
		NiNode* node = bone ? bone->GetAsNiNode() : 0;
		if (!node) return;
		auto children = NINODE_CHILDREN(node);
		//log("Children has %d %d %d %d\n", children->m_arrayBufLen, children->m_emptyRunStart, children->m_size, children->m_growSize);
		for (int i = 0; i < children->m_emptyRunStart; i++) {
			if (depth < maxDepth || maxDepth < 0)
				logChildren(children->m_data[i], depth + 1, maxDepth, filter);
		}
	}
		
	//Only do this if locational damage is installed.
	void PerformHooks()
	{
		if (!TactsuitVR::locationalDamageInstalled)
		{
			return;
		}
		if(g_trampolineInterface==nullptr)
		{
			LOG("g_trampolineInterface is null");
			return;
		}
		
		struct Code : Xbyak::CodeGenerator
		{
			Code(void* buf) : Xbyak::CodeGenerator(4096, buf)
			{			
				push(rax);
				push(rcx);
				push(rdx);
				push(r8);
				push(r9);
				push(r10);
				push(r11);
				sub(rsp, 0x68); // Need to keep the stack SIXTEEN BYTE ALIGNED
				movsd(ptr[rsp], xmm0);
				movsd(ptr[rsp + 0x10], xmm1);
				movsd(ptr[rsp + 0x20], xmm2);
				movsd(ptr[rsp + 0x30], xmm3);
				movsd(ptr[rsp + 0x40], xmm4);
				movsd(ptr[rsp + 0x50], xmm5);

				//  .text:000000014074CFBA                 lea     r9, [r13 + 0Ch]; a4
				lea(r9, ptr[r13 + 0x0C]);
				// 	.text:000000014074CFBE                 mov[rsp + 180h + a6], 0; a6  (WARNING NOTE: Stack Offsets changed to 0x28, 0x20 fix Impact VFX bug in VR but this may be totally wrong in SE!!)
				mov(byte[rsp + 0x28], 0);
				// 	.text:000000014074CFC3                 mov[rsp + 180h + a5], ecx; a5
				mov(dword[rsp + 0x20], ecx);
				// 	.text:000000014074CFC7                 mov     r8, r13; a3
				mov(r8, r13);
				// 	.text:000000014074CFCA                 mov     rdx, rbx; a2
				mov(rdx, rbx);
				// 	.text:000000014074CFCD                 mov     rcx, r14; a1
				mov(rcx, r14);

				// Call our hook
				mov(rax, (uintptr_t)OnProjectileHitFunctionHooked);
				call(rax);

				movsd(xmm0, ptr[rsp]);
				movsd(xmm1, ptr[rsp + 0x10]);
				movsd(xmm2, ptr[rsp + 0x20]);
				movsd(xmm3, ptr[rsp + 0x30]);
				movsd(xmm4, ptr[rsp + 0x40]);
				movsd(xmm5, ptr[rsp + 0x50]);
				add(rsp, 0x68);
				pop(r11);
				pop(r10);
				pop(r9);
				pop(r8);
				pop(rdx);
				pop(rcx);
				pop(rax);

				mov(rdx, directAddressX);
				jmp(rdx);
			}
		};

		void* codeBuf = g_localTrampoline.StartAlloc();
		Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		g_branchTrampoline.Write6Branch(OnProjectileHitHookLocation.GetUIntPtr(), uintptr_t(code.getCode()));

		_MESSAGE("ProjectileHit hook complete");
	
	}

	int64_t OnProjectileHitFunctionHooked(Projectile* akProjectile, TESObjectREFR* akTarget, NiPoint3* point, uintptr_t unk1, UInt32 unk2, UInt8 unk3)
	{
		if(akProjectile != nullptr && akTarget == (*g_thePlayer) && (akProjectile->spell != nullptr && (akProjectile->castingSource != Projectile::CastingSource::kInstant) && ((SpellItem*)(akProjectile->spell)) != nullptr && ((SpellItem*)(akProjectile->spell))->data.spellType != SpellItem::SpellType::kVoicePower))
		{
			LOG("Spell hit on player. FormId: %x ShooterHandle:%x", akProjectile->spell->formID, akProjectile->shooter);
		}
		if (akProjectile != nullptr && akTarget == (*g_thePlayer) && akProjectile->shooter > 0)
		{
			NiPoint3 hit_pos = *point;

			NiPointer<TESObjectREFR> shooterRefr = nullptr;
			LookupREFRByHandle(akProjectile->shooter, shooterRefr);
			if (shooterRefr != nullptr)
			{
				Actor* shooterActor = DYNAMIC_CAST(shooterRefr, TESObjectREFR, Actor);

				if (shooterActor != nullptr && shooterActor != (*g_thePlayer))
				{
					if ((((akProjectile->weaponSource != nullptr && (akProjectile->weaponSource->gameData.type == TESObjectWEAP::GameData::kType_Bow || akProjectile->weaponSource->gameData.type == TESObjectWEAP::GameData::kType_Bow2 || akProjectile->weaponSource->gameData.type == TESObjectWEAP::GameData::kType_CrossBow || akProjectile->weaponSource->gameData.type == TESObjectWEAP::GameData::kType_CBow))))
						|| (akProjectile->spell != nullptr && (akProjectile->castingSource != Projectile::CastingSource::kInstant) && ((SpellItem*)(akProjectile->spell)) != nullptr && ((SpellItem*)(akProjectile->spell))->data.spellType != SpellItem::SpellType::kVoicePower))
					{
						ProjectileHitLocation hitLocation = Unknown;
						auto impactData = *(Projectile::ImpactData**)((UInt64)akProjectile + 0x98);
						if (impactData != nullptr)
						{
							if (impactData->node != nullptr)
							{
								BipedModel* biped = (*g_thePlayer)->GetBipedSmall();
								if (biped != nullptr)
								{
									Biped* bipedData = biped->bipedData;
									if (bipedData != nullptr)
									{
										if (bipedData->unk10[9].object != nullptr && bipedData->unk10[9].object->GetAsNiNode() == impactData->node)//Shield
										{
											if (bipedData->unk10[9].item != nullptr)
											{
												TESObjectARMO* armor = DYNAMIC_CAST(bipedData->unk10[9].item, TESForm, TESObjectARMO);

												if (armor && armor->IsPlayable() && armor->keyword.HasKeyword(KeywordArmorShield))
												{
													LOG("Projectile hit on Shield");
													hitLocation = LeftArmItem;
												}
											}
										}
										else
										{
											if (impactData->node && impactData->node->m_name != nullptr && !Contains(impactData->node->m_name, "\"") && !Contains(impactData->node->m_name, "/"))
											{
												LOG("Projectile hit on Node: %s", impactData->node->m_name);
												if (strcmp(impactData->node->m_name, "NPC Head [Head]") == 0)
												{
													hitLocation = Head;
												}
												else if (strcmp(impactData->node->m_name, "NPC L UpperArm [LUar]") == 0
													|| strcmp(impactData->node->m_name, "NPC L Forearm [LLar]") == 0
													|| strcmp(impactData->node->m_name, "NPC L Hand [LHnd]") == 0)
												{
													hitLocation = LeftArm;
												}
												else if (strcmp(impactData->node->m_name, "NPC R UpperArm [RUar]") == 0
													|| strcmp(impactData->node->m_name, "NPC R Forearm [RLar]") == 0
													|| strcmp(impactData->node->m_name, "NPC R Hand [RHnd]") == 0)
												{
													hitLocation = RightArm;
												}
												else if (strcmp(impactData->node->m_name, "NPC Spine2 [Spn2]") == 0
													|| strcmp(impactData->node->m_name, "NPC L Calf [LClf]") == 0
													|| strcmp(impactData->node->m_name, "NPC R Calf [RClf]") == 0
													|| strcmp(impactData->node->m_name, "NPC Spine1 [Spn1]") == 0
													|| strcmp(impactData->node->m_name, "NPC L Thigh [LThg]") == 0
													|| strcmp(impactData->node->m_name, "NPC R Thigh [RThg]") == 0
													|| strcmp(impactData->node->m_name, "NPC Spine [Spn0]") == 0)
												{
													hitLocation = Body;
												}
											}
											for (int i = 33; i <= 40; i++)
											{
												if (bipedData->unk10[i].object != nullptr && bipedData->unk10[i].object->GetAsNiNode() == impactData->node)
												{
													if (bipedData->unk10[i].item != nullptr)
													{
														TESObjectWEAP* weapon = DYNAMIC_CAST(bipedData->unk10[i].item, TESForm, TESObjectWEAP);

														if (weapon != nullptr)
														{
															TESForm* rightEquippedObject = (*g_thePlayer)->GetEquippedObject(false);
															TESForm* leftEquippedObject = (*g_thePlayer)->GetEquippedObject(true);

															if (rightEquippedObject != nullptr && leftEquippedObject != nullptr && rightEquippedObject->formID == weapon->formID && leftEquippedObject->formID == weapon->formID)
															{
																if ((bipedData->unk10[i].object->m_parent != nullptr && bipedData->unk10[i].object->m_parent->m_parent != nullptr && bipedData->unk10[i].object->m_parent->m_parent->m_name && strcmp(bipedData->unk10[i].object->m_parent->m_parent->m_name, "NPC L Hand [LHnd]") == 0))
																{
																	hitLocation = LeftArmItem;
																}
																else
																{
																	hitLocation = RightArmItem;
																}
															}
															else if (rightEquippedObject != nullptr && rightEquippedObject->formID == weapon->formID)
															{
																hitLocation = RightArmItem;
															}
															else if (leftEquippedObject != nullptr && leftEquippedObject->formID == weapon->formID)
															{
																hitLocation = LeftArmItem;
															}

															if (bipedData->unk10[i].object->m_name != nullptr)
															{
																LOG("Projectile hit on Weapon. Parent: %s", (bipedData->unk10[i].object->m_parent != nullptr && bipedData->unk10[i].object->m_parent->m_name) ? bipedData->unk10[i].object->m_parent->m_name : "null");
															}
														}
													}
												}
											}
										}
									}
								}
							}
							else if (impactData->materialType != nullptr)
							{
								if (impactData->materialType->formID == 0x16978 || impactData->materialType->formID == 0x16979
									|| (impactData->materialType->havokImpactDataSet != nullptr && (impactData->materialType->havokImpactDataSet->formID == 0x363AB || impactData->materialType->havokImpactDataSet->formID == 0x363AA)))
								{
									LOG("Projectile hit on Shield");
									hitLocation = LeftArmItem;
								}
							}
							LOG("Impact Material FormId: %x hitLocation: %d", impactData->materialType != nullptr ? impactData->materialType->formID : 0, hitLocation);
						}

						if ((akProjectile->weaponSource != nullptr && (akProjectile->weaponSource->gameData.type == TESObjectWEAP::GameData::kType_Bow || akProjectile->weaponSource->gameData.type == TESObjectWEAP::GameData::kType_Bow2 || akProjectile->weaponSource->gameData.type == TESObjectWEAP::GameData::kType_CrossBow || akProjectile->weaponSource->gameData.type == TESObjectWEAP::GameData::kType_CBow)))
						{
							std::thread t13(RangedProjectileFeedback, shooterActor->pos, hit_pos, hitLocation);
							t13.detach();
						}
						else if (akProjectile->spell != nullptr && (akProjectile->castingSource != Projectile::CastingSource::kInstant) && ((SpellItem*)(akProjectile->spell)) != nullptr && ((SpellItem*)(akProjectile->spell))->data.spellType != SpellItem::SpellType::kVoicePower)
						{
							std::thread t13(SpellProjectileFeedback, ((SpellItem*)(akProjectile->spell)), shooterActor->pos, hit_pos, hitLocation);
							t13.detach();
						}
					}
				}
			}
		}

		return OnProjectileHitFunction(akProjectile, akTarget, point, unk1, unk2, unk3);
	}
		
	//Prog's code
	bool vlibIsSeated()
	{
		// Seeing 0xC000 when seated - but will test for 0x4000
		// 0x8000 becomes 1 first
		// 0x4000 becomes 1 next after 3 frames
		return ((*g_thePlayer)->actorState.flags04 & 0x4000);
	}
	bool vlibIsMounted()
	{
		return (sub_240690(*g_thePlayer) != 0);
	}
	bool vlibIsOnCart()
	{
		// Cart rides count as sitting, do NOT count as mounted.  Skeletons / HMD are not reparented...
		// False if the player is not sitting
		if (!vlibIsSeated())
			return false;

		// Get the furniture reference the player is sitting on
		TESObjectREFR* ref = papyrusActor::GetFurnitureReference(*g_thePlayer);
		if (ref) 
		{
			// If this is an object reference to a "CartFurniturePassenger" Form, then the player is on a cart
			if (ref->baseForm && ref->baseForm->formID == 0x00103445) 
			{
				return true;
			}
		}

		// May also be able to check "If sitting and menu controls disabled" as an alternative method
		return isOnCart.load();
	}

	bool isSprinting()
	{
		return ((*g_thePlayer)->actorState.flags04 & ActorState::kState_Sprinting) == ActorState::kState_Sprinting;
	}
}
