// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "StarLink.h"
#include "StarComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTLUNA_API UStarComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditAnywhere)
	FString PreviousStarName;

	UStarComponent();
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void initialize(UStarComponent *prevStar, TSubclassOf<class AStarLink> greyLinkClass, TSubclassOf<class AStarLink> whiteLinkClass, UWorld *world, bool defaultVisible);
	void activate(bool state);

private:
	AStarLink *m_greyLink;
	AStarLink *m_whiteLink;
};
