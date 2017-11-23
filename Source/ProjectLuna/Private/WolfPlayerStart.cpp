#include "ProjectLuna.h"
#include "WolfPlayerStart.h"


AWolfPlayerStart::AWolfPlayerStart()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AWolfPlayerStart::BeginPlay()
{
	Super::BeginPlay();

	((AProjectLunaGameMode*)GetWorld()->GetAuthGameMode())->getEventManager()
		.raiseEvent("OnCheckpointReached", new OnCheckpointReachedData(GetActorLocation(), GetActorRotation()));
}

