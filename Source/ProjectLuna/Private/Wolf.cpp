// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "Wolf.h"
#include "Checkpoint.h"
#include "IceBlock.h"
#include "Orqua.h"
#include "GenericEventData.h"

DEFINE_LUNA_LOG(Wolf);

AWolf::AWolf()
{
 	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->AttachTo(CameraBoom);

	NosePositionComponent = CreateDefaultSubobject<USceneComponent>(TEXT("NosePositionComponent"));
	NosePositionComponent->AttachTo(GetMesh());

	HeadComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HeadComponent"));
	HeadComponent->AttachTo(GetMesh());

	RearPawsComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RearPawsComponent"));
	RearPawsComponent->AttachTo(RootComponent);

	FrontPawsComponent = CreateDefaultSubobject<USceneComponent>(TEXT("FrontPawsComponent"));
	FrontPawsComponent->AttachTo(RootComponent);
}

void AWolf::BeginPlay()
{
	Super::BeginPlay();

	if (m_world) return;

	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode*)m_world->GetAuthGameMode();

	m_gameMode->getEventManager().subscribe("OnWaterLevelChanged", this, &AWolf::onWaterLevelChanged);

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AWolf::OnBeginOverlap);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AWolf::OnEndOverlap);
	GetMesh()->OnComponentHit.AddDynamic(this, &AWolf::OnHit);

	cacheCameraActors();

	m_overviewTransitioNCam = m_world->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);

	m_scaledHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() * GetActorScale().Z;
	m_meshScale = GetMesh()->GetRelativeTransform().GetScale3D().Z;


}

void AWolf::cacheCameraActors()
{
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(m_world, ACameraActor::StaticClass(), foundActors);

	for (AActor *actor : foundActors)
	{
		if (actor->GetName().Contains("Overview_Zone_"))
		{
			m_cameraActors.Add(actor->GetName(), Cast<ACameraActor>(actor));

			//LUNA_WARN(Wolf, "CAMERA: %s", *actor->GetName());
		}
	}
}

void AWolf::BeginDestroy()
{
	Super::BeginDestroy();

	if (m_gameMode)
	{
		m_gameMode->getEventManager().unsubscribe("OnWaterLevelChanged", this, &AWolf::onWaterLevelChanged);
	}
}

void AWolf::OnBeginOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	FString triggerName;
	OtherActor->GetName(triggerName);

	if (triggerName.Contains("Overview_Zone_"))
	{
		//LUNA_WARN(Wolf, "FOUND OVERVIEW!! %s", *triggerName);

		// That's pretty much as ad-hoc as it can gets, yay for crunch time

		FString cameraName = triggerName + "_Camera";

		if (m_cameraActors.Contains(cameraName))
		{
			//LUNA_WARN(Wolf, "FOUND CAMERA!! %s", *cameraName);

			m_currentOverviewCam = m_cameraActors[cameraName];
		}
	}
}

void AWolf::OnEndOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	FString triggerName;
	OtherActor->GetName(triggerName);

	if (m_currentOverviewCam && triggerName.Contains("Overview_Zone_"))
	{
		// That's pretty much as ad-hoc as it can gets, yay for crunch time

		FString cameraName = triggerName + "_Camera";
		FString currentCameraName;

		m_currentOverviewCam->GetName(currentCameraName);

		if (m_currentOverviewCam && currentCameraName == cameraName)
		{
			m_currentOverviewCam = nullptr;
		}
	}
}

void AWolf::OnHit(AActor* SelfActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Hit Bone: %s"), *Hit.BoneName.ToString());
}

