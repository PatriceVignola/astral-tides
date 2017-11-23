// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "Orqua.h"
#include "Wolf.h"
#include "WaterPlane.h"
#include "PointOfInterest.h"
#include "LunaLevelScriptActor.h"
#include "ProjectLunaGameMode.h"

DEFINE_LUNA_LOG(Orqua)
DEFINE_LUNA_LOG(Ai1);
DEFINE_LUNA_LOG(Ai2);

AOrqua::OrquaAi::OrquaAi(AOrqua* owner)
	: m_owner(owner) {}

AOrqua::Level1Ai::Level1Ai(AOrqua* owner, UWorld* world)
	: OrquaAi(owner)
	, m_timeStamp(0.f)
	, m_isWaiting(false)
	, m_wasWaiting(false)
	, m_isDoneWithPath(false)
	, m_nextWaitingTime(0.f)
	, m_world(world)
	, m_currentPointIndex(0) {

	for(TActorIterator<AActor> ActorItr(m_world); ActorItr; ++ActorItr) {
		AActor *actor = *ActorItr;
		if(actor->IsA(APointOfInterest::StaticClass()) && actor->GetName().Contains("RandomL1Pt")) {
			m_pointsOfInterestRandom.Add(actor);
		} else if(actor->IsA(APointOfInterest::StaticClass()) && actor->GetName().Contains("RunAroundL1Pt")) {
			m_pointsOfInterestRunAround.Add(actor);
		} else if(actor->IsA(APointOfInterest::StaticClass()) && actor->GetName().Contains("PathL1Pt")) {
			m_pointsOfInterestPath.Add(actor);
		}
	}

	m_pointsOfInterestRunAround.Sort([](const AActor& left, const AActor& right) -> bool {
		int l = FCString::Atoi(*(left.GetName().RightChop(13)));
		int r = FCString::Atoi(*(right.GetName().RightChop(13)));

		return l < r;
	});
	m_pointsOfInterestPath.Sort([](const AActor& left, const AActor& right) -> bool {
		int l = FCString::Atoi(*(left.GetName().RightChop(8)));
		int r = FCString::Atoi(*(right.GetName().RightChop(8)));

		return l < r;
	});

	if(m_pointsOfInterestRunAround.Num() > 0) {
		m_owner->m_currentPointOfInterest = m_pointsOfInterestRunAround[m_currentPointIndex]->GetActorLocation();
	}
}

void AOrqua::Level1Ai::run(int starsCount) {
	if(starsCount <= 1) {
		runAroundBehavior();
	} else if(!m_isDoneWithPath) {
		followPathBehavior();
	} else {
		moveRandomBehavior();
	}
}

void AOrqua::Level1Ai::runAroundBehavior() {
	m_owner->m_dummyOrqua->setRaycastDistance(250.f);
	if(m_pointsOfInterestRunAround.Num() == 0) {
		return;
	}

	const UNavigationSystem* navSys = m_world->GetNavigationSystem();

	if(m_owner->hasReachedPointOfInterest()) {
		m_currentPointIndex++;
		if(m_currentPointIndex >= m_pointsOfInterestRunAround.Num()) {
			m_currentPointIndex = 0;
		}
		m_owner->m_currentPointOfInterest = m_pointsOfInterestRunAround[m_currentPointIndex]->GetActorLocation();
		auto currentLocation = m_owner->GetActorLocation();
		m_owner->m_dummyOrqua->findRotation(m_owner->m_currentPointOfInterest.X, m_owner->m_currentPointOfInterest.Y, m_owner->m_waterPosition - m_owner->m_currentStreamDivingOffset);
	}

	navSys->SimpleMoveToLocation(m_owner->GetController(), m_owner->m_currentPointOfInterest);
}

