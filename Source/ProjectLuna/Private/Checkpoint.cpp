// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "Checkpoint.h"
#include "Wolf.h"

DEFINE_LUNA_LOG(Checkpoint);

// Sets default values
ACheckpoint::ACheckpoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;
}

// Called when the game starts or when spawned
void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();
	
	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode*)m_world->GetAuthGameMode();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnBeginOverlap);

	m_gameMode->getEventManager().subscribe("OnWolfJumped", this, &ACheckpoint::onWolfJumped);
	m_gameMode->getEventManager().subscribe("OnWolfLanded", this, &ACheckpoint::onWolfLanded);
}

void ACheckpoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	TriggerBox->OnComponentBeginOverlap.RemoveDynamic(this, &ACheckpoint::OnBeginOverlap);

	if (m_gameMode)
	{
		m_gameMode->getEventManager().unsubscribe("OnWolfJumped", this, &ACheckpoint::onWolfJumped);
		m_gameMode->getEventManager().unsubscribe("OnWolfLanded", this, &ACheckpoint::onWolfLanded);
		m_gameMode = nullptr;
	}

	m_world = nullptr;
}

void ACheckpoint::OnBeginOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor->IsA(AWolf::StaticClass()))
	{
		AWolf *wolf = (AWolf*)OtherActor;

		if (wolf->getLastCheckpointReference() != this)
		{
			//LUNA_WARN(IceBlock, "New checkpoint has been reached");

			wolf->setLastCheckpointReference(this);

			m_gameMode->getEventManager().raiseEvent("OnCheckpointReached", new OnCheckpointReachedData(GetActorLocation(), GetActorRotation()));
		}
	}
}

void ACheckpoint::onWolfJumped(OnWolfJumpedData *eventData)
{
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACheckpoint::onWolfLanded(OnWolfLandedData *eventData)
{
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}