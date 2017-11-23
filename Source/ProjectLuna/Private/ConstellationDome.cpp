// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "ConstellationDome.h"

DEFINE_LUNA_LOG(ConstellationDome)

// Sets default values
AConstellationDome::AConstellationDome()
{
	PrimaryActorTick.bCanEverTick = false;

	InvisibleDome = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InvisibleDome"));
	RootComponent = InvisibleDome;

	LupusWolfConstellation = CreateDefaultSubobject<USceneComponent>(TEXT("LupusWolfConstellation"));
	LupusWolfConstellation->AttachTo(RootComponent);

	MenuCassiopeiaConstellation = CreateDefaultSubobject<USceneComponent>(TEXT("MenuCassiopeiaConstellation"));
	MenuCassiopeiaConstellation->AttachTo(RootComponent);

	MenuUrsaMajorConstellation = CreateDefaultSubobject<USceneComponent>(TEXT("MenuUrsaMajorConstellation"));
	MenuUrsaMajorConstellation->AttachTo(RootComponent);

	MenuLeoConstellation = CreateDefaultSubobject<USceneComponent>(TEXT("MenuLeoConstellation"));
	MenuLeoConstellation->AttachTo(RootComponent);

}

// Called when the game starts or when spawned
void AConstellationDome::BeginPlay()
{
	Super::BeginPlay();

	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode *)m_world->GetAuthGameMode();

	// TODO: Make all this more generic
	cacheStars();

	cacheCassiopeiaStars();
	cacheUrsaMajorStars();
	cacheLeoStars();

	initStars();

	m_gameMode->getEventManager().subscribe("OnStarCollected", this, &AConstellationDome::onStarCollected);
	m_gameMode->getEventManager().subscribe("OnCheckpointReached", this, &AConstellationDome::onCheckpointReached);
	m_gameMode->getEventManager().subscribe("OnLevelLoaded", this, &AConstellationDome::onLevelLoaded);
	//m_gameMode->getEventManager().subscribe("OnMenuLevelSelected", this, &AConstellationDome::onMenuLevelSelected);
}

void AConstellationDome::cacheStars()
{
	TArray<USceneComponent *> stars;

	LupusWolfConstellation->GetChildrenComponents(false, stars);

	int starsCount = stars.Num();

	for (int i = 0; i < starsCount; i++)
	{
		USceneComponent *star = stars[i];

		if (star->IsA(UStarComponent::StaticClass()))
		{
			FString starName;

			UStarComponent *castedStar = Cast<UStarComponent>(star);
			castedStar->GetName(starName);

			m_stars.Add(starName, castedStar);
			m_checkpointStates.Add(starName, false);
		}
	}
}

void AConstellationDome::cacheCassiopeiaStars()
{
	TArray<USceneComponent *> stars;

	MenuCassiopeiaConstellation->GetChildrenComponents(false, stars);

	int starsCount = stars.Num();

	for (int i = 0; i < starsCount; i++)
	{
		USceneComponent *star = stars[i];

		if (star->IsA(UStarComponent::StaticClass()))
		{
			FString starName;

			UStarComponent *castedStar = Cast<UStarComponent>(star);
			castedStar->GetName(starName);

			m_menuCassiopeiaStars.Add(starName, castedStar);
		}
	}
}

void AConstellationDome::cacheUrsaMajorStars()
{
	TArray<USceneComponent *> stars;

	MenuUrsaMajorConstellation->GetChildrenComponents(false, stars);

	int starsCount = stars.Num();

	for (int i = 0; i < starsCount; i++)
	{
		USceneComponent *star = stars[i];

		if (star->IsA(UStarComponent::StaticClass()))
		{
			FString starName;

			UStarComponent *castedStar = Cast<UStarComponent>(star);
			castedStar->GetName(starName);

			m_menuUrsaMajorStars.Add(starName, castedStar);
		}
	}
}