void AOrqua::Level1Ai::followPathBehavior() {
	m_owner->m_dummyOrqua->setRaycastDistance(1000.f);
	if(m_pointsOfInterestPath.Num() == 0) {
		LUNA_LOG(Ai1, "Done With the Path");
		m_isDoneWithPath = true;
		return;
	}

	const UNavigationSystem* navSys = m_world->GetNavigationSystem();

	if(m_owner->hasReachedPointOfInterest()) {
		m_currentPointIndex = 0;
		m_owner->m_currentPointOfInterest = m_pointsOfInterestPath[m_currentPointIndex]->GetActorLocation();
		m_pointsOfInterestPath.RemoveSingle(m_pointsOfInterestPath[m_currentPointIndex]);
		auto currentLocation = m_owner->GetActorLocation();
		m_owner->m_dummyOrqua->findRotation(m_owner->m_currentPointOfInterest.X, m_owner->m_currentPointOfInterest.Y, m_owner->m_waterPosition - m_owner->m_currentStreamDivingOffset);
	}

	navSys->SimpleMoveToLocation(m_owner->GetController(), m_owner->m_currentPointOfInterest);
}

void AOrqua::Level1Ai::moveRandomBehavior() {
	m_owner->m_dummyOrqua->setRaycastDistance(1500.f);
	if(m_currentPointIndex >= m_pointsOfInterestRandom.Num()) {
		m_currentPointIndex = 0;
	}

	const UNavigationSystem* navSys = m_world->GetNavigationSystem();

	navSys->SimpleMoveToLocation(m_owner->GetController(), m_owner->m_currentPointOfInterest);

	if(!m_isWaiting && m_owner->hasReachedPointOfInterest()) {
		m_isWaiting = false;
		//Should it wait?
		if(FMath::RandRange(0, 10) > 7) {
			m_currentPointIndex = FMath::RandRange(0, m_pointsOfInterestRandom.Num() - 1);
			m_owner->m_currentPointOfInterest = m_pointsOfInterestRandom[m_currentPointIndex]->GetActorLocation();
			auto currentLocation = m_owner->GetActorLocation();
			m_owner->m_dummyOrqua->findRotation(m_owner->m_currentPointOfInterest.X, m_owner->m_currentPointOfInterest.Y, m_owner->m_waterPosition - m_owner->m_currentStreamDivingOffset);

			//Changing Speed
			if(FMath::RandRange(0, 10) > 7) {
				m_owner->GetCharacterMovement()->MaxWalkSpeed = FMath::FRandRange(m_owner->MinimumSpeed, m_owner->m_maxSpeed);
			}
		} else {
			m_timeStamp = FPlatformTime::Seconds();
			m_nextWaitingTime = FMath::FRandRange(0.5f, 1.75f);
			m_isWaiting = true;
		}
	}

	if(FPlatformTime::Seconds() - m_timeStamp > m_nextWaitingTime) {
		m_isWaiting = false;
	}
}

AOrqua::Level2Ai::Level2Ai(AOrqua* owner, UWorld* world)
	: OrquaAi(owner)
	, m_world(world)
	, m_wolf(nullptr)
	, m_leaveStreamWatch("Leave Stream Watch")
	, m_currentPointIndex(0)
	, m_currentStreamSection(nullptr)
	, m_justSwamAround(false)
	, m_isLeavingStream(false)
	, m_hasBeenAccelerated(false)
	, m_timeoutWatch("Timeout Watch")
	, m_currentStream(nullptr)
	, m_waitWatch("Wait Watch")
	, m_streamTimeout("Stream Timeout") {

	for(TActorIterator<AActor> ActorItr(m_world); ActorItr; ++ActorItr) {
		AActor *actor = *ActorItr;
		if(actor->IsA(AWolf::StaticClass()) && !m_wolf) {
			m_wolf = Cast<AWolf>(actor);
		} else if(actor->IsA(AStreamEntity::StaticClass())) {
			/*if(actor->GetName().Contains("courants4")
			|| actor->GetName().Contains("courants6")
			|| actor->GetName().Contains("courants5")) {
			LUNA_LOG(OrquaDummy, "Not adding inner stream");
			} else {*/
			m_streams.Add(Cast<AStreamEntity>(actor));
			//}
		} else if(actor->IsA(APointOfInterest::StaticClass()) && actor->GetName().Contains("PathL2Pt")) {
			m_firstPath.Add(actor);
		}
	}

	m_firstPath.Sort([](const AActor& left, const AActor& right) -> bool {
		int l = FCString::Atoi(*(left.GetName().RightChop(8)));
		int r = FCString::Atoi(*(right.GetName().RightChop(8)));

		return l < r;
	});

	if(m_firstPath.Num() > 0) {
		m_owner->m_currentPointOfInterest = m_firstPath[m_currentPointIndex]->GetActorLocation();
		LUNA_LOG(Ai2, "First Path Found");
	}
	m_owner->m_dummyOrqua->setRaycastDistance(1500.f);
}

