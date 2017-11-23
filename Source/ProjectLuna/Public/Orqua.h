// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "LunaLog.h"
#include "OrquaDummy.h"
#include "Wolf.h"
#include "StreamEntity.h"
#include "Timewatch.h"
#include "Orqua.generated.h"

class ALunaLevelScriptActor;
class AProjectLunaGameMode;

DECLARE_LUNA_LOG(Orqua);
DECLARE_LUNA_LOG(Ai1);
DECLARE_LUNA_LOG(Ai2);

UCLASS()
class PROJECTLUNA_API AOrqua : public ACharacter {
	GENERATED_BODY()

private:
	class OrquaAi {
	public:
		OrquaAi(AOrqua* owner);
		virtual ~OrquaAi() {}

		virtual void run(int starsCount) = 0;

	protected:
		AOrqua* m_owner;
	};

	class Level1Ai
		: public OrquaAi {
	public:
		Level1Ai(AOrqua* owner, UWorld* world);

		void run(int starsCount);

	private:
		void runAroundBehavior();
		void followPathBehavior();
		void moveRandomBehavior();

	private:
		float m_timeStamp;
		bool m_isWaiting;
		bool m_wasWaiting;
		float m_nextWaitingTime;
		bool m_isDoneWithPath;
		UWorld* m_world;
		TArray<AActor*> m_pointsOfInterestRandom;
		TArray<AActor*> m_pointsOfInterestRunAround;
		TArray<AActor*> m_pointsOfInterestPath;
		int m_currentPointIndex;
	};

	class Level2Ai
		: public OrquaAi {
	public:
		Level2Ai(AOrqua* owner, UWorld* world);

		void run(int starsCount);

	private:
		AStreamEntity* isWolfNearStream();
		void followFirstPath();
		void reverseStream();
		void leaveStream(AStreamEntity* stream);

	private:
		UWorld* m_world;
		TArray<AStreamEntity*> m_streams;
		TArray<AStreamEntity*> m_reversedStream;
		TArray<AActor*> m_firstPath;
		AWolf* m_wolf;
		Timewatch m_leaveStreamWatch;
		int m_currentPointIndex;
		TArray<StreamSection*> m_streamSections;
		StreamSection* m_currentStreamSection;
		bool m_justSwamAround;
		bool m_isLeavingStream;
		bool m_hasBeenAccelerated;
		Timewatch m_timeoutWatch;
		AStreamEntity* m_currentStream;
		Timewatch m_waitWatch;
		Timewatch m_streamTimeout;
	};

public:
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class AOrquaDummy> OrquaDummyClass;

	UPROPERTY(EditAnywhere)
		float AcceptableRadius;

	UPROPERTY(EditAnywhere)
		float MinimumSpeed;

	// Sets default values for this character's properties
	AOrqua();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	void setLevelAi(unsigned int level);

private:
	void spawnDummy();
	void updateDummyPosition();
	bool hasReachedPointOfInterest();

	void onCheckpointReached();
	void onWolfRespawned();

private:
	bool m_isDummy;
	UWorld* m_world;
	float m_waterPosition;
	AOrquaDummy* m_dummyOrqua;
	FVector m_currentPointOfInterest;
	float m_maxSpeed;
	ALunaLevelScriptActor* m_lunaLevelScript;
	TSharedPtr<OrquaAi> m_ai;
	AProjectLunaGameMode* m_gameMode;
	bool m_isInStream;
	float m_currentStreamDivingOffset;
	float m_maxStreamDivingOffset;
};