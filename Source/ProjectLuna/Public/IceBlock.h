#pragma once

#include "GameFramework/Actor.h"
#include "LunaLog.h"
#include "BuoyantObject.h"
#include "IceBlock.generated.h"

DECLARE_LUNA_LOG(IceBlock);

UCLASS()
class PROJECTLUNA_API AIceBlock : public ABuoyantObject
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AIceBlock();

	float getWidth();
};
