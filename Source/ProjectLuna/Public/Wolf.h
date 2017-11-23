// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "IceBlockRuntime.h"
#include "LunaLog.h"
#include "ProjectLunaGameMode.h"
#include "OnHowledData.h"
#include "OnWolfJumpedData.h"
#include "OnWolfLandedData.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "OnWaterLevelChangedData.h"
#include "StreamEntity.h"
#include "Wolf.generated.h"

#define ICE_BLOCK_CHANNEL ECC_GameTraceChannel1

DECLARE_LUNA_LOG(Wolf);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFootStepDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDiedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartedPushingBlockDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoppedPushingBlockDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWolfRespawnedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStreamAttenuationDelegate, float, ratio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrquaSoundAttenuation, float, ratio);

class ACheckpoint;
class AOrqua;

UCLASS()
class PROJECTLUNA_API AWolf : public ACharacter
{
	GENERATED_BODY()

public:
	enum class WolfState {
		Idle, Jumping, WalkingTowardsBlock, PushingBlock, ChangingTides, Dying, LerpingToOverview, LerpingFromOverview
	};

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent *CameraBoom;
	
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent *FollowCamera;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent *NosePositionComponent;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent *RearPawsComponent;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent *FrontPawsComponent;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent *HeadComponent;

	UPROPERTY(EditAnywhere)
	FVector CamDefaultLocation = FVector(0, 0, 8.5);

	UPROPERTY(EditAnywhere)
	FRotator CamDefaultRotation = FRotator(0, 0, 0);

	UPROPERTY(EditAnywhere)
	float CamTurnRate;

	UPROPERTY(EditAnywhere)
	float CamLookUpRate;

	// TODO: Replace thosse with UFUNCTION() get functions to block other scripts from modifying the values

	UPROPERTY(BlueprintReadOnly)
	bool IsHowling;

	UPROPERTY(BlueprintReadOnly)
	bool IsInteractingWithIceBlock;

	UPROPERTY(BlueprintReadOnly)
	float IsDying;

	UPROPERTY(BlueprintReadOnly)
	float YawDelta;

	// Maximum distance between an ice block and the wolf to be able to push/pull it
	UPROPERTY(EditAnywhere)
	float IceBlockTriggerDistance;

	// The speed of the wolf when he's pushing/pulling an ice block
	UPROPERTY(EditAnywhere)
	float IceBlockInteractionSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool InCinematic = false;

	/* The distance between the rear left foot and the ground */
	UPROPERTY(BlueprintReadOnly)
	float RearLeftFootGroundLevelIKOffset;

	/* The distance between the rear right foot and the ground */
	UPROPERTY(BlueprintReadOnly)
	float RearRightFootGroundLevelIKOffset;

	/* The distance between the front left foot and the ground */
	UPROPERTY(BlueprintReadOnly)
	float FrontLeftFootGroundLevelIKOffset;

	/* The distance between the front right foot and the ground */
	UPROPERTY(BlueprintReadOnly)
	float FrontRightFootGroundLevelIKOffset;

	/* If we are right in front of a wall */
	UPROPERTY(BlueprintReadOnly)
	bool ForwardWallIsClose;

	/* Speed at which we want to move the body parts depending on the IK offsets */
	UPROPERTY(EditDefaultsOnly)
	float IKInterpSpeed = 15;

	UPROPERTY(EditDefaultsOnly)
	float CamMaxAutoRotateCooldown = 2;

	UPROPERTY(EditDefaultsOnly)
	FName RearLeftFootSocketName = "FootL01S";

	UPROPERTY(EditDefaultsOnly)
	FName RearRightFootSocketName = "FootR01S";

	UPROPERTY(EditDefaultsOnly)
	FName FrontLeftFootSocketName = "HandL01S";

	UPROPERTY(EditDefaultsOnly)
	FName FrontRightFootSocketName = "HandR01S";

