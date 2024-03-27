#include "TactsuitVR.h"

#include "Domain/BakedSensationsParser.h"
#include "Domain/GameAuth.h"
#include "Controller/OWO.h"
#include "Infrastructure/UDPNetwork.h"

using namespace OWOGame;

namespace TactsuitVR {
	bool systemInitialized = false;
	bool owoLoopUpdating = false;
	int owoUpdateWaitMs = 100;

	float TOLERANCE = 0.00001f;

	unsigned long initialConnectiontimeoutMs = 60 * 1000;

	sharedPtr<OWO> owo;
	std::clock_t startTime;

	std::unordered_map<FeedbackType, Feedback> feedbackMap;

	std::vector<FeedbackType> spellCastingRight = { FeedbackType::PlayerSpellAlterationRight, FeedbackType::PlayerSpellConjurationRight, FeedbackType::PlayerSpellFireRight, FeedbackType::PlayerSpellIceRight, FeedbackType::PlayerSpellIllusionRight, FeedbackType::PlayerSpellLightRight, FeedbackType::PlayerSpellOtherRight, FeedbackType::PlayerSpellRestorationRight, FeedbackType::PlayerSpellShockRight, FeedbackType::PlayerSpellWardRight };
	std::vector<FeedbackType> spellCastingLeft = { FeedbackType::PlayerSpellAlterationLeft, FeedbackType::PlayerSpellConjurationLeft, FeedbackType::PlayerSpellFireLeft, FeedbackType::PlayerSpellIceLeft, FeedbackType::PlayerSpellIllusionLeft, FeedbackType::PlayerSpellLightLeft, FeedbackType::PlayerSpellOtherLeft, FeedbackType::PlayerSpellRestorationLeft, FeedbackType::PlayerSpellShockLeft, FeedbackType::PlayerSpellWardLeft };

	std::vector<FeedbackType> playerAttackingLeft = { FeedbackType::PlayerAttackLeft, FeedbackType::PlayerPowerAttackLeft, FeedbackType::PlayerBashLeft, FeedbackType::PlayerBlockLeft, FeedbackType::PlayerBowHoldLeft, FeedbackType::PlayerBowPullLeft };
	std::vector<FeedbackType> playerAttackingRight = { FeedbackType::PlayerAttackRight, FeedbackType::PlayerPowerAttackRight, FeedbackType::PlayerBashRight, FeedbackType::PlayerBlockRight, FeedbackType::PlayerBowHoldRight, FeedbackType::PlayerBowPullRight };

	std::string ToString(ConnectionState cs) {
		switch (cs)
		{
			case ConnectionState::Connected:   return "Connected";
			case ConnectionState::Disconnected:   return "Disconnected";
			case ConnectionState::Connecting: return "Connecting";
			default:      return "[Unknown]";
		}
	}

	void CreateSystem()
	{
		if (!systemInitialized)
		{
			owoVector<owoString> bakedSensations = ReadSensationFiles();
			// TODO: Add gameId when approved
			auto auth = GameAuth::Create(bakedSensations);

			startTime = std::clock();
			owo = OWO::Create<UDPNetwork>();
			owo->Configure(auth);
			ConnectionState connectionState = owo->AutoConnect();

			owoLoopUpdating = true;
			std::thread owoUpdateThread(owoUpdateLoop);
			owoUpdateThread.detach();

			LOG("OWO System Initialized 2");
		}
		systemInitialized = true;
	}

	std::clock_t getTimeSinceStart() {
		return std::clock() - startTime;
	}

	void owoUpdateLoop() {
		ConnectionState prevConnectionState = ConnectionState::Disconnected;
		ConnectionState curConnectionState;

		while (owoLoopUpdating) {
			curConnectionState = owo->UpdateStatus(getTimeSinceStart());
			if (prevConnectionState != curConnectionState) {
				prevConnectionState = curConnectionState;
				LOG("OWO connection state changed from %s to %s", ToString(prevConnectionState).c_str(), ToString(curConnectionState).c_str());
			}
			WaitFor(owoUpdateWaitMs);
		}
	}

	sharedPtr<BakedSensation> TactFileRegister(std::string &configPath, std::string &filename, Feedback feedback)
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

			sharedPtr<BakedSensation> sensation = std::move(BakedSensationsParser::Parse(tactFileStr));
			feedbackMap[feedback.feedbackType].feedbackSensations.push_back(sensation);

