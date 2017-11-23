// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LunaLog.h"
#include "GameFramework/Actor.h"
#include "Classes/Components/SplineComponent.h"
#include "Classes/Components/SplineMeshComponent.h"
#include "StreamSection.h"
#include "ProjectLunaGameMode.h"
#include "OnStreamSpawnedData.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "OnWaterLevelChangedData.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "StreamEntity.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FChangeStreamTypeDelegate, float, matWind, float, matType);

class ALunaLevelScriptActor;

UCLASS()
class PROJECTLUNA_API AStreamEntity : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere)
	float InitialWaterLevel = 70;

	UPROPERTY(EditAnywhere)
	float Width;

	UPROPERTY(EditAnywhere)
	float MinHeight;

	UPROPERTY(EditAnywhere)
	float MaxHeight;

	UPROPERTY(EditAnywhere)
	float Force;

	UPROPERTY(EditAnywhere)
	float NormalMod;

	UPROPERTY(EditAnywhere)
	float NormalSpeedMod;

	UPROPERTY(EditAnywhere)
	float TargetSpeed;

	UPROPERTY(EditAnywhere)
	float SectionLength;

	UPROPERTY(EditAnywhere)
	bool ShowMarks;

	UPROPERTY(EditAnywhere)
	float LineWidth;

	UPROPERTY(EditAnywhere)
	float MarksHeight;

	// This event has to be listened in the blueprint
	UPROPERTY(BlueprintAssignable)
	FChangeStreamTypeDelegate OnChangeStreamType;

	// Sets default values for this actor's properties
	AStreamEntity();
	~AStreamEntity();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Applies its force on a component that passes through it
	virtual void ApplyForce(UPrimitiveComponent *comp);

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	bool IsReversed();
	void Reverse();
	void SetReversed(bool isReversed);

	StreamSection* IsInsideStream(FVector pos);
	StreamSection* IsInsideStream2(FVector pos);

	const TArray<StreamSection*>& getSections() const;

	void applyImpulse(UPrimitiveComponent *comp);

	FVector getClosestPointOnSpline(FVector otherPoint);

	bool containsBlock();

private:
	enum class StreamState {Surface, InTheAir, UnderWater};

	StreamState currentState;

	void CreateSections(USplineComponent* spline);
	void onWaterLevelChanged(OnWaterLevelChangedData *eventData);

	// Hack
	float m_initialWaterLevel = 70;

	bool m_hasAlertedObjects;

	UWorld *m_world;
	ALunaLevelScriptActor *m_levelScript;
	AProjectLunaGameMode *m_gameMode;

	TArray<StreamSection*> m_sections;
	FVector m_min;
	FVector m_max;

	bool m_isReversed;

	TArray<USplineMeshComponent*> m_meshes;

	void updateStreamMaterial();

	float currentWaterLevel;

	USplineComponent *m_spline;

	bool bContainsBlock;
};