void AWolf::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	this->computeRotationSpeed(deltaSeconds);

	switch (m_currentState)
	{
		// Idle is walking/running too
		case WolfState::Idle:
			if (howlUpAxisValue != 0 || howlDownAxisValue != 0)
			{
				engageTideMode();
			}
			break;

		case WolfState::Jumping:
			if (!GetCharacterMovement()->IsFalling())
			{
				m_currentState = WolfState::Idle;
				m_gameMode->getEventManager().raiseEvent("OnWolfLanded", new OnWolfLandedData());
			}
			break;

		case WolfState::WalkingTowardsBlock:
			moveTowardIceBlock();
			break;

		case WolfState::ChangingTides:
			manageTideControl(deltaSeconds);
			break;

		case WolfState::LerpingToOverview:
			lerpToOverview(deltaSeconds);
			break;

		case WolfState::LerpingFromOverview:
			lerpFromOverview(deltaSeconds);
			break;
	}

	checkIceBlock();
	raycastGround();
	manageIKTraces(deltaSeconds);

	if (m_camAutoRotateCooldown > 0)
	{
		m_camAutoRotateCooldown -= deltaSeconds;
	}

	if (m_mustAutoRotateCam)
	{
		pitchCamera(deltaSeconds);
	}

	if (m_hasMovedSinceLastFrame)
	{
		lerpToTargetPitch(deltaSeconds);
		m_hasMovedSinceLastFrame = false;

		if (m_camAutoRotateCooldown <= 0)
		{
			m_mustAutoRotateCam = true;
		}
	}

	checkStreams();

	traceFootSteps();
}

void AWolf::checkStreams()
{
	// TODO: Really not performant, improve it

	float smallestDistance = INFINITY;

	for(TActorIterator<AActor> ActorItr(m_world); ActorItr; ++ActorItr) {
		if (ActorItr->IsA(AStreamEntity::StaticClass()))
		{
			AStreamEntity *streamEntity = Cast<AStreamEntity>(*ActorItr);
			
			if(!streamEntity->containsBlock()) continue;

			FVector closestSplinePoint = streamEntity->getClosestPointOnSpline(GetActorLocation());

			float distance = (GetActorLocation() - closestSplinePoint).Size();

			if (distance < smallestDistance)
			{
				smallestDistance = distance;
			}

			//DrawDebugLine(GetWorld(), GetActorLocation(), closestSplinePoint, FColor::Red);
		}
		else if(ActorItr->IsA(AOrqua::StaticClass()))
		{
			AOrqua *orqua = Cast<AOrqua>(*ActorItr);

			float distance = (GetActorLocation() - orqua->GetActorLocation()).Size2D();

			//DrawDebugLine(GetWorld(), GetActorLocation(), orqua->GetActorLocation(), FColor::Red);

			float ratioOrqua = FMath::Clamp(1.f - distance / minOrquaSoundDistance, 0.f, 1.f);

			//UE_LOG(LogTemp, Warning, TEXT("%f"), distance);

			if(OnOrquaSoundAttenuation.IsBound()) {
				OnOrquaSoundAttenuation.Broadcast(ratioOrqua);
			}
		}
	}

	//UE_LOG(LogTemp, Warning, TEXT("%f"), smallestDistance);

	float ratio = FMath::Clamp(1.f - smallestDistance / minStreamSoundDistance, 0.f, 1.f);

	if(OnStreamAttenuation.IsBound()) {
		OnStreamAttenuation.Broadcast(ratio);
	}
}

void AWolf::lerpToOverview(float deltaTime)
{
	m_overviewRatio += deltaTime / 1;

	float smoothRatio = FMath::SmoothStep(0, 1, m_overviewRatio);

	FRotator startRot = FollowCamera->GetComponentRotation();
	FVector startLoc = FollowCamera->GetComponentLocation();

	FRotator endRot = m_currentOverviewCam->GetActorRotation();
	FVector endLoc = m_currentOverviewCam->GetActorLocation();

	FQuat newQuat = FQuat::Slerp(startRot.Quaternion(), endRot.Quaternion(), smoothRatio);
	FVector newLoc = FMath::Lerp(startLoc, endLoc, smoothRatio);

	m_overviewTransitioNCam->SetActorRotation(newQuat);
	m_overviewTransitioNCam->SetActorLocation(newLoc);

	if (m_overviewRatio >= 1)
	{
		m_overviewRatio = 0;
		isInOverviewingState = true;
		setState(WolfState::Idle);
	}
}