void AOrqua::Level2Ai::run(int starsCount) {

	if(starsCount == 0) {
		followFirstPath();
	} else if(starsCount >= 1) {
		reverseStream();
	}
}

void AOrqua::Level2Ai::followFirstPath() {
	if(m_firstPath.Num() == 0) {
		return;
	}

	const UNavigationSystem* navSys = m_world->GetNavigationSystem();

	if(m_owner->hasReachedPointOfInterest()) {
		m_currentPointIndex++;
		if(m_currentPointIndex >= m_firstPath.Num()) {
			m_currentPointIndex = 0;
		}
		m_owner->m_currentPointOfInterest = m_firstPath[m_currentPointIndex]->GetActorLocation();
		auto currentLocation = m_owner->GetActorLocation();
		m_owner->m_dummyOrqua->findRotation(m_owner->m_currentPointOfInterest.X, m_owner->m_currentPointOfInterest.Y, m_owner->m_waterPosition - m_owner->m_currentStreamDivingOffset);
	}

	navSys->SimpleMoveToLocation(m_owner->GetController(), m_owner->m_currentPointOfInterest);
}

void AOrqua::Level2Ai::reverseStream() {
	auto stream = isWolfNearStream();
	if(stream) {
		if(m_streamSections.Num() <= 1 || (m_leaveStreamWatch.isRunning() && m_leaveStreamWatch.hasExpired())) {
			if(m_justSwamAround) {
				m_streams.RemoveSingle(stream);
				m_currentStream = nullptr;
				m_reversedStream.Add(stream);
				leaveStream(stream);
			} else {
				const UNavigationSystem* navSys = m_world->GetNavigationSystem();

				m_owner->m_currentPointOfInterest = stream->GetActorLocation();
				m_owner->m_currentPointOfInterest.Z = m_owner->GetActorLocation().Z;
				auto currentLocation = m_owner->GetActorLocation();
				m_owner->m_dummyOrqua->findRotation(m_owner->m_currentPointOfInterest.X, m_owner->m_currentPointOfInterest.Y, m_owner->m_waterPosition - m_owner->m_currentStreamDivingOffset);
				navSys->SimpleMoveToLocation(m_owner->GetController(), m_owner->m_currentPointOfInterest);
				m_owner->m_isInStream = true;

				if(!m_hasBeenAccelerated) {
					m_owner->GetCharacterMovement()->MaxWalkSpeed *= 2;
					m_hasBeenAccelerated = true;
				}

				FVector orquaLoc = m_owner->m_dummyOrqua->GetActorLocation();
				orquaLoc.Z = m_owner->m_waterPosition;
				if(stream->IsInsideStream(orquaLoc)) {
					LUNA_LOG(Ai2, "Reversing");
					stream->Reverse();
					m_streamSections = stream->getSections();
					LUNA_LOG(Ai2, "Amount of Sections: %d", m_streamSections.Num());
					m_leaveStreamWatch.start(7.0);
					m_justSwamAround = true;
					m_owner->GetCharacterMovement()->MaxWalkSpeed /= 2;
					m_hasBeenAccelerated = false;
				}
			}
		} else {
			//Going around in the stream
			FVector orquaLoc = m_owner->m_dummyOrqua->GetActorLocation();
			orquaLoc.Z = m_owner->m_waterPosition;
			auto streamSection = stream->IsInsideStream(orquaLoc);

			if(m_currentStreamSection != streamSection) {
				if(m_streamSections.Find(m_currentStreamSection) != INDEX_NONE) {
					m_streamSections.RemoveSingle(m_currentStreamSection);
				}
				m_currentStreamSection = streamSection;
				int index = m_streamSections.Find(m_currentStreamSection);
				if(!stream->IsReversed()) {
					index += 2;

					if(index >= m_streamSections.Num()) {
						index = 0;
					}
				} else {
					index -= 2;
					if(index < 0) {
						index = m_streamSections.Num() - 1;
					}
				}

				m_owner->m_currentPointOfInterest = m_streamSections[index]->GetCentre();
				m_owner->m_currentPointOfInterest.Z = -2000;
				auto currentLocation = m_owner->GetActorLocation();
				m_owner->m_dummyOrqua->findRotation(m_owner->m_currentPointOfInterest.X, m_owner->m_currentPointOfInterest.Y, m_owner->m_waterPosition - m_owner->m_currentStreamDivingOffset);
			}

			const UNavigationSystem* navSys = m_world->GetNavigationSystem();
			navSys->SimpleMoveToLocation(m_owner->GetController(), m_owner->m_currentPointOfInterest);
			//m_justSwamAround = true;
		}
	} else {
		const UNavigationSystem* navSys = m_world->GetNavigationSystem();
		bool hasReached = m_owner->hasReachedPointOfInterest();
		if(hasReached) {
			m_waitWatch.start(3.f);
		}
		if((hasReached && m_waitWatch.hasExpired()) || (m_timeoutWatch.hasExpired())) {
			FNavLocation location;
			FVector actorLoc = m_wolf->GetActorLocation();
			actorLoc.Z = m_owner->GetActorLocation().Z;
			FVector v(0, 0, 0);
			int offset = 0;
			while(v.X < 1750 && v.Y < 1750) {
				navSys->GetRandomReachablePointInRadius(actorLoc, 3750 + offset, location);
				v.X = FMath::Abs(location.Location.X - actorLoc.X);
				v.Y = FMath::Abs(location.Location.Y - actorLoc.Y);
				offset += 500;
			}

			m_owner->m_currentPointOfInterest = location.Location;
			m_owner->m_currentPointOfInterest.Z = m_owner->GetActorLocation().Z;
			auto currentLocation = m_owner->GetActorLocation();
			m_owner->m_dummyOrqua->findRotation(m_owner->m_currentPointOfInterest.X, m_owner->m_currentPointOfInterest.Y, m_owner->m_waterPosition - m_owner->m_currentStreamDivingOffset);
			LUNA_LOG(Ai2, "New Location to go: %f %f %f", m_owner->m_currentPointOfInterest.X, m_owner->m_currentPointOfInterest.Y, m_owner->m_currentPointOfInterest.Z);
			m_timeoutWatch.start(7.f);
			m_waitWatch.stop();
		}

		/*FVector orquaLoc = m_owner->m_dummyOrqua->GetActorLocation();
		orquaLoc.Z = m_owner->m_waterPosition;*/
		navSys->SimpleMoveToLocation(m_owner->GetController(), m_owner->m_currentPointOfInterest);
	}
}

