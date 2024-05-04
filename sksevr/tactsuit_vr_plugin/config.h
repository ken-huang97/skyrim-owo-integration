#pragma once
#include <iostream>
#include <string>
#include <fstream>

#include "skse64\GameSettings.h"
#include "Utility.hpp"


namespace TactsuitVR {

	void loadConfig();
	
	extern int magicsleepduration;
	extern int bowsleepduration;

	extern int rainvestsleepduration;
	extern int rainheadsleepduration;
	extern int rainarmsleepduration;
	extern int raineffectduration;

	extern int swimmingsleepduration;

	extern int horseridingsleepduration;

	extern int leftHandedMode;
	extern int logging;

	extern int useShooterPosition;

	extern int processVisualEffects;

	extern int UseControllerHapticsForEnvironmentAttack;

	extern int WindEffectMinSpeed;

	extern float FallEffectMinHeight;

	extern float intensityMultiplierMagic;
	extern float intensityMultiplierShout;
	extern float intensityMultiplierRanged;

	extern float intensityMultiplierMeleePowerAttack;
	extern float intensityMultiplierMeleeBash;
	extern float intensityMultiplierMeleeAxe;
	extern float intensityMultiplierMeleeDagger;
	extern float intensityMultiplierMeleeMace;
	extern float intensityMultiplierMeleeSword;
	extern float intensityMultiplierMelee2HAxe;
	extern float intensityMultiplierMelee2HSword;
	extern float intensityMultiplierMeleeFist;

	extern float intensityMultiplierBite;
	extern float intensityMultiplierDefault;

	extern float intensityMultiplierHeartBeatFast;
	extern float intensityMultiplierHeartBeat;

	extern float intensityMultiplierWordWall;
	extern float intensityMultiplierDragonSoul;

	extern float intensityMultiplierRaindropVest;
	extern float intensityMultiplierRaindropArm;
	extern float intensityMultiplierRaindropHead;

	extern float intensityMultiplierPlayerShout;
	extern float intensityMultiplierPlayerSpell;
	extern float intensityMultiplierPlayerStaff;
	extern float intensityMultiplierPlayerBlock;
	extern float intensityMultiplierPlayerPowerAttack;
	extern float intensityMultiplierPlayerBash;
	extern float intensityMultiplierPlayerAttack;
	extern float intensityMultiplierPlayerBowPullRight;
	extern float intensityMultiplierPlayerBowPullLeft;
	extern float intensityMultiplierPlayerBowHold;
	extern float intensityMultiplierPlayerSwimming;
	extern float intensityMultiplierPlayerEnvironmentAttack;

	extern float intensityMultiplierHolster;

	extern float intensityMultiplierDrink;
	extern float intensityMultiplierEat;

	extern float intensityMultiplierShoutFire;
	extern float intensityMultiplierShoutFireball;
	extern float intensityMultiplierShoutFrost;
	extern float intensityMultiplierShoutSteam;
	extern float intensityMultiplierShoutLightning;
	extern float intensityMultiplierHiggsPull;
	extern float intensityMultiplierPlayerEnvironmentHit;
	extern float intensityMultiplierPlayerThrow;
	extern float intensityMultiplierPlayerCatch;

	extern float intensityMultiplierSpellWheelOpen;

	extern float intensityMultiplierTravelEffect;
	extern float intensityMultiplierTeleport;
	
	extern float intensityMultiplierPlayerShoutBind;
	
	extern float intensityMultiplierPlayerPower;

	extern float intensityMultiplierEnvironmentRumble;
	extern float intensityMultiplierDragonLanding;

	extern float intensityMultiplierEquipUnequip;

	extern float intensityMultiplierLearned;

	extern float intensityMultiplierArrowShoulderEquip;

	extern float intensityMultiplierExplosion;
	extern float intensityMultiplierEnvironmentalPoison;
	extern float intensityMultiplierEnvironmentalFire;
	extern float intensityMultiplierEnvironmentalShock;
	extern float intensityMultiplierEnvironmentalFrost;

