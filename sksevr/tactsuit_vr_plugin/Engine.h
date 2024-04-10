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

}