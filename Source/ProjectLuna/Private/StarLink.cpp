// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "ParticleEmitterInstances.h"
#include "StarLink.h"


// Sets default values
AStarLink::AStarLink()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Beam = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Beam"));
	Beam->AttachTo(RootComponent);
}

// Called when the game starts or when spawned
void AStarLink::BeginPlay()
{
	Super::BeginPlay();
	
}

void AStarLink::setStartEndPositions(FVector startPos, FVector endPos)
{
	Beam->EmitterInstances[0]->SetBeamSourcePoint(startPos, 0);
	Beam->EmitterInstances[0]->SetBeamTargetPoint(endPos, 0);
}

