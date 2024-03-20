#include "TactsuitVR.h"



namespace TactsuitVR {
	bool systemInitialized = false;
	float TOLERANCE = 0.00001f;

	std::unordered_map<FeedbackType, Feedback> feedbackMap;

	std::vector<FeedbackType> spellCastingRight = { FeedbackType::PlayerSpellAlterationRight, FeedbackType::PlayerSpellConjurationRight, FeedbackType::PlayerSpellFireRight, FeedbackType::PlayerSpellIceRight, FeedbackType::PlayerSpellIllusionRight, FeedbackType::PlayerSpellLightRight, FeedbackType::PlayerSpellOtherRight, FeedbackType::PlayerSpellRestorationRight, FeedbackType::PlayerSpellShockRight, FeedbackType::PlayerSpellWardRight };
	std::vector<FeedbackType> spellCastingLeft = { FeedbackType::PlayerSpellAlterationLeft, FeedbackType::PlayerSpellConjurationLeft, FeedbackType::PlayerSpellFireLeft, FeedbackType::PlayerSpellIceLeft, FeedbackType::PlayerSpellIllusionLeft, FeedbackType::PlayerSpellLightLeft, FeedbackType::PlayerSpellOtherLeft, FeedbackType::PlayerSpellRestorationLeft, FeedbackType::PlayerSpellShockLeft, FeedbackType::PlayerSpellWardLeft };

	std::vector<FeedbackType> playerAttackingLeft = { FeedbackType::PlayerAttackLeft, FeedbackType::PlayerPowerAttackLeft, FeedbackType::PlayerBashLeft, FeedbackType::PlayerBlockLeft, FeedbackType::PlayerBowHoldLeft, FeedbackType::PlayerBowPullLeft };
	std::vector<FeedbackType> playerAttackingRight = { FeedbackType::PlayerAttackRight, FeedbackType::PlayerPowerAttackRight, FeedbackType::PlayerBashRight, FeedbackType::PlayerBlockRight, FeedbackType::PlayerBowHoldRight, FeedbackType::PlayerBowPullRight };

	void CreateSystem()
	{
		if (!systemInitialized)
		{
			Initialise("MOD_SKVR", "Mod_Skyrim_VR");
			LOG("System Initialized.");
			RegisterFeedbackFiles();
		}
		systemInitialized = true;
	}

	void TactFileRegister(std::string &configPath, std::string &filename, Feedback feedback)
	{
		if (feedbackMap.find(feedback.feedbackType) != feedbackMap.end())
		{
			feedbackMap[feedback.feedbackType].feedbackFileCount++;
			std::string tactKey = feedback.prefix;
			tactKey += std::to_string(feedbackMap[feedback.feedbackType].feedbackFileCount);

			std::string path = configPath;
			path += "\\";
			path += filename;

			std::ostringstream sstream;
			std::ifstream fs(path);
			sstream << fs.rdbuf();
			const std::string tactFileStr(sstream.str());

			RegisterFeedbackFromTactFile(tactKey.c_str(), tactFileStr.c_str());
		}
	}

	bool TactFileRegisterFilename(std::string& filename)
	{
		std::string	runtimeDirectory = GetRuntimeDirectory();
		if (!runtimeDirectory.empty())
		{
			std::string configPath = runtimeDirectory + "Data\\SKSE\\Plugins\\bHaptics";
			std::string path = configPath;
			path += "\\";
			path += filename;
			path += ".tact";

			LoadAndRegisterFeedback(filename.c_str(), path.c_str());
			return true;
		}
		else
		{
			return false;
		}
	}