void AOrqua::Level2Ai::leaveStream(AStreamEntity* stream) {
	const UNavigationSystem* navSys = m_world->GetNavigationSystem();
	if(!m_isLeavingStream) {
		FNavLocation location;
		FVector actorLoc = stream->GetActorLocation();
		FVector v(0, 0, 0);
		while(v.X < 1750 && v.Y < 1750) {
			//navSys->GetRandomPointInNavigableRadius(m_owner->GetActorLocation(), 3750, location);
			navSys->GetRandomReachablePointInRadius(m_owner->GetActorLocation(), 3750, location);
			v.X = FMath::Abs(location.Location.X - actorLoc.X);
			v.Y = FMath::Abs(location.Location.Y - actorLoc.Y);
		}

		m_owner->m_currentPointOfInterest = location.Location;
		m_owner->m_currentPointOfInterest.Z = -2000;
		auto currentLocation = m_owner->GetActorLocation();
		m_owner->m_dummyOrqua->findRotation(m_owner->m_currentPointOfInterest.X, m_owner->m_currentPointOfInterest.Y, m_owner->m_waterPosition - m_owner->m_currentStreamDivingOffset);
		LUNA_LOG(Ai2, "Done Swimming Around, moving out of stream");
		LUNA_LOG(Ai2, "%f %f %f", m_owner->m_currentPointOfInterest.X, m_owner->m_currentPointOfInterest.Y, m_owner->m_currentPointOfInterest.Z);
		m_isLeavingStream = true;
		m_streamSections.Empty();
		m_owner->m_isInStream = false;
	}

	FVector orquaLoc = m_owner->m_dummyOrqua->GetActorLocation();
	orquaLoc.Z = m_owner->m_waterPosition;
	if(stream->IsInsideStream(orquaLoc)) {
		LUNA_LOG(Ai2, "Still inside stream while leaving");
		navSys->SimpleMoveToLocation(m_owner->GetController(), m_owner->m_currentPointOfInterest);
	} else {
		m_isLeavingStream = false;
		m_justSwamAround = false;
		m_leaveStreamWatch.stop();
		m_currentStreamSection = nullptr;
	}
}