			_MESSAGE("%f total duration!", sensation->TotalDuration());

			return sensation;
			// RegisterFeedbackFromTactFile(tactKey.c_str(), tactFileStr.c_str());
		}

		return NULL;
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
	// 
	// 		LoadAndRegisterFeedback(filename.c_str(), path.c_str());
	// 		return true;
	// 	}
	// 	else
	// 	{
	// 		return false;
	// 	}
	// }


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
						sharedPtr<BakedSensation> sensation(TactFileRegister(configPath, filename, element.second));
						if (sensation == NULL) {
							LOG("Unknown sensation %s. Skipping.", filename.c_str());
							break;
						}

						ret.push_back(sensation->ToString());
						LOG("Successfully added %s to %s", filename.c_str(), element.second.prefix.c_str());

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

	void PauseHapticFeedBack(FeedbackType effect)
	{
		_MESSAGE("PauseHapticFeedBack with arg");
		owo->Stop();
	}	

	void PauseHapticFeedBack()
	{
		_MESSAGE("PauseHapticFeedBack");
		owo->Stop();
	}

	void PauseHapticFeedBackSpellCastingRight()
	{
		_MESSAGE("PauseHapticFeedBackSpellCastingRight");
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
		_MESSAGE("PauseHapticFeedBackSpellCastingLeft");
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
		_MESSAGE("isPlayingHapticFeedBackAttackRight");
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
		_MESSAGE("isPlayingHapticFeedBackAttackLeft");
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

	void ProvideDotFeedback(Muscle muscle, int index, int intensity, int durationMillis)
	{
		_MESSAGE("ProvideDotFeedback");
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
		_MESSAGE("ProvideHapticFeedback %s %f %f", feedbackTypeToString(effect).c_str(), locationAngle, locationHeight);
		if (intensityMultiplier > TOLERANCE)
		{
			std::thread t0(ProvideHapticFeedbackThread, locationAngle, locationHeight, effect, intensityMultiplier, waitToPlay, playInMenu);
			t0.detach();
		}
	}

	bool IsPlayingKeyAll(FeedbackType effect)
	{
		_MESSAGE("IsPlayingKeyAll %s", feedbackTypeToString(effect).c_str());
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
		_MESSAGE("LateFeedback %s %f", feedbackTypeToString(feedback).c_str(), locationAngle);
		for (int i = 0; i < count; i++)
		{
			Sleep(sleepAmount);
			ProvideHapticFeedback(locationAngle, 0, feedback, intensity, waitToPlay, playInMenu);
		}
	}

	void ProvideHapticFeedbackThread(float locationAngle, float locationHeight, FeedbackType effect, float intensityMultiplier, bool waitToPlay, bool playInMenu)
	{
		_MESSAGE("ProvideHapticFeedbackThread %s %f %f", feedbackTypeToString(effect).c_str(), locationAngle, locationHeight);
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

				std::vector<std::shared_ptr<BakedSensation>> & feedbackSensations = feedbackMap[effect].feedbackSensations;
				
				std::shared_ptr<BakedSensation> sensation(feedbackSensations[(randi(0, feedbackSensations.size()-1))]);

				_MESSAGE("%d sensations", feedbackSensations.size());
				_MESSAGE("%s stringify", sensation->Stringify().c_str());
				_MESSAGE("%f total duration", sensation->TotalDuration());
				// Sensation sensation = feedbackMap[effect].feedbackSensations[(randi(0, feedbackMap[effect].feedbackSensations.size()-1))];

				// bhaptics::RotationOption RotOption;
				// RotOption.OffsetAngleX = locationAngle;
				// RotOption.OffsetY = locationHeight;

				// bhaptics::ScaleOption scaleOption;
				// scaleOption.Duration = 1.0f;
				// scaleOption.Intensity = (intensityMultiplier != 1.0f) ? intensityMultiplier : 1.0f;

				// _MESSAGE("Key: %s  OffsetY: %g  OffsetAngleX: %g  Intensity: %g", key.c_str(), locationHeight, locationAngle, scaleOption.Intensity);


				owo->Send(BakedSensationsParser::Parse(sensation->Stringify()));
			}
			else
			{
				_MESSAGE("No file found for %s", feedbackMap[effect].prefix.c_str());
			}
		}
	}

	void ProvideHapticFeedbackSpecificFile(float locationAngle, float locationHeight, std::string feedbackFileName, float intensityMultiplier, bool waitToPlay)
	{

		_MESSAGE("These dont work rn %s", feedbackFileName);
		_MESSAGE("WTF is happening here, bad external file %s", feedbackFileName.c_str());

		return;

		try {
			//if (feedbackFileName.empty() || feedbackFileName.c_str() == NULL) {

				_MESSAGE("WTF is happening here, bad external file %s", feedbackFileName);
				_MESSAGE("WTF is happening here, bad external file %s", feedbackFileName.c_str());
			//}


			_MESSAGE("ProvideHapticFeedbackSpecificFile %s %f %f %f", feedbackFileName.c_str(), locationAngle, locationHeight, intensityMultiplier);

			if (intensityMultiplier > TOLERANCE)
			{
				std::thread t0(ProvideHapticFeedbackThreadSpecificFile, locationAngle, locationHeight, feedbackFileName, intensityMultiplier, waitToPlay);
				t0.detach();
			}
		}
		catch (...) {
			_MESSAGE("Caught");
		}
	}

	void ProvideHapticFeedbackThreadSpecificFile(float locationAngle, float locationHeight, std::string feedbackFileName, float intensityMultiplier, bool waitToPlay)
	{
		_MESSAGE("ProvideHapticFeedbackThreadSpecificFile %s %f %f", feedbackFileName.c_str(), locationAngle, locationHeight);
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

		_MESSAGE("Key: %s  OffsetY: %g  OffsetAngleX: %g", feedbackFileName, locationHeight, locationAngle);

		// bhaptics::RotationOption RotOption;
		// RotOption.OffsetAngleX = locationAngle;
		// RotOption.OffsetY = locationHeight;

		// bhaptics::ScaleOption scaleOption;
		// scaleOption.Duration = 1.0f;
		// scaleOption.Intensity = (intensityMultiplier != 1.0f) ? intensityMultiplier : 1.0f;
		
		// SubmitRegisteredAlt(feedbackFileName.c_str(), feedbackFileName.c_str(), scaleOption, RotOption);
		// owo->Send()

		// _MESSAGE("ProvideHapticFeedback submit successful");
		_MESSAGE("ProvideHapticFeedback not implemented");
	}

	// Thank you florianfahrenberger for this function
	Muscle angleToMuscle(float locationAngle, float locationHeight) {
		Muscle myMuscle = Muscle::Pectoral_R();
		// two parameters can be given to the pattern to move it on the vest:
		// 1. An angle in degrees [0, 360] to turn the pattern to the left
		// 2. A shift [-0.5, 0.5] in y-direction (up and down) to move it up or down
		if ((locationAngle < 90.0))
		{
			if (locationHeight >= 0.0) myMuscle = Muscle::Pectoral_L();
			else myMuscle = Muscle::Abdominal_L();
		}
		if ((locationAngle > 90.0) && (locationAngle < 180.0))
		{
			if (locationHeight >= 0.0) myMuscle = Muscle::Dorsal_L();
			else myMuscle = Muscle::Lumbar_L();
		}
		if ((locationAngle > 180.0) && (locationAngle < 270.0))
		{
			if (locationHeight >= 0.0) myMuscle = Muscle::Dorsal_R();
			else myMuscle = Muscle::Lumbar_R();
		}
		if ((locationAngle > 270.0))
		{
			if (locationHeight >= 0.0) myMuscle = Muscle::Pectoral_R();
			else myMuscle = Muscle::Abdominal_R();
		}

		return myMuscle;
	}

	std::string feedbackTypeToString(FeedbackType feedbackType) {
		switch (feedbackType) {
		case FeedbackType::NoFeedback: return "NoFeedback";
		case FeedbackType::MeleeSwordRight: return "MeleeSwordRight";
		case FeedbackType::MeleeAxeRight: return "MeleeAxeRight";
		case FeedbackType::MeleeMaceRight: return "MeleeMaceRight";
		case FeedbackType::MeleeDaggerRight: return "MeleeDaggerRight";
		case FeedbackType::Melee2HAxeRight: return "Melee2HAxeRight";
		case FeedbackType::Melee2HSwordRight: return "Melee2HSwordRight";
		case FeedbackType::MeleeSwordLeft: return "MeleeSwordLeft";
		case FeedbackType::MeleeAxeLeft: return "MeleeAxeLeft";
		case FeedbackType::MeleeMaceLeft: return "MeleeMaceLeft";
		case FeedbackType::MeleeDaggerLeft: return "MeleeDaggerLeft";
		case FeedbackType::Melee2HAxeLeft: return "Melee2HAxeLeft";
		case FeedbackType::Melee2HSwordLeft: return "Melee2HSwordLeft";
		case FeedbackType::MeleeFist: return "MeleeFist";
		case FeedbackType::MeleeBash: return "MeleeBash";
		case FeedbackType::MeleePowerAttack: return "MeleePowerAttack";
		case FeedbackType::Ranged: return "Ranged";
		case FeedbackType::MagicFire: return "MagicFire";
		case FeedbackType::MagicShock: return "MagicShock";
		case FeedbackType::MagicIce: return "MagicIce";
		case FeedbackType::MagicAlteration: return "MagicAlteration";
		case FeedbackType::MagicIllusion: return "MagicIllusion";
		case FeedbackType::MagicRestoration: return "MagicRestoration";
		case FeedbackType::MagicPoison: return "MagicPoison";
		case FeedbackType::MagicOther: return "MagicOther";
		case FeedbackType::MagicContinuousFire: return "MagicContinuousFire";
		case FeedbackType::MagicContinuousShock: return "MagicContinuousShock";
		case FeedbackType::MagicContinuousIce: return "MagicContinuousIce";
		case FeedbackType::MagicContinuousAlteration: return "MagicContinuousAlteration";
		case FeedbackType::MagicContinuousIllusion: return "MagicContinuousIllusion";
		case FeedbackType::MagicContinuousRestoration: return "MagicContinuousRestoration";
		case FeedbackType::MagicContinuousPoison: return "MagicContinuousPoison";
		case FeedbackType::MagicContinuousOther: return "MagicContinuousOther";
		case FeedbackType::HeartBeat: return "HeartBeat";
		case FeedbackType::HeartBeatFast: return "HeartBeatFast";
		case FeedbackType::GreybeardPowerAbsorb: return "GreybeardPowerAbsorb";
		case FeedbackType::DragonSoul: return "DragonSoul";
		case FeedbackType::WordWall: return "WordWall";
		case FeedbackType::PlayerSpellFireLeft: return "PlayerSpellFireLeft";
		case FeedbackType::PlayerSpellIceLeft: return "PlayerSpellIceLeft";
		case FeedbackType::PlayerSpellShockLeft: return "PlayerSpellShockLeft";
		case FeedbackType::PlayerSpellAlterationLeft: return "PlayerSpellAlterationLeft";
		case FeedbackType::PlayerSpellIllusionLeft: return "PlayerSpellIllusionLeft";
		case FeedbackType::PlayerSpellLightLeft: return "PlayerSpellLightLeft";
		case FeedbackType::PlayerSpellRestorationLeft: return "PlayerSpellRestorationLeft";
		case FeedbackType::PlayerSpellWardLeft: return "PlayerSpellWardLeft";
		case FeedbackType::PlayerSpellConjurationLeft: return "PlayerSpellConjurationLeft";
		case FeedbackType::PlayerSpellOtherLeft: return "PlayerSpellOtherLeft";
		case FeedbackType::PlayerSpellFireRight: return "PlayerSpellFireRight";
		case FeedbackType::PlayerSpellIceRight: return "PlayerSpellIceRight";
		case FeedbackType::PlayerSpellShockRight: return "PlayerSpellShockRight";
		case FeedbackType::PlayerSpellAlterationRight: return "PlayerSpellAlterationRight";
		case FeedbackType::PlayerSpellIllusionRight: return "PlayerSpellIllusionRight";
		case FeedbackType::PlayerSpellLightRight: return "PlayerSpellLightRight";
		case FeedbackType::PlayerSpellRestorationRight: return "PlayerSpellRestorationRight";
		case FeedbackType::PlayerSpellWardRight: return "PlayerSpellWardRight";
		case FeedbackType::PlayerSpellConjurationRight: return "PlayerSpellConjurationRight";
		case FeedbackType::PlayerSpellOtherRight: return "PlayerSpellOtherRight";
		case FeedbackType::PlayerPowerAttackLeft: return "PlayerPowerAttackLeft";
		case FeedbackType::PlayerBashLeft: return "PlayerBashLeft";
		case FeedbackType::PlayerAttackLeft: return "PlayerAttackLeft";
		case FeedbackType::PlayerPowerAttackRight: return "PlayerPowerAttackRight";
		case FeedbackType::PlayerBashRight: return "PlayerBashRight";
		case FeedbackType::PlayerAttackRight: return "PlayerAttackRight";
		case FeedbackType::PlayerBlockLeft: return "PlayerBlockLeft";
		case FeedbackType::PlayerBlockRight: return "PlayerBlockRight";
		case FeedbackType::PlayerBowPullLeft: return "PlayerBowPullLeft";
		case FeedbackType::PlayerBowPullRight: return "PlayerBowPullRight";
		case FeedbackType::PlayerBowHoldLeft: return "PlayerBowHoldLeft";
		case FeedbackType::PlayerBowHoldRight: return "PlayerBowHoldRight";
		case FeedbackType::PlayerShout: return "PlayerShout";
		case FeedbackType::PlayerShoutBindHands: return "PlayerShoutBindHands";
		case FeedbackType::PlayerShoutBindVest: return "PlayerShoutBindVest";
		case FeedbackType::PlayerCrossbowFireLeft: return "PlayerCrossbowFireLeft";
		case FeedbackType::PlayerCrossbowFireRight: return "PlayerCrossbowFireRight";
		case FeedbackType::PlayerCrossbowKickbackLeft: return "PlayerCrossbowKickbackLeft";
		case FeedbackType::PlayerCrossbowKickbackRight: return "PlayerCrossbowKickbackRight";
		case FeedbackType::Bite: return "Bite";
		case FeedbackType::SleeveHolsterStoreLeft: return "SleeveHolsterStoreLeft";
		case FeedbackType::SleeveHolsterStoreRight: return "SleeveHolsterStoreRight";
		case FeedbackType::SleeveHolsterRemoveLeft: return "SleeveHolsterRemoveLeft";
		case FeedbackType::SleeveHolsterRemoveRight: return "SleeveHolsterRemoveRight";
		case FeedbackType::BackpackStoreLeft: return "BackpackStoreLeft";
		case FeedbackType::BackpackStoreRight: return "BackpackStoreRight";
		case FeedbackType::BackpackRemoveLeft: return "BackpackRemoveLeft";
		case FeedbackType::BackpackRemoveRight: return "BackpackRemoveRight";
		case FeedbackType::StomachStore: return "StomachStore";
		case FeedbackType::StomachRemove: return "StomachRemove";
		case FeedbackType::ChestStore: return "ChestStore";
		case FeedbackType::ChestRemove: return "ChestRemove";
		case FeedbackType::HipHolsterStoreLeft: return "HipHolsterStoreLeft";
		case FeedbackType::HipHolsterStoreRight: return "HipHolsterStoreRight";
		case FeedbackType::HipHolsterRemoveLeft: return "HipHolsterRemoveLeft";
		case FeedbackType::HipHolsterRemoveRight: return "HipHolsterRemoveRight";
		case FeedbackType::Shout: return "Shout";
		case FeedbackType::ShoutFire: return "ShoutFire";
		case FeedbackType::ShoutFrost: return "ShoutFrost";
		case FeedbackType::ShoutSteam: return "ShoutSteam";
		case FeedbackType::ShoutLightning: return "ShoutLightning";
		case FeedbackType::HiggsPullLeft: return "HiggsPullLeft";
		case FeedbackType::HiggsPullRight: return "HiggsPullRight";
		case FeedbackType::PlayerEnvironmentHitLeft: return "PlayerEnvironmentHitLeft";
		case FeedbackType::PlayerEnvironmentHitRight: return "PlayerEnvironmentHitRight";
		case FeedbackType::PlayerThrowLeft: return "PlayerThrowLeft";
		case FeedbackType::PlayerThrowRight: return "PlayerThrowRight";
		case FeedbackType::PlayerCatchLeft: return "PlayerCatchLeft";
		case FeedbackType::PlayerCatchRight: return "PlayerCatchRight";
		case FeedbackType::TravelEffect: return "TravelEffect";
		case FeedbackType::Teleport: return "Teleport";
		case FeedbackType::EnvironmentRumble: return "EnvironmentRumble";
		case FeedbackType::DragonLanding: return "DragonLanding";
		case FeedbackType::EquipHelmet: return "EquipHelmet";
		case FeedbackType::EquipCuirass: return "EquipCuirass";
		case FeedbackType::EquipGauntlets: return "EquipGauntlets";
		case FeedbackType::EquipClothing: return "EquipClothing";
		case FeedbackType::EquipHood: return "EquipHood";
		case FeedbackType::UnequipHelmet: return "UnequipHelmet";
		case FeedbackType::UnequipCuirass: return "UnequipCuirass";
		case FeedbackType::UnequipGauntlets: return "UnequipGauntlets";
		case FeedbackType::UnequipClothing: return "UnequipClothing";
		case FeedbackType::UnequipHood: return "UnequipHood";
		case FeedbackType::Learned: return "Learned";
		case FeedbackType::UnholsterArrowLeftShoulder: return "UnholsterArrowLeftShoulder";
		case FeedbackType::UnholsterArrowRightShoulder: return "UnholsterArrowRightShoulder";
		case FeedbackType::ConsumableDrink: return "ConsumableDrink";
		case FeedbackType::ConsumableFood: return "ConsumableFood";
		case FeedbackType::Explosion: return "Explosion";
		case FeedbackType::EnvironmentalPoison: return "EnvironmentalPoison";
		case FeedbackType::EnvironmentalFire: return "EnvironmentalFire";
		case FeedbackType::EnvironmentalElectric: return "EnvironmentalElectric";
		case FeedbackType::EnvironmentalFrost: return "EnvironmentalFrost";
		case FeedbackType::EnvironmentalFireCloak: return "EnvironmentalFireCloak";
		case FeedbackType::EnvironmentalElectricCloak: return "EnvironmentalElectricCloak";
		case FeedbackType::EnvironmentalFrostCloak: return "EnvironmentalFrostCloak";
		case FeedbackType::TentacleAttack: return "TentacleAttack";
		case FeedbackType::GiantStomp: return "GiantStomp";
		case FeedbackType::GiantClubLeft: return "GiantClubLeft";
		case FeedbackType::GiantClubRight: return "GiantClubRight";
		case FeedbackType::UnarmedDefault: return "UnarmedDefault";
		case FeedbackType::UnarmedDragon: return "UnarmedDragon";
		case FeedbackType::UnarmedFrostbiteSpider: return "UnarmedFrostbiteSpider";
		case FeedbackType::UnarmedSabreCat: return "UnarmedSabreCat";
		case FeedbackType::UnarmedSkeever: return "UnarmedSkeever";
		case FeedbackType::UnarmedSlaughterfish: return "UnarmedSlaughterfish";
		case FeedbackType::UnarmedWisp: return "UnarmedWisp";
		case FeedbackType::UnarmedDragonPriest: return "UnarmedDragonPriest";
		case FeedbackType::UnarmedDraugr: return "UnarmedDraugr";
		case FeedbackType::UnarmedWolf: return "UnarmedWolf";
		case FeedbackType::UnarmedGiant: return "UnarmedGiant";
		case FeedbackType::UnarmedIceWraith: return "UnarmedIceWraith";
		case FeedbackType::UnarmedChaurus: return "UnarmedChaurus";
		case FeedbackType::UnarmedMammoth: return "UnarmedMammoth";
		case FeedbackType::UnarmedFrostAtronach: return "UnarmedFrostAtronach";
		case FeedbackType::UnarmedFalmer: return "UnarmedFalmer";
		case FeedbackType::UnarmedHorse: return "UnarmedHorse";
		case FeedbackType::UnarmedStormAtronach: return "UnarmedStormAtronach";
		case FeedbackType::UnarmedElk: return "UnarmedElk";
		case FeedbackType::UnarmedDwarvenSphere: return "UnarmedDwarvenSphere";
		case FeedbackType::UnarmedDwarvenSteam: return "UnarmedDwarvenSteam";
		case FeedbackType::UnarmedDwarvenSpider: return "UnarmedDwarvenSpider";
		case FeedbackType::UnarmedBear: return "UnarmedBear";
		case FeedbackType::UnarmedFlameAtronach: return "UnarmedFlameAtronach";
		case FeedbackType::UnarmedWitchlight: return "UnarmedWitchlight";
		case FeedbackType::UnarmedHorker: return "UnarmedHorker";
		case FeedbackType::UnarmedTroll: return "UnarmedTroll";
		case FeedbackType::UnarmedHagraven: return "UnarmedHagraven";
		case FeedbackType::UnarmedSpriggan: return "UnarmedSpriggan";
		case FeedbackType::UnarmedMudcrab: return "UnarmedMudcrab";
		case FeedbackType::UnarmedWerewolf: return "UnarmedWerewolf";
		case FeedbackType::UnarmedChaurusFlyer: return "UnarmedChaurusFlyer";
		case FeedbackType::UnarmedGargoyle: return "UnarmedGargoyle";
		case FeedbackType::UnarmedRiekling: return "UnarmedRiekling";
		case FeedbackType::UnarmedScrib: return "UnarmedScrib";
		case FeedbackType::UnarmedSeeker: return "UnarmedSeeker";
		case FeedbackType::UnarmedMountedRiekling: return "UnarmedMountedRiekling";
		case FeedbackType::UnarmedNetch: return "UnarmedNetch";
		case FeedbackType::UnarmedBenthicLurker: return "UnarmedBenthicLurker";
		case FeedbackType::DefaultHead: return "DefaultHead";
		case FeedbackType::UnarmedHead: return "UnarmedHead";
		case FeedbackType::RangedHead: return "RangedHead";
		case FeedbackType::MeleeHead: return "MeleeHead";
		case FeedbackType::MagicHeadFire: return "MagicHeadFire";
		case FeedbackType::MagicHeadShock: return "MagicHeadShock";
		case FeedbackType::MagicHeadIce: return "MagicHeadIce";
		case FeedbackType::MagicHeadAlteration: return "MagicHeadAlteration";
		case FeedbackType::MagicHeadIllusion: return "MagicHeadIllusion";
		case FeedbackType::MagicHeadRestoration: return "MagicHeadRestoration";
		case FeedbackType::RangedLeftArm: return "RangedLeftArm";
		case FeedbackType::RangedRightArm: return "RangedRightArm";
		case FeedbackType::MagicLeftArmFire: return "MagicLeftArmFire";
		case FeedbackType::MagicLeftArmShock: return "MagicLeftArmShock";
		case FeedbackType::MagicLeftArmIce: return "MagicLeftArmIce";
		case FeedbackType::MagicLeftArmAlteration: return "MagicLeftArmAlteration";
		case FeedbackType::MagicLeftArmIllusion: return "MagicLeftArmIllusion";
		case FeedbackType::MagicLeftArmRestoration: return "MagicLeftArmRestoration";
		case FeedbackType::MagicRightArmFire: return "MagicRightArmFire";
		case FeedbackType::MagicRightArmShock: return "MagicRightArmShock";
		case FeedbackType::MagicRightArmIce: return "MagicRightArmIce";
		case FeedbackType::MagicRightArmAlteration: return "MagicRightArmAlteration";
		case FeedbackType::MagicRightArmIllusion: return "MagicRightArmIllusion";
		case FeedbackType::MagicRightArmRestoration: return "MagicRightArmRestoration";
		case FeedbackType::HorseRidingSlow: return "HorseRidingSlow";
		case FeedbackType::HorseRiding: return "HorseRiding";
		case FeedbackType::FallEffect: return "FallEffect";
		case FeedbackType::SwimVest20: return "SwimVest20";
		case FeedbackType::SwimVest40: return "SwimVest40";
		case FeedbackType::SwimVest60: return "SwimVest60";
		case FeedbackType::SwimVest80: return "SwimVest80";
		case FeedbackType::SwimVest100: return "SwimVest100";
		case FeedbackType::DrowningEffectVest: return "DrowningEffectVest";
		case FeedbackType::DrowningEffectHead: return "DrowningEffectHead";
		case FeedbackType::Wind: return "Wind";
		case FeedbackType::MagicArmorSpell: return "MagicArmorSpell";
		case FeedbackType::SpellWheelOpen: return "SpellWheelOpen";
		case FeedbackType::Default: return "Default";
		default: return "Unknown";
		}
	}


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


		feedbackMap[FeedbackType::SpellWheelOpen] = Feedback(FeedbackType::SpellWheelOpen, "SpellWheelOpen_");

		feedbackMap[FeedbackType::Default] = Feedback(FeedbackType::Default, "Default_"); //1		
	}
}
