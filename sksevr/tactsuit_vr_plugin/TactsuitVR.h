#pragma once
#include "skse64/PapyrusNativeFunctions.h"
#include "skse64\GameReferences.h"
#include "skse64\GameRTTI.h"
#include "skse64\NiNodes.h"
#include "skse64\NiTypes.h"
#include "skse64/GameForms.h"
#include "skse64\PapyrusEvents.h"
#include "skse64\PapyrusVM.h"
#include "skse64\PapyrusForm.h"
#include "skse64\PapyrusActorValueInfo.h"
#include "skse64/GameData.h"
#include "skse64_common/Utilities.h"
#include "skse64\GameExtraData.h"

#include "config.h"
#include "shared/HapticLibrary.h"
#include "MenuChecker.h"

#include <list>
#include <thread>
#include <iostream>
#include <string>
#include <fstream>

namespace TactsuitVR
{
	const std::string MOD_VERSION = "2.0.7";

	enum FeedbackType
	{
		NoFeedback = 0,
		MeleeSwordRight = 1,
		MeleeAxeRight,
		MeleeMaceRight,
		MeleeDaggerRight,
		Melee2HAxeRight,
		Melee2HSwordRight,
		MeleeSwordLeft,
		MeleeAxeLeft,
		MeleeMaceLeft,
		MeleeDaggerLeft,
		Melee2HAxeLeft,
		Melee2HSwordLeft,
		MeleeFist,
		MeleeBash,
		MeleePowerAttack,
		Ranged,
		MagicFire,
		MagicShock,
		MagicIce,
		MagicAlteration,
		MagicIllusion,
		MagicRestoration,
		MagicPoison,
		MagicOther,
		MagicContinuousFire,
		MagicContinuousShock,
		MagicContinuousIce,
		MagicContinuousAlteration,
		MagicContinuousIllusion,
		MagicContinuousRestoration,
		MagicContinuousPoison,
		MagicContinuousOther,
		HeartBeat,
		HeartBeatFast,
		GreybeardPowerAbsorb,
		DragonSoul,
		WordWall,
		PlayerSpellFireLeft,
		PlayerSpellIceLeft,
		PlayerSpellShockLeft,
		PlayerSpellAlterationLeft,
		PlayerSpellIllusionLeft,
		PlayerSpellLightLeft,
		PlayerSpellRestorationLeft,
		PlayerSpellWardLeft,
		PlayerSpellConjurationLeft,
		PlayerSpellOtherLeft,
		PlayerSpellFireRight,
		PlayerSpellIceRight,
		PlayerSpellShockRight,
		PlayerSpellAlterationRight,
		PlayerSpellIllusionRight,
		PlayerSpellLightRight,
		PlayerSpellRestorationRight,
		PlayerSpellWardRight,
		PlayerSpellConjurationRight,
		PlayerSpellOtherRight,
		PlayerPowerAttackLeft,
		PlayerBashLeft,
		PlayerAttackLeft,
		PlayerPowerAttackRight,
		PlayerBashRight,
		PlayerAttackRight,
		PlayerBlockLeft,
		PlayerBlockRight,
		PlayerBowPullLeft,
		PlayerBowPullRight,
		PlayerBowHoldLeft,
		PlayerBowHoldRight,
		PlayerShout,
		PlayerShoutBindHands,
		PlayerShoutBindVest,
		PlayerCrossbowFireLeft,
		PlayerCrossbowFireRight,
		PlayerCrossbowKickbackLeft,
		PlayerCrossbowKickbackRight,
		
		Bite,

		SleeveHolsterStoreLeft,
		SleeveHolsterStoreRight,
		SleeveHolsterRemoveLeft,
		SleeveHolsterRemoveRight,
		BackpackStoreLeft,
		BackpackStoreRight,
		BackpackRemoveLeft,
		BackpackRemoveRight,

		StomachStore,//
		StomachRemove,//
		ChestStore,//
		ChestRemove,//
		HipHolsterStoreLeft,//
		HipHolsterStoreRight,//
		HipHolsterRemoveLeft,//
		HipHolsterRemoveRight,//
		Shout,//

		ShoutFire,
		ShoutFrost,//
		ShoutSteam,//
		ShoutLightning,
		HiggsPullLeft,//
		HiggsPullRight,//
		PlayerEnvironmentHitLeft,//
		PlayerEnvironmentHitRight,//
		PlayerThrowLeft,//
		PlayerThrowRight,//
		PlayerCatchLeft,//
		PlayerCatchRight,//

		TravelEffect,//
		Teleport,//

		EnvironmentRumble,

		DragonLanding,

		EquipHelmet,
		EquipCuirass,
		EquipGauntlets,
		EquipClothing,
		EquipHood,
		UnequipHelmet,
		UnequipCuirass,
		UnequipGauntlets,
		UnequipClothing,
		UnequipHood,

		Learned,