AStreamEntity* AOrqua::Level2Ai::isWolfNearStream() {
	if(!m_wolf) {
		LUNA_ERROR(Ai2, "Wolf is not found and known");
		return nullptr;
	}

	if(m_currentStream && !(m_streamTimeout.isRunning() && m_streamTimeout.hasExpired())) {
		return m_currentStream;
	}
	m_streamTimeout.stop();
	/*if(m_streamTimeout.isRunning() && m_streamTimeout.hasExpired()) {
	m_currentStream = nullptr;
	m_streamTimeout.stop();
	}

	if(m_currentStream) {
	return m_currentStream;
	}*/

	TMap<FVector, AStreamEntity*> distances;
	for(int i = 0; i < m_streams.Num(); ++i) {
		FVector distance;
		distance.X = FMath::Abs(FMath::Abs(m_wolf->GetActorLocation().X) - FMath::Abs(m_streams[i]->GetActorLocation().X));
		distance.Y = FMath::Abs(FMath::Abs(m_wolf->GetActorLocation().Y) - FMath::Abs(m_streams[i]->GetActorLocation().Y));
		distance.Z = FMath::Abs(FMath::Abs(m_wolf->GetActorLocation().Z) - FMath::Abs(m_streams[i]->GetActorLocation().Z));
		distances.Add(distance, m_streams[i]);
	}

	TArray<FVector> distanceArray;
	distances.GenerateKeyArray(distanceArray);

	distanceArray.Sort([](const FVector& left, const FVector& right) -> bool {
		return (FMath::Abs(left.X) + FMath::Abs(left.Y) + FMath::Abs(left.Z)) < (FMath::Abs(right.X) + FMath::Abs(right.Y) + FMath::Abs(right.Z));
	});

	/*FVector& first = distanceArray[0];

	if(first.X <= 1200 && first.Y <= 1200 && first.Z <= 200) {
	return distances[first];
	}*/

	for(int i = 0; i < distances.Num(); ++i) {
		FVector& distance = distanceArray[i];

		if(distance.X <= 1200 && distance.Y <= 1200 && distance.Z <= 300) {
			m_currentStream = distances[distance];
			m_streamTimeout.start(3.f);
			LUNA_LOG(Ai2, "Found Viable Stream %s", *(m_currentStream->GetName()));
			return m_currentStream;
		}
	}

	return nullptr;
}

// Sets default values
AOrqua::AOrqua()
	: m_isDummy(false)
	, m_dummyOrqua(nullptr)
	, m_lunaLevelScript(nullptr)
	, m_ai(nullptr)
	, m_gameMode(nullptr)
	, m_isInStream(false)
	, m_maxStreamDivingOffset(250)
	, m_currentStreamDivingOffset(0) {
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_maxSpeed = GetCharacterMovement()->MaxWalkSpeed;
}

// Called when the game starts or when spawned
void AOrqua::BeginPlay() {
	Super::BeginPlay();

	m_world = GetWorld();

	for(TActorIterator<AActor> ActorItr(m_world); ActorItr; ++ActorItr) {
		AActor *actor = *ActorItr;

		if(actor->IsA(AWaterPlane::StaticClass())) {
			m_waterPosition = actor->GetActorLocation().Z;
			break;
		}
	}

	m_lunaLevelScript = Cast<ALunaLevelScriptActor>(m_world->GetLevelScriptActor());
	/*m_gameMode = (AProjectLunaGameMode *)m_world->GetAuthGameMode();
	m_gameMode->getEventManager().subscribe("OnCheckpointReached", this, &AOrqua::onCheckpointReached);
	m_gameMode->getEventManager().subscribe("OnWolfRespawned", this, &AOrqua::onWolfRespawned);*/

	spawnDummy();
}

