// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LunaLog.h"
#include "OnWolfRespawnedData.h"
#include "OnCheckpointReachedData.h"
#include "OnBuoyantObjectSpawnedData.h"
#include "ProjectLunaGameMode.h"
#include "Kismet/KismetMathLibrary.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"
#include "StreamEntity.h"
#include "BuoyantObjectRuntime.h"
#include "IceBlockRuntime.h"
#include "BuoyantObject.generated.h"

// Forward declaraction
class AWaterPlane;
class AIceBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartFallingSignature);

DECLARE_LUNA_LOG(BuoyantObject);

UCLASS()
class PROJECTLUNA_API ABuoyantObject : public AActor
{
	GENERATED_BODY()

public:
	enum class BuoyantObjectState { Grounded, FreeFalling, Floating };

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent *Mesh;

	UPROPERTY(EditAnywhere)
	float Acceleration;
	UPROPERTY(EditAnywhere)
	float MaxSpeed;
	UPROPERTY(EditAnywhere)
	float Radius;
	UPROPERTY(EditAnywhere)
	float NormalMod;
	//UPROPERTY(VisibleAnywhere)
	//float Speed;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ABuoyantObjectRuntime> BuoyantObjectRuntimeClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AIceBlockRuntime> IceBlockRuntimeClass;

	// Sets default values for this actor's properties
	ABuoyantObject();

	virtual void BeginPlay() override;

protected:
	BuoyantObjectState m_currentState;
	FVector m_blockSize; // cm
	UBoxComponent *m_boundingBox;

private:
	TArray<FVector> m_unitVectors; // This is the i, j, k vectors

	UWorld *m_world;
	AProjectLunaGameMode *m_gameMode;

	float m_blockVolume; // cm^3

	void instantiateBoundingBoxCollider(const FVector &boxExtent, const FVector &origin);
};