	UFUNCTION()
	void OnBeginOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnEndOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnHit(AActor* SelfActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	UPROPERTY(BlueprintAssignable)
	FFootStepDelegate OnFootStep;

	UPROPERTY(BlueprintAssignable)
	FStreamAttenuationDelegate OnStreamAttenuation;

	UPROPERTY(BlueprintAssignable)
	FOrquaSoundAttenuation OnOrquaSoundAttenuation;

	UPROPERTY(BlueprintAssignable)
	FDiedDelegate OnDied;

	UPROPERTY(BlueprintAssignable)
	FStartedPushingBlockDelegate OnStartedPushingBlock;

	UPROPERTY(BlueprintAssignable)
	FStoppedPushingBlockDelegate OnStoppedPushingBlock;

	UPROPERTY(BlueprintAssignable)
	FWolfRespawnedDelegate OnWolfRespawned;

	// Sets default values for this character's properties
	AWolf();

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void Tick(float deltaSeconds) override;
	virtual void Jump();

	void setDyingState(bool state);
	bool isDead();

	void enableIceBlockInteraction();
	void disableIceBlockInteraction();
	void moveRight(float axisValue);
	void moveForward(float axisValue);
	void turnCamera(float axisValue);
	void lookUpCamera(float axisValue);
	void turnCameraMouse(float axisValue);
	void lookUpCameraMouse(float axisValue);
	void howlUp(float axisValue);
	void howlDown(float axisValue);
	void toggleOverview();

	bool isOverviewing();

	void setLastCheckpointReference(ACheckpoint *lastCheckpoint);
	ACheckpoint *getLastCheckpointReference();


	void setCanDie(bool canDie);
	bool getCanDie();
	void quitOverview();

private:
	bool rearLeftFootOnGround = true;
	bool rearRightFootOnGround = true;
	bool frontLeftFootOnGround = true;
	bool frontRightFootOnGround = true;

	const float minStreamSoundDistance = 1700;
	const float minOrquaSoundDistance = 6000;

	UWorld *m_world;
	AProjectLunaGameMode *m_gameMode;

	WolfState m_currentState;
	AIceBlockRuntime *m_currentIceBlock;
	FHitResult m_iceBlockResult;
	FRotator m_lastFrameRotation;
	UPhysicsConstraintComponent *m_currentBlockConstraint;

	bool m_hasMovedSinceLastFrame;
	bool m_hasMovedCamSinceLastFrame;
	float m_camAutoRotateCooldown = 0;
	bool m_mustAutoRotateCam;

	float m_targetPitch;

	bool isInOverviewingState;
	bool m_canDie = true;

	float howlUpAxisValue;
	float howlDownAxisValue;
	float m_tideControlElapsedTime;
	bool m_wasNearIceBlock;

	ACameraActor *m_overviewTransitioNCam;

	// Initially, we can't change the tides
	bool m_reachedLowestTide = true;
	bool m_reachedHighestTide = true;

	float m_overviewRatio;

	float m_scaledHalfHeight;
	float m_meshScale;

	TMap<FString, ACameraActor *> m_cameraActors;

	UFUNCTION()
	void unbindIceBlock();

	ACameraActor *m_currentOverviewCam;

	FHitResult checkIceBlock();

	void cacheCameraActors();

	void moveTowardIceBlock();
	void computeRotationSpeed(float deltaSeconds);
	void bindIceBlock();
	bool canJump();
	void engageTideMode();
	void disengageTideMode();
	void manageTideControl(float deltaSeconds);
	void setState(WolfState newState);
	void raycastGround();
	void lerpToTargetPitch(float deltaTime);
	void pitchCamera(float deltaTime);
	void onWaterLevelChanged(OnWaterLevelChangedData *eventData);

	void lerpToOverview(float deltaTime);
	void lerpFromOverview(float deltaTime);

	void manageIKTraces(float deltaTime);
	float getFootGroundLevelIKOffset(FName socketName);
	bool forwardWallIsClose();

	ACheckpoint *m_lastCheckpointReference;

	/* The default position of the rear left foot */
	FVector m_defaultRearLeftFootPos;

	bool howlingUp;

	void traceFootSteps();
	void traceFootStepBySocket(FName socketName, bool &footOnGround);

	void checkStreams();

	bool m_blockIsMoving;
};
