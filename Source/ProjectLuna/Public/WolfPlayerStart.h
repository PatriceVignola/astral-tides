#pragma once

#include "GameFramework/Actor.h"
#include "OnCheckpointReachedData.h"
#include "ProjectLunaGameMode.h"
#include "WolfPlayerStart.generated.h"

UCLASS()
class PROJECTLUNA_API AWolfPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:	
	AWolfPlayerStart();
	virtual void BeginPlay() override;
};
