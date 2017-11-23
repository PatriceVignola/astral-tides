// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "OrquaDummy.generated.h"

class AOrqua;

DECLARE_LUNA_LOG(OrquaDummy);

UCLASS()
class PROJECTLUNA_API AOrquaDummy : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AOrquaDummy();

	void setOwner(AOrqua* orqua);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	void startMoving();
	void stopMoving();
	const bool isMoving() const;
	void dive();
	void surface();
	void setWaterLevel(float waterLevel);
	void move(float x, float y, float waterLevel);
	void setRaycastDistance(float distance);
	/*void setHasReached(bool reached);*/
	void findRotation(float x, float y, float waterLevel);

private:
	AOrqua* m_owner;
	UWorld* m_world;
	FVector m_previousLocation;
	bool m_isMoving;
	float m_waterLevel;
	bool m_isUnderwater;
	FCollisionQueryParams m_traceParams;
	float m_divingOffset;
	float m_maxDivingOffset;
	float m_divingStep;
	float m_distanceRaycast;
	FRotator m_targetRot;
	FRotator m_currentRot;
	/*bool m_hasReached;*/
};