void AWolf::lerpFromOverview(float deltaTime)
{
	m_overviewRatio += deltaTime / 1;

	float smoothRatio = FMath::SmoothStep(0, 1, m_overviewRatio);

	FRotator startRot = m_currentOverviewCam->GetActorRotation();
	FVector startLoc = m_currentOverviewCam->GetActorLocation();

	FRotator endRot = FollowCamera->GetComponentRotation();
	FVector endLoc = FollowCamera->GetComponentLocation();

	FQuat newQuat = FQuat::Slerp(startRot.Quaternion(), endRot.Quaternion(), smoothRatio);
	FVector newLoc = FMath::Lerp(startLoc, endLoc, smoothRatio);

	m_overviewTransitioNCam->SetActorRotation(newQuat);
	m_overviewTransitioNCam->SetActorLocation(newLoc);

	if (m_overviewRatio >= 1)
	{
		m_overviewRatio = 0;

		((APlayerController *)GetController())->SetViewTarget(this, FViewTargetTransitionParams());

		setState(WolfState::Idle);
		isInOverviewingState = false;
	}
}

void AWolf::raycastGround()
{
	if (m_currentState == WolfState::Jumping) return;

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	FVector rearStartPos = RearPawsComponent->GetComponentLocation();
	FVector rearEndPos = rearStartPos - GetActorUpVector() - RearPawsComponent->GetUpVector() * 70;

	FHitResult rearPawHit;
	m_world->LineTraceSingleByChannel(rearPawHit, rearStartPos, rearEndPos, ECollisionChannel::ECC_WorldStatic, TraceParams);


	FVector frontStartPos = FrontPawsComponent->GetComponentLocation();
	FVector frontEndPos = frontStartPos - GetActorUpVector() - FrontPawsComponent->GetUpVector() * 70;

	FHitResult frontPawHit;
	m_world->LineTraceSingleByChannel(frontPawHit, frontStartPos, frontEndPos, ECollisionChannel::ECC_WorldStatic, TraceParams);

	/*
	DrawDebugLine(GetWorld(), frontStartPos, frontEndPos, FColor::Red);
	DrawDebugLine(GetWorld(), rearStartPos, rearEndPos, FColor::Red);*/


	if (rearPawHit.Actor != nullptr && frontPawHit.Actor != nullptr)
	{
		FVector slopeVector = frontPawHit.ImpactPoint - rearPawHit.ImpactPoint;
		FVector fwd = GetActorForwardVector();

		float angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(fwd, slopeVector) / (fwd.Size() * slopeVector.Size())));

		if (frontPawHit.Location.Z > rearPawHit.Location.Z)
		{
			angle = -angle;
		}

		m_targetPitch = angle;
	}
	else
	{
		m_targetPitch = 0;
	}
}

void AWolf::lerpToTargetPitch(float deltaTime)
{
	FVector rotTransform = GetMesh()->GetRelativeTransform().GetRotation().Euler();

	float deltaSign = FMath::Sign(m_targetPitch - rotTransform.Y);
	float newPitch = rotTransform.Y + deltaTime * 60 * deltaSign;

	if (deltaSign != FMath::Sign(m_targetPitch - newPitch))
	{
		newPitch = m_targetPitch;
	}

	GetMesh()->SetRelativeRotation(FRotator(newPitch, rotTransform.Z, rotTransform.X));
}

void AWolf::pitchCamera(float deltaTime)
{
	float currentPitch = FollowCamera->GetComponentRotation().Pitch;
	float targetCamPitch = -GetMesh()->GetComponentRotation().Pitch - 30;

	float deltaPitch = targetCamPitch - currentPitch;
	float deltaSign = 1;// FMath::Sign(deltaPitch);

	float newPitch = deltaTime * -deltaSign * deltaPitch;

	if (FMath::Sign(targetCamPitch - newPitch) != deltaSign)
	{
		this->AddControllerPitchInput(newPitch);
	}
}

