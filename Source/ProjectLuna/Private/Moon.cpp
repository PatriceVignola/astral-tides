#include "ProjectLuna.h"
#include "Moon.h"
#include "LunaLevelScriptActor.h"

DEFINE_LUNA_LOG(Moon);

AMoon::AMoon()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->AttachTo(RootComponent);
}

void AMoon::BeginPlay()
{
	Super::BeginPlay();

	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode*)m_world->GetAuthGameMode();

	// We cast the level script to our own, custom level script
	if (m_world->GetLevelScriptActor() && m_world->GetLevelScriptActor()->IsA(ALunaLevelScriptActor::StaticClass()))
	{
		m_levelScript = Cast<ALunaLevelScriptActor>(m_world->GetLevelScriptActor());
	}
	else
	{
		LUNA_ERROR(Moon, "Error: The level blueprint needs to inherit from ALunaLevelScriptActor");
	}

	updateFullMoonRatio(0);

	m_gameMode->getEventManager().subscribe("OnWaterLevelChanged", this, &AMoon::onWaterLevelChanged);
	m_gameMode->getEventManager().subscribe("OnLevelLoaded", this, &AMoon::onLevelLoaded);
}

void AMoon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (m_gameMode)
	{
		m_gameMode->getEventManager().unsubscribe("OnLevelLoaded", this, &AMoon::onLevelLoaded);
		m_gameMode->getEventManager().unsubscribe("OnWaterLevelChanged", this, &AMoon::onWaterLevelChanged);

		m_levelScript = nullptr;
		m_gameMode = nullptr;
		m_world = nullptr;
	}
}

void AMoon::onWaterLevelChanged(OnWaterLevelChangedData *eventData)
{
	updateFullMoonRatio(eventData->getNewWaterDelta());
}

void AMoon::updateFullMoonRatio(float newWaterDelta)
{
	if (!m_levelScript || m_gameMode->getLevelNumber() == 0)
	{
		LUNA_WARN(Moon, "The level blueprint needs to inherit from ALunaLevelScriptActor");
		return;
	}

	float fullMoonRatio = FMath::Max(0.1f, newWaterDelta / m_levelScript->getMaxWaterDelta());

	// This event will be listened in the blueprint
	if (OnWaterChanged.IsBound())
	{
		OnWaterChanged.Broadcast(fullMoonRatio);
	}
	else
	{
		LUNA_WARN(Moon, "The Moon needs to be bound to the OnWaterChanged event to react to the water level");
	}
}

void AMoon::onLevelLoaded(OnLevelLoadedData *eventData)
{
	if (OnWaterChanged.IsBound())
	{
		if (eventData->getLevelNumber() == 0)
		{
			OnWaterChanged.Broadcast(1);
		}
		else
		{
			OnWaterChanged.Broadcast(0.1);
		}
	}
}