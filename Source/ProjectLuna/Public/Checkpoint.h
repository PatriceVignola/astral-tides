// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LunaLog.h"
#include "OnCheckpointReachedData.h"
#include "ProjectLunaGameMode.h"
#include "EventManager.h"
#include "OnWolfJumpedData.h"
#include "OnWolfLandedData.h"
#include "Checkpoint.generated.h"

DECLARE_LUNA_LOG(Checkpoint);

UCLASS()
class PROJECTLUNA_API ACheckpoint : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent *TriggerBox;

	// Sets default values for this actor's properties
	ACheckpoint();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnBeginOverlap(AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	void onWolfJumped(OnWolfJumpedData *eventData);
	void onWolfLanded(OnWolfLandedData *eventData);

private:
	UWorld *m_world;
	AProjectLunaGameMode *m_gameMode;
};