	void FillFeedbackList()
	{
		feedbackMap.clear();
		feedbackMap[FeedbackType::MeleeSwordRight] = Feedback(FeedbackType::MeleeSwordRight, "MeleeSwordRight_", 0); //1
		feedbackMap[FeedbackType::MeleeAxeRight] = Feedback(FeedbackType::MeleeAxeRight, "MeleeAxeRight_", 0); //1
		feedbackMap[FeedbackType::MeleeMaceRight] = Feedback(FeedbackType::MeleeMaceRight, "MeleeMaceRight_", 0); //1
		feedbackMap[FeedbackType::MeleeDaggerRight] = Feedback(FeedbackType::MeleeDaggerRight, "MeleeDaggerRight_", 0); //1
		feedbackMap[FeedbackType::Melee2HAxeRight] = Feedback(FeedbackType::Melee2HAxeRight, "Melee2HAxeRight_", 0); //1
		feedbackMap[FeedbackType::Melee2HSwordRight] = Feedback(FeedbackType::Melee2HSwordRight, "Melee2HSwordRight_", 0); //1
		feedbackMap[FeedbackType::MeleeSwordLeft] = Feedback(FeedbackType::MeleeSwordLeft, "MeleeSwordLeft_", 0); //1
		feedbackMap[FeedbackType::MeleeAxeLeft] = Feedback(FeedbackType::MeleeAxeLeft, "MeleeAxeLeft_", 0); //1
		feedbackMap[FeedbackType::MeleeMaceLeft] = Feedback(FeedbackType::MeleeMaceLeft, "MeleeMaceLeft_", 0); //1
		feedbackMap[FeedbackType::MeleeDaggerLeft] = Feedback(FeedbackType::MeleeDaggerLeft, "MeleeDaggerLeft_", 0); //1
		feedbackMap[FeedbackType::Melee2HAxeLeft] = Feedback(FeedbackType::Melee2HAxeLeft, "Melee2HAxeLeft_", 0); //1
		feedbackMap[FeedbackType::Melee2HSwordLeft] = Feedback(FeedbackType::Melee2HSwordLeft, "Melee2HSwordLeft_", 0); //1
		feedbackMap[FeedbackType::MeleeFist] = Feedback(FeedbackType::MeleeFist, "MeleeFist_", 0); //1
		feedbackMap[FeedbackType::MeleeBash] = Feedback(FeedbackType::MeleeBash, "MeleeBash_", 0); //1
		feedbackMap[FeedbackType::MeleePowerAttack] = Feedback(FeedbackType::MeleePowerAttack, "MeleePowerAttack_", 0); //1
		feedbackMap[FeedbackType::Ranged] = Feedback(FeedbackType::Ranged, "Ranged_", 0); //1
		feedbackMap[FeedbackType::MagicFire] = Feedback(FeedbackType::MagicFire, "MagicFire_", 0); //1
		feedbackMap[FeedbackType::MagicShock] = Feedback(FeedbackType::MagicShock, "MagicShock_", 0); //1
		feedbackMap[FeedbackType::MagicIce] = Feedback(FeedbackType::MagicIce, "MagicIce_", 0); //1
		feedbackMap[FeedbackType::MagicAlteration] = Feedback(FeedbackType::MagicAlteration, "MagicAlteration_", 0); //1
		feedbackMap[FeedbackType::MagicIllusion] = Feedback(FeedbackType::MagicIllusion, "MagicIllusion_", 0); //1
		feedbackMap[FeedbackType::MagicRestoration] = Feedback(FeedbackType::MagicRestoration, "MagicRestoration_", 0); //1
		feedbackMap[FeedbackType::MagicPoison] = Feedback(FeedbackType::MagicPoison, "MagicPoison_", 0); //1
		feedbackMap[FeedbackType::MagicOther] = Feedback(FeedbackType::MagicOther, "MagicOther_", 0); //1
		feedbackMap[FeedbackType::MagicContinuousFire] = Feedback(FeedbackType::MagicContinuousFire, "MagicContinuousFire_", 0); //1
		feedbackMap[FeedbackType::MagicContinuousShock] = Feedback(FeedbackType::MagicContinuousShock, "MagicContinuousShock_", 0); //1
		feedbackMap[FeedbackType::MagicContinuousIce] = Feedback(FeedbackType::MagicContinuousIce, "MagicContinuousIce_", 0); //1
		feedbackMap[FeedbackType::MagicContinuousAlteration] = Feedback(FeedbackType::MagicContinuousAlteration, "MagicContinuousAlteration_", 0); //1
		feedbackMap[FeedbackType::MagicContinuousIllusion] = Feedback(FeedbackType::MagicContinuousIllusion, "MagicContinuousIllusion_", 0); //1
		feedbackMap[FeedbackType::MagicContinuousRestoration] = Feedback(FeedbackType::MagicContinuousRestoration, "MagicContinuousRestoration_", 0); //1
		feedbackMap[FeedbackType::MagicContinuousOther] = Feedback(FeedbackType::MagicContinuousOther, "MagicContinuousOther_", 0); //1
		feedbackMap[FeedbackType::MagicContinuousPoison] = Feedback(FeedbackType::MagicContinuousPoison, "MagicContinuousPoison_", 0); //1
		feedbackMap[FeedbackType::Shout] = Feedback(FeedbackType::Shout, "Shout_", 0); //1
		feedbackMap[FeedbackType::HeartBeat] = Feedback(FeedbackType::HeartBeat, "HeartBeat_", 0); //1
		feedbackMap[FeedbackType::HeartBeatFast] = Feedback(FeedbackType::HeartBeatFast, "HeartBeatFast_", 0); //1
		feedbackMap[FeedbackType::GreybeardPowerAbsorb] = Feedback(FeedbackType::GreybeardPowerAbsorb, "GreybeardPowerAbsorb_", 0); //1
		feedbackMap[FeedbackType::DragonSoul] = Feedback(FeedbackType::DragonSoul, "DragonSoul_", 0); //1
		feedbackMap[FeedbackType::WordWall] = Feedback(FeedbackType::WordWall, "WordWall_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellFireLeft] = Feedback(FeedbackType::PlayerSpellFireLeft, "PlayerSpellFireLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellIceLeft] = Feedback(FeedbackType::PlayerSpellIceLeft, "PlayerSpellIceLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellShockLeft] = Feedback(FeedbackType::PlayerSpellShockLeft, "PlayerSpellShockLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellAlterationLeft] = Feedback(FeedbackType::PlayerSpellAlterationLeft, "PlayerSpellAlterationLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellIllusionLeft] = Feedback(FeedbackType::PlayerSpellIllusionLeft, "PlayerSpellIllusionLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellLightLeft] = Feedback(FeedbackType::PlayerSpellLightLeft, "PlayerSpellLightLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellRestorationLeft] = Feedback(FeedbackType::PlayerSpellRestorationLeft, "PlayerSpellRestorationLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellWardLeft] = Feedback(FeedbackType::PlayerSpellWardLeft, "PlayerSpellWardLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellConjurationLeft] = Feedback(FeedbackType::PlayerSpellConjurationLeft, "PlayerSpellConjurationLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellOtherLeft] = Feedback(FeedbackType::PlayerSpellOtherLeft, "PlayerSpellOtherLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellFireRight] = Feedback(FeedbackType::PlayerSpellFireRight, "PlayerSpellFireRight_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellIceRight] = Feedback(FeedbackType::PlayerSpellIceRight, "PlayerSpellIceRight_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellShockRight] = Feedback(FeedbackType::PlayerSpellShockRight, "PlayerSpellShockRight_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellAlterationRight] = Feedback(FeedbackType::PlayerSpellAlterationRight, "PlayerSpellAlterationRight_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellIllusionRight] = Feedback(FeedbackType::PlayerSpellIllusionRight, "PlayerSpellIllusionRight_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellLightRight] = Feedback(FeedbackType::PlayerSpellLightRight, "PlayerSpellLightRight_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellRestorationRight] = Feedback(FeedbackType::PlayerSpellRestorationRight, "PlayerSpellRestorationRight_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellWardRight] = Feedback(FeedbackType::PlayerSpellWardRight, "PlayerSpellWardRight_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellConjurationRight] = Feedback(FeedbackType::PlayerSpellConjurationRight, "PlayerSpellConjurationRight_", 0); //1
		feedbackMap[FeedbackType::PlayerSpellOtherRight] = Feedback(FeedbackType::PlayerSpellOtherRight, "PlayerSpellOtherRight_", 0); //1
		feedbackMap[FeedbackType::PlayerPowerAttackLeft] = Feedback(FeedbackType::PlayerPowerAttackLeft, "PlayerPowerAttackLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerBashLeft] = Feedback(FeedbackType::PlayerBashLeft, "PlayerBashLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerAttackLeft] = Feedback(FeedbackType::PlayerAttackLeft, "PlayerAttackLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerPowerAttackRight] = Feedback(FeedbackType::PlayerPowerAttackRight, "PlayerPowerAttackRight_", 0); //1
		feedbackMap[FeedbackType::PlayerBashRight] = Feedback(FeedbackType::PlayerBashRight, "PlayerBashRight_", 0); //1
		feedbackMap[FeedbackType::PlayerAttackRight] = Feedback(FeedbackType::PlayerAttackRight, "PlayerAttackRight_", 0); //1
		feedbackMap[FeedbackType::PlayerBlockLeft] = Feedback(FeedbackType::PlayerBlockLeft, "PlayerBlockLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerBlockRight] = Feedback(FeedbackType::PlayerBlockRight, "PlayerBlockRight_", 0); //1
		feedbackMap[FeedbackType::PlayerBowPullLeft] = Feedback(FeedbackType::PlayerBowPullLeft, "PlayerBowPullLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerBowPullRight] = Feedback(FeedbackType::PlayerBowPullRight, "PlayerBowPullRight_", 0); //1
		feedbackMap[FeedbackType::PlayerBowHoldLeft] = Feedback(FeedbackType::PlayerBowHoldLeft, "PlayerBowHoldLeft_", 0); //1
		feedbackMap[FeedbackType::PlayerBowHoldRight] = Feedback(FeedbackType::PlayerBowHoldRight, "PlayerBowHoldRight_", 0); //1
		feedbackMap[FeedbackType::PlayerShout] = Feedback(FeedbackType::PlayerShout, "PlayerShout_", 0); //1
		feedbackMap[FeedbackType::Bite] = Feedback(FeedbackType::Bite, "Bite_", 0); //1
		feedbackMap[FeedbackType::HipHolsterStoreLeft] = Feedback(FeedbackType::HipHolsterStoreLeft, "HipHolsterStoreLeft_", 0); //1
		feedbackMap[FeedbackType::HipHolsterStoreRight] = Feedback(FeedbackType::HipHolsterStoreRight, "HipHolsterStoreRight_", 0); //1
		feedbackMap[FeedbackType::HipHolsterRemoveLeft] = Feedback(FeedbackType::HipHolsterRemoveLeft, "HipHolsterRemoveLeft_", 0); //1
		feedbackMap[FeedbackType::HipHolsterRemoveRight] = Feedback(FeedbackType::HipHolsterRemoveRight, "HipHolsterRemoveRight_", 0); //1
		feedbackMap[FeedbackType::SleeveHolsterStoreLeft] = Feedback(FeedbackType::SleeveHolsterStoreLeft, "SleeveHolsterStoreLeft_", 0); //1
		feedbackMap[FeedbackType::SleeveHolsterStoreRight] = Feedback(FeedbackType::SleeveHolsterStoreRight, "SleeveHolsterStoreRight_", 0); //1
		feedbackMap[FeedbackType::SleeveHolsterRemoveLeft] = Feedback(FeedbackType::SleeveHolsterRemoveLeft, "SleeveHolsterRemoveLeft_", 0); //1
		feedbackMap[FeedbackType::SleeveHolsterRemoveRight] = Feedback(FeedbackType::SleeveHolsterRemoveRight, "SleeveHolsterRemoveRight_", 0); //1
		feedbackMap[FeedbackType::BackpackStoreLeft] = Feedback(FeedbackType::BackpackStoreLeft, "BackpackStoreLeft_", 0); //1
		feedbackMap[FeedbackType::BackpackStoreRight] = Feedback(FeedbackType::BackpackStoreRight, "BackpackStoreRight_", 0); //1
		feedbackMap[FeedbackType::BackpackRemoveLeft] = Feedback(FeedbackType::BackpackRemoveLeft, "BackpackRemoveLeft_", 0); //1
		feedbackMap[FeedbackType::BackpackRemoveRight] = Feedback(FeedbackType::BackpackRemoveRight, "BackpackRemoveRight_", 0); //1
		feedbackMap[FeedbackType::StomachStore] = Feedback(FeedbackType::StomachStore, "StomachStore_", 0); //1
		feedbackMap[FeedbackType::StomachRemove] = Feedback(FeedbackType::StomachRemove, "StomachRemove_", 0); //1
		feedbackMap[FeedbackType::ChestStore] = Feedback(FeedbackType::ChestStore, "ChestStore_", 0); //1
		feedbackMap[FeedbackType::ChestRemove] = Feedback(FeedbackType::ChestRemove, "ChestRemove_", 0); //1
		feedbackMap[FeedbackType::ConsumableDrink] = Feedback(FeedbackType::ConsumableDrink, "ConsumableDrink_", 0); //1
		feedbackMap[FeedbackType::ConsumableFood] = Feedback(FeedbackType::ConsumableFood, "ConsumableFood_", 0); //1

		feedbackMap[FeedbackType::ShoutFire] = Feedback(FeedbackType::ShoutFire, "ShoutFire_", 0);
		feedbackMap[FeedbackType::ShoutFrost] = Feedback(FeedbackType::ShoutFrost, "ShoutFrost_", 0);
		feedbackMap[FeedbackType::ShoutSteam] = Feedback(FeedbackType::ShoutSteam, "ShoutSteam_", 0);
		feedbackMap[FeedbackType::ShoutLightning] = Feedback(FeedbackType::ShoutLightning, "ShoutLightning_", 0);
		feedbackMap[FeedbackType::HiggsPullLeft] = Feedback(FeedbackType::HiggsPullLeft, "HiggsPullLeft_", 0);
		feedbackMap[FeedbackType::HiggsPullRight] = Feedback(FeedbackType::HiggsPullRight, "HiggsPullRight_", 0);
		feedbackMap[FeedbackType::PlayerEnvironmentHitLeft] = Feedback(FeedbackType::PlayerEnvironmentHitLeft, "PlayerEnvironmentHitLeft_", 0);
		feedbackMap[FeedbackType::PlayerEnvironmentHitRight] = Feedback(FeedbackType::PlayerEnvironmentHitRight, "PlayerEnvironmentHitRight_", 0);
		feedbackMap[FeedbackType::PlayerThrowLeft] = Feedback(FeedbackType::PlayerThrowLeft, "PlayerThrowLeft_", 0);
		feedbackMap[FeedbackType::PlayerThrowRight] = Feedback(FeedbackType::PlayerThrowRight, "PlayerThrowRight_", 0);
		feedbackMap[FeedbackType::PlayerCatchLeft] = Feedback(FeedbackType::PlayerCatchLeft, "PlayerCatchLeft_", 0);
		feedbackMap[FeedbackType::PlayerCatchRight] = Feedback(FeedbackType::PlayerCatchRight, "PlayerCatchRight_", 0);

		feedbackMap[FeedbackType::PlayerShoutBindHands] = Feedback(FeedbackType::PlayerShoutBindHands, "PlayerShoutBindHands_", 0); //1
		feedbackMap[FeedbackType::PlayerShoutBindVest] = Feedback(FeedbackType::PlayerShoutBindVest, "PlayerShoutBindVest_", 0); //1
		
		feedbackMap[FeedbackType::TravelEffect] = Feedback(FeedbackType::TravelEffect, "TravelEffect_", 0);
		feedbackMap[FeedbackType::Teleport] = Feedback(FeedbackType::Teleport, "Teleport_", 0);

		feedbackMap[FeedbackType::EnvironmentRumble] = Feedback(FeedbackType::EnvironmentRumble, "EnvironmentRumble_", 0);
		feedbackMap[FeedbackType::DragonLanding] = Feedback(FeedbackType::DragonLanding, "DragonLanding_", 0);

		feedbackMap[FeedbackType::EquipHelmet] = Feedback(FeedbackType::EquipHelmet, "EquipHelmet_", 0);
		feedbackMap[FeedbackType::EquipCuirass] = Feedback(FeedbackType::EquipCuirass, "EquipCuirass_", 0);
		feedbackMap[FeedbackType::EquipGauntlets] = Feedback(FeedbackType::EquipGauntlets, "EquipGauntlets_", 0);
		feedbackMap[FeedbackType::EquipClothing] = Feedback(FeedbackType::EquipClothing, "EquipClothing_", 0);
		feedbackMap[FeedbackType::EquipHood] = Feedback(FeedbackType::EquipHood, "EquipHood_", 0);
		
		feedbackMap[FeedbackType::UnequipHelmet] = Feedback(FeedbackType::UnequipHelmet, "UnequipHelmet_", 0);
		feedbackMap[FeedbackType::UnequipCuirass] = Feedback(FeedbackType::UnequipCuirass, "UnequipCuirass_", 0);
		feedbackMap[FeedbackType::UnequipGauntlets] = Feedback(FeedbackType::UnequipGauntlets, "UnequipGauntlets_", 0);
		feedbackMap[FeedbackType::UnequipClothing] = Feedback(FeedbackType::UnequipClothing, "UnequipClothing_", 0);
		feedbackMap[FeedbackType::UnequipHood] = Feedback(FeedbackType::UnequipHood, "UnequipHood_", 0);
		
		feedbackMap[FeedbackType::Learned] = Feedback(FeedbackType::Learned, "Learned_", 0);

		feedbackMap[FeedbackType::UnholsterArrowLeftShoulder] = Feedback(FeedbackType::UnholsterArrowLeftShoulder, "UnholsterArrowLeftShoulder_", 0);
		feedbackMap[FeedbackType::UnholsterArrowRightShoulder] = Feedback(FeedbackType::UnholsterArrowRightShoulder, "UnholsterArrowRightShoulder_", 0);


		feedbackMap[FeedbackType::Explosion] = Feedback(FeedbackType::Explosion, "Explosion_", 0);
		feedbackMap[FeedbackType::EnvironmentalPoison] = Feedback(FeedbackType::EnvironmentalPoison, "EnvironmentalPoison_", 0);
		feedbackMap[FeedbackType::EnvironmentalFire] = Feedback(FeedbackType::EnvironmentalFire, "EnvironmentalFire_", 0);
		feedbackMap[FeedbackType::EnvironmentalElectric] = Feedback(FeedbackType::EnvironmentalElectric, "EnvironmentalElectric_", 0);
		feedbackMap[FeedbackType::EnvironmentalFrost] = Feedback(FeedbackType::EnvironmentalFrost, "EnvironmentalFrost_", 0);
		feedbackMap[FeedbackType::EnvironmentalFireCloak] = Feedback(FeedbackType::EnvironmentalFireCloak, "EnvironmentalFireCloak_", 0);
		feedbackMap[FeedbackType::EnvironmentalElectricCloak] = Feedback(FeedbackType::EnvironmentalElectricCloak, "EnvironmentalElectricCloak_", 0);
		feedbackMap[FeedbackType::EnvironmentalFrostCloak] = Feedback(FeedbackType::EnvironmentalFrostCloak, "EnvironmentalFrostCloak_", 0);
		
		feedbackMap[FeedbackType::UnarmedDefault] = Feedback(FeedbackType::UnarmedDefault, "UnarmedDefault_", 0);
		feedbackMap[FeedbackType::UnarmedDragon] = Feedback(FeedbackType::UnarmedDragon, "UnarmedDragon_", 0);
		feedbackMap[FeedbackType::UnarmedFrostbiteSpider] = Feedback(FeedbackType::UnarmedFrostbiteSpider, "UnarmedFrostbiteSpider_", 0);
		feedbackMap[FeedbackType::UnarmedSabreCat] = Feedback(FeedbackType::UnarmedSabreCat, "UnarmedSabreCat_", 0);
		feedbackMap[FeedbackType::UnarmedSkeever] = Feedback(FeedbackType::UnarmedSkeever, "UnarmedSkeever_", 0);
		feedbackMap[FeedbackType::UnarmedSlaughterfish] = Feedback(FeedbackType::UnarmedSlaughterfish, "UnarmedSlaughterfish_", 0);
		feedbackMap[FeedbackType::UnarmedWisp] = Feedback(FeedbackType::UnarmedWisp, "UnarmedWisp_", 0);
		feedbackMap[FeedbackType::UnarmedDragonPriest] = Feedback(FeedbackType::UnarmedDragonPriest, "UnarmedDragonPriest_", 0);
		feedbackMap[FeedbackType::UnarmedDraugr] = Feedback(FeedbackType::UnarmedDraugr, "UnarmedDraugr_", 0);
		feedbackMap[FeedbackType::UnarmedWolf] = Feedback(FeedbackType::UnarmedWolf, "UnarmedWolf_", 0);
		feedbackMap[FeedbackType::UnarmedGiant] = Feedback(FeedbackType::UnarmedGiant, "UnarmedGiant_", 0);
		feedbackMap[FeedbackType::UnarmedIceWraith] = Feedback(FeedbackType::UnarmedIceWraith, "UnarmedIceWraith_", 0);
		feedbackMap[FeedbackType::UnarmedChaurus] = Feedback(FeedbackType::UnarmedChaurus, "UnarmedChaurus_", 0);
		feedbackMap[FeedbackType::UnarmedMammoth] = Feedback(FeedbackType::UnarmedMammoth, "UnarmedMammoth_", 0);
		feedbackMap[FeedbackType::UnarmedFrostAtronach] = Feedback(FeedbackType::UnarmedFrostAtronach, "UnarmedFrostAtronach_", 0);
		feedbackMap[FeedbackType::UnarmedFalmer] = Feedback(FeedbackType::UnarmedFalmer, "UnarmedFalmer_", 0);
		feedbackMap[FeedbackType::UnarmedHorse] = Feedback(FeedbackType::UnarmedHorse, "UnarmedHorse_", 0);
		feedbackMap[FeedbackType::UnarmedStormAtronach] = Feedback(FeedbackType::UnarmedStormAtronach, "UnarmedStormAtronach_", 0);
		feedbackMap[FeedbackType::UnarmedElk] = Feedback(FeedbackType::UnarmedElk, "UnarmedElk_", 0);
		feedbackMap[FeedbackType::UnarmedDwarvenSphere] = Feedback(FeedbackType::UnarmedDwarvenSphere, "UnarmedDwarvenSphere_", 0);
		feedbackMap[FeedbackType::UnarmedDwarvenSteam] = Feedback(FeedbackType::UnarmedDwarvenSteam, "UnarmedDwarvenSteam_", 0);
		feedbackMap[FeedbackType::UnarmedDwarvenSpider] = Feedback(FeedbackType::UnarmedDwarvenSpider, "UnarmedDwarvenSpider_", 0);
		feedbackMap[FeedbackType::UnarmedBear] = Feedback(FeedbackType::UnarmedBear, "UnarmedBear_", 0);
		feedbackMap[FeedbackType::UnarmedFlameAtronach] = Feedback(FeedbackType::UnarmedFlameAtronach, "UnarmedFlameAtronach_", 0);
		feedbackMap[FeedbackType::UnarmedWitchlight] = Feedback(FeedbackType::UnarmedWitchlight, "UnarmedWitchlight_", 0);
		feedbackMap[FeedbackType::UnarmedHorker] = Feedback(FeedbackType::UnarmedHorker, "UnarmedHorker_", 0);
		feedbackMap[FeedbackType::UnarmedTroll] = Feedback(FeedbackType::UnarmedTroll, "UnarmedTroll_", 0);
		feedbackMap[FeedbackType::UnarmedHagraven] = Feedback(FeedbackType::UnarmedHagraven, "UnarmedHagraven_", 0);
		feedbackMap[FeedbackType::UnarmedSpriggan] = Feedback(FeedbackType::UnarmedSpriggan, "UnarmedSpriggan_", 0);
		feedbackMap[FeedbackType::UnarmedMudcrab] = Feedback(FeedbackType::UnarmedMudcrab, "UnarmedMudcrab_", 0);
		feedbackMap[FeedbackType::UnarmedWerewolf] = Feedback(FeedbackType::UnarmedWerewolf, "UnarmedWerewolf_", 0);
		feedbackMap[FeedbackType::UnarmedChaurusFlyer] = Feedback(FeedbackType::UnarmedChaurusFlyer, "UnarmedChaurusFlyer_", 0);
		feedbackMap[FeedbackType::UnarmedGargoyle] = Feedback(FeedbackType::UnarmedGargoyle, "UnarmedGargoyle_", 0);
		feedbackMap[FeedbackType::UnarmedRiekling] = Feedback(FeedbackType::UnarmedRiekling, "UnarmedRiekling_", 0);
		feedbackMap[FeedbackType::UnarmedScrib] = Feedback(FeedbackType::UnarmedScrib, "UnarmedScrib_", 0);
		feedbackMap[FeedbackType::UnarmedSeeker] = Feedback(FeedbackType::UnarmedSeeker, "UnarmedSeeker_", 0);
		feedbackMap[FeedbackType::UnarmedMountedRiekling] = Feedback(FeedbackType::UnarmedMountedRiekling, "UnarmedMountedRiekling_", 0);
		feedbackMap[FeedbackType::UnarmedNetch] = Feedback(FeedbackType::UnarmedNetch, "UnarmedNetch_", 0);
		feedbackMap[FeedbackType::UnarmedBenthicLurker] = Feedback(FeedbackType::UnarmedBenthicLurker, "UnarmedBenthicLurker_", 0);

		feedbackMap[FeedbackType::DefaultHead] = Feedback(FeedbackType::DefaultHead, "DefaultHead_", 0);
		feedbackMap[FeedbackType::UnarmedHead] = Feedback(FeedbackType::UnarmedHead, "UnarmedHead_", 0);
		feedbackMap[FeedbackType::RangedHead] = Feedback(FeedbackType::RangedHead, "RangedHead_", 0);
		feedbackMap[FeedbackType::MeleeHead] = Feedback(FeedbackType::MeleeHead, "MeleeHead_", 0);

		feedbackMap[FeedbackType::MagicHeadFire] = Feedback(FeedbackType::MagicHeadFire, "MagicHeadFire_", 0);
		feedbackMap[FeedbackType::MagicHeadShock] = Feedback(FeedbackType::MagicHeadShock, "MagicHeadShock_", 0);
		feedbackMap[FeedbackType::MagicHeadIce] = Feedback(FeedbackType::MagicHeadIce, "MagicHeadIce_", 0);
		feedbackMap[FeedbackType::MagicHeadAlteration] = Feedback(FeedbackType::MagicHeadAlteration, "MagicHeadAlteration_", 0);
		feedbackMap[FeedbackType::MagicHeadIllusion] = Feedback(FeedbackType::MagicHeadIllusion, "MagicHeadIllusion_", 0);
		feedbackMap[FeedbackType::MagicHeadRestoration] = Feedback(FeedbackType::MagicHeadRestoration, "MagicHeadRestoration_", 0);

		feedbackMap[FeedbackType::RangedLeftArm] = Feedback(FeedbackType::RangedLeftArm, "RangedLeftArm_", 0);
		feedbackMap[FeedbackType::RangedRightArm] = Feedback(FeedbackType::RangedRightArm, "RangedRightArm_", 0);
		feedbackMap[FeedbackType::MagicLeftArmFire] = Feedback(FeedbackType::MagicLeftArmFire, "MagicLeftArmFire_", 0);
		feedbackMap[FeedbackType::MagicLeftArmShock] = Feedback(FeedbackType::MagicLeftArmShock, "MagicLeftArmShock_", 0);
		feedbackMap[FeedbackType::MagicLeftArmIce] = Feedback(FeedbackType::MagicLeftArmIce, "MagicLeftArmIce_", 0);
		feedbackMap[FeedbackType::MagicLeftArmAlteration] = Feedback(FeedbackType::MagicLeftArmAlteration, "MagicLeftArmAlteration_", 0);
		feedbackMap[FeedbackType::MagicLeftArmIllusion] = Feedback(FeedbackType::MagicLeftArmIllusion, "MagicLeftArmIllusion_", 0);
		feedbackMap[FeedbackType::MagicLeftArmRestoration] = Feedback(FeedbackType::MagicLeftArmRestoration, "MagicLeftArmRestoration_", 0);
		feedbackMap[FeedbackType::MagicRightArmFire] = Feedback(FeedbackType::MagicRightArmFire, "MagicRightArmFire_", 0);
		feedbackMap[FeedbackType::MagicRightArmShock] = Feedback(FeedbackType::MagicRightArmShock, "MagicRightArmShock_", 0);
		feedbackMap[FeedbackType::MagicRightArmIce] = Feedback(FeedbackType::MagicRightArmIce, "MagicRightArmIce_", 0);
		feedbackMap[FeedbackType::MagicRightArmAlteration] = Feedback(FeedbackType::MagicRightArmAlteration, "MagicRightArmAlteration_", 0);
		feedbackMap[FeedbackType::MagicRightArmIllusion] = Feedback(FeedbackType::MagicRightArmIllusion, "MagicRightArmIllusion_", 0);
		feedbackMap[FeedbackType::MagicRightArmRestoration] = Feedback(FeedbackType::MagicRightArmRestoration, "MagicRightArmRestoration_", 0);

		feedbackMap[FeedbackType::HorseRiding] = Feedback(FeedbackType::HorseRiding, "HorseRiding_", 0);
		feedbackMap[FeedbackType::HorseRidingSlow] = Feedback(FeedbackType::HorseRidingSlow, "HorseRidingSlow_", 0);

		feedbackMap[FeedbackType::TentacleAttack] = Feedback(FeedbackType::TentacleAttack, "TentacleAttack_", 0);


		feedbackMap[FeedbackType::GiantStomp] = Feedback(FeedbackType::GiantStomp, "GiantStomp_", 0);
		feedbackMap[FeedbackType::GiantClubLeft] = Feedback(FeedbackType::GiantClubLeft, "GiantClubLeft_", 0);
		feedbackMap[FeedbackType::GiantClubRight] = Feedback(FeedbackType::GiantClubRight, "GiantClubRight_", 0);

		feedbackMap[FeedbackType::FallEffect] = Feedback(FeedbackType::FallEffect, "FallEffect_", 0);

		feedbackMap[FeedbackType::SwimVest20] = Feedback(FeedbackType::SwimVest20, "SwimVest20_", 0);
		feedbackMap[FeedbackType::SwimVest40] = Feedback(FeedbackType::SwimVest40, "SwimVest40_", 0);
		feedbackMap[FeedbackType::SwimVest60] = Feedback(FeedbackType::SwimVest60, "SwimVest60_", 0);
		feedbackMap[FeedbackType::SwimVest80] = Feedback(FeedbackType::SwimVest80, "SwimVest80_", 0);
		feedbackMap[FeedbackType::SwimVest100] = Feedback(FeedbackType::SwimVest100, "SwimVest100_", 0);

		feedbackMap[FeedbackType::DrowningEffectVest] = Feedback(FeedbackType::DrowningEffectVest, "DrowningEffectVest_", 0);
		feedbackMap[FeedbackType::DrowningEffectHead] = Feedback(FeedbackType::DrowningEffectHead, "DrowningEffectHead_", 0);

		feedbackMap[FeedbackType::PlayerCrossbowFireLeft] = Feedback(FeedbackType::PlayerCrossbowFireLeft, "PlayerCrossbowFireLeft_", 0);
		feedbackMap[FeedbackType::PlayerCrossbowFireRight] = Feedback(FeedbackType::PlayerCrossbowFireRight, "PlayerCrossbowFireRight_", 0);
		feedbackMap[FeedbackType::PlayerCrossbowKickbackLeft] = Feedback(FeedbackType::PlayerCrossbowKickbackLeft, "PlayerCrossbowKickbackLeft_", 0);
		feedbackMap[FeedbackType::PlayerCrossbowKickbackRight] = Feedback(FeedbackType::PlayerCrossbowKickbackRight, "PlayerCrossbowKickbackRight_", 0);

		feedbackMap[FeedbackType::Wind] = Feedback(FeedbackType::Wind, "Wind_", 0);

		feedbackMap[FeedbackType::MagicArmorSpell] = Feedback(FeedbackType::MagicArmorSpell, "MagicArmorSpell_", 0);


		feedbackMap[FeedbackType::Default] = Feedback(FeedbackType::Default, "Default_", 0); //1		
	}

	void RegisterFeedbackFiles()
	{
		std::string	runtimeDirectory = GetRuntimeDirectory();
		if (!runtimeDirectory.empty())
		{
			std::string configPath = runtimeDirectory + "Data\\SKSE\\Plugins\\bHaptics";

			auto tactList = get_all_files_names_within_folder(configPath.c_str());

			for (std::size_t i = 0; i < tactList.size(); i++)
			{
				std::string filename = tactList.at(i);

				if (filename == "." || filename == "..")
					continue;

				LOG("File found: %s", filename.c_str());

				bool found = false;
				for (std::pair<FeedbackType, Feedback> element : feedbackMap)
				{
					if (stringStartsWith(filename, element.second.prefix))
					{
						TactFileRegister(configPath, filename, element.second);
						found = true;
						break;
					}
				}

				if (!found)
				{
					LOG("File category unknown: %s", filename.c_str());
					skipTactExtension(filename);
					TactFileRegisterFilename(filename);
				}
			}
		}
	}

	void PauseHapticFeedBack(FeedbackType effect)
	{
		for (int i = 1; i <= feedbackMap[effect].feedbackFileCount; i++)
		{
			TurnOffKey((feedbackMap[effect].prefix + std::to_string(i)).c_str());
		}
	}	

	void PauseHapticFeedBackSpellCastingRight()
	{
		for each(FeedbackType effectType in spellCastingRight)
		{
			for (int i = 1; i <= feedbackMap[effectType].feedbackFileCount; i++)
			{
				TurnOffKey((feedbackMap[effectType].prefix + std::to_string(i)).c_str());
			}
		}
	}

	void PauseHapticFeedBackSpellCastingLeft()
	{
		for each(FeedbackType effectType in spellCastingLeft)
		{
			for (int i = 1; i <= feedbackMap[effectType].feedbackFileCount; i++)
			{
				TurnOffKey((feedbackMap[effectType].prefix + std::to_string(i)).c_str());
			}
		}
	}

	bool isPlayingHapticFeedBackAttackRight()
	{
		for each (FeedbackType effectType in playerAttackingRight)
		{
			for (int i = 1; i <= feedbackMap[effectType].feedbackFileCount; i++)
			{
				std::string key = feedbackMap[effectType].prefix + std::to_string(i);
				if (IsPlayingKey(key.c_str()))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool isPlayingHapticFeedBackAttackLeft()
	{
		for each (FeedbackType effectType in playerAttackingLeft)
		{
			for (int i = 1; i <= feedbackMap[effectType].feedbackFileCount; i++)
			{
				std::string key = feedbackMap[effectType].prefix + std::to_string(i);
				if (IsPlayingKey(key.c_str()))
				{
					return true;
				}
			}
		}
		return false;
	}

	void ProvideDotFeedback(bhaptics::PositionType position, int index, int intensity, int durationMillis)
	{
		if (intensity < TOLERANCE)
			return;

		if (isGameStoppedNoDialogue())
		{
			return;
		}

		if (!systemInitialized)
			CreateSystem();

		std::string key;

		std::vector<bhaptics::DotPoint> points;
		bhaptics::DotPoint point = bhaptics::DotPoint(index, intensity);
		points.emplace_back(point);
		SubmitDot(key.c_str(), position, points, durationMillis);
	}

	void ProvideHapticFeedback(float locationAngle, float locationHeight, FeedbackType effect, float intensityMultiplier, bool waitToPlay, bool playInMenu)
	{
		if (intensityMultiplier > TOLERANCE)
		{
			std::thread t0(ProvideHapticFeedbackThread, locationAngle, locationHeight, effect, intensityMultiplier, waitToPlay, playInMenu);
			t0.detach();
		}
	}

	bool IsPlayingKeyAll(FeedbackType effect)
	{
		std::string prefix = feedbackMap[effect].prefix;
		int feedbackFileCount = feedbackMap[effect].feedbackFileCount;
		for (int i = 1; i <= feedbackFileCount; i++)
		{
			std::string key = prefix + std::to_string(i);
			if (IsPlayingKey(key.c_str()))
			{
				return true;
			}
		}
		return false;
	}

	void LateFeedback(float locationAngle, FeedbackType feedback, float intensity, int sleepAmount, int count, bool waitToPlay, bool playInMenu)
	{
		for (int i = 0; i < count; i++)
		{
			Sleep(sleepAmount);
			ProvideHapticFeedback(locationAngle, 0, feedback, intensity, waitToPlay, playInMenu);
		}
	}

	void ProvideHapticFeedbackThread(float locationAngle, float locationHeight, FeedbackType effect, float intensityMultiplier, bool waitToPlay, bool playInMenu)
	{
		if (intensityMultiplier < TOLERANCE)
			return;

		if (isGameStoppedNoDialogue() && !playInMenu)
		{
			return;
		}

		if (!systemInitialized)
			CreateSystem();

		if (feedbackMap.find(effect) != feedbackMap.end())
		{
			if (feedbackMap[effect].feedbackFileCount > 0)
			{
				if (waitToPlay)
				{
					if (IsPlayingKeyAll(effect))
					{
						return;
					}
				}

				if (locationHeight < -0.45f)
					locationHeight = -0.45f;
				else if (locationHeight > 0.45f)
					locationHeight = 0.45f;

				const std::string key = feedbackMap[effect].prefix + std::to_string(randi(1, feedbackMap[effect].feedbackFileCount));

				bhaptics::RotationOption RotOption;
				RotOption.OffsetAngleX = locationAngle;
				RotOption.OffsetY = locationHeight;

				bhaptics::ScaleOption scaleOption;
				scaleOption.Duration = 1.0f;
				scaleOption.Intensity = (intensityMultiplier != 1.0f) ? intensityMultiplier : 1.0f;

				LOG("Key: %s  OffsetY: %g  OffsetAngleX: %g  Intensity: %g", key.c_str(), locationHeight, locationAngle, scaleOption.Intensity);

				SubmitRegisteredAlt(key.c_str(), key.c_str(), scaleOption, RotOption);

				LOG("ProvideHapticFeedback submit successful");
			}
			else
			{
				LOG("No file found for %s", feedbackMap[effect].prefix.c_str());
			}
		}
	}

	void ProvideHapticFeedbackSpecificFile(float locationAngle, float locationHeight, std::string feedbackFileName, float intensityMultiplier, bool waitToPlay)
	{
		if (intensityMultiplier > TOLERANCE)
		{
			std::thread t0(ProvideHapticFeedbackThreadSpecificFile, locationAngle, locationHeight, feedbackFileName, intensityMultiplier, waitToPlay);
			t0.detach();
		}
	}

	void ProvideHapticFeedbackThreadSpecificFile(float locationAngle, float locationHeight, std::string feedbackFileName, float intensityMultiplier, bool waitToPlay)
	{
		if (intensityMultiplier < TOLERANCE)
			return;

		if (isGameStoppedNoDialogue())
		{
			return;
		}

		if (!systemInitialized)
			CreateSystem();

		//bool registered = false;

		//if(!IsFeedbackRegistered(feedbackFileName.c_str()))
		//{
		//	registered = TactFileRegisterFilename(feedbackFileName);
		//}
		//else
		//{
		//	registered = true;
		//}
				
		if (waitToPlay)
		{
			if (IsPlayingKey(feedbackFileName.c_str()))
				return;
		}

		if (locationHeight < -0.5f)
			locationHeight = -0.5f;
		else if (locationHeight > 0.5f)
			locationHeight = 0.5f;

		LOG("Key: %s  OffsetY: %g  OffsetAngleX: %g", feedbackFileName.c_str(), locationHeight, locationAngle);

		bhaptics::RotationOption RotOption;
		RotOption.OffsetAngleX = locationAngle;
		RotOption.OffsetY = locationHeight;

		bhaptics::ScaleOption scaleOption;
		scaleOption.Duration = 1.0f;
		scaleOption.Intensity = (intensityMultiplier != 1.0f) ? intensityMultiplier : 1.0f;
		
		SubmitRegisteredAlt(feedbackFileName.c_str(), feedbackFileName.c_str(), scaleOption, RotOption);

		LOG("ProvideHapticFeedback submit successful");
	}
}