	extern float intensityMultiplierUnarmedDefault		  ;
	extern float intensityMultiplierUnarmedDragon		  ;
	extern float intensityMultiplierUnarmedFrostbiteSpider;
	extern float intensityMultiplierUnarmedSabreCat		  ;
	extern float intensityMultiplierUnarmedSkeever		  ;
	extern float intensityMultiplierUnarmedSlaughterfish  ;
	extern float intensityMultiplierUnarmedWisp			  ;
	extern float intensityMultiplierUnarmedDragonPriest	  ;
	extern float intensityMultiplierUnarmedDraugr		  ;
	extern float intensityMultiplierUnarmedWolf			  ;
	extern float intensityMultiplierUnarmedGiant		  ;
	extern float intensityMultiplierUnarmedIceWraith	  ;
	extern float intensityMultiplierUnarmedChaurus		  ;
	extern float intensityMultiplierUnarmedMammoth		  ;
	extern float intensityMultiplierUnarmedFrostAtronach  ;
	extern float intensityMultiplierUnarmedFalmer		  ;
	extern float intensityMultiplierUnarmedHorse		  ;
	extern float intensityMultiplierUnarmedStormAtronach  ;
	extern float intensityMultiplierUnarmedElk			  ;
	extern float intensityMultiplierUnarmedDwarvenSphere  ;
	extern float intensityMultiplierUnarmedDwarvenSteam	  ;
	extern float intensityMultiplierUnarmedDwarvenSpider  ;
	extern float intensityMultiplierUnarmedBear			  ;
	extern float intensityMultiplierUnarmedFlameAtronach  ;
	extern float intensityMultiplierUnarmedWitchlight	  ;
	extern float intensityMultiplierUnarmedHorker		  ;
	extern float intensityMultiplierUnarmedTroll		  ;
	extern float intensityMultiplierUnarmedHagraven		  ;
	extern float intensityMultiplierUnarmedSpriggan		  ;
	extern float intensityMultiplierUnarmedMudcrab		  ;
	extern float intensityMultiplierUnarmedWerewolf		  ;
	extern float intensityMultiplierUnarmedChaurusFlyer	  ;
	extern float intensityMultiplierUnarmedGargoyle		  ;
	extern float intensityMultiplierUnarmedRiekling		  ;
	extern float intensityMultiplierUnarmedScrib		  ;
	extern float intensityMultiplierUnarmedSeeker;
	extern float intensityMultiplierUnarmedMountedRiekling;
	extern float intensityMultiplierUnarmedNetch		  ;
	extern float intensityMultiplierUnarmedBenthicLurker  ;


	extern float intensityMultiplierDefaultHead;
	extern float intensityMultiplierUnarmedHead;
	extern float intensityMultiplierRangedHead;
	extern float intensityMultiplierMeleeHead;
	extern float intensityMultiplierMagicHead;


	extern float intensityMultiplierMagicArm;
	extern float intensityMultiplierRangedArm;

	extern float intensityMultiplierHorseRidingSlow;
	extern float intensityMultiplierHorseRiding;

	extern float intensityMultiplierFallEffect;

	extern float intensityMultiplierWindEffect;
	extern float intensityMultiplierBlizzardEffect;

	extern std::vector<std::string> ignoredSpells;
	extern std::vector<std::string> shortSpells;

	extern float intensityMultiplierDrowningVest;
	extern float intensityMultiplierDrowningHead;

	extern float intensityMultiplierPlayerCrossbowFire;
	extern float intensityMultiplierPlayerCrossbowKickback;

	extern float intensityMultiplierMagicArmorSpell;

	
    extern float intensityMultiplierSoulTrap;
    extern float lowhealthpercentage;
    extern float verylowhealthpercentage;


	void Log(const int msgLogLevel, const char* fmt, ...);
	enum eLogLevels
	{
		LOGLEVEL_ERR = 0,
		LOGLEVEL_WARN,
		LOGLEVEL_INFO,
	};

#define LOG(fmt, ...) Log(LOGLEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) Log(LOGLEVEL_ERR, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) Log(LOGLEVEL_INFO, fmt, ##__VA_ARGS__)

}