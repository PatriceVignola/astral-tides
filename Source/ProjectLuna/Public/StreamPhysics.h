// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "StreamEntity.h"
#include "StreamPhysics.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTLUNA_API UStreamPhysics : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStreamPhysics();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

private:

	TArray<AStreamEntity*> m_streams;
	
};