void AConstellationDome::cacheLeoStars()
{
	TArray<USceneComponent *> stars;

	MenuLeoConstellation->GetChildrenComponents(false, stars);

	int starsCount = stars.Num();

	for (int i = 0; i < starsCount; i++)
	{
		USceneComponent *star = stars[i];

		if (star->IsA(UStarComponent::StaticClass()))
		{
			FString starName;

			UStarComponent *castedStar = Cast<UStarComponent>(star);
			castedStar->GetName(starName);

			m_menuLeoStars.Add(starName, castedStar);
		}
	}
}

void AConstellationDome::initStars()
{
	// TODO: Later, since the star pieces will be assigned to a unique constellation star, we will have a
	// graph of connected stars and we will put them in a string array in the blueprint instead of only the
	// previous one

	
	for (auto kvp : m_menuCassiopeiaStars)
	{
		if (m_menuCassiopeiaStars.Find(kvp.Value->PreviousStarName) == nullptr) continue;

		UStarComponent *currentStar = kvp.Value;
		UStarComponent *linkedStar = m_menuCassiopeiaStars[currentStar->PreviousStarName];

		currentStar->initialize(linkedStar, StarLinkGreyClass, StarLinkWhiteClass, m_world, true);
	}

	for (auto kvp : m_menuUrsaMajorStars)
	{
		if (m_menuUrsaMajorStars.Find(kvp.Value->PreviousStarName) == nullptr) continue;

		UStarComponent *currentStar = kvp.Value;
		UStarComponent *linkedStar = m_menuUrsaMajorStars[currentStar->PreviousStarName];

		currentStar->initialize(linkedStar, StarLinkGreyClass, StarLinkWhiteClass, m_world, true);
	}

	for (auto kvp : m_menuLeoStars)
	{
		if (m_menuLeoStars.Find(kvp.Value->PreviousStarName) == nullptr) continue;

		UStarComponent *currentStar = kvp.Value;
		UStarComponent *linkedStar = m_menuLeoStars[currentStar->PreviousStarName];

		currentStar->initialize(linkedStar, StarLinkGreyClass, StarLinkWhiteClass, m_world, false);
	}

	/*
	for (auto kvp : m_stars)
	{
		if (m_stars.Find(kvp.Value->PreviousStarName) == nullptr) continue;
		
		UStarComponent *currentStar = kvp.Value;
		UStarComponent *linkedStar = m_stars[currentStar->PreviousStarName];

		currentStar->initialize(linkedStar, StarLinkGreyClass, StarLinkWhiteClass, m_world);
	}*/
}

void AConstellationDome::onStarCollected(OnStarCollectedData *eventData)
{
	// TODO: The level index isn't used for now, but it will be used when we have more than 1 level and more than 1 constellation

	FString starName = "Star_" + FString::FromInt(eventData->getStarIndex() + 1);
	m_stars[starName]->activate(true);
}

void AConstellationDome::onCheckpointReached(OnCheckpointReachedData *eventData)
{
	for (auto kvp : m_stars)
	{
		m_checkpointStates[kvp.Key] = kvp.Value->IsVisible();
	}
}

void AConstellationDome::onWolfRespawned(OnWolfDrownData *eventData)
{
	for (auto kvp : m_checkpointStates)
	{
		m_stars[kvp.Key]->activate(kvp.Value);
	}
}

void AConstellationDome::onLevelLoaded(OnLevelLoadedData *eventData)
{
	if (eventData->getLevelNumber() == 0)
	{
		MenuCassiopeiaConstellation->SetVisibility(true, true);
		MenuUrsaMajorConstellation->SetVisibility(true, true);
		MenuLeoConstellation->SetVisibility(true, true);

		LupusWolfConstellation->SetVisibility(false, true); // Temporary
	}
	else
	{
		MenuCassiopeiaConstellation->SetVisibility(false, true);
		MenuUrsaMajorConstellation->SetVisibility(false, true);
		MenuLeoConstellation->SetVisibility(false, true);

		LupusWolfConstellation->SetVisibility(false, true); // Temporary
	}
}

/*
void AConstellationDome::onMenuLevelSelected(OnMenuLevelSelectedData *eventData)
{

}*/