void AWolf::manageTideControl(float deltaSeconds)
{
	// TODO: Set a value for the blueprint to know if they wolf tried to change the tides but can't
	if (m_reachedLowestTide && howlDownAxisValue + howlUpAxisValue <= 0 ||
		m_reachedHighestTide && howlDownAxisValue + howlUpAxisValue >= 0)
	{
		disengageTideMode();
		return;
	}

	// If both triggers are maxed (which would equal 0), we still don't want to disengage
	if (howlUpAxisValue == 0 && howlDownAxisValue == 0)
	{
		disengageTideMode();
		return;
	}
	
	float howlAxisValue = howlUpAxisValue + howlDownAxisValue;

	m_tideControlElapsedTime += deltaSeconds;

	if (howlAxisValue != 0)
	{
		m_gameMode->getEventManager().raiseEvent("OnHowled", new OnHowledData(deltaSeconds, howlAxisValue));
	}
}

//Moves the wolf towards the ice block by taking appropriate and smooth pathing (no instant rotation)
void AWolf::moveTowardIceBlock()
{
	// TODO: Improve this algorithm by finding the right startVector that would make it possible to finish exactly at the center of the block

	// We smoothly lerp the dog to make him perpendicular to the ice block and directly in front of it
	float totalNormDist = FVector::DotProduct(m_iceBlockResult.ImpactPoint - m_iceBlockResult.TraceStart, -m_iceBlockResult.ImpactNormal);// FHitResult.Distance;
	float totalRemainingDist = FVector::DotProduct(m_iceBlockResult.ImpactPoint - NosePositionComponent->GetComponentLocation(), -m_iceBlockResult.ImpactNormal); //(IceBlockHitResult.ImpactPoint - NosePositionComponent->GetComponentLocation()).Size();

	FVector startVector = (m_iceBlockResult.ImpactPoint - m_iceBlockResult.TraceStart).GetSafeNormal();
	FVector endVector = -m_iceBlockResult.ImpactNormal;

	float t;

	if (FMath::IsNearlyZero(totalNormDist, 0.1f))
	{
		t = 1;
	}
	else
	{
		t = (totalNormDist - totalRemainingDist) / totalNormDist;

		// This is a small hack for when the angle with the block is too big
		if (t < -0.1)
		{
			t = 1;
		}
	}

	FVector newDir = FMath::Lerp(startVector, endVector, t);

	if (t < 1.f)
	{
		// TODO: Change this magic number for a value editable in the editor
		this->AddMovementInput(newDir, 0.3f);
	}
	else
	{
		// We don't care about the Z component and don't want the wolf to to be slanted if the block is slanted
		FVector fixedNormal = -m_iceBlockResult.ImpactNormal;
		fixedNormal.Z = 0;

		// After the lerp, we force the rotation in case the wolf is not perfectly perpendicular to the ice block
		this->SetActorRotation(fixedNormal.Rotation());

		bindIceBlock();
	}
}

void AWolf::bindIceBlock()
{
	m_currentState = WolfState::PushingBlock;
	IsInteractingWithIceBlock = true;

	FVector minBound, maxBound;

	m_currentIceBlock->Mesh->GetLocalBounds(minBound, maxBound);

	minBound.X = 0;
	minBound.Y = 0;

	m_currentIceBlock->Mesh->SetCenterOfMass(minBound);

	m_gameMode->getEventManager().raiseEvent("OnStartPush", new GenericEventData());
}

void AWolf::unbindIceBlock()
{
	IsInteractingWithIceBlock = false;
	m_currentIceBlock->Mesh->SetCenterOfMass(FVector::ZeroVector);

	m_currentState = WolfState::Idle;
	m_currentIceBlock = nullptr;

	m_gameMode->getEventManager().raiseEvent("OnStopPush", new GenericEventData());
}

