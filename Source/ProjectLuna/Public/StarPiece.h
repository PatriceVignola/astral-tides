// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LunaLog.h"
#include "Wolf.h"
#include "ProjectLunaGameMode.h"
#include "OnCheckpointReachedData.h"
#include "GenericEventData.h"
#include "Runtime/Engine/Classes/Sound/SoundCue.h"
#include "StarPiece.generated.h"

class ALunaLevelScriptActor;

DECLARE_LUNA_LOG(StarPiece);

UCLASS()
class PROJECTLUNA_API AStarPiece : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent *Mesh;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent *Collider;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundCue* CollectSound;

	AStarPiece();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick( float DeltaSeconds ) override;

	// The UFUNCTION() macro is necessary for collision callbacks
	UFUNCTION()
	void OnBeginOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

private:
	UWorld *m_world;
	AProjectLunaGameMode *m_gameMode;
	ALunaLevelScriptActor *m_levelScript;
};
