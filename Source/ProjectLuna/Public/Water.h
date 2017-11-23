// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Classes/Components/SplineComponent.h"
#include "Classes/Components/SplineMeshComponent.h"
#include "Wolf.h"
#include "WolfController.h"
#include "LunaLog.h"
#include "Water.generated.h"

DECLARE_LUNA_LOG(Water);

UCLASS()
class PROJECTLUNA_API AWater : public AActor
{
	GENERATED_BODY()
	
public:	
	
	UPROPERTY(VisibleAnywhere)
	class USplineComponent *WaterSpline;
	
	UPROPERTY(VisibleAnywhere)
	class UStaticMesh *WaterMesh;

	AWater();

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void OnConstruction(const FTransform& Transform);

	// The UFUNCTION() macro is necessary for collision callbacks
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

private:
	// TODO: Improve after prototype
	float m_deltaHeight = 200;
	float m_currentTarget;
	bool m_isRising;
	float m_elapsedTime;

	void raiseWaterLevel();
};