// Checks if an ice block is ahead of the wolf and in the IceBlockTriggerDistance range
FHitResult AWolf::checkIceBlock()
{
	FVector fwdVector = NosePositionComponent->GetForwardVector();

	FVector startPos = NosePositionComponent->GetComponentLocation() + fwdVector;
	FVector endPos = startPos + fwdVector * IceBlockTriggerDistance;
	
	FHitResult outHit;
	m_world->LineTraceSingleByChannel(outHit, startPos, endPos, ICE_BLOCK_CHANNEL);

	AActor *iceBlock = outHit.GetActor();

	if (m_currentState == WolfState::Idle && iceBlock && iceBlock->IsA(AIceBlockRuntime::StaticClass()) && Cast<AIceBlockRuntime>(iceBlock)->isMovable())
	{
		m_gameMode->getEventManager().raiseEvent("OnWolfNearIceBlock", new OnWolfNearIceBlockData(true, iceBlock->GetActorLocation()));

		m_wasNearIceBlock = true;
	}
	else if (m_wasNearIceBlock)
	{
		m_gameMode->getEventManager().raiseEvent("OnWolfNearIceBlock", new OnWolfNearIceBlockData(false, FVector::ZeroVector));

		m_wasNearIceBlock = false;
	}

	return outHit;
}

// Start pushing the block towards the plane's inversed normal direction
void AWolf::enableIceBlockInteraction()
{
	if (m_currentState != WolfState::Idle || isInOverviewingState) return;

	FHitResult hitResult = checkIceBlock();
	AActor *iceBlock = hitResult.GetActor();

	if (iceBlock && iceBlock->IsA(AIceBlockRuntime::StaticClass()))
	{
		//DrawDebugLine(m_world, hitResult.ImpactPoint, hitResult.ImpactPoint + hitResult.ImpactNormal * 40, FColor(255, 0, 0), false);

		AIceBlockRuntime *castedIceBlock = Cast<AIceBlockRuntime>(iceBlock);

		if (castedIceBlock->isMovable())
		{
			m_currentState = WolfState::WalkingTowardsBlock;
			m_currentIceBlock = castedIceBlock;
			m_iceBlockResult = hitResult;
		}
	}
}

void AWolf::disableIceBlockInteraction()
{
	m_currentState = WolfState::Idle;

	if (m_currentIceBlock)
	{
		this->GetCapsuleComponent()->IgnoreActorWhenMoving(m_currentIceBlock, false);
		unbindIceBlock();
	}
}

void AWolf::computeRotationSpeed(float delaSeconds)
{
	FRotator rotation = this->GetActorRotation();
	YawDelta = (rotation.Yaw - m_lastFrameRotation.Yaw) / delaSeconds;
	m_lastFrameRotation = rotation;
}

bool AWolf::canJump()
{
	return m_currentState != WolfState::WalkingTowardsBlock &&
		m_currentState != WolfState::WalkingTowardsBlock &&
		m_currentState != WolfState::ChangingTides &&
		!isOverviewing();
}

bool AWolf::isOverviewing()
{
	return isInOverviewingState ||
		m_currentState == WolfState::LerpingToOverview ||
		m_currentState == WolfState::LerpingFromOverview;
}

/* Actions handling */

void AWolf::Jump()
{
	if (Super::CanJump() && canJump())
	{
		m_currentState = WolfState::Jumping;
		Super::Jump();

		m_gameMode->getEventManager().raiseEvent("OnWolfJumped", new OnWolfJumpedData());
	}
}

void AWolf::moveRight(float axisValue)
{
	if (m_currentState == WolfState::WalkingTowardsBlock ||
		m_currentState == WolfState::PushingBlock ||
		m_currentState == WolfState::ChangingTides ||
		isOverviewing()) return;

	if (axisValue != 0.f)
	{
		FRotator controlRot = this->GetControlRotation();

		controlRot.Roll = 0.f; // Rotation around x
		controlRot.Pitch = 0.f; // Rotation around y

		FVector rightVector = UKismetMathLibrary::GetRightVector(controlRot);
		this->AddMovementInput(rightVector, axisValue);

		if (!m_hasMovedCamSinceLastFrame)
		{
			float turnValue = CamTurnRate * axisValue * m_world->DeltaTimeSeconds;
			this->AddControllerYawInput(turnValue);
		}

		m_hasMovedSinceLastFrame = true;
	}

	m_hasMovedCamSinceLastFrame = false;
}

