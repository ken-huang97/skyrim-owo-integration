#include "TactsuitVR.h"

#include "Domain/BakedSensationsParser.h"
#include "Domain/GameAuth.h"
#include "Controller/OWO.h"

namespace TactsuitVR {
	bool systemInitialized = false;
	float TOLERANCE = 0.00001f;

	std::shared_ptr<OWOGame::OWO> owo;

	std::unordered_map<FeedbackType, Feedback> feedbackMap;

	std::vector<FeedbackType> spellCastingRight = { FeedbackType::PlayerSpellAlterationRight, FeedbackType::PlayerSpellConjurationRight, FeedbackType::PlayerSpellFireRight, FeedbackType::PlayerSpellIceRight, FeedbackType::PlayerSpellIllusionRight, FeedbackType::PlayerSpellLightRight, FeedbackType::PlayerSpellOtherRight, FeedbackType::PlayerSpellRestorationRight, FeedbackType::PlayerSpellShockRight, FeedbackType::PlayerSpellWardRight };
	std::vector<FeedbackType> spellCastingLeft = { FeedbackType::PlayerSpellAlterationLeft, FeedbackType::PlayerSpellConjurationLeft, FeedbackType::PlayerSpellFireLeft, FeedbackType::PlayerSpellIceLeft, FeedbackType::PlayerSpellIllusionLeft, FeedbackType::PlayerSpellLightLeft, FeedbackType::PlayerSpellOtherLeft, FeedbackType::PlayerSpellRestorationLeft, FeedbackType::PlayerSpellShockLeft, FeedbackType::PlayerSpellWardLeft };

	std::vector<FeedbackType> playerAttackingLeft = { FeedbackType::PlayerAttackLeft, FeedbackType::PlayerPowerAttackLeft, FeedbackType::PlayerBashLeft, FeedbackType::PlayerBlockLeft, FeedbackType::PlayerBowHoldLeft, FeedbackType::PlayerBowPullLeft };
	std::vector<FeedbackType> playerAttackingRight = { FeedbackType::PlayerAttackRight, FeedbackType::PlayerPowerAttackRight, FeedbackType::PlayerBashRight, FeedbackType::PlayerBlockRight, FeedbackType::PlayerBowHoldRight, FeedbackType::PlayerBowPullRight };

	void CreateSystem()
	{

		if (!systemInitialized)
		{
			// Initialise("MOD_SKVR", "Mod_Skyrim_VR");
			
			// TODO: Add gameId when approved
			owoVector<owoString> bakedSensations = owoVector<owoString>(); // TODO: Generate sensations for create
			auto auth = OWOGame::GameAuth::Create(bakedSensations);

			owo = OWOGame::OWO::Create();
			owo->Configure(auth);
			//owo->AutoConnect();
			
			std::string ip = "192.168.0.195";

			OWOGame::ConnectionState connectionState = owo->Connect({ ip });
			LOG("Connecting to %s", ip.c_str());

			int breaker = 0;
			int maxWait = 30;
			int waitMs = 1000;

			while (owo->State() != OWOGame::ConnectionState::Connected && breaker < maxWait) {
				std::string cs;

				switch (owo->State())
				{
					case OWOGame::ConnectionState::Connected:   cs= "Connected"; break;
					case OWOGame::ConnectionState::Disconnected:   cs ="Disconnected";  break;
					case OWOGame::ConnectionState::Connecting: cs = "Connecting"; break;
					default:      cs = "[Unknown]"; break;
				}

				LOG("OWO State: %s", cs.c_str());
				WaitFor(waitMs);
				breaker++;

				if (breaker >= maxWait) {
					LOG("OWO failed connection, breaking");
				}
			}

			owo->UpdateStatus(1000); // TODO Need to pass how long since start?

			LOG("OWO System Initialized.");
			// RegisterFeedbackFiles();
		}
		systemInitialized = true;
	}


	OWOGame::Sensation* TactFileRegister(std::string &configPath, std::string &filename, Feedback feedback)
	{
		if (feedbackMap.find(feedback.feedbackType) != feedbackMap.end())
		{

			std::string path = configPath;
			path += "\\";
			path += filename;

			std::ostringstream sstream;
			std::ifstream fs(path);
			sstream << fs.rdbuf();
			const std::string tactFileStr(sstream.str());

			sharedPtr<OWOGame::BakedSensation> sensation = std::move(OWOGame::BakedSensationsParser::Parse(tactFileStr));
			feedbackMap[feedback.feedbackType].feedbackSensations.emplace_back(sensation);

			return sensation.get();
			// RegisterFeedbackFromTactFile(tactKey.c_str(), tactFileStr.c_str());
		}
	}

	// bool TactFileRegisterFilename(std::string& filename)
	// {
	// 	std::string	runtimeDirectory = GetRuntimeDirectory();
	// 	if (!runtimeDirectory.empty())
	// 	{
	// 		std::string configPath = runtimeDirectory + "Data\\SKSE\\Plugins\\bHaptics";
	// 		std::string path = configPath;
	// 		path += "\\";
	// 		path += filename;
	// 		path += ".tact";

	// 		LoadAndRegisterFeedback(filename.c_str(), path.c_str());
	// 		return true;
	// 	}
	// 	else
	// 	{
	// 		return false;
	// 	}
	// }

