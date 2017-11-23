// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "OrquaDummy.h"
#include "Orqua.h"
#include "WaterPlane.h"

DEFINE_LUNA_LOG(OrquaDummy);

// Sets default values
AOrquaDummy::AOrquaDummy()
	: m_owner(nullptr)
	, m_isMoving(false)
	, m_isUnderwater(false)
	, m_divingOffset(0.f)
	, m_maxDivingOffset(250)
	, m_divingStep(5)
	, m_distanceRaycast(1000.f)
	/*, m_hasReached(true)*/
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AOrquaDummy::setOwner(AOrqua* orqua) {
	m_owner = orqua;
	m_traceParams.AddIgnoredActor(orqua);
}

// Called when the game starts or when spawned
void AOrquaDummy::BeginPlay() {
	Super::BeginPlay();

	m_world = GetWorld();
	m_previousLocation = GetActorLocation();

	m_traceParams.AddIgnoredActor(this);
	for(TActorIterator<AActor> ActorItr(m_world); ActorItr; ++ActorItr) {
		AActor *actor = *ActorItr;
		if(actor->IsA(AWaterPlane::StaticClass())) {
			m_traceParams.AddIgnoredActor(actor);
		}
	}
}

void AOrquaDummy::EndPlay(const EEndPlayReason::Type reason) {
	Super::EndPlay(reason);

	m_owner = nullptr;
}

// Called every frame
void AOrquaDummy::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	FRotator a1 = GetActorRotation() + (m_targetRot - GetActorRotation()).GetNormalized() * 2 * DeltaTime;

	SetActorRotation(a1);

	/*
	FRotator a2 = FMath::Lerp(GetActorRotation(), m_targetRot, 0.05);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *a1.ToString());
	UE_LOG(LogTemp, Warning, TEXT("%s"), *a2.ToString());
	UE_LOG(LogTemp, Warning, TEXT("**********************"));*/
}

void AOrquaDummy::startMoving() {
	m_isMoving = true;
}

void AOrquaDummy::stopMoving() {
	m_isMoving = false;
}

const bool AOrquaDummy::isMoving() const {
	return m_isMoving;
}

void AOrquaDummy::dive() {
	m_isUnderwater = true;
}

void AOrquaDummy::surface() {
	/*auto currentLocation = GetActorLocation();

	SetActorLocation(FVector(currentLocation.X, currentLocation.Y, m_waterLevel));*/
	m_isUnderwater = false;
}

void AOrquaDummy::setWaterLevel(float waterLevel) {
	m_waterLevel = waterLevel;
}

void AOrquaDummy::move(float x, float y, float waterLevel) {
	int xDirection = (x < m_previousLocation.X) ? -1 : 1;
	int yDirection = (y < m_previousLocation.Y) ? -1 : 1;

	FVector frontEnd(x + (m_distanceRaycast * xDirection), y + (m_distanceRaycast * yDirection), GetActorLocation().Z);

	FHitResult frontHit;
	m_world->LineTraceSingleByChannel(frontHit, GetActorLocation(), frontEnd, ECollisionChannel::ECC_WorldStatic, m_traceParams);

	if(frontHit.Actor != nullptr) {
		dive();
	}

	if(m_isUnderwater) {
		FVector underwaterEnd(x /*+ ((m_distanceRaycast) * xDirection)*/, y /*+ ((m_distanceRaycast) * yDirection)*/, waterLevel);

		FHitResult underwaterHit;
		m_world->LineTraceSingleByChannel(underwaterHit, GetActorLocation(), underwaterEnd, ECollisionChannel::ECC_WorldStatic, m_traceParams);

		if(underwaterHit.Actor == nullptr) {
			surface();
		}
	}

	if(m_isUnderwater) {
		m_divingOffset += m_divingStep;
		if(m_divingOffset >= m_maxDivingOffset + waterLevel) {
			m_divingOffset = m_maxDivingOffset + waterLevel;
		}
	} else {
		m_divingOffset -= m_divingStep;
		if(m_divingOffset < 0) {
			m_divingOffset = 0;
		}
	}

	/*FVector v(x, y, waterLevel);
	auto rot = FRotationMatrix::MakeFromYX(v - GetActorLocation(), GetActorLocation());*/

	/*auto rotY = FRotationMatrix::MakeFromYX(v - GetActorLocation(), GetActorLocation()).Rotator();
	SetActorRotation(rotY);*/

	/*FVector dummyLoc = GetActorLocation();
	FVector2D nextLoc(x, y);

	FVector2D result;
	result.X = -(nextLoc.X - dummyLoc.X);
	result.Y = nextLoc.Y - dummyLoc.Y;

	float angle = FMath::RadiansToDegrees(FMath::Atan2(result.Y, result.X));

	if(result.X < 0 || result.Y < 0) {
	angle = 360 - angle;
	}

	FRotator rot(0, -149.99986, 0);

	SetActorRotation(rot);*/
	/*FVector v;
	FVector dummyLoc = GetActorLocation();
	v.X = x - dummyLoc.X;
	v.Y = y - dummyLoc.Y;
	v.Z = waterLevel - m_divingOffset - dummyLoc.Z;

	if(!m_hasReached) {
	m_targetRot = v.Rotation();
	m_targetRot.Yaw -= 90;
	}
	_
	rot.Yaw -= 90;*/
	//if(v.X != 0 || v.Y != 0) {
	/*if(m_currentRot != m_targetRot) {
	m_currentRot.Yaw += m_targetRot.Yaw * 0.01;
	}*/

	/*if(!m_hasReached) {
	SetActorRotation(m_currentRot);
	}*/

	//}

	/*FVector v;
	FVector dummyLoc = GetActorLocation();
	v.X = x - dummyLoc.X;
	v.Y = y - dummyLoc.Y;
	v.Z = waterLevel - m_divingOffset - dummyLoc.Z;
	FRotator rot = v.Rotation();*/

	/*SetActorRotation(rot);*/

	SetActorLocation(FVector(x, y, waterLevel - m_divingOffset));
}

void AOrquaDummy::setRaycastDistance(float distance) {
	m_distanceRaycast = distance;
}

//void AOrquaDummy::setHasReached(bool reached) {
//	m_hasReached = reached;
//}

void AOrquaDummy::findRotation(float x, float y, float waterLevel) {
	FVector v;
	FVector dummyLoc = GetActorLocation();
	v.X = x - dummyLoc.X;
	v.Y = y - dummyLoc.Y;
	v.Z = waterLevel - m_divingOffset - dummyLoc.Z;
	m_targetRot = v.Rotation();
	m_targetRot.Yaw -= 90;
	/*static bool m = false;
	if(!m) {
	m = true;
	m_currentRot = m_targetRot;
	m_currentRot.Yaw *= 0.01;
	}*/
	/*m_currentRot = m_targetRot;
	m_currentRot.Yaw *= 0.01;*/
}