void AWolf::moveForward(float axisValue)
{
	if (m_blockIsMoving && axisValue == 0.f)
	{
		if (OnStoppedPushingBlock.IsBound())
		{
			OnStoppedPushingBlock.Broadcast();
		}

		m_blockIsMoving = false;
	}

	if (axisValue == 0.f ||
		m_currentState == WolfState::WalkingTowardsBlock ||
		m_currentState == WolfState::ChangingTides ||
		isOverviewing()) return;

	m_hasMovedSinceLastFrame = true;

	if (m_currentState == WolfState::PushingBlock)
	{
		if(!m_blockIsMoving)
		{
			if (OnStartedPushingBlock.IsBound())
			{
				OnStartedPushingBlock.Broadcast();
			}
			
			m_blockIsMoving = true;
		}

		// We push/pull the block and the wolf by changing their velocity
		FVector dir = -m_iceBlockResult.Normal * FMath::Sign(axisValue);

		this->GetCharacterMovement()->Velocity = dir * IceBlockInteractionSpeed + FVector(0, 0, this->GetCharacterMovement()->Velocity.Z);

		// Since the default Unreal character velocity doesn't work the same way as physics velocity, we need to tweak it a bit
		// So far, this is for testing the limits of Unreal's physics and prototyping
		FVector blockVelocity = dir * (IceBlockInteractionSpeed - (axisValue < 0 ? 50 : 86));

		m_currentIceBlock->push(blockVelocity);

		FVector distance = m_currentIceBlock->GetActorLocation() - NosePositionComponent->GetComponentLocation();
		
		if (!m_currentIceBlock->isMovable() || distance.Size() > m_currentIceBlock->getWidth() + 5)
		{
			unbindIceBlock();
		}
	}
	else
	{
		FRotator controlRot = this->GetControlRotation();

		controlRot.Roll = 0.f; // Rotation around x
		controlRot.Pitch = 0.f; // Rotation around y

		FVector fwdVector = UKismetMathLibrary::GetForwardVector(controlRot);
		this->AddMovementInput(fwdVector, axisValue);
	}
}

void AWolf::turnCamera(float axisValue)
{
	if (axisValue == 0 || isOverviewing()) return;

	m_mustAutoRotateCam = false;

	m_hasMovedCamSinceLastFrame = true;

	float turnValue = CamTurnRate * axisValue * m_world->DeltaTimeSeconds;
	this->AddControllerYawInput(turnValue);

	m_camAutoRotateCooldown = CamMaxAutoRotateCooldown;
}

void AWolf::lookUpCamera(float axisValue)
{
	if (axisValue == 0 ||
		isOverviewing() ||
		FollowCamera->GetComponentRotation().Pitch < -50 && axisValue > 0) return;

	m_mustAutoRotateCam = false;

	m_hasMovedCamSinceLastFrame = true;

	float lookUpValue = CamLookUpRate * axisValue *m_world->DeltaTimeSeconds;
	this->AddControllerPitchInput(lookUpValue);

	m_camAutoRotateCooldown = CamMaxAutoRotateCooldown;
}

void AWolf::turnCameraMouse(float axisValue)
{
	if (axisValue == 0) return;

	m_mustAutoRotateCam = false;
	m_hasMovedCamSinceLastFrame = true;

	AddControllerYawInput(axisValue);
	m_camAutoRotateCooldown = CamMaxAutoRotateCooldown;
}

void AWolf::lookUpCameraMouse(float axisValue)
{
	if (axisValue == 0) return;

	m_mustAutoRotateCam = false;
	m_hasMovedCamSinceLastFrame = true;

	AddControllerPitchInput(axisValue);
	m_camAutoRotateCooldown = CamMaxAutoRotateCooldown;
}

void AWolf::engageTideMode()
{
	// TODO: Set a value for the blueprint to know if they wolf tried to change the tides but can't

	if (m_reachedLowestTide && howlDownAxisValue + howlUpAxisValue <= 0) return;
	if (m_reachedHighestTide && howlDownAxisValue + howlUpAxisValue >= 0) return;

	m_tideControlElapsedTime = 0;

	m_currentState = WolfState::ChangingTides;

	if (!IsHowling)
	{
		if (howlDownAxisValue + howlUpAxisValue >= 0)
		{
			m_gameMode->getEventManager().raiseEvent("OnStartHowlUp", new GenericEventData());
			howlingUp = true;
		}
		else if (howlDownAxisValue + howlUpAxisValue < 0)
		{
			m_gameMode->getEventManager().raiseEvent("OnStartHowlDown", new GenericEventData());
			howlingUp = false;
		}

		// TODO: Do something for anims
		IsHowling = true;
	}
}

