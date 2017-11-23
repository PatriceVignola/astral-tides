#include "ProjectLuna.h"
#include "StarComponent.h"


UStarComponent::UStarComponent()
{
	bWantsBeginPlay = false;
	PrimaryComponentTick.bCanEverTick = false;
}

void UStarComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

void UStarComponent::initialize(UStarComponent *prevStar, TSubclassOf<class AStarLink> greyLinkClass, TSubclassOf<class AStarLink> whiteLinkClass, UWorld *world, bool defaultVisible)
{
	if (!prevStar) return;

	FVector location = GetComponentLocation();
	FVector prevLocation = prevStar->GetComponentLocation();

	if (!defaultVisible)
	{
		m_greyLink = world->SpawnActor<AStarLink>(greyLinkClass, FVector::ZeroVector, FRotator::ZeroRotator);
		m_greyLink->setStartEndPositions(location, prevLocation);
		m_greyLink->GetRootComponent()->AttachTo(this);
	}
	else
	{
		m_whiteLink = world->SpawnActor<AStarLink>(whiteLinkClass, FVector::ZeroVector, FRotator::ZeroRotator);
		m_whiteLink->setStartEndPositions(location, prevLocation);
		m_whiteLink->GetRootComponent()->AttachTo(this);
	}
}

void UStarComponent::activate(bool state)
{
	if (m_greyLink)
	{
		m_greyLink->GetRootComponent()->SetVisibility(!state, true);
	}

	if (m_whiteLink)
	{
		m_whiteLink->GetRootComponent()->SetVisibility(state, true);
	}

	SetVisibility(state);
}