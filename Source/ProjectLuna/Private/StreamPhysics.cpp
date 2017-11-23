// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "StreamPhysics.h"


// Sets default values for this component's properties
UStreamPhysics::UStreamPhysics()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}


// Called when the game starts
void UStreamPhysics::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("DEPRECATED CLASS"));

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStreamEntity::StaticClass(), FoundActors);

	for (AActor* TActor : FoundActors)
	{
		AStreamEntity* MyActor = (AStreamEntity*)TActor;

		if (MyActor != nullptr)
		{
			m_streams.Add(MyActor);

			//TODO Remove
			//UE_LOG(LogTemp, Warning, TEXT("Found a stream!"));
		}
	}
}


// Called every frame
void UStreamPhysics::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	for (AStreamEntity* stream : m_streams)
	{
		//stream->ApplyForce(GetOwner());
	}
}