void AWolf::disengageTideMode()
{
	m_currentState = WolfState::Idle;

	UE_LOG(LogTemp, Warning, TEXT("%f"), howlDownAxisValue + howlUpAxisValue);

	if (IsHowling)
	{
		if(howlingUp) {
			m_gameMode->getEventManager().raiseEvent("OnStopHowlUp", new GenericEventData());
			UE_LOG(LogTemp, Warning, TEXT("Raiseup"));
		} else {
			m_gameMode->getEventManager().raiseEvent("OnStopHowlDown", new GenericEventData());
			UE_LOG(LogTemp, Warning, TEXT("Raisedown"));
		}

		// TODO: Do something for anims
		IsHowling = false;
	}

	howlingUp = false;

	howlUpAxisValue = 0;
	howlDownAxisValue = 0;
}

void AWolf::howlUp(float axisValue)
{
	howlUpAxisValue = axisValue;
}

void AWolf::howlDown(float axisValue)
{
	howlDownAxisValue = axisValue;
}

void AWolf::toggleOverview()
{
	if (!m_currentOverviewCam) return;

	if (m_currentState == WolfState::Idle)
	{
		if (!isInOverviewingState)
		{
			((APlayerController *)GetController())->SetViewTarget(m_overviewTransitioNCam, FViewTargetTransitionParams());

			m_overviewTransitioNCam->SetActorLocation(FollowCamera->GetComponentLocation());
			m_overviewTransitioNCam->SetActorRotation(FollowCamera->GetComponentRotation());

			setState(WolfState::LerpingToOverview);
		}
		else
		{
			setState(WolfState::LerpingFromOverview);
		}
		
	}
}

void AWolf::quitOverview()
{
	if (isInOverviewingState)
	{
		toggleOverview();
	}
}

