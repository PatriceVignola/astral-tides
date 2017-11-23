#pragma once

#include "GameFramework/Actor.h"
#include "LunaLog.h"
#include "ProjectLunaGameMode.h"
#include "OnWaterLevelChangedData.h"
#include "EventManager.h"
#include "OnLevelLoadedData.h"
#include "Moon.generated.h"

class ALunaLevelScriptActor;

DECLARE_LUNA_LOG(Moon);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMoonWaterChangedDelegate, float, fullMoonRatio);

UCLASS()
class PROJECTLUNA_API AMoon : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent *Mesh;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent *Root;

	// This event has to be listened in the blueprint
	UPROPERTY(BlueprintAssignable)
	FMoonWaterChangedDelegate OnWaterChanged;

	AMoon();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UWorld *m_world;
	ALunaLevelScriptActor *m_levelScript;
	AProjectLunaGameMode *m_gameMode;

	void onWaterLevelChanged(OnWaterLevelChangedData *eventData);
	void updateFullMoonRatio(float newWaterDelta);
	void onLevelLoaded(OnLevelLoadedData *eventData);
};
