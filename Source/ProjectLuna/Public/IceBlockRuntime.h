#pragma once

#include "GameFramework/Actor.h"
#include "LunaLog.h"
#include "BuoyantObjectRuntime.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "IceBlockRuntime.generated.h"

DECLARE_LUNA_LOG(IceBlockRuntime);

UCLASS()
class PROJECTLUNA_API AIceBlockRuntime : public ABuoyantObjectRuntime
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AIceBlockRuntime();

	virtual void BeginPlay() override;
	virtual void Tick(float deltaSeconds) override;

	float getWidth();

	bool isMovable();
	void push(const FVector &velocity);

	void setRaycastReceiver();
};