		UnholsterArrowLeftShoulder,
		UnholsterArrowRightShoulder,

		ConsumableDrink,
		ConsumableFood,

		Explosion,
		EnvironmentalPoison,
		EnvironmentalFire,
		EnvironmentalElectric,
		EnvironmentalFrost,

			EnvironmentalFireCloak,
			EnvironmentalElectricCloak,
			EnvironmentalFrostCloak,

		TentacleAttack,

		GiantStomp,
		GiantClubLeft,
		GiantClubRight,
		

		UnarmedDefault,
		UnarmedDragon,
		UnarmedFrostbiteSpider,
		UnarmedSabreCat,
		UnarmedSkeever,
		UnarmedSlaughterfish,
		UnarmedWisp,
		UnarmedDragonPriest,
		UnarmedDraugr,
		UnarmedWolf,
		UnarmedGiant,
		UnarmedIceWraith,
		UnarmedChaurus,
		UnarmedMammoth,
		UnarmedFrostAtronach,
		UnarmedFalmer,
		UnarmedHorse,
		UnarmedStormAtronach,
		UnarmedElk,
		UnarmedDwarvenSphere,
		UnarmedDwarvenSteam,
		UnarmedDwarvenSpider,
		UnarmedBear,
		UnarmedFlameAtronach,
		UnarmedWitchlight,
		UnarmedHorker,
		UnarmedTroll,
		UnarmedHagraven,
		UnarmedSpriggan,
		UnarmedMudcrab,
		UnarmedWerewolf,
		UnarmedChaurusFlyer,
		UnarmedGargoyle,
		UnarmedRiekling,
		UnarmedScrib,
		UnarmedSeeker,
		UnarmedMountedRiekling,
		UnarmedNetch,
		UnarmedBenthicLurker,

		//Attacks on Player's head
		DefaultHead,
		UnarmedHead,
		RangedHead,
		MeleeHead,

		MagicHeadFire,
		MagicHeadShock,
		MagicHeadIce,
		MagicHeadAlteration,
		MagicHeadIllusion,
		MagicHeadRestoration,

		RangedLeftArm,
		RangedRightArm,		
		MagicLeftArmFire,
		MagicLeftArmShock,
		MagicLeftArmIce,
		MagicLeftArmAlteration,
		MagicLeftArmIllusion,
		MagicLeftArmRestoration,
		MagicRightArmFire,
		MagicRightArmShock,
		MagicRightArmIce,
		MagicRightArmAlteration,
		MagicRightArmIllusion,
		MagicRightArmRestoration,

			HorseRidingSlow,
			HorseRiding,

		FallEffect,

			SwimVest20,
			SwimVest40,
			SwimVest60,
			SwimVest80,
			SwimVest100,

		DrowningEffectVest,
		DrowningEffectHead,

		Wind,

		MagicArmorSpell,

		Default
	};

	struct Feedback
	{
		Feedback()
		{

		}
		Feedback(FeedbackType _feedbackType, std::string _prefix, int _feedbackFileCount)
		{
			feedbackType = _feedbackType;
			prefix = _prefix;
			feedbackFileCount = _feedbackFileCount;
		}
		FeedbackType feedbackType;
		std::string prefix;
		int feedbackFileCount = 0;
	};

    extern float TOLERANCE;

	extern std::unordered_map<FeedbackType, Feedback> feedbackMap;

	void FillFeedbackList();

	void ProvideDotFeedback(bhaptics::PositionType position, int index, int intensity, int durationMillis);
	void ProvideHapticFeedback(float locationAngle, float locationHeight, FeedbackType effect, float intensityMultiplier = 1.0f, bool waitToPlay = false, bool playInMenu = false);
	void ProvideHapticFeedbackThread(float locationAngle, float locationHeight, FeedbackType effect, float intensityMultiplier = 1.0f, bool waitToPlay = false, bool playInMenu = false);
	void PauseHapticFeedBack(FeedbackType effect);
	void PauseHapticFeedBackSpellCastingRight();
	void PauseHapticFeedBackSpellCastingLeft();
	bool isPlayingHapticFeedBackAttackLeft();
	bool isPlayingHapticFeedBackAttackRight();
	void LateFeedback(float locationAngle, FeedbackType feedback, float intensity, int sleepAmount, int count, bool waitToPlay, bool playInMenu);
	bool IsPlayingKeyAll(FeedbackType effect);

	bool TactFileRegisterFilename(std::string& filename);
	void ProvideHapticFeedbackSpecificFile(float locationAngle, float locationHeight, std::string feedbackFileName, float intensityMultiplier, bool waitToPlay);
	void ProvideHapticFeedbackThreadSpecificFile(float locationAngle, float locationHeight, std::string feedbackFileName, float intensityMultiplier, bool waitToPlay);

	void CreateSystem();

	void TactFileRegister(std::string &configPath, std::string &filename, Feedback feedback);


	void RegisterFeedbackFiles();
}
