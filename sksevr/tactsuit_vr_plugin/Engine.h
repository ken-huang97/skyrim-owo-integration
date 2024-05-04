#include "TactsuitVR.h"



#include "api/PapyrusVRAPI.h"
#include "api/VRManagerAPI.h"
#include "skse64\PapyrusSpell.h"
#include "skse64\PapyrusActor.h"
#include "skse64\PapyrusPotion.h"
#include "skse64\GameMenus.h"
#include "skse64_common/SafeWrite.h"
#include "Sky.h"
#include <thread>
#include <atomic>
#include <openvr.h>
#include <mutex>
#include <unordered_set>
#include <xbyak/xbyak.h>
#include "skse64/NiExtraData.h"
#include "skse64_common/BranchTrampoline.h"
#include <skse64/NiProperties.h>
#include <skse64/NiObjects.h>
#include "skse64/InternalTasks.h"

namespace DirectX
{
	struct XMFLOAT4X4
	{
	public:
		// members
		float m[4][4];
	};
	STATIC_ASSERT(sizeof(XMFLOAT4X4) == 0x40);
}
extern SKSETaskInterface* g_task;

namespace TactsuitVR {

#define NINODE_CHILDREN(ninode) ((NiTArray <NiAVObject *> *) ((char*)(&((ninode)->m_children))))
	extern bool systemInitialized;

	enum ProjectileHitLocation
	{
		Head,
		LeftArm,
		RightArm,
		LeftArmItem,
		RightArmItem,
		Body,
		Unknown
	};

	enum class MovementState { Grounded, Jumping, InAir, Unknown };
	MovementState vlibGetMovementState();

	// Functions inside SkyrimVR.exe for detecting player movement state //Prog's code
	typedef uintptr_t(__fastcall* _sub685270)(ActorProcessManager* processManager);
	extern RelocAddr <_sub685270> sub685270;
	typedef uintptr_t(__fastcall* _subAF4960)(uintptr_t hkpCharacterContext, unsigned int a2);
	extern RelocAddr <_subAF4960> subAF4960;
	
	void GameLoad();
	bool RegisterFuncs(VMClassRegistry* registry);

	void SetActorValueInfoHealth();

	void Heartbeat();

	bool isItWordWall(TESObjectREFR* ref);
	
	FeedbackType DecideMagicFeedbackType(SpellItem* spell);

	FeedbackType DecideShoutFeedbackType(std::string str);
		bool IsBlockingInternal(Actor* actor, TESObjectREFR* actorRef);
	void LeftHandedModeChange();

	void WordWallCheck();
	void WeatherCheck();

	bool GetWhichArm(Actor* player, TESObjectREFR* target);
	int CheckAttackLocation(Actor* player, Actor* actor, bool rightAttack);
	

	class HitEventHandler : public BSTEventSink <TESHitEvent>
	{
	public:
		virtual	EventResult ReceiveEvent(TESHitEvent * evn, EventDispatcher<TESHitEvent> * dispatcher);
	};

	extern EventDispatcher<TESHitEvent>* g_HitEventDispatcher;
	extern HitEventHandler g_HitEventHandler;


	class MYSKSEActionEvent : public BSTEventSink <SKSEActionEvent>
	{
	public:
		virtual	EventResult ReceiveEvent(SKSEActionEvent * evn, EventDispatcher<SKSEActionEvent> * dispatcher);
	};

	extern EventDispatcher<SKSEActionEvent>* g_skseActionEventDispatcher;
	extern MYSKSEActionEvent mySKSEActionEvent;


	class ContainerChangedEventHandler : public BSTEventSink <TESContainerChangedEvent>
	{
	public:
		virtual	EventResult ReceiveEvent(TESContainerChangedEvent * evn, EventDispatcher<TESContainerChangedEvent> * dispatcher);
	};

	extern EventDispatcher<TESContainerChangedEvent>* g_ContainerChangedEventDispatcher;
	extern ContainerChangedEventHandler g_ContainerChangedEventHandler;

	class ActiveEffectApplyRemoveEventEventHandler : public BSTEventSink <TESActiveEffectApplyRemoveEvent>
	{
	public:
		virtual	EventResult ReceiveEvent(TESActiveEffectApplyRemoveEvent* evn, EventDispatcher<TESActiveEffectApplyRemoveEvent>* dispatcher);
	};

	extern EventDispatcher<TESActiveEffectApplyRemoveEvent>* g_ActiveEffectApplyRemoveEventDispatcher;
	extern ActiveEffectApplyRemoveEventEventHandler g_ActiveEffectApplyRemoveEventHandler;
	
	class TESEquipEventHandler : public BSTEventSink <TESEquipEvent>
	{
	public:
		virtual	EventResult ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>* dispatcher);
	};

	extern EventDispatcher<TESEquipEvent>* g_TESEquipEventDispatcher;
	extern TESEquipEventHandler g_TESEquipEventHandler;


	class TESQuestStageEventHandler : public BSTEventSink <TESQuestStageEvent>
	{
	public:
		virtual	EventResult ReceiveEvent(TESQuestStageEvent* evn, EventDispatcher<TESQuestStageEvent>* dispatcher);
	};

	extern EventDispatcher<TESQuestStageEvent>* g_TESQuestStageEventDispatcher;
	extern TESQuestStageEventHandler g_TESQuestStageEventHandler;


	extern std::atomic<bool> raining;
	//Weather stuff to get if it's raining etc.
		
	void RaindropVestEffect();
	void RaindropArmEffect(bool left);
	void RaindropHeadEffect();
	

