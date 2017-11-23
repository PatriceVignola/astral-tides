// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "WaterPlane.h"
#include "BuoyantObjectRuntime.h"

DEFINE_LUNA_LOG(WaterPlane);

// Book keeping for the water cubes

// Dont rely on the constructor too much since its unreliable in the editor (sometimes executed more than once, particularly for the first run after a new compile)
AWaterPlane::AWaterPlane()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	OverlapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	OverlapMesh->AttachTo(RootComponent);
}

// Called when the game starts or when spawned
void AWaterPlane::BeginPlay()
{
	Super::BeginPlay();

	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode *)m_world->GetAuthGameMode();

	cacheLevelActors();

	initialWaterHeight = GetActorLocation().Z;

	// If the water plane's begin play is called before the wolf is spawned, we need to be notified in the future
	m_gameMode->getEventManager().subscribe("OnWolfSpawned", this, &AWaterPlane::onWolfSpawned);
	m_gameMode->getEventManager().subscribe("OnWolfDestroyed", this, &AWaterPlane::onWolfDestroyed);
}

void AWaterPlane::cacheLevelActors()
{
	for (TActorIterator<AActor> ActorItr(m_world); ActorItr; ++ActorItr)
	{
		// Commented out for now because of the bug in Unreal Engine
		// https://answers.unrealengine.com/questions/294580/get-all-actors-of-class-with-level-streaming-broke.html
		/*
		if (ActorItr->IsA(ABuoyantObjectRuntime::StaticClass()))
		{
			ABuoyantObjectRuntime *buoyantObjectRuntime = Cast<ABuoyantObjectRuntime>(*ActorItr);
			m_levelBuoyantObjectRuntimes.Add(buoyantObjectRuntime);
		}*/

		if (!m_wolf && ActorItr->IsA(AWolf::StaticClass()))
		{
			m_wolf = Cast<AWolf>(*ActorItr);
		}
	}
}

// Called every frame
void AWaterPlane::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	checkWolfOverlap();
	checkBuoyantObjectRuntimesOverlap();
}

void AWaterPlane::checkWolfOverlap()
{
	if (!m_wolf || !m_wolf->IsValidLowLevel()) return;
	if (!m_wolf->getCanDie()) return;

	//FVector colliderExtent = m_wolf->WaterCollider->GetScaledBoxExtent();

	float waterColliderZ = m_wolf->GetActorLocation().Z + 35;
	float colliderExtentZ = 17;


	if (!m_wolf->isDead() && waterColliderZ - colliderExtentZ <= GetActorLocation().Z)
	{
		/*
		LUNA_WARN(WaterPlane, "colliderExtent: %s", *colliderExtent.ToString());
		LUNA_WARN(WaterPlane, "m_wolf->WaterCollider->GetComponentLocation().Z: %f", m_wolf->WaterCollider->GetComponentLocation().Z);
		LUNA_WARN(WaterPlane, "m_wolf->WaterCollider->GetRelativeTransform().Translation.Z: %f", m_wolf->WaterCollider->GetRelativeTransform().GetTranslation().Z);
		LUNA_WARN(WaterPlane, "m_wolf->GetActorLocation().Z: %f", m_wolf->GetActorLocation().Z);
		LUNA_WARN(WaterPlane, "GetActorLocation().Z: %f", GetActorLocation().Z);*/


		LUNA_WARN(WaterPlane, "Debug: Wolf entered water");

		m_gameMode->getEventManager().raiseEvent("OnWolfDrown", new OnWolfDrownData());
	}
}

void AWaterPlane::checkBuoyantObjectRuntimesOverlap()
{
	float planeLocationZ = GetActorLocation().Z;

	// WE WILL TRY THIS TECHNIQUE FOR NOW AND PROFILE TO SEE IF THERE IS ANY SIGNIFICANT PERFORMANCE ISSUE LATER ON
	// UNREAL ENGINE HAS A HARD LIMIT FOR OVERLAPPING ACTORS, AND THE WATER PLANE OVERLAPS WITH A LOT OF ACTORS
	for (ABuoyantObjectRuntime* BuoyantObjectRuntime : m_levelBuoyantObjectRuntimes)
	{
		FVector origin, boxExtent;
		
		BuoyantObjectRuntime->GetActorBounds(true, origin, boxExtent);

		if (BuoyantObjectRuntime->isInWater() && origin.Z - boxExtent.Z > planeLocationZ)
		{
			BuoyantObjectRuntime->endWaterOverlap(this);
		}
		else if (!BuoyantObjectRuntime->isInWater() && origin.Z - boxExtent.Z <= planeLocationZ)
		{
			BuoyantObjectRuntime->beginWaterOverlap(this);
		}
	}
}

void AWaterPlane::setWaterHeightDelta(float heightDelta)
{
	FVector prevPos = this->GetActorLocation();
	FVector newPos = FVector(prevPos.X, prevPos.Y, initialWaterHeight + heightDelta);

	this->SetActorLocation(newPos);
}

void AWaterPlane::onWolfSpawned(OnWolfSpawnedData *eventData)
{
	if (m_levelIndex == 0) return;

	m_wolf = eventData->getWolf();
}

void AWaterPlane::cacheBuoyantObject(ABuoyantObjectRuntime *buoyantObject)
{
	m_levelBuoyantObjectRuntimes.Add(buoyantObject);
}

void AWaterPlane::clearCachedBuoyantObjects()
{
	m_levelBuoyantObjectRuntimes.Empty();

	LUNA_WARN(WaterPlane, "CLEARING CACHE!!!");

	//m_wolf = nullptr;
}

void AWaterPlane::cacheCurrentLevelIndex(int index)
{
	m_levelIndex = index;

	if (!m_wolf && index != 0)
	{
		cacheLevelActors();
	}
}

void AWaterPlane::onWolfDestroyed(OnWolfSpawnedData *eventData)
{
	m_wolf = nullptr;
}