	void FillFeedbackList()
	{
		feedbackMap.clear();
		feedbackMap[FeedbackType::MeleeSwordRight] = Feedback(FeedbackType::MeleeSwordRight, "MeleeSwordRight_"); //1
		feedbackMap[FeedbackType::MeleeAxeRight] = Feedback(FeedbackType::MeleeAxeRight, "MeleeAxeRight_"); //1
		feedbackMap[FeedbackType::MeleeMaceRight] = Feedback(FeedbackType::MeleeMaceRight, "MeleeMaceRight_"); //1
		feedbackMap[FeedbackType::MeleeDaggerRight] = Feedback(FeedbackType::MeleeDaggerRight, "MeleeDaggerRight_"); //1
		feedbackMap[FeedbackType::Melee2HAxeRight] = Feedback(FeedbackType::Melee2HAxeRight, "Melee2HAxeRight_"); //1
		feedbackMap[FeedbackType::Melee2HSwordRight] = Feedback(FeedbackType::Melee2HSwordRight, "Melee2HSwordRight_"); //1
		feedbackMap[FeedbackType::MeleeSwordLeft] = Feedback(FeedbackType::MeleeSwordLeft, "MeleeSwordLeft_"); //1
		feedbackMap[FeedbackType::MeleeAxeLeft] = Feedback(FeedbackType::MeleeAxeLeft, "MeleeAxeLeft_"); //1
		feedbackMap[FeedbackType::MeleeMaceLeft] = Feedback(FeedbackType::MeleeMaceLeft, "MeleeMaceLeft_"); //1
		feedbackMap[FeedbackType::MeleeDaggerLeft] = Feedback(FeedbackType::MeleeDaggerLeft, "MeleeDaggerLeft_"); //1
		feedbackMap[FeedbackType::Melee2HAxeLeft] = Feedback(FeedbackType::Melee2HAxeLeft, "Melee2HAxeLeft_"); //1
		feedbackMap[FeedbackType::Melee2HSwordLeft] = Feedback(FeedbackType::Melee2HSwordLeft, "Melee2HSwordLeft_"); //1
		feedbackMap[FeedbackType::MeleeFist] = Feedback(FeedbackType::MeleeFist, "MeleeFist_"); //1
		feedbackMap[FeedbackType::MeleeBash] = Feedback(FeedbackType::MeleeBash, "MeleeBash_"); //1
		feedbackMap[FeedbackType::MeleePowerAttack] = Feedback(FeedbackType::MeleePowerAttack, "MeleePowerAttack_"); //1
		feedbackMap[FeedbackType::Ranged] = Feedback(FeedbackType::Ranged, "Ranged_"); //1
		feedbackMap[FeedbackType::MagicFire] = Feedback(FeedbackType::MagicFire, "MagicFire_"); //1
		feedbackMap[FeedbackType::MagicShock] = Feedback(FeedbackType::MagicShock, "MagicShock_"); //1
		feedbackMap[FeedbackType::MagicIce] = Feedback(FeedbackType::MagicIce, "MagicIce_"); //1
		feedbackMap[FeedbackType::MagicAlteration] = Feedback(FeedbackType::MagicAlteration, "MagicAlteration_"); //1
		feedbackMap[FeedbackType::MagicIllusion] = Feedback(FeedbackType::MagicIllusion, "MagicIllusion_"); //1
		feedbackMap[FeedbackType::MagicRestoration] = Feedback(FeedbackType::MagicRestoration, "MagicRestoration_"); //1
		feedbackMap[FeedbackType::MagicPoison] = Feedback(FeedbackType::MagicPoison, "MagicPoison_"); //1
		feedbackMap[FeedbackType::MagicOther] = Feedback(FeedbackType::MagicOther, "MagicOther_"); //1
		feedbackMap[FeedbackType::MagicContinuousFire] = Feedback(FeedbackType::MagicContinuousFire, "MagicContinuousFire_"); //1
		feedbackMap[FeedbackType::MagicContinuousShock] = Feedback(FeedbackType::MagicContinuousShock, "MagicContinuousShock_"); //1
		feedbackMap[FeedbackType::MagicContinuousIce] = Feedback(FeedbackType::MagicContinuousIce, "MagicContinuousIce_"); //1
		feedbackMap[FeedbackType::MagicContinuousAlteration] = Feedback(FeedbackType::MagicContinuousAlteration, "MagicContinuousAlteration_"); //1
		feedbackMap[FeedbackType::MagicContinuousIllusion] = Feedback(FeedbackType::MagicContinuousIllusion, "MagicContinuousIllusion_"); //1
		feedbackMap[FeedbackType::MagicContinuousRestoration] = Feedback(FeedbackType::MagicContinuousRestoration, "MagicContinuousRestoration_"); //1
		feedbackMap[FeedbackType::MagicContinuousOther] = Feedback(FeedbackType::MagicContinuousOther, "MagicContinuousOther_"); //1
		feedbackMap[FeedbackType::MagicContinuousPoison] = Feedback(FeedbackType::MagicContinuousPoison, "MagicContinuousPoison_"); //1
		feedbackMap[FeedbackType::Shout] = Feedback(FeedbackType::Shout, "Shout_"); //1
		feedbackMap[FeedbackType::HeartBeat] = Feedback(FeedbackType::HeartBeat, "HeartBeat_"); //1
		feedbackMap[FeedbackType::HeartBeatFast] = Feedback(FeedbackType::HeartBeatFast, "HeartBeatFast_"); //1
		feedbackMap[FeedbackType::GreybeardPowerAbsorb] = Feedback(FeedbackType::GreybeardPowerAbsorb, "GreybeardPowerAbsorb_"); //1
		feedbackMap[FeedbackType::DragonSoul] = Feedback(FeedbackType::DragonSoul, "DragonSoul_"); //1
		feedbackMap[FeedbackType::WordWall] = Feedback(FeedbackType::WordWall, "WordWall_"); //1
		feedbackMap[FeedbackType::PlayerSpellFireLeft] = Feedback(FeedbackType::PlayerSpellFireLeft, "PlayerSpellFireLeft_"); //1
		feedbackMap[FeedbackType::PlayerSpellIceLeft] = Feedback(FeedbackType::PlayerSpellIceLeft, "PlayerSpellIceLeft_"); //1
		feedbackMap[FeedbackType::PlayerSpellShockLeft] = Feedback(FeedbackType::PlayerSpellShockLeft, "PlayerSpellShockLeft_"); //1
		feedbackMap[FeedbackType::PlayerSpellAlterationLeft] = Feedback(FeedbackType::PlayerSpellAlterationLeft, "PlayerSpellAlterationLeft_"); //1
		feedbackMap[FeedbackType::PlayerSpellIllusionLeft] = Feedback(FeedbackType::PlayerSpellIllusionLeft, "PlayerSpellIllusionLeft_"); //1
		feedbackMap[FeedbackType::PlayerSpellLightLeft] = Feedback(FeedbackType::PlayerSpellLightLeft, "PlayerSpellLightLeft_"); //1
		feedbackMap[FeedbackType::PlayerSpellRestorationLeft] = Feedback(FeedbackType::PlayerSpellRestorationLeft, "PlayerSpellRestorationLeft_"); //1
		feedbackMap[FeedbackType::PlayerSpellWardLeft] = Feedback(FeedbackType::PlayerSpellWardLeft, "PlayerSpellWardLeft_"); //1
		feedbackMap[FeedbackType::PlayerSpellConjurationLeft] = Feedback(FeedbackType::PlayerSpellConjurationLeft, "PlayerSpellConjurationLeft_"); //1
		feedbackMap[FeedbackType::PlayerSpellOtherLeft] = Feedback(FeedbackType::PlayerSpellOtherLeft, "PlayerSpellOtherLeft_"); //1
		feedbackMap[FeedbackType::PlayerSpellFireRight] = Feedback(FeedbackType::PlayerSpellFireRight, "PlayerSpellFireRight_"); //1
		feedbackMap[FeedbackType::PlayerSpellIceRight] = Feedback(FeedbackType::PlayerSpellIceRight, "PlayerSpellIceRight_"); //1
		feedbackMap[FeedbackType::PlayerSpellShockRight] = Feedback(FeedbackType::PlayerSpellShockRight, "PlayerSpellShockRight_"); //1
		feedbackMap[FeedbackType::PlayerSpellAlterationRight] = Feedback(FeedbackType::PlayerSpellAlterationRight, "PlayerSpellAlterationRight_"); //1
		feedbackMap[FeedbackType::PlayerSpellIllusionRight] = Feedback(FeedbackType::PlayerSpellIllusionRight, "PlayerSpellIllusionRight_"); //1
		feedbackMap[FeedbackType::PlayerSpellLightRight] = Feedback(FeedbackType::PlayerSpellLightRight, "PlayerSpellLightRight_"); //1
		feedbackMap[FeedbackType::PlayerSpellRestorationRight] = Feedback(FeedbackType::PlayerSpellRestorationRight, "PlayerSpellRestorationRight_"); //1
		feedbackMap[FeedbackType::PlayerSpellWardRight] = Feedback(FeedbackType::PlayerSpellWardRight, "PlayerSpellWardRight_"); //1
		feedbackMap[FeedbackType::PlayerSpellConjurationRight] = Feedback(FeedbackType::PlayerSpellConjurationRight, "PlayerSpellConjurationRight_"); //1
		feedbackMap[FeedbackType::PlayerSpellOtherRight] = Feedback(FeedbackType::PlayerSpellOtherRight, "PlayerSpellOtherRight_"); //1
		feedbackMap[FeedbackType::PlayerPowerAttackLeft] = Feedback(FeedbackType::PlayerPowerAttackLeft, "PlayerPowerAttackLeft_"); //1
		feedbackMap[FeedbackType::PlayerBashLeft] = Feedback(FeedbackType::PlayerBashLeft, "PlayerBashLeft_"); //1
		feedbackMap[FeedbackType::PlayerAttackLeft] = Feedback(FeedbackType::PlayerAttackLeft, "PlayerAttackLeft_"); //1
		feedbackMap[FeedbackType::PlayerPowerAttackRight] = Feedback(FeedbackType::PlayerPowerAttackRight, "PlayerPowerAttackRight_"); //1
		feedbackMap[FeedbackType::PlayerBashRight] = Feedback(FeedbackType::PlayerBashRight, "PlayerBashRight_"); //1
		feedbackMap[FeedbackType::PlayerAttackRight] = Feedback(FeedbackType::PlayerAttackRight, "PlayerAttackRight_"); //1
		feedbackMap[FeedbackType::PlayerBlockLeft] = Feedback(FeedbackType::PlayerBlockLeft, "PlayerBlockLeft_"); //1
		feedbackMap[FeedbackType::PlayerBlockRight] = Feedback(FeedbackType::PlayerBlockRight, "PlayerBlockRight_"); //1
		feedbackMap[FeedbackType::PlayerBowPullLeft] = Feedback(FeedbackType::PlayerBowPullLeft, "PlayerBowPullLeft_"); //1
		feedbackMap[FeedbackType::PlayerBowPullRight] = Feedback(FeedbackType::PlayerBowPullRight, "PlayerBowPullRight_"); //1
		feedbackMap[FeedbackType::PlayerBowHoldLeft] = Feedback(FeedbackType::PlayerBowHoldLeft, "PlayerBowHoldLeft_"); //1
		feedbackMap[FeedbackType::PlayerBowHoldRight] = Feedback(FeedbackType::PlayerBowHoldRight, "PlayerBowHoldRight_"); //1
		feedbackMap[FeedbackType::PlayerShout] = Feedback(FeedbackType::PlayerShout, "PlayerShout_"); //1
		feedbackMap[FeedbackType::Bite] = Feedback(FeedbackType::Bite, "Bite_"); //1
		feedbackMap[FeedbackType::HipHolsterStoreLeft] = Feedback(FeedbackType::HipHolsterStoreLeft, "HipHolsterStoreLeft_"); //1
		feedbackMap[FeedbackType::HipHolsterStoreRight] = Feedback(FeedbackType::HipHolsterStoreRight, "HipHolsterStoreRight_"); //1
		feedbackMap[FeedbackType::HipHolsterRemoveLeft] = Feedback(FeedbackType::HipHolsterRemoveLeft, "HipHolsterRemoveLeft_"); //1
		feedbackMap[FeedbackType::HipHolsterRemoveRight] = Feedback(FeedbackType::HipHolsterRemoveRight, "HipHolsterRemoveRight_"); //1
		feedbackMap[FeedbackType::SleeveHolsterStoreLeft] = Feedback(FeedbackType::SleeveHolsterStoreLeft, "SleeveHolsterStoreLeft_"); //1
		feedbackMap[FeedbackType::SleeveHolsterStoreRight] = Feedback(FeedbackType::SleeveHolsterStoreRight, "SleeveHolsterStoreRight_"); //1
		feedbackMap[FeedbackType::SleeveHolsterRemoveLeft] = Feedback(FeedbackType::SleeveHolsterRemoveLeft, "SleeveHolsterRemoveLeft_"); //1
		feedbackMap[FeedbackType::SleeveHolsterRemoveRight] = Feedback(FeedbackType::SleeveHolsterRemoveRight, "SleeveHolsterRemoveRight_"); //1
		feedbackMap[FeedbackType::BackpackStoreLeft] = Feedback(FeedbackType::BackpackStoreLeft, "BackpackStoreLeft_"); //1
		feedbackMap[FeedbackType::BackpackStoreRight] = Feedback(FeedbackType::BackpackStoreRight, "BackpackStoreRight_"); //1
		feedbackMap[FeedbackType::BackpackRemoveLeft] = Feedback(FeedbackType::BackpackRemoveLeft, "BackpackRemoveLeft_"); //1
		feedbackMap[FeedbackType::BackpackRemoveRight] = Feedback(FeedbackType::BackpackRemoveRight, "BackpackRemoveRight_"); //1
		feedbackMap[FeedbackType::StomachStore] = Feedback(FeedbackType::StomachStore, "StomachStore_"); //1
		feedbackMap[FeedbackType::StomachRemove] = Feedback(FeedbackType::StomachRemove, "StomachRemove_"); //1
		feedbackMap[FeedbackType::ChestStore] = Feedback(FeedbackType::ChestStore, "ChestStore_"); //1
		feedbackMap[FeedbackType::ChestRemove] = Feedback(FeedbackType::ChestRemove, "ChestRemove_"); //1
		feedbackMap[FeedbackType::ConsumableDrink] = Feedback(FeedbackType::ConsumableDrink, "ConsumableDrink_"); //1
		feedbackMap[FeedbackType::ConsumableFood] = Feedback(FeedbackType::ConsumableFood, "ConsumableFood_"); //1

		feedbackMap[FeedbackType::ShoutFire] = Feedback(FeedbackType::ShoutFire, "ShoutFire_");
		feedbackMap[FeedbackType::ShoutFrost] = Feedback(FeedbackType::ShoutFrost, "ShoutFrost_");
		feedbackMap[FeedbackType::ShoutSteam] = Feedback(FeedbackType::ShoutSteam, "ShoutSteam_");
		feedbackMap[FeedbackType::ShoutLightning] = Feedback(FeedbackType::ShoutLightning, "ShoutLightning_");
		feedbackMap[FeedbackType::HiggsPullLeft] = Feedback(FeedbackType::HiggsPullLeft, "HiggsPullLeft_");
		feedbackMap[FeedbackType::HiggsPullRight] = Feedback(FeedbackType::HiggsPullRight, "HiggsPullRight_");
		feedbackMap[FeedbackType::PlayerEnvironmentHitLeft] = Feedback(FeedbackType::PlayerEnvironmentHitLeft, "PlayerEnvironmentHitLeft_");
		feedbackMap[FeedbackType::PlayerEnvironmentHitRight] = Feedback(FeedbackType::PlayerEnvironmentHitRight, "PlayerEnvironmentHitRight_");
		feedbackMap[FeedbackType::PlayerThrowLeft] = Feedback(FeedbackType::PlayerThrowLeft, "PlayerThrowLeft_");
		feedbackMap[FeedbackType::PlayerThrowRight] = Feedback(FeedbackType::PlayerThrowRight, "PlayerThrowRight_");
		feedbackMap[FeedbackType::PlayerCatchLeft] = Feedback(FeedbackType::PlayerCatchLeft, "PlayerCatchLeft_");
		feedbackMap[FeedbackType::PlayerCatchRight] = Feedback(FeedbackType::PlayerCatchRight, "PlayerCatchRight_");

		feedbackMap[FeedbackType::PlayerShoutBindHands] = Feedback(FeedbackType::PlayerShoutBindHands, "PlayerShoutBindHands_"); //1
		feedbackMap[FeedbackType::PlayerShoutBindVest] = Feedback(FeedbackType::PlayerShoutBindVest, "PlayerShoutBindVest_"); //1
		
		feedbackMap[FeedbackType::TravelEffect] = Feedback(FeedbackType::TravelEffect, "TravelEffect_");
		feedbackMap[FeedbackType::Teleport] = Feedback(FeedbackType::Teleport, "Teleport_");

		feedbackMap[FeedbackType::EnvironmentRumble] = Feedback(FeedbackType::EnvironmentRumble, "EnvironmentRumble_");
		feedbackMap[FeedbackType::DragonLanding] = Feedback(FeedbackType::DragonLanding, "DragonLanding_");

		feedbackMap[FeedbackType::EquipHelmet] = Feedback(FeedbackType::EquipHelmet, "EquipHelmet_");
		feedbackMap[FeedbackType::EquipCuirass] = Feedback(FeedbackType::EquipCuirass, "EquipCuirass_");
		feedbackMap[FeedbackType::EquipGauntlets] = Feedback(FeedbackType::EquipGauntlets, "EquipGauntlets_");
		feedbackMap[FeedbackType::EquipClothing] = Feedback(FeedbackType::EquipClothing, "EquipClothing_");
		feedbackMap[FeedbackType::EquipHood] = Feedback(FeedbackType::EquipHood, "EquipHood_");
		
		feedbackMap[FeedbackType::UnequipHelmet] = Feedback(FeedbackType::UnequipHelmet, "UnequipHelmet_");
		feedbackMap[FeedbackType::UnequipCuirass] = Feedback(FeedbackType::UnequipCuirass, "UnequipCuirass_");
		feedbackMap[FeedbackType::UnequipGauntlets] = Feedback(FeedbackType::UnequipGauntlets, "UnequipGauntlets_");
		feedbackMap[FeedbackType::UnequipClothing] = Feedback(FeedbackType::UnequipClothing, "UnequipClothing_");
		feedbackMap[FeedbackType::UnequipHood] = Feedback(FeedbackType::UnequipHood, "UnequipHood_");
		
		feedbackMap[FeedbackType::Learned] = Feedback(FeedbackType::Learned, "Learned_");

		feedbackMap[FeedbackType::UnholsterArrowLeftShoulder] = Feedback(FeedbackType::UnholsterArrowLeftShoulder, "UnholsterArrowLeftShoulder_");
		feedbackMap[FeedbackType::UnholsterArrowRightShoulder] = Feedback(FeedbackType::UnholsterArrowRightShoulder, "UnholsterArrowRightShoulder_");


		feedbackMap[FeedbackType::Explosion] = Feedback(FeedbackType::Explosion, "Explosion_");
		feedbackMap[FeedbackType::EnvironmentalPoison] = Feedback(FeedbackType::EnvironmentalPoison, "EnvironmentalPoison_");
		feedbackMap[FeedbackType::EnvironmentalFire] = Feedback(FeedbackType::EnvironmentalFire, "EnvironmentalFire_");
		feedbackMap[FeedbackType::EnvironmentalElectric] = Feedback(FeedbackType::EnvironmentalElectric, "EnvironmentalElectric_");
		feedbackMap[FeedbackType::EnvironmentalFrost] = Feedback(FeedbackType::EnvironmentalFrost, "EnvironmentalFrost_");
		feedbackMap[FeedbackType::EnvironmentalFireCloak] = Feedback(FeedbackType::EnvironmentalFireCloak, "EnvironmentalFireCloak_");
		feedbackMap[FeedbackType::EnvironmentalElectricCloak] = Feedback(FeedbackType::EnvironmentalElectricCloak, "EnvironmentalElectricCloak_");
		feedbackMap[FeedbackType::EnvironmentalFrostCloak] = Feedback(FeedbackType::EnvironmentalFrostCloak, "EnvironmentalFrostCloak_");
		
		feedbackMap[FeedbackType::UnarmedDefault] = Feedback(FeedbackType::UnarmedDefault, "UnarmedDefault_");
		feedbackMap[FeedbackType::UnarmedDragon] = Feedback(FeedbackType::UnarmedDragon, "UnarmedDragon_");
		feedbackMap[FeedbackType::UnarmedFrostbiteSpider] = Feedback(FeedbackType::UnarmedFrostbiteSpider, "UnarmedFrostbiteSpider_");
		feedbackMap[FeedbackType::UnarmedSabreCat] = Feedback(FeedbackType::UnarmedSabreCat, "UnarmedSabreCat_");
		feedbackMap[FeedbackType::UnarmedSkeever] = Feedback(FeedbackType::UnarmedSkeever, "UnarmedSkeever_");
		feedbackMap[FeedbackType::UnarmedSlaughterfish] = Feedback(FeedbackType::UnarmedSlaughterfish, "UnarmedSlaughterfish_");
		feedbackMap[FeedbackType::UnarmedWisp] = Feedback(FeedbackType::UnarmedWisp, "UnarmedWisp_");
		feedbackMap[FeedbackType::UnarmedDragonPriest] = Feedback(FeedbackType::UnarmedDragonPriest, "UnarmedDragonPriest_");
		feedbackMap[FeedbackType::UnarmedDraugr] = Feedback(FeedbackType::UnarmedDraugr, "UnarmedDraugr_");
		feedbackMap[FeedbackType::UnarmedWolf] = Feedback(FeedbackType::UnarmedWolf, "UnarmedWolf_");
		feedbackMap[FeedbackType::UnarmedGiant] = Feedback(FeedbackType::UnarmedGiant, "UnarmedGiant_");
		feedbackMap[FeedbackType::UnarmedIceWraith] = Feedback(FeedbackType::UnarmedIceWraith, "UnarmedIceWraith_");
		feedbackMap[FeedbackType::UnarmedChaurus] = Feedback(FeedbackType::UnarmedChaurus, "UnarmedChaurus_");
		feedbackMap[FeedbackType::UnarmedMammoth] = Feedback(FeedbackType::UnarmedMammoth, "UnarmedMammoth_");
		feedbackMap[FeedbackType::UnarmedFrostAtronach] = Feedback(FeedbackType::UnarmedFrostAtronach, "UnarmedFrostAtronach_");
		feedbackMap[FeedbackType::UnarmedFalmer] = Feedback(FeedbackType::UnarmedFalmer, "UnarmedFalmer_");
		feedbackMap[FeedbackType::UnarmedHorse] = Feedback(FeedbackType::UnarmedHorse, "UnarmedHorse_");
		feedbackMap[FeedbackType::UnarmedStormAtronach] = Feedback(FeedbackType::UnarmedStormAtronach, "UnarmedStormAtronach_");
		feedbackMap[FeedbackType::UnarmedElk] = Feedback(FeedbackType::UnarmedElk, "UnarmedElk_");
		feedbackMap[FeedbackType::UnarmedDwarvenSphere] = Feedback(FeedbackType::UnarmedDwarvenSphere, "UnarmedDwarvenSphere_");
		feedbackMap[FeedbackType::UnarmedDwarvenSteam] = Feedback(FeedbackType::UnarmedDwarvenSteam, "UnarmedDwarvenSteam_");
		feedbackMap[FeedbackType::UnarmedDwarvenSpider] = Feedback(FeedbackType::UnarmedDwarvenSpider, "UnarmedDwarvenSpider_");
		feedbackMap[FeedbackType::UnarmedBear] = Feedback(FeedbackType::UnarmedBear, "UnarmedBear_");
		feedbackMap[FeedbackType::UnarmedFlameAtronach] = Feedback(FeedbackType::UnarmedFlameAtronach, "UnarmedFlameAtronach_");
		feedbackMap[FeedbackType::UnarmedWitchlight] = Feedback(FeedbackType::UnarmedWitchlight, "UnarmedWitchlight_");
		feedbackMap[FeedbackType::UnarmedHorker] = Feedback(FeedbackType::UnarmedHorker, "UnarmedHorker_");
		feedbackMap[FeedbackType::UnarmedTroll] = Feedback(FeedbackType::UnarmedTroll, "UnarmedTroll_");
		feedbackMap[FeedbackType::UnarmedHagraven] = Feedback(FeedbackType::UnarmedHagraven, "UnarmedHagraven_");
		feedbackMap[FeedbackType::UnarmedSpriggan] = Feedback(FeedbackType::UnarmedSpriggan, "UnarmedSpriggan_");
		feedbackMap[FeedbackType::UnarmedMudcrab] = Feedback(FeedbackType::UnarmedMudcrab, "UnarmedMudcrab_");
		feedbackMap[FeedbackType::UnarmedWerewolf] = Feedback(FeedbackType::UnarmedWerewolf, "UnarmedWerewolf_");
		feedbackMap[FeedbackType::UnarmedChaurusFlyer] = Feedback(FeedbackType::UnarmedChaurusFlyer, "UnarmedChaurusFlyer_");
		feedbackMap[FeedbackType::UnarmedGargoyle] = Feedback(FeedbackType::UnarmedGargoyle, "UnarmedGargoyle_");
		feedbackMap[FeedbackType::UnarmedRiekling] = Feedback(FeedbackType::UnarmedRiekling, "UnarmedRiekling_");
		feedbackMap[FeedbackType::UnarmedScrib] = Feedback(FeedbackType::UnarmedScrib, "UnarmedScrib_");
		feedbackMap[FeedbackType::UnarmedSeeker] = Feedback(FeedbackType::UnarmedSeeker, "UnarmedSeeker_");
		feedbackMap[FeedbackType::UnarmedMountedRiekling] = Feedback(FeedbackType::UnarmedMountedRiekling, "UnarmedMountedRiekling_");
		feedbackMap[FeedbackType::UnarmedNetch] = Feedback(FeedbackType::UnarmedNetch, "UnarmedNetch_");
		feedbackMap[FeedbackType::UnarmedBenthicLurker] = Feedback(FeedbackType::UnarmedBenthicLurker, "UnarmedBenthicLurker_");

		feedbackMap[FeedbackType::DefaultHead] = Feedback(FeedbackType::DefaultHead, "DefaultHead_");
		feedbackMap[FeedbackType::UnarmedHead] = Feedback(FeedbackType::UnarmedHead, "UnarmedHead_");
		feedbackMap[FeedbackType::RangedHead] = Feedback(FeedbackType::RangedHead, "RangedHead_");
		feedbackMap[FeedbackType::MeleeHead] = Feedback(FeedbackType::MeleeHead, "MeleeHead_");

		feedbackMap[FeedbackType::MagicHeadFire] = Feedback(FeedbackType::MagicHeadFire, "MagicHeadFire_");
		feedbackMap[FeedbackType::MagicHeadShock] = Feedback(FeedbackType::MagicHeadShock, "MagicHeadShock_");
		feedbackMap[FeedbackType::MagicHeadIce] = Feedback(FeedbackType::MagicHeadIce, "MagicHeadIce_");
		feedbackMap[FeedbackType::MagicHeadAlteration] = Feedback(FeedbackType::MagicHeadAlteration, "MagicHeadAlteration_");
		feedbackMap[FeedbackType::MagicHeadIllusion] = Feedback(FeedbackType::MagicHeadIllusion, "MagicHeadIllusion_");
		feedbackMap[FeedbackType::MagicHeadRestoration] = Feedback(FeedbackType::MagicHeadRestoration, "MagicHeadRestoration_");

		feedbackMap[FeedbackType::RangedLeftArm] = Feedback(FeedbackType::RangedLeftArm, "RangedLeftArm_");
		feedbackMap[FeedbackType::RangedRightArm] = Feedback(FeedbackType::RangedRightArm, "RangedRightArm_");
		feedbackMap[FeedbackType::MagicLeftArmFire] = Feedback(FeedbackType::MagicLeftArmFire, "MagicLeftArmFire_");
		feedbackMap[FeedbackType::MagicLeftArmShock] = Feedback(FeedbackType::MagicLeftArmShock, "MagicLeftArmShock_");
		feedbackMap[FeedbackType::MagicLeftArmIce] = Feedback(FeedbackType::MagicLeftArmIce, "MagicLeftArmIce_");
		feedbackMap[FeedbackType::MagicLeftArmAlteration] = Feedback(FeedbackType::MagicLeftArmAlteration, "MagicLeftArmAlteration_");
		feedbackMap[FeedbackType::MagicLeftArmIllusion] = Feedback(FeedbackType::MagicLeftArmIllusion, "MagicLeftArmIllusion_");
		feedbackMap[FeedbackType::MagicLeftArmRestoration] = Feedback(FeedbackType::MagicLeftArmRestoration, "MagicLeftArmRestoration_");
		feedbackMap[FeedbackType::MagicRightArmFire] = Feedback(FeedbackType::MagicRightArmFire, "MagicRightArmFire_");
		feedbackMap[FeedbackType::MagicRightArmShock] = Feedback(FeedbackType::MagicRightArmShock, "MagicRightArmShock_");
		feedbackMap[FeedbackType::MagicRightArmIce] = Feedback(FeedbackType::MagicRightArmIce, "MagicRightArmIce_");
		feedbackMap[FeedbackType::MagicRightArmAlteration] = Feedback(FeedbackType::MagicRightArmAlteration, "MagicRightArmAlteration_");
		feedbackMap[FeedbackType::MagicRightArmIllusion] = Feedback(FeedbackType::MagicRightArmIllusion, "MagicRightArmIllusion_");
		feedbackMap[FeedbackType::MagicRightArmRestoration] = Feedback(FeedbackType::MagicRightArmRestoration, "MagicRightArmRestoration_");

		feedbackMap[FeedbackType::HorseRiding] = Feedback(FeedbackType::HorseRiding, "HorseRiding_");
		feedbackMap[FeedbackType::HorseRidingSlow] = Feedback(FeedbackType::HorseRidingSlow, "HorseRidingSlow_");

		feedbackMap[FeedbackType::TentacleAttack] = Feedback(FeedbackType::TentacleAttack, "TentacleAttack_");


		feedbackMap[FeedbackType::GiantStomp] = Feedback(FeedbackType::GiantStomp, "GiantStomp_");
		feedbackMap[FeedbackType::GiantClubLeft] = Feedback(FeedbackType::GiantClubLeft, "GiantClubLeft_");
		feedbackMap[FeedbackType::GiantClubRight] = Feedback(FeedbackType::GiantClubRight, "GiantClubRight_");

		feedbackMap[FeedbackType::FallEffect] = Feedback(FeedbackType::FallEffect, "FallEffect_");

		feedbackMap[FeedbackType::SwimVest20] = Feedback(FeedbackType::SwimVest20, "SwimVest20_");
		feedbackMap[FeedbackType::SwimVest40] = Feedback(FeedbackType::SwimVest40, "SwimVest40_");
		feedbackMap[FeedbackType::SwimVest60] = Feedback(FeedbackType::SwimVest60, "SwimVest60_");
		feedbackMap[FeedbackType::SwimVest80] = Feedback(FeedbackType::SwimVest80, "SwimVest80_");
		feedbackMap[FeedbackType::SwimVest100] = Feedback(FeedbackType::SwimVest100, "SwimVest100_");

		feedbackMap[FeedbackType::DrowningEffectVest] = Feedback(FeedbackType::DrowningEffectVest, "DrowningEffectVest_");
		feedbackMap[FeedbackType::DrowningEffectHead] = Feedback(FeedbackType::DrowningEffectHead, "DrowningEffectHead_");

		feedbackMap[FeedbackType::PlayerCrossbowFireLeft] = Feedback(FeedbackType::PlayerCrossbowFireLeft, "PlayerCrossbowFireLeft_");
		feedbackMap[FeedbackType::PlayerCrossbowFireRight] = Feedback(FeedbackType::PlayerCrossbowFireRight, "PlayerCrossbowFireRight_");
		feedbackMap[FeedbackType::PlayerCrossbowKickbackLeft] = Feedback(FeedbackType::PlayerCrossbowKickbackLeft, "PlayerCrossbowKickbackLeft_");
		feedbackMap[FeedbackType::PlayerCrossbowKickbackRight] = Feedback(FeedbackType::PlayerCrossbowKickbackRight, "PlayerCrossbowKickbackRight_");

		feedbackMap[FeedbackType::Wind] = Feedback(FeedbackType::Wind, "Wind_");

		feedbackMap[FeedbackType::MagicArmorSpell] = Feedback(FeedbackType::MagicArmorSpell, "MagicArmorSpell_");


		feedbackMap[FeedbackType::Default] = Feedback(FeedbackType::Default, "Default_"); //1		
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

			/*	if (!found)
				{
					LOG("File category unknown: %s", filename.c_str());
					skipTactExtension(filename);
					TactFileRegisterFilename(filename);
				}*/
			}
		}
	}

	owoVector<owoString> ReadSensationFiles()
	{
		owoVector<owoString> ret = owoVector<owoString>();

		std::string	runtimeDirectory = GetRuntimeDirectory();
		if (!runtimeDirectory.empty())
		{
			std::string configPath = runtimeDirectory + "Data\\SKSE\\Plugins\\OWO";

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
						ret.emplace_back(TactFileRegister(configPath, filename, element.second)->ToString());
						found = true;
						break;
					}
				}

				// if (!found)
				// {
				// 	LOG("File category unknown: %s", filename.c_str());
				// 	skipTactExtension(filename);
				// 	TactFileRegisterFilename(filename);
				// }
			}
		}

		return ret;
	}

	// void RegisterAllTactFiles()
	// {

	// 	string configPath = Directory.GetCurrentDirectory() + "\\Mods\\OWO";
	// 	DirectoryInfo d = new DirectoryInfo(configPath);
	// 	FileInfo[] Files = d.GetFiles("*.owo", SearchOption.AllDirectories);
	// 	for (int i = 0; i < Files.Length; i++)
	// 	{
	// 		string filename = Files[i].Name;
	// 		string fullName = Files[i].FullName;
	// 		string prefix = Path.GetFileNameWithoutExtension(filename);
	// 		// LOG("Trying to register: " + prefix + " " + fullName);
	// 		if (filename == "." || filename == "..")
	// 			continue;
	// 		string tactFileStr = File.ReadAllText(fullName);
	// 		try
	// 		{
	// 			Sensation test = Sensation.Parse(tactFileStr);
	// 			FeedbackMap.Add(prefix, test);
	// 		}
	// 		catch (Exception e) { LOG(e.ToString()); }

	// 	}

	// 	systemInitialized = true;
	// }
	
	void PauseHapticFeedBack(FeedbackType effect)
	{
		owo->Stop();
	}	

	void PauseHapticFeedBack()
	{
		owo->Stop();
	}

	void PauseHapticFeedBackSpellCastingRight()
	{
		owo->Stop();
		// for each(FeedbackType effectType in spellCastingRight)
		// {
		// 	for (int i = 1; i <= feedbackMap[effectType].feedbackFileCount; i++)
		// 	{
		// 		TurnOffKey((feedbackMap[effectType].prefix + std::to_string(i)).c_str());
		// 	}
		// }

	}

	void PauseHapticFeedBackSpellCastingLeft()
	{
		owo->Stop();
		// for each(FeedbackType effectType in spellCastingLeft)
		// {
		// 	for (int i = 1; i <= feedbackMap[effectType].feedbackFileCount; i++)
		// 	{
		// 		TurnOffKey((feedbackMap[effectType].prefix + std::to_string(i)).c_str());
		// 	}
		// }
	}

	bool isPlayingHapticFeedBackAttackRight()
	{
		// for each (FeedbackType effectType in playerAttackingRight)
		// {
		// 	for (int i = 1; i <= feedbackMap[effectType].feedbackFileCount; i++)
		// 	{
		// 		std::string key = feedbackMap[effectType].prefix + std::to_string(i);
		// 		if (IsPlayingKey(key.c_str()))
		// 		{
		// 			return true;
		// 		}
		// 	}
		// }
		return false;
	}

	bool isPlayingHapticFeedBackAttackLeft()
	{
		// for each (FeedbackType effectType in playerAttackingLeft)
		// {
		// 	for (int i = 1; i <= feedbackMap[effectType].feedbackFileCount; i++)
		// 	{
		// 		std::string key = feedbackMap[effectType].prefix + std::to_string(i);
		// 		if (IsPlayingKey(key.c_str()))
		// 		{
		// 			return true;
		// 		}
		// 	}
		// }
		return false;
	}

	void ProvideDotFeedback(OWOGame::Muscle muscle, int index, int intensity, int durationMillis)
	{
		// if (intensity < TOLERANCE)
		// 	return;

		// if (isGameStoppedNoDialogue())
		// {
		// 	return;
		// }

		// if (!systemInitialized)
		// 	CreateSystem();

		// std::string key;

		// std::vector<bhaptics::DotPoint> points;
		// bhaptics::DotPoint point = bhaptics::DotPoint(index, intensity);
		// points.emplace_back(point);
		// SubmitDot(key.c_str(), position, points, durationMillis);

		// TODO implement DOT
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
		// std::string prefix = feedbackMap[effect].prefix;
		// int feedbackFileCount = feedbackMap[effect].feedbackFileCount;
		// for (int i = 1; i <= feedbackFileCount; i++)
		// {
		// 	std::string key = prefix + std::to_string(i);
		// 	if (IsPlayingKey(key.c_str()))
		// 	{
		// 		return true;
		// 	}
		// }
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
			if (feedbackMap[effect].feedbackSensations.size() > 0)
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

				// const std::string key = feedbackMap[effect].prefix;

				std::vector<std::shared_ptr<OWOGame::BakedSensation>> feedbackSensations = feedbackMap[effect].feedbackSensations;
				
				std::shared_ptr<OWOGame::BakedSensation> sensation = feedbackSensations[(randi(0, feedbackSensations.size()-1))];
				// OWOGame::Sensation sensation = feedbackMap[effect].feedbackSensations[(randi(0, feedbackMap[effect].feedbackSensations.size()-1))];

				// bhaptics::RotationOption RotOption;
				// RotOption.OffsetAngleX = locationAngle;
				// RotOption.OffsetY = locationHeight;

				// bhaptics::ScaleOption scaleOption;
				// scaleOption.Duration = 1.0f;
				// scaleOption.Intensity = (intensityMultiplier != 1.0f) ? intensityMultiplier : 1.0f;

				// LOG("Key: %s  OffsetY: %g  OffsetAngleX: %g  Intensity: %g", key.c_str(), locationHeight, locationAngle, scaleOption.Intensity);

				// SubmitRegisteredAlt(key.c_str(), key.c_str(), scaleOption, RotOption);
				owo->Send(sensation->Clone());

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

		if (waitToPlay)
		{
			// if (IsPlayingKey(feedbackFileName.c_str()))
			// 	return;
		}

		if (locationHeight < -0.5f)
			locationHeight = -0.5f;
		else if (locationHeight > 0.5f)
			locationHeight = 0.5f;

		LOG("Key: %s  OffsetY: %g  OffsetAngleX: %g", feedbackFileName.c_str(), locationHeight, locationAngle);

		// bhaptics::RotationOption RotOption;
		// RotOption.OffsetAngleX = locationAngle;
		// RotOption.OffsetY = locationHeight;

		// bhaptics::ScaleOption scaleOption;
		// scaleOption.Duration = 1.0f;
		// scaleOption.Intensity = (intensityMultiplier != 1.0f) ? intensityMultiplier : 1.0f;
		
		// SubmitRegisteredAlt(feedbackFileName.c_str(), feedbackFileName.c_str(), scaleOption, RotOption);
		// owo->Send()

		// LOG("ProvideHapticFeedback submit successful");
		LOG("ProvideHapticFeedback not implemented");
	}
}