//From HapticSkyrimVR
	enum Hand
	{
		leftHand,
		rightHand
	};

	FeedbackType GetOtherHandSpellFeedback(FeedbackType feedback);
	
	std::vector<unsigned short>* DecideSpellType(SpellItem* spell, bool staff, bool left, FeedbackType& spellFeedBackType);
	void SpellCastingEventDecider(bool fireAndForget, bool dual, bool leftHand);
	void ReleaseBurst(bool dual, bool leftHand);

	FeedbackType GetHeadVersionOfMagic(FeedbackType feedback);
		bool DecideCastType(SpellItem* spell);
	bool GetDeliveryTypeSelf(SpellItem* spell);
	UInt32 GetSchool(SpellItem* spell);
	bool GetEffectTypeRecover(SpellItem* spell);
	

	void StartMagicEffectRight(std::vector<unsigned short>* magicArray, bool shortSpell, bool staff, FeedbackType spellFeedbackType, bool selfHealingSpell);

	void StartMagicEffectLeft(std::vector<unsigned short>* magicArray, bool shortSpell, bool staff, FeedbackType spellFeedbackType, bool selfHealingSpell);

	void StartHealingEffect(bool rightSpell);

	bool isSprinting();

	std::string getNodeDesc(NiAVObject* node);

	//void HealEffectFeedback();

	//void HealEffectWithMenuWait();
	
	void StartBowHoldEffect();

	void OnVRHapticEventListener(UInt32 axisId, UInt32 pulseDuration, PapyrusVR::VRDevice device);
	
	void OnVRButtonEventListener(PapyrusVR::VREventType eventType, PapyrusVR::EVRButtonId buttonId, PapyrusVR::VRDevice device);
		
	extern std::atomic<bool> rightMagic;
	extern std::atomic<bool> leftMagic;

	extern vr::ETrackedControllerRole leftControllerRole;
	extern vr::ETrackedControllerRole rightControllerRole;

	extern PapyrusVR::VRDevice leftController;
	extern PapyrusVR::VRDevice rightController;
		
	bool IsTriggerPressed(bool left);

	extern std::vector<unsigned short> fireArray1;

	extern std::vector<unsigned short> fireReadyArray1;

	extern std::vector<unsigned short> iceArray1;

	extern std::vector<unsigned short> iceReadyArray1;

	extern std::vector<unsigned short> shockArray1;

	extern std::vector<unsigned short> shockReadyArray1;

	extern std::vector<unsigned short> wardArray1;

	extern std::vector<unsigned short> alterationArray1;

	extern std::vector<unsigned short> alterationReadyArray1;

	extern std::vector<unsigned short> conjurationReadyArray1;

	extern std::vector<unsigned short> illusionArray1;

	extern std::vector<unsigned short> illusionReadyArray1;

	extern std::vector<unsigned short> lightReadyArray1;

	extern std::vector<unsigned short> restorationArray1;

	extern std::vector<unsigned short> restorationReadyArray1;


	extern std::vector<unsigned short> emptyArray;

	//HIGGS Integration
	void higgspull(bool isLeft, TESObjectREFR* pulledRefr);
	void higgsstash(bool isLeft, TESForm* stashedForm);
	void higgsconsume(bool isLeft, TESForm* consumedForm);
	void higgscollide(bool isLeft, float mass, float separatingVelocity);

	void RangedProjectileFeedback(NiPoint3 actorPos, NiPoint3 impactPosition, ProjectileHitLocation hitLocation);
	

	class BSTempEffect : public NiObject
	{
	public:

		virtual ~BSTempEffect();

		// members
		float		   lifetime;	 // 10
		std::uint32_t  pad14;		 // 14
		TESObjectCELL* cell;		 // 18
		float		   age;			 // 20
		bool		   initialized;	 // 24
		std::uint8_t   pad25;		 // 25
		std::uint16_t  pad26;		 // 26
		std::uint32_t  effectID;	 // 28
		std::uint32_t  pad2C;		 // 2C
	};
	STATIC_ASSERT(sizeof(BSTempEffect) == 0x30);


	class ReferenceEffect : public BSTempEffect
	{
	public:

		virtual ~ReferenceEffect();

		// members
		UInt32						controller;	// 30
		UInt32						targetRefHandle;		   // 38
		UInt32						aimAtTargetRefHandle;	   // 3C
		bool					   finished;	   // 40
		bool					   ownController;  // 41
		std::uint16_t			   pad42;		   // 42
		std::uint32_t			   pad44;		   // 44
	};
	STATIC_ASSERT(sizeof(ReferenceEffect) == 0x48);

	struct AttachTechniqueInput
	{
	public:
		virtual ~AttachTechniqueInput();  // 00

		// add
		virtual void Unk_01(void);	// 01

		// members
		void* unk08;  // 08 - smart ptr
		void* unk10;  // 10 - smart ptr
		std::uint32_t unk18;  // 18
		std::uint32_t unk1C;  // 1C
	};
	STATIC_ASSERT(sizeof(AttachTechniqueInput) == 0x20);

	class RefAttachTechniqueInput : public AttachTechniqueInput
	{
	public:
		virtual ~RefAttachTechniqueInput();	 // 00
		// members
		std::uint64_t unk20;  // 20
		std::uint64_t unk28;  // 28
		std::uint64_t unk30;  // 30
		std::uint32_t unk38;  // 38
		std::uint32_t unk3C;  // 3C
		BSFixedString unk40;  // 40
	};
	STATIC_ASSERT(sizeof(RefAttachTechniqueInput) == 0x48);
	

	class SimpleAnimationGraphManagerLoadingTask;

	class SimpleAnimationGraphManagerHolder : public IAnimationGraphManagerHolder
	{
	public:
		// add
		virtual void Unk_13(void);	// 13 - { return; }

		// members
		BSTSmartPointer<BSAnimationGraphManager>		  animationGraphManager;  // 08
		NiPointer<SimpleAnimationGraphManagerLoadingTask> loadingTask;			  // 10
	};
	STATIC_ASSERT(sizeof(SimpleAnimationGraphManagerHolder) == 0x18);

	class ModelReferenceEffect : public ReferenceEffect,
		public SimpleAnimationGraphManagerHolder,	// 48
		public BSTEventSink<BSAnimationGraphEvent>	// 60
	{
	public:

		virtual ~ModelReferenceEffect();  // 00
		// 
		// members
		RefAttachTechniqueInput hitEffectArtData;  // 68
		std::uint64_t			unkB0;			   // B0
		BGSArtObject*			artObject;		   // B8
		std::uint64_t			unkC0;			   // C0
		NiPointer<NiAVObject>	artObject3D;	   // C8
		std::uint64_t			unkD0;			   // D0
	};
	STATIC_ASSERT(offsetof(ModelReferenceEffect, artObject) == 0xB8);
	STATIC_ASSERT(sizeof(ModelReferenceEffect) == 0xD8);

	class AIProcessManager
	{
	public:
		static AIProcessManager* GetSingleton();

		// members
		bool						enableDetection;				// 001
		bool						enableDetectionStats;			// 002
		UInt8						pad003;							// 003
		UInt32						trackedDetectionHandle;			// 004
		bool						enableHighProcessing;			// 008
		bool						enableLowProcessing;			// 009
		bool						enableMiddleHighProcessing;		// 00A
		bool						enableMiddleLowProcessing;		// 00B
		UInt16						unk00C;							// 00C
		UInt8						unk00E;							// 00E
		UInt8						pad00F;							// 00F
		SInt32						numActorsInHighProcess;			// 010
		float						unk014;							// 014
		UInt32						unk018;							// 018
		float						removeExcessComplexDeadTime;	// 01C
		HANDLE						semaphore;						// 020
		UInt32						unk028;							// 028
		UInt32						pad02C;							// 02C
		tArray<UInt32>				highProcesses;					// 030
		tArray<UInt32>				lowProcesses;					// 048
		tArray<UInt32>				middleLowProcesses;				// 060
		tArray<UInt32>				middleHighProcesses;			// 078
		tArray<UInt32>* highProcessesPtr;				// 090
		tArray<UInt32>* lowProcessesPtr;				// 098
		tArray<UInt32>* middleLowProcessesPtr;			// 0A0
		tArray<UInt32>* middleHighProcessesPtr;			// 0A8
		UInt64						unk0B0;							// 0B0
		UInt64						unk0B8;							// 0B8
		UInt64						unk0C0;							// 0C0
		UInt64						unk0C8;							// 0C8
		UInt64						unk0D0;							// 0D0
		UInt64						unk0D8;							// 0D8
		UInt64						unk0E0;							// 0E0
		tArray<BSTempEffect*>		tempEffects;					// 0E8
		SimpleLock					tempEffectsLock;				// 100
		tArray<BSTempEffect*>	referenceEffects;				// 108
		SimpleLock					referenceEffectsLock;			// 120
		tArray<void*>				unk128;							// 128
		UInt64						unk140;							// 140
		UInt64						unk148;							// 148
		UInt64						unk150;							// 150
		tArray<UInt32>				unk158;							// 158
		UInt32						unk170;							// 170
		UInt32						pad174;							// 174
		UInt64						unk178;							// 178
		tArray<void*>				unk180;							// 180
		SimpleLock					unk198;							// 198
		tArray<UInt32>				unk1A0;							// 1A0
		tArray<void*>				unk1B8;							// 1B8
		float						unk1D0;							// 1D0
		float						unk1D4;							// 1D4
		UInt64						unk1D8;							// 1D8
		UInt32						unk1E0;							// 1E0
		bool						enableAIProcessing;				// 1E4
		bool						enableMovementProcessing;		// 1E5
		bool						enableAnimationProcessing;		// 1E6
		UInt8						unk1E7;							// 1E7
		UInt64						unk1E8;							// 1E8


		MEMBER_FN_PREFIX(AIProcessManager);
		DEFINE_MEMBER_FN(StopArtObject, void, 0x007048E0, TESObjectREFR*, BGSArtObject*);

	};
	STATIC_ASSERT(sizeof(AIProcessManager) == 0x1F0);


	class ActorCause
	{
	public:
		SInt32	DecRefCount();
		SInt32	IncRefCount();
		SInt32	GetRefCount() const;


		// members
		UInt32		actor;			// 00
		NiPoint3				origin;			// 04
		UInt32					actorCauseID;	// 10
		volatile mutable SInt32	refCount;		// 14
	};
	STATIC_ASSERT(sizeof(ActorCause) == 0x18);



	enum class COL_LAYER
	{
		kUnidentified = 0,
		kStatic = 1,
		kAnimStatic = 2,
		kTransparent = 3,
		kClutter = 4,
		kWeapon = 5,
		kProjectile = 6,
		kSpell = 7,
		kBiped = 8,
		kTrees = 9,
		kProps = 10,
		kWater = 11,
		kTrigger = 12,
		kTerrain = 13,
		kTrap = 14,
		kNonCollidable = 15,
		kCloudTrap = 16,
		kGround = 17,
		kPortal = 18,
		kDebrisSmall = 19,
		kDebrisLarge = 20,
		kAcousticSpace = 21,
		kActorZone = 22,
		kProjectileZone = 23,
		kGasTrap = 24,
		kShellCasting = 25,
		kTransparentWall = 26,
		kInvisibleWall = 27,
		kTransparentSmallAnim = 28,
		kClutterLarge = 29,
		kCharController = 30,
		kStairHelper = 31,
		kDeadBip = 32,
		kBipedNoCC = 33,
		kAvoidBox = 34,
		kCollisionBox = 35,
		kCameraSphere = 36,
		kDoorDetection = 37,
		kConeProjectile = 38,
		kCamera = 39,
		kItemPicker = 40,
		kLOS = 41,
		kPathingPick = 42,
		kUnused0 = 43,
		kUnused1 = 44,
		kSpellExplosion = 45,
		kDroppingPick = 46
	};

	// Must be aligned to 16 bytes (128 bits) as it's a simd type
	__declspec(align(16)) struct hkVector4
	{
		float x;
		float y;
		float z;
		float w;
	};
	STATIC_ASSERT(sizeof(hkVector4) == 0x10);

	struct hkTransform
	{
		float m_rotation[12]; // 00 - 3x4 matrix, 3 rows of hkVector4
		hkVector4 m_translation; // 30
	};
	STATIC_ASSERT(sizeof(hkTransform) == 0x40);

	struct hkSweptTransform
	{
		hkVector4 m_centerOfMass0; // 00
		hkVector4 m_centerOfMass1; // 10
		hkVector4 m_rotation0; // 20 - Quaternion
		hkVector4 m_rotation1; // 30 - Quaternion
		hkVector4 m_centerOfMassLocal; // 40 - Often all 0's
	};
	STATIC_ASSERT(sizeof(hkSweptTransform) == 0x50);

	struct hkMotionState
	{
		hkTransform m_transform; // 00
		hkSweptTransform m_sweptTransform; // 40

		hkVector4 m_deltaAngle; // 90
		float m_objectRadius; // A0
		float m_linearDamping; // A4
		float m_angularDamping; // A8
								// These next 2 are hkUFloat8, 8-bit floats
		UInt8 m_maxLinearVelocity; // AC
		UInt8 m_maxAngularVelocity; // AD
		UInt8 m_deactivationClass; // AE
		UInt8 padAF;
	};
	STATIC_ASSERT(sizeof(hkMotionState) == 0xB0);

	struct hkArray
	{
		void* m_data;
		int m_size;
		int m_capacityAndFlags;
	};
	STATIC_ASSERT(sizeof(hkArray) == 0x10);

	struct hkpTypedBroadPhaseHandle
	{
		// Inherited from hkpBroadPhaseHandle
		UInt32 m_id; // 00

		SInt8 m_type; // 04
		SInt8 m_ownerOffset; // 05
		SInt8 m_objectQualityType; // 06
		UInt8 pad07;
		UInt32 m_collisionFilterInfo; // 08
	};
	STATIC_ASSERT(sizeof(hkpTypedBroadPhaseHandle) == 0x0C);

	struct hkpCdBody
	{
		void* m_shape; // 00
		UInt32 m_shapeKey; // 08
		UInt32 pad0C;
		void* m_motion; // 10
		hkpCdBody* m_parent; // 18
	};
	STATIC_ASSERT(sizeof(hkpCdBody) == 0x20);

	struct hkpCollidable : public hkpCdBody
	{
		struct BoundingVolumeData
		{
			UInt32 m_min[3]; // 00
			UInt8 m_expansionMin[3]; // 0C
			UInt8 m_expansionShift; // 0F
			UInt32 m_max[3]; // 10
			UInt8 m_expansionMax[3]; // 1C
			UInt8 m_padding; // 1F
			UInt16 m_numChildShapeAabbs; // 20
			UInt16 m_capacityChildShapeAabbs; // 22
			UInt32 pad24;
			void* m_childShapeAabbs; // 28 - it's a hkAabbUint32 *
			UInt32* m_childShapeKeys; // 30
		};
		STATIC_ASSERT(sizeof(BoundingVolumeData) == 0x38);

		SInt8 m_ownerOffset; // 20
		SInt8 m_forceCollideOntoPpu; // 21
		SInt16 m_shapeSizeOnSpu; // 22
		hkpTypedBroadPhaseHandle m_broadPhaseHandle; // 24
		BoundingVolumeData m_boundingVolumeData; // 30
		float m_allowedPenetrationDepth; // 68
		UInt32 pad6C;
	};
	STATIC_ASSERT(sizeof(hkpCollidable) == 0x70);

	struct hkpLinkedCollidable : public hkpCollidable
	{
		hkArray m_collisionEntries; // 70
	};
	STATIC_ASSERT(sizeof(hkpLinkedCollidable) == 0x80);

	struct hkpSimpleShapePhantom
	{
		void* vtbl; // 00
		// These 3 inherited from hkReferencedObject
		UInt16 m_memSizeAndFlags; // 08
		SInt16 m_referenceCount; // 0A
		UInt32 pad0C; // 0C

		void* world; // 10

		void* userData; // 18

		hkpLinkedCollidable m_collidable; // 20

		UInt64 todo[10];

		hkMotionState m_motionState; // F0

									 // more...
	};
	STATIC_ASSERT(offsetof(hkpSimpleShapePhantom, m_motionState) == 0xF0);

	struct bhkSimpleShapePhantom
	{
		void* vtbl; // 00
		volatile SInt32    m_uiRefCount;    // 08
		UInt32    pad0C;    // 0C

		hkpSimpleShapePhantom* phantom; // 10
	};

	struct bhkCollisionObject : NiRefObject
	{
		NiNode* node; // 10 - points back to the NiNode pointing to this
		UInt64 unk18; // bit 3 is set => we should update rotation of NiNode?
		//bhkRigidBody* body; // 20
		// more?
	};
	STATIC_ASSERT(offsetof(bhkCollisionObject, node) == 0x10);



	class Projectile : public TESObjectREFR
	{
	public:

		struct ImpactData
		{
		public:
			// members
			NiPoint3						  position;	 // 00
			NiPoint3						  rotation;	 // 0C
			UInt32						  collidee;	 // 18
			UInt64						  colObj;	 // 20
			BGSMaterialType*			  materialType;	 // 28
			UInt32						  unk30;
			UInt32						  unk34;
			NiNode*						  node;	 // 38 //NiNode?
			UInt64						  unk40;
			UInt32						  unk48;
			UInt32						  unk4C;
			
		};
		STATIC_ASSERT(offsetof(ImpactData, node) == 0x38);
		STATIC_ASSERT(offsetof(ImpactData, unk40) == 0x40);
		STATIC_ASSERT(offsetof(ImpactData, unk48) == 0x48);
		STATIC_ASSERT(offsetof(ImpactData, unk4C) == 0x4C);
		STATIC_ASSERT(sizeof(ImpactData) == 0x50);

		enum class CastingSource
		{
			kLeftHand = 0,
			kRightHand = 1,
			kOther = 2,
			kInstant = 3
		};
		
		// A8
		class LaunchData
		{
		public:
			virtual ~LaunchData();

			UInt8	unk08[0xA0]; // 08 TODO                                                             
		};

		tList<ImpactData*>			impacts;			   // 098
		float					   unk0A8;			   // 0A8
		float					   unk0AC;			   // 0AC
		UInt64						unk0B0;				// 0B0
		float						unk0B8;				// 0B8
		float						unk0BC;				// 0BC
		UInt64						unk0C0;				// 0C0
		float						unk0C8;				// 0C8
		float						unk0CC;				// 0CC
		UInt64						unk0D0;				// 0D0
		float						unk0D8;				// 0D8
		float						unk0DC;				// 0DC
		bhkSimpleShapePhantom* phantom;

		UInt8	unkE8[0xF0 - 0xE8];
		NiPoint3 point;
		NiPoint3 velocity;
		void* unk108;				// 108 - smart ptr
		void* unk110;				// 110 - smart ptr
		UInt8	unk118[0x120 - 0x118];
		UInt32				shooter;			// 120
		UInt32				desiredTarget;			// 124
		UInt8	unk124[0x140 - 0x128];
		UInt32* unk140;				// 140
		InventoryEntryData* unk148;				// 148
		BGSExplosion* explosion;		   // 150
		MagicItem* spell;			   // 158
		CastingSource castingSource;	   // 160
		std::uint32_t			   pad164;			   // 164
		EffectSetting* magicEffect;		// 168
		UInt8	unk170[0x178 - 0x170];
		UInt64						unk178;				// 178
		UInt64						unk180;				// 180
		float						unk188;				// 188
		float						unk18C;				// 18C
		float						range;				// 190
		UInt32						unk194;				// 194
		float						unk198;				// 198
		float						unk19C;				// 19C
		UInt64						unk1A0;				// 1A0
		UInt64						unk1A8;				// 1A8
		TESObjectWEAP* weaponSource;		// 1B0
		TESAmmo* ammoSource;			// 1B8
		float						distanceMoved;		// 1C0
		UInt32						unk1C4;				// 1C4
		UInt32						unk1C8;				// 1C8
		UInt32						flags;				// 1CC
		UInt64						unk1D0;				// 1D0
	};
	STATIC_ASSERT(offsetof(Projectile, point) == 0xF0);
	STATIC_ASSERT(offsetof(Projectile, velocity) == 0xFC);
	STATIC_ASSERT(offsetof(Projectile, shooter) == 0x120);
	STATIC_ASSERT(offsetof(Projectile, phantom) == 0xE0);
	STATIC_ASSERT(offsetof(Projectile, impacts) == 0x98);
	STATIC_ASSERT(offsetof(Projectile, distanceMoved) == 0x1C0);
	STATIC_ASSERT(sizeof(Projectile) == 0x1D8);


	void UpdateProjectile(Projectile* proj, float delta);

	void PerformHooks();


	typedef bool(*_IsInCombatNative)(Actor* actor);

	typedef void(*GetVelocityOriginalFunctionProjectile)(Projectile* ref, float delta);

	void GetVelocity_HookProjectile(Projectile* ref, float delta);

	int64_t OnProjectileHitFunctionHooked(Projectile* akProjectile, TESObjectREFR* akTarget, NiPoint3* point, uintptr_t unk1, UInt32 unk2, UInt8 unk3);

	struct Capsule
	{
		NiPoint3 center; // Center of the capsule
		float height;    // Height of the capsule
		float radius;    // Radius of the capsule
	};

	class BSRenderPass
	{
	public:
		struct LODMode
		{
			std::uint8_t index : 7;
			bool         singleLevel : 1;
		};
		STATIC_ASSERT(sizeof(LODMode) == 0x1);

		// members
		void* shader;            // 00
		BSShaderProperty* shaderProperty;    // 08
		BSGeometry* geometry;          // 10
		std::uint32_t     passEnum;          // 18
		std::uint8_t      accumulationHint;  // 1C
		std::uint8_t      extraParam;        // 1D
		LODMode           LODMode;           // 1E
		std::uint8_t      numLights;         // 1F
		std::uint16_t     unk20;             // 20
		BSRenderPass* next;              // 28
		BSRenderPass* passGroupNext;     // 30
		void** sceneLights;       // 38
		std::uint32_t     cachePoolId;       // 40
		std::uint32_t     pad44;             // 44
	};
	STATIC_ASSERT(sizeof(BSRenderPass) == 0x48);

	class BSParticleShaderProperty;

	class BSParticleShaderEmitter : public NiRefObject
	{
	public:
		enum class EMITTER_TYPE
		{
			kGeometry = 0,
			kCollision = 1,
			kRay = 2,
			kSnow = 3,
			kRain = 4
		};

		class TextureAnimInfo
		{
		public:
			// members
			float         currentFrame;  // 00
			std::uint32_t totalFrames;   // 04
		};
		STATIC_ASSERT(sizeof(TextureAnimInfo) == 0x08);

		struct ParticleData
		{
		public:
			// members
			float        XPos;           // 00
			float        YPos;           // 04
			float        ZPos;           // 08
			float        age;            // 0C
			float        XVel;           // 10
			float        YVel;           // 14
			float        ZVel;           // 18
			float        lifeAdjust;     // 1C
			float        rotationStart;  // 20
			float        rotationSpeed;  // 24
			float        speedMult;      // 28
			std::uint8_t texCoordU;      // 29
			std::uint8_t texCoordV;      // 2A
			std::uint8_t vertexX;        // 2B
			std::uint8_t vertexY;        // 2C
		};
		STATIC_ASSERT(sizeof(ParticleData) == 0x30);

		// members
		BSParticleShaderProperty* property;          // 10
		std::uint16_t									 emitterType;       // 18
		std::uint16_t                                 particleCount;     // 1A
		float                                         alpha;             // 1C
		float                                         generateReminder;  // 20
		float                                         maxParticleRatio;  // 24
		TextureAnimInfo* textureAnims;      // 28
		ParticleData                                  instanceData[78];  // 30
	};
	STATIC_ASSERT(sizeof(BSParticleShaderEmitter) == 0xED0);

	class BSParticleShaderCubeEmitter : public BSParticleShaderEmitter
	{
	public:

		// members
		alignas(0x10) DirectX::XMFLOAT4X4 occlusionProjection;  // ED0
		NiPoint3 cameraOffsetVector;                            // F10
		NiPoint3 offsetVector;                                  // F1C
		NiPoint3 compositeOffsetVector;                         // F28
		NiPoint3 frameVelocityVector;                           // F34
		NiPoint3 windVelocity;                                  // F40
		NiPoint3 gravityVelocity;                               // F4C
		float    rotation;                                      // F58
		float    rotationVelocity;                              // F5C
		float    cubeSize;                                      // F60
		float    density;                                       // F64
		NiPoint3 instanceOffsets[10];                           // F68
	};
	STATIC_ASSERT(sizeof(BSParticleShaderCubeEmitter) == 0xFE0);

	class BSParticleShaderGeometry;

	class BSParticleShaderProperty : public BSShaderProperty
	{
	public:                                                                               // 37 - { return particleShaderTexture; }

		// members
		bool                                         useWorldSpace;                    // 088
		bool                                         particleGreyscaleAlpha;           // 08A
		std::uint8_t                                 unk8B;                            // 08B
		float                                        particleLifetime;                 // 08C
		float                                        particleLifetimeVariance;         // 090
		float                                        initialSpeedAlongNormal;          // 094
		float                                        initialSpeedAlongNormalVariance;  // 098
		float                                        accelerationAlongNormal;          // 09C
		float                                        initialVelocity1;                 // 0A0
		float                                        initialVelocity2;                 // 0A4
		float                                        intialVelocity3;                  // 0A8
		float                                        acceleration1;                    // 0AC
		float                                        acceleration2;                    // 0B0
		float                                        acceleration3;                    // 0B4
		float                                        birthPositionOffset;              // 0B8
		float                                        birthPositionVarianceOffset;      // 0BC
		float                                        particleShaderInitialRotation;    // 0C0
		float                                        intialRotationVariance;           // 0C4
		float                                        rotationSpeed;                    // 0C8
		float                                        rotationSpeedVariance;            // 0CC
		bool                                         unk0D0;                           // 0D0
		std::uint8_t                                 pad0D1;                           // 0D1
		std::uint16_t                                pad0D2;                           // 0D2
		float                                        animatedStartFrame;               // 0D4
		float                                        animatedStartFrameVariance;       // 0D8
		float                                        animatedEndFrame;                 // 0DC
		float                                        animatedEndFrameVariance;         // 0E0
		float                                        animatedLoopStartVariance;        // 0E4
		float                                        animatedFrameCount;               // 0E8
		float                                        animatedFrameCountVariance;       // 0EC
		std::uint32_t                                colorScale;                       // 0F0
		float                                        colorKey1Time;                    // 0F4
		float                                        colorKey2Time;                    // 0F8
		float                                        colorKey3Time;                    // 0FC
		NiColorA                                     colorKey1;                        // 100
		NiColorA                                     colorKey2;                        // 110
		NiColorA                                     colorKey3;                        // 120
		float                                        scaleKey1;                        // 130
		float                                        scaleKey2;                        // 134
		float                                        scaleKey1Time;                    // 138
		float                                        scaleKey2Time;                    // 13C
		NiPointer<NiSourceTexture>                   particleShaderTexture;            // 140
		NiPointer<NiSourceTexture>                   particlePaletteTexture;           // 148
		tArray<NiPointer<BSParticleShaderEmitter>> particleEmitters;                 // 150
		NiPointer<BSParticleShaderGeometry>          particleShaderGeometry;           // 168
		float                                        unk170;                           // 170
		float                                        textureCountU;                    // 174
		float                                        textureCountV;                    // 178
		NiPoint3                                     windPoint;                        // 17C
		float                                        explosionWindSpeed;               // 188
		std::uint32_t                                unk18C;                           // 18C
		BSParticleShaderEmitter* particleEmitter;                  // 190
	};
	STATIC_ASSERT(sizeof(BSParticleShaderProperty) == 0x198);


	extern bool locationalDamageInstalled;
	extern SKSETrampolineInterface* g_trampolineInterface;