void AWolf::setDyingState(bool state)
{
	if (state)
	{
		setState(WolfState::Dying);
		IsDying = true;
		GetCharacterMovement()->DisableMovement();

		if (OnDied.IsBound())
		{
			OnDied.Broadcast();
		}
	}
	else if (m_currentState == WolfState::Dying)
	{
		setState(WolfState::Idle);

		if (OnWolfRespawned.IsBound())
		{
			OnWolfRespawned.Broadcast();

		}

		IsDying = false;
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
}

bool AWolf::isDead()
{
	return m_currentState == WolfState::Dying;
}

void AWolf::setState(WolfState newState)
{
	if (m_currentState == WolfState::ChangingTides)
	{
		disengageTideMode();
	}

	m_currentState = newState;
}

void AWolf::onWaterLevelChanged(OnWaterLevelChangedData *eventData)
{
	m_reachedLowestTide = eventData->getNewWaterDelta() <= eventData->getMinDelta();
	m_reachedHighestTide = eventData->getNewWaterDelta() >= eventData->getCurrentMaxDelta();
}

void AWolf::setLastCheckpointReference(ACheckpoint *lastCheckpoint)
{
	m_lastCheckpointReference = lastCheckpoint;
}

void AWolf::setCanDie(bool canDie)
{
	m_canDie = canDie;
}

bool AWolf::getCanDie()
{
	return m_canDie;
}

void AWolf::manageIKTraces(float deltaTime)
{
	RearLeftFootGroundLevelIKOffset = UKismetMathLibrary::FInterpTo(
		RearLeftFootGroundLevelIKOffset,
		getFootGroundLevelIKOffset(RearLeftFootSocketName),
		deltaTime,
		IKInterpSpeed);

	RearRightFootGroundLevelIKOffset = UKismetMathLibrary::FInterpTo(
		RearRightFootGroundLevelIKOffset,
		getFootGroundLevelIKOffset(RearRightFootSocketName),
		deltaTime,
		IKInterpSpeed);

	FrontLeftFootGroundLevelIKOffset = UKismetMathLibrary::FInterpTo(
		FrontLeftFootGroundLevelIKOffset,
		getFootGroundLevelIKOffset(FrontLeftFootSocketName),
		deltaTime,
		IKInterpSpeed);

	FrontRightFootGroundLevelIKOffset = UKismetMathLibrary::FInterpTo(
		FrontRightFootGroundLevelIKOffset,
		getFootGroundLevelIKOffset(FrontRightFootSocketName),
		deltaTime,
		IKInterpSpeed);

	ForwardWallIsClose = forwardWallIsClose();
}

bool AWolf::forwardWallIsClose()
{
	FVector traceStart = GetCapsuleComponent()->GetComponentLocation();
	traceStart.Z = HeadComponent->GetComponentLocation().Z;

	FVector traceEnd = traceStart + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius() + 5;

	FCollisionQueryParams queryParams = FCollisionQueryParams();
	queryParams.AddIgnoredActor(this);

	FCollisionShape sphere = FCollisionShape::MakeCapsule(10, 15);

	return GetWorld()->SweepTestByChannel(traceStart, traceEnd, GetCapsuleComponent()->GetComponentQuat(), ECC_Visibility, sphere, queryParams);
}

float AWolf::getFootGroundLevelIKOffset(FName socketName)
{
	FHitResult hitResult;
	
	FVector traceStart = GetMesh()->GetSocketLocation(socketName);
	traceStart.Z = GetActorLocation().Z;

	FVector traceEnd = traceStart - FVector(0, 0, m_scaledHalfHeight);

	FCollisionQueryParams queryParams = FCollisionQueryParams();
	queryParams.AddIgnoredActor(this);

	bool hitGround = GetWorld()->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, ECollisionChannel::ECC_Visibility, queryParams);

	float offset = 0;

	if (hitGround)
	{
		offset = (traceEnd - hitResult.ImpactPoint).Size() / GetActorScale().Z;
	}
	/*
	else
	{
		// If the feet are not under the ground, they are over the ground so we try to lower them
		traceStart = traceEnd;
		traceEnd = traceStart - FVector(0, 0, 50);

		hitGround = GetWorld()->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, ECollisionChannel::ECC_Visibility, queryParams);
		DrawDebugLine(GetWorld(), traceStart, traceEnd, FColor::Red);
		
		offset = hitGround ? -hitResult.Distance / GetActorScale().Z : 0;

		UE_LOG(LogTemp, Warning, TEXT("%f"), offset);
	}*/

	

	return offset;
}

ACheckpoint *AWolf::getLastCheckpointReference()
{
	return m_lastCheckpointReference;
}

void AWolf::traceFootSteps()
{
	traceFootStepBySocket(RearLeftFootSocketName, rearLeftFootOnGround);
	traceFootStepBySocket(RearRightFootSocketName, rearRightFootOnGround);
	traceFootStepBySocket(FrontLeftFootSocketName, frontLeftFootOnGround);
	traceFootStepBySocket(FrontRightFootSocketName, frontRightFootOnGround);
}

void AWolf::traceFootStepBySocket(FName socketName, bool &footOnGround)
{
	FHitResult hitResult;

	FVector traceStart = GetMesh()->GetSocketLocation(socketName);
	FVector traceEnd = traceStart - FVector(0, 0, 5);

	FCollisionQueryParams queryParams = FCollisionQueryParams();
	queryParams.AddIgnoredActor(this);

	bool hitGround = GetWorld()->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, ECollisionChannel::ECC_Visibility, queryParams);

	float offset = 0;

	if(!footOnGround && hitGround)
	{
		offset = (traceEnd - hitResult.ImpactPoint).Size() / GetActorScale().Z;

		if (OnFootStep.IsBound())
		{
			OnFootStep.Broadcast();
		}

		//UE_LOG(LogTemp, Warning, TEXT("Foot hit the ground"));
	}

	footOnGround = hitGround;
}