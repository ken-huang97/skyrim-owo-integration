#include "TactsuitVR.h"

#include "Domain/BakedSensationsParser.h"
#include "Domain/SensationWithMusclesParser.h"
#include "Domain/SensationsParser.h"
#include "Domain/GameAuth.h"
#include "Controller/OWO.h"
#include "Infrastructure/UDPNetwork.h"

using namespace OWOGame;

namespace TactsuitVR {
	bool systemInitialized = false;
	bool owoLoopUpdating = false;
	int owoUpdateWaitMs = 10;

	float TOLERANCE = 0.00001f;

	sharedPtr<OWO> owo;
	std::clock_t startTime;

	std::unordered_map<FeedbackType, Feedback> feedbackMap;
	std::unordered_map<std::string, Feedback> unknownFeedbackMap;

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
			default:      return "Unknown";
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

			LOG("OWO System Initialized");
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
				_MESSAGE("OWO connection state changed from %s to %s", ToString(prevConnectionState).c_str(), ToString(curConnectionState).c_str());

				if (curConnectionState == ConnectionState::Connecting && prevConnectionState == ConnectionState::Connected) {
					owo->AutoConnect();
				}
				prevConnectionState = curConnectionState;
			}
			WaitFor(owoUpdateWaitMs);
		}
	}

	sharedPtr<BakedSensation> TactFileRegister(std::string &configPath, std::string &filename, Feedback feedback)
	{
		std::string path = configPath;
		path += "\\";
		path += filename;

		std::ostringstream sstream;
		std::ifstream fs(path);
		sstream << fs.rdbuf();
		const std::string tactFileStr(sstream.str());

		sharedPtr<BakedSensation> sensation(BakedSensationsParser::Parse(tactFileStr));
		sensation->SetPriority(feedback.priority);

		if (feedbackMap.find(feedback.feedbackType) != feedbackMap.end())
		{
			feedbackMap[feedback.feedbackType].feedbackSensations.push_back(sensation);
			_MESSAGE("Added sensation with duration %f to %s", sensation->TotalDuration(), feedbackTypeToString(feedback.feedbackType));
		}
		else {
			_MESSAGE("WARN: This should not have happened: No feedback in map exists for %s", filename.c_str());
		}
		
		return sensation;
	}

	sharedPtr<BakedSensation> TactFileRegisterFilename(std::string& configPath, std::string& filename)
	{
		std::string path = configPath;
		path += "\\";
		path += filename;

		std::ostringstream sstream;
		std::ifstream fs(path);
		sstream << fs.rdbuf();
		const std::string tactFileStr(sstream.str());

		sharedPtr<BakedSensation> sensation(BakedSensationsParser::Parse(tactFileStr));
		sensation->SetPriority(2);

		auto sensName = removeOwoExtension(filename);

		// TODO support this
		if (unknownFeedbackMap.find(sensName) != unknownFeedbackMap.end()) {
			_MESSAGE("Duplicate unknown files found for %s. Overwriting", filename.c_str());
		}

		Feedback feedback(FeedbackType::Default, sensName);
		feedback.feedbackSensations.push_back(sensation);
		unknownFeedbackMap[sensName] = feedback;
		_MESSAGE("Added %s to unknown feedback %s", filename.c_str(), sensName.c_str());

		return sensation;
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
						sharedPtr<BakedSensation> sensation(TactFileRegister(configPath, filename, element.second));
						if (sensation == NULL) {
							LOG("Unknown sensation %s. Skipping.", filename.c_str());
							break;
						}

						ret.push_back(sensation->Stringify());
						LOG("Successfully added %s to %s", filename.c_str(), element.second.prefix.c_str());

						found = true;
						break;
					}
				}

				if (!found && filename.find(".owo") != std::string::npos)
				{
					LOG("File feedback unknown: %s", filename.c_str());
					sharedPtr<BakedSensation> sensation(TactFileRegisterFilename(configPath, filename));
					ret.push_back(sensation->Stringify());
				}
			}
		}

		return ret;
	}

	void ProvideHapticFeedback(float locationAngle, float locationHeight, FeedbackType effect, float intensityMultiplier, bool waitToPlay, bool playInMenu)
	{
		//_MESSAGE("ProvideHapticFeedback");
		if (intensityMultiplier > TOLERANCE)
		{
			std::thread t0(ProvideHapticFeedbackThread, locationAngle, locationHeight, effect, intensityMultiplier, waitToPlay, playInMenu);
			t0.detach();
		}
	}

	// This approximates IsPlayingFeedback, since it uses expiry time instead of actually checking if it's currently playing.
	// If another sensation plays after one is playing, this may be inaccurate
	bool IsPlayingFeedback(Feedback& feedback)
	{
		auto ret = getTimeSinceStart() <= feedback.expiryTime;
		/*
		if (ret) {
			_MESSAGE("IsPlayingFeedback %s true", feedbackTypeToString(feedback.feedbackType).c_str());
		}
		else {
			_MESSAGE("IsPlayingFeedback %s false", feedbackTypeToString(feedback.feedbackType).c_str());
		}*/
		return ret;
	}

	// This approximates IsPlayingFeedback, since it uses expiry time instead of actually checking if it's currently playing.
	// If another sensation plays after one is playing, this may be inaccurate
	bool IsPlayingFeedback(FeedbackType feedbackType)
	{
		return IsPlayingFeedback(feedbackMap[feedbackType]);
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


	std::string unbake(const std::string& input) {
		std::vector<std::string> parts;
		std::string token;
		std::istringstream stream(input);
		while (std::getline(stream, token, '~')) {
			parts.push_back(token);
		}

		return (parts.size() >= 4) ? parts[2] : input;
	}

	void sendSensation(float locationAngle, float locationHeight, sharedPtr<BakedSensation> sensation) {
		//_MESSAGE("sendSensation");
		if (owo->State() != ConnectionState::Connected) {
			_MESSAGE("WARN: Not connected. Skipping.");
			return;
		}

		// auto sensationWithId = SensationsParser::Parse(std::to_string(sensation->id));
		auto sensationWithId = SensationsParser::Parse(unbake(sensation->Stringify()));
		sensationWithId->SetPriority(sensation->GetPriority());
		if (locationHeight < -0.5f)
			locationHeight = -0.5f;
		else if (locationHeight > 0.5f)
			locationHeight = 0.5f;

		_MESSAGE("Sending %s with priority %d", sensation->name.c_str(), sensation->GetPriority());
		if (SensationWithMusclesParser::CanParse(sensation->Stringify())) {
			owo->Send(sensationWithId);
		}
		else {
			std::vector<Muscle> muscles = { angleToMuscle(locationAngle, locationHeight) };
			_MESSAGE("Sending with muscles from angle %d height %d", locationAngle, locationHeight);
			owo->Send(sensationWithId->WithMuscles(muscles));
		}
	}

	std::shared_ptr<BakedSensation> getRandomSensation(Feedback& feedback) {
		//TODO only need to store the ID and whether it needs muscle?

		std::vector<std::shared_ptr<BakedSensation>>& feedbackSensations = feedback.feedbackSensations;
		return feedbackSensations[(randi(0, feedbackSensations.size() - 1))];
	}

	void sendFeedback(float locationAngle, float locationHeight, Feedback& feedback, float intensityMultiplier, bool waitToPlay)
	{
		//_MESSAGE("sendFeedback");
		if (intensityMultiplier < TOLERANCE || (isGameStoppedNoDialogue()))
			return;

		if (!systemInitialized)
			CreateSystem();

		if (feedback.feedbackSensations.size() > 0)
		{
			if (waitToPlay && IsPlayingFeedback(feedback))
			{
				return;
			}


			auto sensation = getRandomSensation(feedback);
			
			auto durationSecs = (long double)sensation->TotalDuration();
			auto timeSinceStartSecs = (long double) getTimeSinceStart() / CLOCKS_PER_SEC;
			feedback.expiryTime = static_cast<clock_t>((durationSecs + timeSinceStartSecs) * CLOCKS_PER_SEC);
			//_MESSAGE("Sending %s Expiry: %Lf Current time %Lf", feedbackTypeToString(feedback.feedbackType).c_str(), (long double) feedback.expiryTime / CLOCKS_PER_SEC, timeSinceStartSecs);

			sendSensation(locationAngle, locationHeight, sensation);
		}
		else
		{
			_MESSAGE("Warn: No file found for %s", feedback.prefix.c_str());
		}
	}

	void ProvideHapticFeedbackThread(float locationAngle, float locationHeight, FeedbackType effect, float intensityMultiplier, bool waitToPlay, bool playInMenu)
	{
		//_MESSAGE("ProvideHapticFeedbackThread");
		//_MESSAGE("ProvideHapticFeedbackThread %s %f %f", feedbackTypeToString(effect).c_str(), locationAngle, locationHeight);
		if (isGameStoppedNoDialogue() && !playInMenu)
			return;

		if (feedbackMap.find(effect) != feedbackMap.end())
		{
			sendFeedback(locationAngle, locationHeight, feedbackMap[effect], intensityMultiplier, waitToPlay);
		}
		else {
			_MESSAGE("Warn: feedback not in map %s", feedbackTypeToString(effect));
		}
	}

	std::string remove_after_underscore(const std::string& str) {
		size_t underscore_pos = str.find('_');
		return (underscore_pos == std::string::npos) ? str : str.substr(0, underscore_pos);
	}

	void ProvideHapticFeedbackSpecificFile(float locationAngle, float locationHeight, std::string feedbackFileName, float intensityMultiplier, bool waitToPlay)
	{
		//_MESSAGE("ProvideHapticFeedbackSpecificFile");
		auto feedbackType = stringToFeedbackType(remove_after_underscore(feedbackFileName));

		if (feedbackType != FeedbackType::Default) {
			return ProvideHapticFeedback(locationAngle, locationHeight, feedbackType, intensityMultiplier, waitToPlay);
		} else {
			if (intensityMultiplier > TOLERANCE)
			{
				std::thread t0(sendFeedback, locationAngle, locationHeight, unknownFeedbackMap[feedbackFileName], intensityMultiplier, waitToPlay);
				t0.detach();
			}
		}
	}

	static const std::unordered_map<FeedbackType, std::string> feedbackStringMap = {
		 {FeedbackType::NoFeedback, "NoFeedback"},
		 {FeedbackType::MeleeSwordRight, "MeleeSwordRight"},
		 {FeedbackType::MeleeAxeRight, "MeleeAxeRight"},
		 {FeedbackType::MeleeMaceRight, "MeleeMaceRight"},
		 {FeedbackType::MeleeDaggerRight, "MeleeDaggerRight"},
		 {FeedbackType::Melee2HAxeRight, "Melee2HAxeRight"},
		 {FeedbackType::Melee2HSwordRight, "Melee2HSwordRight"},
		 {FeedbackType::MeleeSwordLeft, "MeleeSwordLeft"},
		 {FeedbackType::MeleeAxeLeft, "MeleeAxeLeft"},
		 {FeedbackType::MeleeMaceLeft, "MeleeMaceLeft"},
		 {FeedbackType::MeleeDaggerLeft, "MeleeDaggerLeft"},
		 {FeedbackType::Melee2HAxeLeft, "Melee2HAxeLeft"},
		 {FeedbackType::Melee2HSwordLeft, "Melee2HSwordLeft"},
		 {FeedbackType::MeleeFist, "MeleeFist"},
		 {FeedbackType::MeleeBash, "MeleeBash"},
		 {FeedbackType::MeleePowerAttack, "MeleePowerAttack"},
		 {FeedbackType::Ranged, "Ranged"},
		 {FeedbackType::MagicFire, "MagicFire"},
		 {FeedbackType::MagicShock, "MagicShock"},
		 {FeedbackType::MagicIce, "MagicIce"},
		 {FeedbackType::MagicAlteration, "MagicAlteration"},
		 {FeedbackType::MagicIllusion, "MagicIllusion"},
		 {FeedbackType::MagicRestoration, "MagicRestoration"},
		 {FeedbackType::MagicPoison, "MagicPoison"},
		 {FeedbackType::MagicOther, "MagicOther"},
		 {FeedbackType::MagicContinuousFire, "MagicContinuousFire"},
		 {FeedbackType::MagicContinuousShock, "MagicContinuousShock"},
		 {FeedbackType::MagicContinuousIce, "MagicContinuousIce"},
		 {FeedbackType::MagicContinuousAlteration, "MagicContinuousAlteration"},
		 {FeedbackType::MagicContinuousIllusion, "MagicContinuousIllusion"},
		 {FeedbackType::MagicContinuousRestoration, "MagicContinuousRestoration"},
		 {FeedbackType::MagicContinuousPoison, "MagicContinuousPoison"},
		 {FeedbackType::MagicContinuousOther, "MagicContinuousOther"},
		 {FeedbackType::HeartBeat, "HeartBeat"},
		 {FeedbackType::HeartBeatFast, "HeartBeatFast"},
		 {FeedbackType::GreybeardPowerAbsorb, "GreybeardPowerAbsorb"},
		 {FeedbackType::DragonSoul, "DragonSoul"},
		 {FeedbackType::WordWall, "WordWall"},
		 {FeedbackType::PlayerSpellFireLeft, "PlayerSpellFireLeft"},
		 {FeedbackType::PlayerSpellIceLeft, "PlayerSpellIceLeft"},
		 {FeedbackType::PlayerSpellShockLeft, "PlayerSpellShockLeft"},
		 {FeedbackType::PlayerSpellAlterationLeft, "PlayerSpellAlterationLeft"},
		 {FeedbackType::PlayerSpellIllusionLeft, "PlayerSpellIllusionLeft"},
		 {FeedbackType::PlayerSpellLightLeft, "PlayerSpellLightLeft"},
		 {FeedbackType::PlayerSpellRestorationLeft, "PlayerSpellRestorationLeft"},
		 {FeedbackType::PlayerSpellWardLeft, "PlayerSpellWardLeft"},
		 {FeedbackType::PlayerSpellConjurationLeft, "PlayerSpellConjurationLeft"},
		 {FeedbackType::PlayerSpellOtherLeft, "PlayerSpellOtherLeft"},
		 {FeedbackType::PlayerSpellFireRight, "PlayerSpellFireRight"},
		 {FeedbackType::PlayerSpellIceRight, "PlayerSpellIceRight"},
		 {FeedbackType::PlayerSpellShockRight, "PlayerSpellShockRight"},
		 {FeedbackType::PlayerSpellAlterationRight, "PlayerSpellAlterationRight"},
		 {FeedbackType::PlayerSpellIllusionRight, "PlayerSpellIllusionRight"},
		 {FeedbackType::PlayerSpellLightRight, "PlayerSpellLightRight"},
		 {FeedbackType::PlayerSpellRestorationRight, "PlayerSpellRestorationRight"},
		 {FeedbackType::PlayerSpellWardRight, "PlayerSpellWardRight"},
		 {FeedbackType::PlayerSpellConjurationRight, "PlayerSpellConjurationRight"},
		 {FeedbackType::PlayerSpellOtherRight, "PlayerSpellOtherRight"},
		 {FeedbackType::PlayerPowerAttackLeft, "PlayerPowerAttackLeft"},
		 {FeedbackType::PlayerBashLeft, "PlayerBashLeft"},
		 {FeedbackType::PlayerAttackLeft, "PlayerAttackLeft"},
		 {FeedbackType::PlayerPowerAttackRight, "PlayerPowerAttackRight"},
		 {FeedbackType::PlayerBashRight, "PlayerBashRight"},
		 {FeedbackType::PlayerAttackRight, "PlayerAttackRight"},
		 {FeedbackType::PlayerBlockLeft, "PlayerBlockLeft"},
		 {FeedbackType::PlayerBlockRight, "PlayerBlockRight"},
		 {FeedbackType::PlayerBowPullLeft, "PlayerBowPullLeft"},
		 {FeedbackType::PlayerBowPullRight, "PlayerBowPullRight"},
		 {FeedbackType::PlayerBowHoldLeft, "PlayerBowHoldLeft"},
		 {FeedbackType::PlayerBowHoldRight, "PlayerBowHoldRight"},
		 {FeedbackType::PlayerShout, "PlayerShout"},
		 {FeedbackType::PlayerShoutBindHands, "PlayerShoutBindHands"},
		 {FeedbackType::PlayerShoutBindVest, "PlayerShoutBindVest"},
		 {FeedbackType::PlayerCrossbowFireLeft, "PlayerCrossbowFireLeft"},
		 {FeedbackType::PlayerCrossbowFireRight, "PlayerCrossbowFireRight"},
		 {FeedbackType::PlayerCrossbowKickbackLeft, "PlayerCrossbowKickbackLeft"},
		 {FeedbackType::PlayerCrossbowKickbackRight, "PlayerCrossbowKickbackRight"},
		 {FeedbackType::Bite, "Bite"},
		 {FeedbackType::SleeveHolsterStoreLeft, "SleeveHolsterStoreLeft"},
		 {FeedbackType::SleeveHolsterStoreRight, "SleeveHolsterStoreRight"},
		 {FeedbackType::SleeveHolsterRemoveLeft, "SleeveHolsterRemoveLeft"},
		 {FeedbackType::SleeveHolsterRemoveRight, "SleeveHolsterRemoveRight"},
		 {FeedbackType::BackpackStoreLeft, "BackpackStoreLeft"},
		 {FeedbackType::BackpackStoreRight, "BackpackStoreRight"},
		 {FeedbackType::BackpackRemoveLeft, "BackpackRemoveLeft"},
		 {FeedbackType::BackpackRemoveRight, "BackpackRemoveRight"},
		 {FeedbackType::StomachStore, "StomachStore"},
		 {FeedbackType::StomachRemove, "StomachRemove"},
		 {FeedbackType::ChestStore, "ChestStore"},
		 {FeedbackType::ChestRemove, "ChestRemove"},
		 {FeedbackType::HipHolsterStoreLeft, "HipHolsterStoreLeft"},
		 {FeedbackType::HipHolsterStoreRight, "HipHolsterStoreRight"},
		 {FeedbackType::HipHolsterRemoveLeft, "HipHolsterRemoveLeft"},
		 {FeedbackType::HipHolsterRemoveRight, "HipHolsterRemoveRight"},
		 {FeedbackType::Shout, "Shout"},
		 {FeedbackType::ShoutFire, "ShoutFire"},
		 {FeedbackType::ShoutFrost, "ShoutFrost"},
		 {FeedbackType::ShoutSteam, "ShoutSteam"},
		 {FeedbackType::ShoutLightning, "ShoutLightning"},
		 {FeedbackType::HiggsPullLeft, "HiggsPullLeft"},
		 {FeedbackType::HiggsPullRight, "HiggsPullRight"},
		 {FeedbackType::PlayerEnvironmentHitLeft, "PlayerEnvironmentHitLeft"},
		 {FeedbackType::PlayerEnvironmentHitRight, "PlayerEnvironmentHitRight"},
		 {FeedbackType::PlayerThrowLeft, "PlayerThrowLeft"},
		 {FeedbackType::PlayerThrowRight, "PlayerThrowRight"},
		 {FeedbackType::PlayerCatchLeft, "PlayerCatchLeft"},
		 {FeedbackType::PlayerCatchRight, "PlayerCatchRight"},
		 {FeedbackType::TravelEffect, "TravelEffect"},
		 {FeedbackType::Teleport, "Teleport"},
		 {FeedbackType::EnvironmentRumble, "EnvironmentRumble"},
		 {FeedbackType::DragonLanding, "DragonLanding"},
		 {FeedbackType::EquipHelmet, "EquipHelmet"},
		 {FeedbackType::EquipCuirass, "EquipCuirass"},
		 {FeedbackType::EquipGauntlets, "EquipGauntlets"},
		 {FeedbackType::EquipClothing, "EquipClothing"},
		 {FeedbackType::EquipHood, "EquipHood"},
		 {FeedbackType::UnequipHelmet, "UnequipHelmet"},
		 {FeedbackType::UnequipCuirass, "UnequipCuirass"},
		 {FeedbackType::UnequipGauntlets, "UnequipGauntlets"},
		 {FeedbackType::UnequipClothing, "UnequipClothing"},
		 {FeedbackType::UnequipHood, "UnequipHood"},
		 {FeedbackType::Learned, "Learned"},
		 {FeedbackType::UnholsterArrowLeftShoulder, "UnholsterArrowLeftShoulder"},
		 {FeedbackType::UnholsterArrowRightShoulder, "UnholsterArrowRightShoulder"},
		 {FeedbackType::ConsumableDrink, "ConsumableDrink"},
		 {FeedbackType::ConsumableFood, "ConsumableFood"},
		 {FeedbackType::Explosion, "Explosion"},
		 {FeedbackType::EnvironmentalPoison, "EnvironmentalPoison"},
		 {FeedbackType::EnvironmentalFire, "EnvironmentalFire"},
		 {FeedbackType::EnvironmentalElectric, "EnvironmentalElectric"},
		 {FeedbackType::EnvironmentalFrost, "EnvironmentalFrost"},
		 {FeedbackType::EnvironmentalFireCloak, "EnvironmentalFireCloak"},
		 {FeedbackType::EnvironmentalElectricCloak, "EnvironmentalElectricCloak"},
		 {FeedbackType::EnvironmentalFrostCloak, "EnvironmentalFrostCloak"},
		 {FeedbackType::TentacleAttack, "TentacleAttack"},
		 {FeedbackType::GiantStomp, "GiantStomp"},
		 {FeedbackType::GiantClubLeft, "GiantClubLeft"},
		 {FeedbackType::GiantClubRight, "GiantClubRight"},
		 {FeedbackType::UnarmedDefault, "UnarmedDefault"},
		 {FeedbackType::UnarmedDragon, "UnarmedDragon"},
		 {FeedbackType::UnarmedFrostbiteSpider, "UnarmedFrostbiteSpider"},
		 {FeedbackType::UnarmedSabreCat, "UnarmedSabreCat"},
		 {FeedbackType::UnarmedSkeever, "UnarmedSkeever"},
		 {FeedbackType::UnarmedSlaughterfish, "UnarmedSlaughterfish"},
		 {FeedbackType::UnarmedWisp, "UnarmedWisp"},
		 {FeedbackType::UnarmedDragonPriest, "UnarmedDragonPriest"},
		 {FeedbackType::UnarmedDraugr, "UnarmedDraugr"},
		 {FeedbackType::UnarmedWolf, "UnarmedWolf"},
		 {FeedbackType::UnarmedGiant, "UnarmedGiant"},
		 {FeedbackType::UnarmedIceWraith, "UnarmedIceWraith"},
		 {FeedbackType::UnarmedChaurus, "UnarmedChaurus"},
		 {FeedbackType::UnarmedMammoth, "UnarmedMammoth"},
		 {FeedbackType::UnarmedFrostAtronach, "UnarmedFrostAtronach"},
		 {FeedbackType::UnarmedFalmer, "UnarmedFalmer"},
		 {FeedbackType::UnarmedHorse, "UnarmedHorse"},
		 {FeedbackType::UnarmedStormAtronach, "UnarmedStormAtronach"},
		 {FeedbackType::UnarmedElk, "UnarmedElk"},
		 {FeedbackType::UnarmedDwarvenSphere, "UnarmedDwarvenSphere"},
		 {FeedbackType::UnarmedDwarvenSteam, "UnarmedDwarvenSteam"},
		 {FeedbackType::UnarmedDwarvenSpider, "UnarmedDwarvenSpider"},
		 {FeedbackType::UnarmedBear, "UnarmedBear"},
		 {FeedbackType::UnarmedFlameAtronach, "UnarmedFlameAtronach"},
		 {FeedbackType::UnarmedWitchlight, "UnarmedWitchlight"},
		 {FeedbackType::UnarmedHorker, "UnarmedHorker"},
		 {FeedbackType::UnarmedTroll, "UnarmedTroll"},
		 {FeedbackType::UnarmedHagraven, "UnarmedHagraven"},
		 {FeedbackType::UnarmedSpriggan, "UnarmedSpriggan"},
		 {FeedbackType::UnarmedMudcrab, "UnarmedMudcrab"},
		 {FeedbackType::UnarmedWerewolf, "UnarmedWerewolf"},
		 {FeedbackType::UnarmedChaurusFlyer, "UnarmedChaurusFlyer"},
		 {FeedbackType::UnarmedGargoyle, "UnarmedGargoyle"},
		 {FeedbackType::UnarmedRiekling, "UnarmedRiekling"},
		 {FeedbackType::UnarmedScrib, "UnarmedScrib"},
		 {FeedbackType::UnarmedSeeker, "UnarmedSeeker"},
		 {FeedbackType::UnarmedMountedRiekling, "UnarmedMountedRiekling"},
		 {FeedbackType::UnarmedNetch, "UnarmedNetch"},
		 {FeedbackType::UnarmedBenthicLurker, "UnarmedBenthicLurker"},
		 {FeedbackType::DefaultHead, "DefaultHead"},
		 {FeedbackType::UnarmedHead, "UnarmedHead"},
		 {FeedbackType::RangedHead, "RangedHead"},
		 {FeedbackType::MeleeHead, "MeleeHead"},
		 {FeedbackType::MagicHeadFire, "MagicHeadFire"},
		 {FeedbackType::MagicHeadShock, "MagicHeadShock"},
		 {FeedbackType::MagicHeadIce, "MagicHeadIce"},
		 {FeedbackType::MagicHeadAlteration, "MagicHeadAlteration"},
		 {FeedbackType::MagicHeadIllusion, "MagicHeadIllusion"},
		 {FeedbackType::MagicHeadRestoration, "MagicHeadRestoration"},
		 {FeedbackType::RangedLeftArm, "RangedLeftArm"},
		 {FeedbackType::RangedRightArm, "RangedRightArm"},
		 {FeedbackType::MagicLeftArmFire, "MagicLeftArmFire"},
		 {FeedbackType::MagicLeftArmShock, "MagicLeftArmShock"},
		 {FeedbackType::MagicLeftArmIce, "MagicLeftArmIce"},
		 {FeedbackType::MagicLeftArmAlteration, "MagicLeftArmAlteration"},
		 {FeedbackType::MagicLeftArmIllusion, "MagicLeftArmIllusion"},
		 {FeedbackType::MagicLeftArmRestoration, "MagicLeftArmRestoration"},
		 {FeedbackType::MagicRightArmFire, "MagicRightArmFire"},
		 {FeedbackType::MagicRightArmShock, "MagicRightArmShock"},
		 {FeedbackType::MagicRightArmIce, "MagicRightArmIce"},
		 {FeedbackType::MagicRightArmAlteration, "MagicRightArmAlteration"},
		 {FeedbackType::MagicRightArmIllusion, "MagicRightArmIllusion"},
		 {FeedbackType::MagicRightArmRestoration, "MagicRightArmRestoration"},
		 {FeedbackType::HorseRidingSlow, "HorseRidingSlow"},
		 {FeedbackType::HorseRiding, "HorseRiding"},
		 {FeedbackType::FallEffect, "FallEffect"},
		 {FeedbackType::SwimVest20, "SwimVest20"},
		 {FeedbackType::SwimVest40, "SwimVest40"},
		 {FeedbackType::SwimVest60, "SwimVest60"},
		 {FeedbackType::SwimVest80, "SwimVest80"},
		 {FeedbackType::SwimVest100, "SwimVest100"},
		 {FeedbackType::DrowningEffectVest, "DrowningEffectVest"},
		 {FeedbackType::DrowningEffectHead, "DrowningEffectHead"},
		 {FeedbackType::Wind, "Wind"},
		 {FeedbackType::MagicArmorSpell, "MagicArmorSpell"},
		 {FeedbackType::SpellWheelOpenRight, "SpellWheelOpenRight"},
		 {FeedbackType::SpellWheelOpenLeft, "SpellWheelOpenLeft"},
		 {FeedbackType::Default, "Default"},
	};

	std::string feedbackTypeToString(FeedbackType feedbackType) {
		auto it = feedbackStringMap.find(feedbackType);
		return (it != feedbackStringMap.end()) ? it->second : "Unknown";
	}

	static std::unordered_map<std::string, FeedbackType> stringToFeedbackMap = {};

	std::unordered_map<std::string, FeedbackType> generateStringToFeedbackTypeMap(const std::unordered_map<FeedbackType, std::string>& feedbackStringMap) {
		std::unordered_map<std::string, FeedbackType> reverseMap;
		for (auto& it : feedbackStringMap) {
			reverseMap[it.second] = it.first;
		}
		return reverseMap;
	}

	FeedbackType stringToFeedbackType(std::string str) {
		if (stringToFeedbackMap.size() == 0) stringToFeedbackMap = generateStringToFeedbackTypeMap(feedbackStringMap);

		// Find the corresponding string in the map
		auto it = stringToFeedbackMap.find(str);
		if (it != stringToFeedbackMap.end()) {
			return it->second;
		}
		else {
			_MESSAGE("Failed to find %s, defaulting", str.c_str());
			return FeedbackType::Default;
		}
	}

	void FillFeedbackList()
	{
		feedbackMap.clear();
		feedbackMap[FeedbackType::MeleeSwordRight] = Feedback(FeedbackType::MeleeSwordRight, "MeleeSwordRight_");
		feedbackMap[FeedbackType::MeleeAxeRight] = Feedback(FeedbackType::MeleeAxeRight, "MeleeAxeRight_");
		feedbackMap[FeedbackType::MeleeMaceRight] = Feedback(FeedbackType::MeleeMaceRight, "MeleeMaceRight_");
		feedbackMap[FeedbackType::MeleeDaggerRight] = Feedback(FeedbackType::MeleeDaggerRight, "MeleeDaggerRight_");
		feedbackMap[FeedbackType::Melee2HAxeRight] = Feedback(FeedbackType::Melee2HAxeRight, "Melee2HAxeRight_");
		feedbackMap[FeedbackType::Melee2HSwordRight] = Feedback(FeedbackType::Melee2HSwordRight, "Melee2HSwordRight_");
		feedbackMap[FeedbackType::MeleeSwordLeft] = Feedback(FeedbackType::MeleeSwordLeft, "MeleeSwordLeft_");
		feedbackMap[FeedbackType::MeleeAxeLeft] = Feedback(FeedbackType::MeleeAxeLeft, "MeleeAxeLeft_");
		feedbackMap[FeedbackType::MeleeMaceLeft] = Feedback(FeedbackType::MeleeMaceLeft, "MeleeMaceLeft_");
		feedbackMap[FeedbackType::MeleeDaggerLeft] = Feedback(FeedbackType::MeleeDaggerLeft, "MeleeDaggerLeft_");
		feedbackMap[FeedbackType::Melee2HAxeLeft] = Feedback(FeedbackType::Melee2HAxeLeft, "Melee2HAxeLeft_");
		feedbackMap[FeedbackType::Melee2HSwordLeft] = Feedback(FeedbackType::Melee2HSwordLeft, "Melee2HSwordLeft_");
		feedbackMap[FeedbackType::MeleeFist] = Feedback(FeedbackType::MeleeFist, "MeleeFist_");
		feedbackMap[FeedbackType::MeleeBash] = Feedback(FeedbackType::MeleeBash, "MeleeBash_");
		feedbackMap[FeedbackType::MeleePowerAttack] = Feedback(FeedbackType::MeleePowerAttack, "MeleePowerAttack_");
		feedbackMap[FeedbackType::Ranged] = Feedback(FeedbackType::Ranged, "Ranged_");
		feedbackMap[FeedbackType::MagicFire] = Feedback(FeedbackType::MagicFire, "MagicFire_", 10);
		feedbackMap[FeedbackType::MagicShock] = Feedback(FeedbackType::MagicShock, "MagicShock_", 10);
		feedbackMap[FeedbackType::MagicIce] = Feedback(FeedbackType::MagicIce, "MagicIce_", 10);
		feedbackMap[FeedbackType::MagicAlteration] = Feedback(FeedbackType::MagicAlteration, "MagicAlteration_", 10);
		feedbackMap[FeedbackType::MagicIllusion] = Feedback(FeedbackType::MagicIllusion, "MagicIllusion_", 10);
		feedbackMap[FeedbackType::MagicRestoration] = Feedback(FeedbackType::MagicRestoration, "MagicRestoration_", 10);
		feedbackMap[FeedbackType::MagicPoison] = Feedback(FeedbackType::MagicPoison, "MagicPoison_", 10);
		feedbackMap[FeedbackType::MagicOther] = Feedback(FeedbackType::MagicOther, "MagicOther_", 10);
		feedbackMap[FeedbackType::MagicContinuousFire] = Feedback(FeedbackType::MagicContinuousFire, "MagicContinuousFire_", 10);
		feedbackMap[FeedbackType::MagicContinuousShock] = Feedback(FeedbackType::MagicContinuousShock, "MagicContinuousShock_", 10);
		feedbackMap[FeedbackType::MagicContinuousIce] = Feedback(FeedbackType::MagicContinuousIce, "MagicContinuousIce_", 10);
		feedbackMap[FeedbackType::MagicContinuousAlteration] = Feedback(FeedbackType::MagicContinuousAlteration, "MagicContinuousAlteration_", 10);
		feedbackMap[FeedbackType::MagicContinuousIllusion] = Feedback(FeedbackType::MagicContinuousIllusion, "MagicContinuousIllusion_", 10);
		feedbackMap[FeedbackType::MagicContinuousRestoration] = Feedback(FeedbackType::MagicContinuousRestoration, "MagicContinuousRestoration_", 10);
		feedbackMap[FeedbackType::MagicContinuousOther] = Feedback(FeedbackType::MagicContinuousOther, "MagicContinuousOther_", 10);
		feedbackMap[FeedbackType::MagicContinuousPoison] = Feedback(FeedbackType::MagicContinuousPoison, "MagicContinuousPoison_", 10);
		feedbackMap[FeedbackType::Shout] = Feedback(FeedbackType::Shout, "Shout_", 100);
		feedbackMap[FeedbackType::HeartBeat] = Feedback(FeedbackType::HeartBeat, "HeartBeat_");
		feedbackMap[FeedbackType::HeartBeatFast] = Feedback(FeedbackType::HeartBeatFast, "HeartBeatFast_");
		feedbackMap[FeedbackType::GreybeardPowerAbsorb] = Feedback(FeedbackType::GreybeardPowerAbsorb, "GreybeardPowerAbsorb_", 100);
		feedbackMap[FeedbackType::DragonSoul] = Feedback(FeedbackType::DragonSoul, "DragonSoul_", 100);
		feedbackMap[FeedbackType::WordWall] = Feedback(FeedbackType::WordWall, "WordWall_", 100);
		feedbackMap[FeedbackType::PlayerSpellFireLeft] = Feedback(FeedbackType::PlayerSpellFireLeft, "PlayerSpellFireLeft_");
		feedbackMap[FeedbackType::PlayerSpellIceLeft] = Feedback(FeedbackType::PlayerSpellIceLeft, "PlayerSpellIceLeft_");
		feedbackMap[FeedbackType::PlayerSpellShockLeft] = Feedback(FeedbackType::PlayerSpellShockLeft, "PlayerSpellShockLeft_");
		feedbackMap[FeedbackType::PlayerSpellAlterationLeft] = Feedback(FeedbackType::PlayerSpellAlterationLeft, "PlayerSpellAlterationLeft_");
		feedbackMap[FeedbackType::PlayerSpellIllusionLeft] = Feedback(FeedbackType::PlayerSpellIllusionLeft, "PlayerSpellIllusionLeft_");
		feedbackMap[FeedbackType::PlayerSpellLightLeft] = Feedback(FeedbackType::PlayerSpellLightLeft, "PlayerSpellLightLeft_");
		feedbackMap[FeedbackType::PlayerSpellRestorationLeft] = Feedback(FeedbackType::PlayerSpellRestorationLeft, "PlayerSpellRestorationLeft_");
		feedbackMap[FeedbackType::PlayerSpellWardLeft] = Feedback(FeedbackType::PlayerSpellWardLeft, "PlayerSpellWardLeft_");
		feedbackMap[FeedbackType::PlayerSpellConjurationLeft] = Feedback(FeedbackType::PlayerSpellConjurationLeft, "PlayerSpellConjurationLeft_");
		feedbackMap[FeedbackType::PlayerSpellOtherLeft] = Feedback(FeedbackType::PlayerSpellOtherLeft, "PlayerSpellOtherLeft_");
		feedbackMap[FeedbackType::PlayerSpellFireRight] = Feedback(FeedbackType::PlayerSpellFireRight, "PlayerSpellFireRight_");
		feedbackMap[FeedbackType::PlayerSpellIceRight] = Feedback(FeedbackType::PlayerSpellIceRight, "PlayerSpellIceRight_");
		feedbackMap[FeedbackType::PlayerSpellShockRight] = Feedback(FeedbackType::PlayerSpellShockRight, "PlayerSpellShockRight_");
		feedbackMap[FeedbackType::PlayerSpellAlterationRight] = Feedback(FeedbackType::PlayerSpellAlterationRight, "PlayerSpellAlterationRight_");
		feedbackMap[FeedbackType::PlayerSpellIllusionRight] = Feedback(FeedbackType::PlayerSpellIllusionRight, "PlayerSpellIllusionRight_");
		feedbackMap[FeedbackType::PlayerSpellLightRight] = Feedback(FeedbackType::PlayerSpellLightRight, "PlayerSpellLightRight_");
		feedbackMap[FeedbackType::PlayerSpellRestorationRight] = Feedback(FeedbackType::PlayerSpellRestorationRight, "PlayerSpellRestorationRight_");
		feedbackMap[FeedbackType::PlayerSpellWardRight] = Feedback(FeedbackType::PlayerSpellWardRight, "PlayerSpellWardRight_");
		feedbackMap[FeedbackType::PlayerSpellConjurationRight] = Feedback(FeedbackType::PlayerSpellConjurationRight, "PlayerSpellConjurationRight_");
		feedbackMap[FeedbackType::PlayerSpellOtherRight] = Feedback(FeedbackType::PlayerSpellOtherRight, "PlayerSpellOtherRight_");
		feedbackMap[FeedbackType::PlayerPowerAttackLeft] = Feedback(FeedbackType::PlayerPowerAttackLeft, "PlayerPowerAttackLeft_");
		feedbackMap[FeedbackType::PlayerBashLeft] = Feedback(FeedbackType::PlayerBashLeft, "PlayerBashLeft_");
		feedbackMap[FeedbackType::PlayerAttackLeft] = Feedback(FeedbackType::PlayerAttackLeft, "PlayerAttackLeft_");
		feedbackMap[FeedbackType::PlayerPowerAttackRight] = Feedback(FeedbackType::PlayerPowerAttackRight, "PlayerPowerAttackRight_");
		feedbackMap[FeedbackType::PlayerBashRight] = Feedback(FeedbackType::PlayerBashRight, "PlayerBashRight_");
		feedbackMap[FeedbackType::PlayerAttackRight] = Feedback(FeedbackType::PlayerAttackRight, "PlayerAttackRight_");
		feedbackMap[FeedbackType::PlayerBlockLeft] = Feedback(FeedbackType::PlayerBlockLeft, "PlayerBlockLeft_");
		feedbackMap[FeedbackType::PlayerBlockRight] = Feedback(FeedbackType::PlayerBlockRight, "PlayerBlockRight_");
		feedbackMap[FeedbackType::PlayerBowPullLeft] = Feedback(FeedbackType::PlayerBowPullLeft, "PlayerBowPullLeft_");
		feedbackMap[FeedbackType::PlayerBowPullRight] = Feedback(FeedbackType::PlayerBowPullRight, "PlayerBowPullRight_");
		feedbackMap[FeedbackType::PlayerBowHoldLeft] = Feedback(FeedbackType::PlayerBowHoldLeft, "PlayerBowHoldLeft_");
		feedbackMap[FeedbackType::PlayerBowHoldRight] = Feedback(FeedbackType::PlayerBowHoldRight, "PlayerBowHoldRight_");
		feedbackMap[FeedbackType::PlayerShout] = Feedback(FeedbackType::PlayerShout, "PlayerShout_");
		feedbackMap[FeedbackType::Bite] = Feedback(FeedbackType::Bite, "Bite_");
		feedbackMap[FeedbackType::HipHolsterStoreLeft] = Feedback(FeedbackType::HipHolsterStoreLeft, "HipHolsterStoreLeft_");
		feedbackMap[FeedbackType::HipHolsterStoreRight] = Feedback(FeedbackType::HipHolsterStoreRight, "HipHolsterStoreRight_");
		feedbackMap[FeedbackType::HipHolsterRemoveLeft] = Feedback(FeedbackType::HipHolsterRemoveLeft, "HipHolsterRemoveLeft_");
		feedbackMap[FeedbackType::HipHolsterRemoveRight] = Feedback(FeedbackType::HipHolsterRemoveRight, "HipHolsterRemoveRight_");
		feedbackMap[FeedbackType::SleeveHolsterStoreLeft] = Feedback(FeedbackType::SleeveHolsterStoreLeft, "SleeveHolsterStoreLeft_");
		feedbackMap[FeedbackType::SleeveHolsterStoreRight] = Feedback(FeedbackType::SleeveHolsterStoreRight, "SleeveHolsterStoreRight_");
		feedbackMap[FeedbackType::SleeveHolsterRemoveLeft] = Feedback(FeedbackType::SleeveHolsterRemoveLeft, "SleeveHolsterRemoveLeft_");
		feedbackMap[FeedbackType::SleeveHolsterRemoveRight] = Feedback(FeedbackType::SleeveHolsterRemoveRight, "SleeveHolsterRemoveRight_");
		feedbackMap[FeedbackType::BackpackStoreLeft] = Feedback(FeedbackType::BackpackStoreLeft, "BackpackStoreLeft_");
		feedbackMap[FeedbackType::BackpackStoreRight] = Feedback(FeedbackType::BackpackStoreRight, "BackpackStoreRight_");
		feedbackMap[FeedbackType::BackpackRemoveLeft] = Feedback(FeedbackType::BackpackRemoveLeft, "BackpackRemoveLeft_");
		feedbackMap[FeedbackType::BackpackRemoveRight] = Feedback(FeedbackType::BackpackRemoveRight, "BackpackRemoveRight_");
		feedbackMap[FeedbackType::StomachStore] = Feedback(FeedbackType::StomachStore, "StomachStore_");
		feedbackMap[FeedbackType::StomachRemove] = Feedback(FeedbackType::StomachRemove, "StomachRemove_");
		feedbackMap[FeedbackType::ChestStore] = Feedback(FeedbackType::ChestStore, "ChestStore_");
		feedbackMap[FeedbackType::ChestRemove] = Feedback(FeedbackType::ChestRemove, "ChestRemove_");
		feedbackMap[FeedbackType::ConsumableDrink] = Feedback(FeedbackType::ConsumableDrink, "ConsumableDrink_");
		feedbackMap[FeedbackType::ConsumableFood] = Feedback(FeedbackType::ConsumableFood, "ConsumableFood_");

		feedbackMap[FeedbackType::ShoutFire] = Feedback(FeedbackType::ShoutFire, "ShoutFire_");
		feedbackMap[FeedbackType::ShoutFrost] = Feedback(FeedbackType::ShoutFrost, "ShoutFrost_");
		feedbackMap[FeedbackType::ShoutSteam] = Feedback(FeedbackType::ShoutSteam, "ShoutSteam_");
		feedbackMap[FeedbackType::ShoutLightning] = Feedback(FeedbackType::ShoutLightning, "ShoutLightning_");
		feedbackMap[FeedbackType::HiggsPullLeft] = Feedback(FeedbackType::HiggsPullLeft, "HiggsPullLeft_");
		feedbackMap[FeedbackType::HiggsPullRight] = Feedback(FeedbackType::HiggsPullRight, "HiggsPullRight_");
		feedbackMap[FeedbackType::PlayerEnvironmentHitLeft] = Feedback(FeedbackType::PlayerEnvironmentHitLeft, "PlayerEnvironmentHitLeft_", 0);
		feedbackMap[FeedbackType::PlayerEnvironmentHitRight] = Feedback(FeedbackType::PlayerEnvironmentHitRight, "PlayerEnvironmentHitRight_", 0);
		feedbackMap[FeedbackType::PlayerThrowLeft] = Feedback(FeedbackType::PlayerThrowLeft, "PlayerThrowLeft_");
		feedbackMap[FeedbackType::PlayerThrowRight] = Feedback(FeedbackType::PlayerThrowRight, "PlayerThrowRight_");
		feedbackMap[FeedbackType::PlayerCatchLeft] = Feedback(FeedbackType::PlayerCatchLeft, "PlayerCatchLeft_");
		feedbackMap[FeedbackType::PlayerCatchRight] = Feedback(FeedbackType::PlayerCatchRight, "PlayerCatchRight_");

		feedbackMap[FeedbackType::PlayerShoutBindHands] = Feedback(FeedbackType::PlayerShoutBindHands, "PlayerShoutBindHands_");
		feedbackMap[FeedbackType::PlayerShoutBindVest] = Feedback(FeedbackType::PlayerShoutBindVest, "PlayerShoutBindVest_");

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

		feedbackMap[FeedbackType::UnarmedDefault] = Feedback(FeedbackType::UnarmedDefault, "UnarmedDefault_", 20);
		feedbackMap[FeedbackType::UnarmedDragon] = Feedback(FeedbackType::UnarmedDragon, "UnarmedDragon_", 20);
		feedbackMap[FeedbackType::UnarmedFrostbiteSpider] = Feedback(FeedbackType::UnarmedFrostbiteSpider, "UnarmedFrostbiteSpider_", 20);
		feedbackMap[FeedbackType::UnarmedSabreCat] = Feedback(FeedbackType::UnarmedSabreCat, "UnarmedSabreCat_", 20);
		feedbackMap[FeedbackType::UnarmedSkeever] = Feedback(FeedbackType::UnarmedSkeever, "UnarmedSkeever_", 20);
		feedbackMap[FeedbackType::UnarmedSlaughterfish] = Feedback(FeedbackType::UnarmedSlaughterfish, "UnarmedSlaughterfish_", 20);
		feedbackMap[FeedbackType::UnarmedWisp] = Feedback(FeedbackType::UnarmedWisp, "UnarmedWisp_", 20);
		feedbackMap[FeedbackType::UnarmedDragonPriest] = Feedback(FeedbackType::UnarmedDragonPriest, "UnarmedDragonPriest_", 20);
		feedbackMap[FeedbackType::UnarmedDraugr] = Feedback(FeedbackType::UnarmedDraugr, "UnarmedDraugr_", 20);
		feedbackMap[FeedbackType::UnarmedWolf] = Feedback(FeedbackType::UnarmedWolf, "UnarmedWolf_", 20);
		feedbackMap[FeedbackType::UnarmedGiant] = Feedback(FeedbackType::UnarmedGiant, "UnarmedGiant_", 20);
		feedbackMap[FeedbackType::UnarmedIceWraith] = Feedback(FeedbackType::UnarmedIceWraith, "UnarmedIceWraith_", 20);
		feedbackMap[FeedbackType::UnarmedChaurus] = Feedback(FeedbackType::UnarmedChaurus, "UnarmedChaurus_", 20);
		feedbackMap[FeedbackType::UnarmedMammoth] = Feedback(FeedbackType::UnarmedMammoth, "UnarmedMammoth_", 20);
		feedbackMap[FeedbackType::UnarmedFrostAtronach] = Feedback(FeedbackType::UnarmedFrostAtronach, "UnarmedFrostAtronach_", 20);
		feedbackMap[FeedbackType::UnarmedFalmer] = Feedback(FeedbackType::UnarmedFalmer, "UnarmedFalmer_", 20);
		feedbackMap[FeedbackType::UnarmedHorse] = Feedback(FeedbackType::UnarmedHorse, "UnarmedHorse_", 20);
		feedbackMap[FeedbackType::UnarmedStormAtronach] = Feedback(FeedbackType::UnarmedStormAtronach, "UnarmedStormAtronach_", 20);
		feedbackMap[FeedbackType::UnarmedElk] = Feedback(FeedbackType::UnarmedElk, "UnarmedElk_", 20);
		feedbackMap[FeedbackType::UnarmedDwarvenSphere] = Feedback(FeedbackType::UnarmedDwarvenSphere, "UnarmedDwarvenSphere_", 20);
		feedbackMap[FeedbackType::UnarmedDwarvenSteam] = Feedback(FeedbackType::UnarmedDwarvenSteam, "UnarmedDwarvenSteam_", 20);
		feedbackMap[FeedbackType::UnarmedDwarvenSpider] = Feedback(FeedbackType::UnarmedDwarvenSpider, "UnarmedDwarvenSpider_", 20);
		feedbackMap[FeedbackType::UnarmedBear] = Feedback(FeedbackType::UnarmedBear, "UnarmedBear_", 20);
		feedbackMap[FeedbackType::UnarmedFlameAtronach] = Feedback(FeedbackType::UnarmedFlameAtronach, "UnarmedFlameAtronach_", 20);
		feedbackMap[FeedbackType::UnarmedWitchlight] = Feedback(FeedbackType::UnarmedWitchlight, "UnarmedWitchlight_", 20);
		feedbackMap[FeedbackType::UnarmedHorker] = Feedback(FeedbackType::UnarmedHorker, "UnarmedHorker_", 20);
		feedbackMap[FeedbackType::UnarmedTroll] = Feedback(FeedbackType::UnarmedTroll, "UnarmedTroll_", 20);
		feedbackMap[FeedbackType::UnarmedHagraven] = Feedback(FeedbackType::UnarmedHagraven, "UnarmedHagraven_", 20);
		feedbackMap[FeedbackType::UnarmedSpriggan] = Feedback(FeedbackType::UnarmedSpriggan, "UnarmedSpriggan_", 20);
		feedbackMap[FeedbackType::UnarmedMudcrab] = Feedback(FeedbackType::UnarmedMudcrab, "UnarmedMudcrab_", 20);
		feedbackMap[FeedbackType::UnarmedWerewolf] = Feedback(FeedbackType::UnarmedWerewolf, "UnarmedWerewolf_", 20);
		feedbackMap[FeedbackType::UnarmedChaurusFlyer] = Feedback(FeedbackType::UnarmedChaurusFlyer, "UnarmedChaurusFlyer_", 20);
		feedbackMap[FeedbackType::UnarmedGargoyle] = Feedback(FeedbackType::UnarmedGargoyle, "UnarmedGargoyle_", 20);
		feedbackMap[FeedbackType::UnarmedRiekling] = Feedback(FeedbackType::UnarmedRiekling, "UnarmedRiekling_", 20);
		feedbackMap[FeedbackType::UnarmedScrib] = Feedback(FeedbackType::UnarmedScrib, "UnarmedScrib_", 20);
		feedbackMap[FeedbackType::UnarmedSeeker] = Feedback(FeedbackType::UnarmedSeeker, "UnarmedSeeker_", 20);
		feedbackMap[FeedbackType::UnarmedMountedRiekling] = Feedback(FeedbackType::UnarmedMountedRiekling, "UnarmedMountedRiekling_", 20);
		feedbackMap[FeedbackType::UnarmedNetch] = Feedback(FeedbackType::UnarmedNetch, "UnarmedNetch_", 20);
		feedbackMap[FeedbackType::UnarmedBenthicLurker] = Feedback(FeedbackType::UnarmedBenthicLurker, "UnarmedBenthicLurker_", 20);

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

		feedbackMap[FeedbackType::HorseRiding] = Feedback(FeedbackType::HorseRiding, "HorseRiding_", 0);
		feedbackMap[FeedbackType::HorseRidingSlow] = Feedback(FeedbackType::HorseRidingSlow, "HorseRidingSlow_", 0);

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

		feedbackMap[FeedbackType::Wind] = Feedback(FeedbackType::Wind, "Wind_", 0);

		feedbackMap[FeedbackType::MagicArmorSpell] = Feedback(FeedbackType::MagicArmorSpell, "MagicArmorSpell_", 10);


		feedbackMap[FeedbackType::SpellWheelOpenLeft] = Feedback(FeedbackType::SpellWheelOpenLeft, "SpellWheelOpenLeft_");
		feedbackMap[FeedbackType::SpellWheelOpenRight] = Feedback(FeedbackType::SpellWheelOpenRight, "SpellWheelOpenRight_");

		feedbackMap[FeedbackType::Default] = Feedback(FeedbackType::Default, "Default_");
	}

	//=======================================================================================================================
	// So far these seem unused
	// =======================================================================================================================
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

	void ProvideDotFeedback(MusclesGroup muscle, int index, int intensity, int durationMillis)
	{
		_MESSAGE("ProvideDotFeedback");
		if (intensity < TOLERANCE)
			return;

		if (isGameStoppedNoDialogue())
		{
			return;
		}

		if (!systemInitialized)
			CreateSystem();
		std::string key;

		//std::vector<bhaptics::DotPoint> points;
		//bhaptics::DotPoint point = bhaptics::DotPoint(index, intensity);
		//points.emplace_back(point);
		//SubmitDot(key.c_str(), position, points, durationMillis);

		//TODO 
		//owo->Send(new Sensation())
	}
}