#define ONPROJECTILEHIT_HOOKLOCATION							0x00777E2A  // in VR, this is not called from HealthDamageFunctor_CTOR, so we will try to call it from BeamProjectile_vf_sub_140777A30 instead
#define ONPROJECTILEHIT_INNERFUNCTION							0x0077E4E0

	extern UInt64 directAddressX;

	typedef int64_t(*_OnProjectileHitFunction)(Projectile* akProjectile, TESObjectREFR* akTarget, NiPoint3* point, uintptr_t unk1, UInt32 unk2, UInt8 unk3);
	extern RelocAddr<_OnProjectileHitFunction> OnProjectileHitFunction;
	extern RelocAddr<uintptr_t> OnProjectileHitHookLocation;

	typedef NiAVObject* (*_FindCollidableNode)(hkpCollidable* a_collidable);
	extern RelocAddr<_FindCollidableNode> FindCollidableNode;

	// The Skyrim function to determine if an actor is mounted
	typedef int64_t(__fastcall* sub240690)(Actor* actor);
	extern RelocAddr <sub240690> sub_240690;

	bool vlibIsMounted();
	bool vlibIsOnCart();

	class BSMultiBoundShape : public NiObject
	{
	public:
		virtual ~BSMultiBoundShape();  // 00

		// members
		std::uint32_t unk10;  // 10
		std::uint32_t pad14;  // 14
	};
	STATIC_ASSERT(sizeof(BSMultiBoundShape) == 0x18);
	
	class BSMultiBoundAABB : public BSMultiBoundShape
	{
	public:
		virtual ~BSMultiBoundAABB();  // 00

		// members
		std::uint32_t pad18;   // 18
		NiPoint3	  center;  // 1C - world coordinates 
		std::uint32_t pad28;   // 28
		NiPoint3	  size;	   // 2C
		std::uint32_t pad38;   // 38
		std::uint32_t pad3C;   // 3C
	};
	STATIC_ASSERT(sizeof(BSMultiBoundAABB) == 0x40);

	class TESWaterDisplacement;
	class TESWaterNormals;
	class TESWaterReflections;
	class WadingWaterData;

	class NiPlane
	{
	public:
		// members
		NiPoint3 normal;	// 00
		float	 constant;	// 0C
	};
	STATIC_ASSERT(sizeof(NiPlane) == 0x10);
	
	class TESWaterObject : public NiRefObject
	{
	public:
		virtual ~TESWaterObject();	// 00


		// members
		NiPlane							plane;		   // 10
		NiPointer<BSTriShape>			shape;		   // 20
		NiPointer<NiAVObject>			fadeNode;	   // 28
		TESWaterForm* waterForm;	   // 30
		std::uint64_t					unk38;		   // 38
		NiPointer<TESWaterReflections>	reflections;   // 40
		NiPointer<TESWaterDisplacement> displacement;  // 48
		NiPointer<TESWaterNormals>		normals;	   // 50
		tArray<BSMultiBoundAABB*>		multiBounds;   // 58
		std::uint8_t					flags;		   // 70
		std::uint8_t					pad71;		   // 71
		std::uint16_t					pad72;		   // 72
		std::uint32_t					pad74;		   // 74
	};
	STATIC_ASSERT(sizeof(TESWaterObject) == 0x78);

	class BGSWaterSystemManager
	{
	public:
		static BGSWaterSystemManager* GetSingleton();

		// members
		std::uint32_t								 pad00;					   // 000
		BSFixedString* type;					   // 008
		std::uint32_t								 unk10;					   // 010
		std::uint32_t								 pad014;				   // 014
		std::uint32_t								 unk18;					   // 018
		std::uint32_t								 unk1C;					   // 01C
		tArray<TESWaterObject*>					 waterObjects;			   // 020
		tArray<void*>								 unk038;				   // 038 - TESWaterReflection
		tArray<void*>								 unk050;				   // 050
		tArray<void*>								 unk068;				   // 068
		std::uint32_t								 unk080;				   // 080
		NiPoint2									 unk084;				   // 084
		std::uint32_t								 pad08C;				   // 08C
		NiPointer<NiNode>							 proceduralWaterNode;	   // 090
		std::uint64_t								 unk098;				   // 098
		std::uint32_t								 reflectionExteriorCount;  // 0A0
		std::uint32_t								 reflectionInteriorCount;  // 0A4
		TESWorldSpace* worldSpace;			   // 0A8
		bool										 enabled;				   // 0B0
		std::uint8_t								 pad0B1;				   // 0B1
		std::uint16_t								 pad0B2;				   // 0B2
		std::uint32_t								 unk0B4;				   // 0B4
		std::uint8_t								 unk0B8;				   // 0B8
		std::uint8_t								 pad0B9;				   // 0B9
		std::int8_t									 unk0BA;				   // 0BA
		std::uint8_t								 pad0BB;				   // 0BB
		std::uint32_t								 unk0BC;				   // 0BC
		float										 pad0C0;				   // 0C0
		bool										 waterRadiusState;		   // 0C4
		std::uint8_t								 pad0C5;				   // 0C5
		std::uint8_t								 unk0C6;				   // 0C6
		bool										 showProcedualWater;	   // 0C7
		std::uint8_t								 unk0C8;				   // 0C8
		std::uint8_t								 padC9;					   // 0C9
		std::uint16_t								 padCA;					   // 0CA
		std::uint32_t								 unk0CC;				   // 0CC
		std::uint32_t								 unk0D0;				   // 0D0
		float										 unk0D4;				   // 0D4
		NiTPointerMap<UInt32, WadingWaterData*> wadingWaterData;		   // 0D8
		std::uint64_t								 unk0F8;				   // 0F8
		std::uint64_t								 unk100;				   // 108
		std::uint32_t								 unk108;				   // 108
		std::uint32_t								 pad10C;				   // 10C
		std::uint8_t								 unk110;				   // 110
		std::uint8_t								 pad111;				   // 111
		std::uint16_t								 pad112;				   // 112
		std::uint32_t								 pad114;				   // 114
		TESObjectCELL* unk118;				   // 118
		mutable SimpleLock							 lock;					   // 120
		NiPointer<BSTriShape>						 water2048;				   // 128
	};
	STATIC_ASSERT(sizeof(BGSWaterSystemManager) == 0x130);


	void CheckAndPlayMagicArmorSpellEffect(SpellItem* spell);

	///////////////////////////////////////
	//void SkyVRaanHooks();

	//typedef UInt32* (__fastcall* TESWaterReflections_CTOR)(UInt32* Src, UInt32* a2, UInt32* a3, __int16 flags, const char* staticReflectionTexture);

	//UInt32* TESWaterReflectionsFunc_Hook(UInt32* Src, UInt32* a2, UInt32* a3, __int16 flags, const char* staticReflectionTexture);
	////////////////////////////////

	using hkpShapeKey = std::uint32_t;
	constexpr hkpShapeKey HK_INVALID_SHAPE_KEY = static_cast<hkpShapeKey>(-1);

	class hkClass;
	class hkStatisticsCollector;
	struct hkpCollidable;

	class hkBaseObject
	{
	public:
		virtual ~hkBaseObject() = default;  // 00
	};
	STATIC_ASSERT(sizeof(hkBaseObject) == 0x8);

	class hkpCollidableCollidableFilter
	{
	public:
		virtual ~hkpCollidableCollidableFilter();  // 00

		// add
		virtual bool IsCollisionEnabled(const hkpCollidable& a_collidableA, const hkpCollidable& a_collidableB) const = 0;  // 01
	};
	STATIC_ASSERT(sizeof(hkpCollidableCollidableFilter) == 0x8);


	class hkReferencedObject : public hkBaseObject
	{
	public:
		enum class LockMode
		{
			kNone = 0,
			kAuto,
			kManual
		};

		enum
		{
			kMemSize = 0x7FFF
		};

		hkReferencedObject();

		// add
		virtual const hkClass* GetClassType() const;                                                                     // 01 - { return 0; }
		virtual void           CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const;  // 02

		void         AddReference() const;
		std::int32_t GetAllocatedSize() const;
		std::int32_t GetReferenceCount() const;
		void         RemoveReference() const;

		// members
		std::uint16_t                 memSizeAndFlags;  // 08
		volatile mutable std::int16_t referenceCount;   // 0A
		std::uint32_t                 pad0C;            // 0C
	};
	STATIC_ASSERT(sizeof(hkReferencedObject) == 0x10);

	class hkpBvTreeShape;
	class hkpWorld;
	class hkpBvTreeShape;
	struct hkpCdBody;
	class hkpShapeContainer;

	struct hkpCollisionInput;
	class hkpShapeContainer;

	struct hkpShapeRayCastInput;
	struct hkpCollidable;

	struct hkpWorldRayCastInput;

	class hkAabb;

	class hkpRayCollidableFilter
	{
	public:
		virtual ~hkpRayCollidableFilter();  // 00

		// add
		virtual bool IsCollisionEnabled(const hkpWorldRayCastInput& a_input, const hkpCollidable& a_collidable) const = 0;  // 01
	};
	STATIC_ASSERT(sizeof(hkpRayCollidableFilter) == 0x08);

	class hkpRayShapeCollectionFilter
	{
	public:
		// add
		virtual bool IsCollisionEnabled(const hkpShapeRayCastInput& a_input, const hkpShapeContainer& a_container, hkpShapeKey a_key) const = 0;  // 00

		virtual ~hkpRayShapeCollectionFilter();  // 01
	};
	STATIC_ASSERT(sizeof(hkpRayShapeCollectionFilter) == 0x08);

	class hkpShapeCollectionFilter
	{
	public:
		// add
		virtual bool         IsCollisionEnabled1(const hkpCollisionInput& a_input, const hkpCdBody& a_bodyA, const hkpCdBody& a_bodyB, const hkpShapeContainer& a_shapeB, hkpShapeKey a_key) const = 0;                                                                      // 00
		virtual bool         IsCollisionEnabled2(const hkpCollisionInput& a_input, const hkpCdBody& a_bodyA, const hkpCdBody& a_bodyB, const hkpShapeContainer& a_shapeA, const hkpShapeContainer& a_shapeB, hkpShapeKey a_keyA, hkpShapeKey a_keyB) const = 0;              // 01
		virtual std::int32_t NumShapeKeyHitsLimitBreached(const hkpCollisionInput& a_input, const hkpCdBody& a_bodyA, const hkpCdBody& a_bodyB, const hkpBvTreeShape* a_shapeB, hkAabb& a_AABB, hkpShapeKey* a_shapeKeysInOut, std::int32_t a_shapeKeysCapacity) const = 0;  // 02

		virtual ~hkpShapeCollectionFilter();  // 03
	};
	STATIC_ASSERT(sizeof(hkpShapeCollectionFilter) == 0x08);

	class hkpCollisionFilter :
		public hkReferencedObject,             // 00
		public hkpCollidableCollidableFilter,  // 08
		public hkpShapeCollectionFilter,       // 10
		public hkpRayShapeCollectionFilter,    // 18
		public hkpRayCollidableFilter          // 20
	{
	public:
		enum class hkpFilterType
		{
			kUnknown = 0,
			kNull = 1,
			kGroup = 2,
			kList = 3,
			kFilterCustom = 4,
			kFilterPair = 5,
			kFilterConstraint = 6,
		};

		// add
		virtual void Init(hkpWorld* world);

		// members
		std::uint32_t                                  pad30;  // 30
		std::uint32_t                                  pad34;  // 34
		std::uint32_t									 type;   // 38
		std::uint32_t                                  pad3C;  // 3C
		std::uint32_t                                  pad40;  // 40
		std::uint32_t                                  pad44;  // 44
	};
	STATIC_ASSERT(sizeof(hkpCollisionFilter) == 0x48);

	class bhkCollisionFilter : public hkpCollisionFilter
	{
	public:
		[[nodiscard]] static bhkCollisionFilter* GetSingleton()
		{
			RelocPtr<bhkCollisionFilter*> singleton(0x1f89190);
			return *singleton;
		}

		std::uint32_t GetNewSystemGroup()
		{
			nextSystemGroup = nextSystemGroup + 1;
			if (nextSystemGroup == 65535) {
				nextSystemGroup = 10;
			}
			return nextSystemGroup;
		}

		// members
		std::uint32_t unk48;                     // 048
		std::uint32_t nextSystemGroup;           // 04C
		std::uint32_t bipedBitfields[32];        // 050
		std::uint32_t layerCollisionGroups[64];  // 0D0
		std::uint64_t layerBitfields[64];        // 1D0
		std::uint64_t triggerField;              // 3D0
		std::uint64_t sensorField;               // 3D8
		BSFixedString collisionLayerNames[64];   // 3E0
		std::uint8_t  unk5E0[256];               // 5E0
		BSFixedString collisionBoneNames[32];    // 6E0
	};
	STATIC_ASSERT(sizeof(bhkCollisionFilter) == 0x7E0);

	struct hkpWorldRayCastInput
	{
	public:
		// members
		hkVector4     from;                                  // 00
		hkVector4     to;                                    // 10
		bool          enableShapeCollectionFilter{ false };  // 20
		std::uint32_t filterInfo{ 0 };                       // 24
	};
	STATIC_ASSERT(sizeof(hkpWorldRayCastInput) == 0x30);


	struct hkpShapeRayCastCollectorOutput
	{
	public:
		[[nodiscard]] constexpr bool HasHit() const noexcept { return hitFraction < 1.0f; }

		constexpr void Reset() noexcept
		{
			hitFraction = 1.0f;
			shapeKey = HK_INVALID_SHAPE_KEY;
			extraInfo = -1;
		}

		// members
		hkVector4    normal;                            // 00
		float        hitFraction{ 1.0F };               // 10
		std::int32_t extraInfo{ -1 };                   // 14
		hkpShapeKey  shapeKey{ HK_INVALID_SHAPE_KEY };  // 18
		std::int32_t pad1C{ 0 };                        // 1C
	};
	STATIC_ASSERT(sizeof(hkpShapeRayCastCollectorOutput) == 0x20);

	struct hkpShapeRayCastOutput : public hkpShapeRayCastCollectorOutput
	{
	public:
		enum
		{
			kMaxHierarchyDepth = 8
		};

		constexpr void ChangeLevel(std::int32_t a_delta) noexcept
		{
			assert(shapeKeyIndex + a_delta < kMaxHierarchyDepth);
			shapeKeyIndex += a_delta;
		}

		constexpr void SetKey(hkpShapeKey a_key) noexcept { shapeKeys[shapeKeyIndex] = a_key; }

		constexpr void Reset() noexcept
		{
			hkpShapeRayCastCollectorOutput::Reset();
			shapeKeys[0] = HK_INVALID_SHAPE_KEY;
			shapeKeyIndex = 0;
		}

		// members
		hkpShapeKey   shapeKeys[kMaxHierarchyDepth]{ HK_INVALID_SHAPE_KEY };  // 20
		std::int32_t  shapeKeyIndex{ 0 };                                     // 40
		std::uint32_t pad44;                                                  // 44
		std::uint64_t pad48;                                                  // 48
	};
	STATIC_ASSERT(sizeof(hkpShapeRayCastOutput) == 0x50);

	struct hkpCollidable;

	struct hkpWorldRayCastOutput : public hkpShapeRayCastOutput
	{
	public:
		[[nodiscard]] constexpr bool HasHit() const noexcept { return rootCollidable != nullptr; }

		constexpr void Reset() noexcept
		{
			hkpShapeRayCastOutput::Reset();
			rootCollidable = nullptr;
		}

		// members
		hkpCollidable* rootCollidable{ nullptr };  // 50
		std::uint64_t        pad58;                      // 58
	};
	STATIC_ASSERT(sizeof(hkpWorldRayCastOutput) == 0x60);

	struct hkpCdBody;

	struct hkpShapeRayCastCollectorOutput;

	class hkpRayHitCollector
	{
	public:

		// add
		virtual void AddRayHit(const hkpCdBody& a_body, const hkpWorldRayCastOutput& a_hitInfo) = 0;  // 00

		//virtual ~hkpRayHitCollector();  // 01

		constexpr void Reset() noexcept { earlyOutHitFraction = 1.0f; }

		// members
		float         earlyOutHitFraction{ 1.0f };  // 08
		std::uint32_t pad0C;                        // 0C
	};
	STATIC_ASSERT(sizeof(hkpRayHitCollector) == 0x10);

	struct RayHitCollector : public hkpRayHitCollector
	{
	public:
		void RayHitCollector::reset()
		{
			earlyOutHitFraction = 1.0f;
			m_doesHitExist = false;
		}

		RayHitCollector::RayHitCollector()
		{
			reset();
		}

		virtual void AddRayHit(const hkpCdBody& cdBody, const hkpWorldRayCastOutput& hitInfo) override;

		hkpWorldRayCastOutput m_closestHitInfo; //hkpShapeRayCastCollectorOutput m_closestHitInfo;
		bool m_doesHitExist = false;
		//const hkpCdBody *m_closestCollidable = nullptr;
	};

	struct AllRayHitCollector : public hkpRayHitCollector
	{
	public:
		AllRayHitCollector::AllRayHitCollector()
		{
			reset();
		}

		void AllRayHitCollector::reset()
		{
			earlyOutHitFraction = 1.0f;
			m_hits.clear();
		}

		virtual void AddRayHit(const hkpCdBody& cdBody, const hkpWorldRayCastOutput& hitInfo) override;

		std::vector<std::pair<hkpCdBody*, hkpShapeRayCastCollectorOutput>> m_hits;
	};


	class hkpClosestRayHitCollector : public hkpRayHitCollector
	{
	public:

		constexpr bool HasHit() noexcept { return rayHit.HasHit(); }

		constexpr void Reset() noexcept
		{
			hkpRayHitCollector::Reset();
			rayHit.Reset();
		}

		// members
		hkpWorldRayCastOutput rayHit;  // 10
	};
	STATIC_ASSERT(sizeof(hkpClosestRayHitCollector) == 0x70);

	struct bhkPickData
	{
	public:
		// members
		hkpWorldRayCastInput       rayInput;                      // 00
		hkpWorldRayCastOutput      rayOutput;                     // 30
		hkVector4                  ray;                           // 90
		hkpClosestRayHitCollector* rayHitCollectorA0{ nullptr };  // A0
		hkpClosestRayHitCollector* rayHitCollectorA8{ nullptr };  // A8
		hkpClosestRayHitCollector* rayHitCollectorB0{ nullptr };  // B0
		hkpClosestRayHitCollector* rayHitCollectorB8{ nullptr };  // B8
		bool                       unkC0{ false };                // C0
		std::uint8_t               padC1;                         // C1
		std::uint16_t              padC2;                         // C2
		std::uint32_t              padC4;                         // C4
		std::uint32_t              padC8;                         // C8
	};
	STATIC_ASSERT(sizeof(bhkPickData) == 0xD0);


	class bhkRefObject : public NiObject
	{
	public:

		~bhkRefObject() override;  // 00

		// override(NiObject)
		virtual NiRTTI* GetRTTI(void) override;  // 02

		// add
		virtual void SetReferencedObject(hkReferencedObject* a_object);  // 25
		virtual void AdjustRefCount(bool a_increment);                   // 26

		// members
		hkReferencedObject* referencedObject;  // 10
	};
	STATIC_ASSERT(sizeof(bhkRefObject) == 0x18);

	class ahkpWorld;
	class hkpWorld;
	class bhkWorld;

	class bhkSerializable : public bhkRefObject
	{
	public:

		~bhkSerializable() override;  // 00

		// override (bhkRefObject)
		virtual NiRTTI* GetRTTI(void) override;                                    // 02
		virtual void          LoadBinary(NiStream* a_stream) override;                     // 18
		virtual void          LinkObject(NiStream* a_stream) override;                     // 19
		virtual bool          RegisterStreamables(NiStream* a_stream) override;            // 1A - { return NiObject::RegisterStreamables(a_stream); }
		virtual void          SaveBinary(NiStream* a_stream) override;                     // 1B
		virtual void          SetReferencedObject(hkReferencedObject* a_object) override;  // 25

		// add
		virtual hkpWorld* GetWorld1();                     // 27 - { return 0; }
		virtual ahkpWorld* GetWorld2();                     // 28 - { return 0; }
		virtual void       MoveToWorld(bhkWorld* a_world);  // 29
		virtual void       RemoveFromCurrentWorld();        // 2A
		virtual void       Unk_2B(void);                    // 2B
		virtual void       Unk_2C(void);                    // 2C - { return 1; }
		virtual void       Unk_2D(void);                    // 2D
		virtual void       Unk_2E(void) = 0;                // 2E
		virtual void       Unk_2F(void) = 0;                // 2F
		virtual void       Unk_30(void);                    // 30
		virtual void       Unk_31(void);                    // 31

		// members
		bhkSerializable* serializable;  // 18
	};
	STATIC_ASSERT(sizeof(bhkSerializable) == 0x20);


	class bhkWorld : public bhkSerializable
	{
	public:
		class bhkConstraintProjector;

		~bhkWorld() override;  // 00

		// override (bhkSerializable)
		virtual NiRTTI* GetRTTI() override;                                    // 02
		virtual void          SetReferencedObject(hkReferencedObject* a_object) override;  // 25
		virtual void          AdjustRefCount(bool a_increment) override;                   // 26
		virtual hkpWorld* GetWorld1() override;                                        // 27 - { return referencedObject.ptr; }
		virtual ahkpWorld* GetWorld2() override;                                        // 28 - { return referencedObject.ptr; }
		virtual void          Unk_2B(void) override;                                       // 2B
		virtual void          Unk_2C(void) override;                                       // 2C - { return 1; }
		virtual void          Unk_2E(void) override;                                       // 2E
		virtual void          Unk_2F(void) override;                                       // 2F

		// add
		virtual void Unk_32(void);                                              // 32
		virtual bool PickObject(bhkPickData& a_pickData);                       // 33
		virtual void Unk_34(void);                                              // 34
		virtual void Unk_35(void);                                              // 35
		virtual void InitHavok(NiAVObject* a_sceneObject, NiAVObject* a_root);  // 36

		// members
		std::uint8_t                  unk0020[0x320];             // 0020
		std::uint8_t                  unk0340[0x6400];            // 0340
		std::uint8_t                  unk6740[0x5DC0];            // 6740
		tArray<void*>               unkC500;                    // C500
		tArray<void*>               unkC518;                    // C518
		tArray<void*>               unkC530;                    // C530
		tArray<void*>               unkC548;                    // C548
		std::uint64_t                 unkC560;                    // C560
		std::uint32_t                 unkC568;                    // C568
		float                         unkC56C;                    // C56C
		bhkConstraintProjector* constraintProjector;        // C570
		std::uint64_t                 unkC578;                    // C578
		std::uint32_t                 unkC580;                    // C580
		float                         unkC584;                    // C584
		std::uint64_t                 unkC588;                    // C588
		std::uint64_t                 unkC590;                    // C590
		mutable BSReadWriteLock       worldLock;                  // C598
		mutable BSReadWriteLock       unkC5A0;                    // C5A0
		std::uint64_t                 unkC5A8;                    // C5A8
		hkVector4                     unkC5B0;                    // C5B0
		std::uint64_t                 unkC5C0;                    // C5C0
		std::uint64_t				  acousticSpaceListener;      // C5C8
		std::uint64_t				  suspendInactiveAgentsUtil;  // C5D0
		std::uint32_t                 unkC5D8;                    // C5D8 - incremented per frame
		std::uint32_t                 unkC5DC;                    // C5DC
		std::uint32_t                 unkC5E0;                    // C5E0
		std::uint32_t                 unkC5E4;                    // C5E4
		std::uint32_t                 unkC5E8;                    // C5E8
		std::uint32_t                 unkC5EC;                    // C5EC
		float                         tau;                        // C5F0
		float                         damping;                    // C5F4
		std::uint8_t                  unkC5F8;                    // C5F8
		bool                          toggleCollision;            // C5F9
		std::uint16_t                 unkC5FA;                    // C5FA
		std::uint16_t                 unkC5FC;                    // C5FC
		std::uint16_t                 unkC5FE;                    // C5FE
	};
	STATIC_ASSERT(sizeof(bhkWorld) == 0xC600);

	class taskCoverageCheck : public TaskDelegate
	{
	public:
		virtual void Run();
		virtual void Dispose();

		taskCoverageCheck();
	};

	typedef bhkWorld* (*_GetHavokWorldFromCell)(TESObjectCELL* cell);
	extern RelocAddr<_GetHavokWorldFromCell> GetHavokWorldFromCell;

	typedef NiAVObject* (*_GetNodeFromCollidable)(hkpCollidable* a_collidable);
	extern RelocAddr<_GetNodeFromCollidable> GetNodeFromCollidable;

	typedef TESObjectREFR* (*_GetRefFromCollidable)(hkpCollidable* a_collidable);
	extern RelocAddr<_GetRefFromCollidable> GetRefFromCollidable;

	typedef void(*_hkpWorld_CastRay)(ahkpWorld* world, hkpWorldRayCastInput* input, hkpRayHitCollector* collector);
	extern RelocAddr<_hkpWorld_CastRay> hkpWorld_CastRay;

	typedef void(*_PlayEffectShader)(TESObjectREFR* ref, TESEffectShader* effectShader, float duration, TESObjectREFR* facingRef, UInt8 a5, UInt8 a6, __int64 a7, bool a8);
	extern RelocAddr <_PlayEffectShader> PlayEffectShader;

}