#include "ProjectLuna.h"
#include "StarPiece.h"
#include "LunaLevelScriptActor.h"

DEFINE_LUNA_LOG(StarPiece);

// Sets default values
AStarPiece::AStarPiece()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));
	Collider->AttachTo(RootComponent);
}

void AStarPiece::BeginPlay()
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
		LUNA_ERROR(TideManager, "Error: The level blueprint needs to inherit from ALunaLevelScriptActor");
	}

	Collider->OnComponentBeginOverlap.AddDynamic(this, &AStarPiece::OnBeginOverlap);

	m_gameMode->getEventManager().raiseEvent("OnStarSpawned", new GenericEventData());

	RootComponent->SetVisibility(true, true);
	Collider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AStarPiece::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (Collider->OnComponentBeginOverlap.IsBound())
	{
		Collider->OnComponentBeginOverlap.RemoveDynamic(this, &AStarPiece::OnBeginOverlap);
	}

	m_world = nullptr;
	m_gameMode = nullptr;
	m_levelScript = nullptr;

}

void AStarPiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AStarPiece::OnBeginOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor->IsA(AWolf::StaticClass()))
	{
		LUNA_WARN(StarPiece, "Debug: Star Piece Collected");

		m_gameMode->getEventManager().raiseEvent("OnCheckpointReached", new OnCheckpointReachedData(OtherActor->GetActorLocation(), OtherActor->GetActorRotation()));

		m_levelScript->incrementStarsCount();

		UGameplayStatics::SpawnSoundAtLocation(this, CollectSound, GetActorLocation());

		Collider->OnComponentBeginOverlap.RemoveDynamic(this, &AStarPiece::OnBeginOverlap);

		RootComponent->SetVisibility(false, true);
		Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}