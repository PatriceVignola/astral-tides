// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "StarLink.generated.h"

UCLASS()
class PROJECTLUNA_API AStarLink : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere)
	USceneComponent *Root;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent *Beam;

	AStarLink();
	virtual void BeginPlay() override;

	void setStartEndPositions(FVector startPos, FVector endPos);
};