void AOrqua::BeginDestroy() {
	Super::BeginDestroy();

	/*if(m_gameMode) {
	m_gameMode->getEventManager().unsubscribe("OnCheckpointReached", this, &AOrqua::onCheckpointReached);
	m_gameMode->getEventManager().unsubscribe("OnWolfRespawned", this, &AOrqua::onWolfRespawned);
	}*/
}

void AOrqua::EndPlay(const EEndPlayReason::Type reason) {
	Super::EndPlay(reason);
	if(m_dummyOrqua->Destroy()) {
		LUNA_LOG(Orqua, "Orqua dead");
	} else {
		LUNA_LOG(Orqua, "Orqua not dead");
	}
	m_ai.Reset();
}

// Called every frame
void AOrqua::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if(m_ai.IsValid()) {
		m_ai->run(m_lunaLevelScript->getCountStarsCollected());
	}

	if(IsValid(m_dummyOrqua)) {
		updateDummyPosition();
	}

}

void AOrqua::spawnDummy() {
	const auto realLocation = GetActorLocation();

	m_dummyOrqua = m_world->SpawnActor<AOrquaDummy>(OrquaDummyClass, FVector(realLocation.X, realLocation.Y, m_waterPosition), FRotator::ZeroRotator);
	m_dummyOrqua->setWaterLevel(m_waterPosition);
	m_dummyOrqua->setOwner(this);
}

void AOrqua::updateDummyPosition() {
	if(!m_dummyOrqua) {
		return;
	}

	auto currentLocation = GetActorLocation();

	for(TActorIterator<AActor> ActorItr(m_world); ActorItr; ++ActorItr) {
		AActor *actor = *ActorItr;

		if(actor->IsA(AWaterPlane::StaticClass())) {
			m_waterPosition = actor->GetActorLocation().Z;
			break;
		}
	}

	if(m_isInStream) {
		m_currentStreamDivingOffset += 1;
		if(m_currentStreamDivingOffset >= m_maxStreamDivingOffset) {
			m_currentStreamDivingOffset = m_maxStreamDivingOffset;
		}
	} else {
		m_currentStreamDivingOffset -= 1;
		if(m_currentStreamDivingOffset < 0) {
			m_currentStreamDivingOffset = 0;
		}
	}
	m_dummyOrqua->move(currentLocation.X, currentLocation.Y, m_waterPosition - m_currentStreamDivingOffset);
	//m_dummyOrqua->SetActorRotation(GetActorRotation());
}

bool AOrqua::hasReachedPointOfInterest() {
	const auto currentLocation = GetActorLocation();

	bool isAtDestination = (currentLocation.X >= m_currentPointOfInterest.X - AcceptableRadius
							&& currentLocation.Y >= m_currentPointOfInterest.Y - AcceptableRadius)
		&&
		(currentLocation.X <= m_currentPointOfInterest.X + AcceptableRadius
		 && currentLocation.Y >= m_currentPointOfInterest.Y - AcceptableRadius)
		&&
		(currentLocation.X >= m_currentPointOfInterest.X - AcceptableRadius
		 && currentLocation.Y <= m_currentPointOfInterest.Y + AcceptableRadius)
		&&
		(currentLocation.X <= m_currentPointOfInterest.X + AcceptableRadius
		 && currentLocation.Y <= m_currentPointOfInterest.Y + AcceptableRadius);

	return isAtDestination;
}

void AOrqua::setLevelAi(unsigned int level) {
	m_ai.Reset();
	if(level == 1) {
		m_ai = TSharedPtr<OrquaAi>(new Level1Ai(this, m_world));
	} else if(level == 2) {
		m_ai = TSharedPtr<OrquaAi>(new Level2Ai(this, m_world));
	} else {
		m_ai = nullptr;
	}
}

void AOrqua::onCheckpointReached() {
	//If we have time
}

void AOrqua::onWolfRespawned() {
	//If we have time
}