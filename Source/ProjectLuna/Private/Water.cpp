// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "Water.h"

DEFINE_LUNA_LOG(Water);

// Sets default values
AWater::AWater()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WaterSpline = CreateDefaultSubobject<USplineComponent>(TEXT("WaterSpline"));
	RootComponent = WaterSpline;
}

// Construction function called when the actor is being edited in the editor
void AWater::OnConstruction(const FTransform& Transform)
{
	// Commented out until we decide to use it

	/*
	if (!WaterMesh) return;

	int32 pointsCount = WaterSpline->GetNumberOfSplinePoints();

	for (int32 i = 0; i < pointsCount - 1; i++)
	{
		USplineMeshComponent* splineMesh = NewObject<USplineMeshComponent>(USplineMeshComponent::StaticClass(), this);

		// Makes sure that the meshes are deleted every time OnConstruction is called
		splineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		splineMesh->SetMobility(EComponentMobility::Movable);
		splineMesh->AttachParent = WaterSpline;

		

		splineMesh->bCastDynamicShadow = false;
		splineMesh->SetStaticMesh(WaterMesh);

		FVector startLoc, startTan, endLoc, endTan;
		WaterSpline->GetLocalLocationAndTangentAtSplinePoint(i, startLoc, startTan);
		WaterSpline->GetLocalLocationAndTangentAtSplinePoint(i + 1, endLoc, endTan);
		FVector startScale = WaterSpline->GetScaleAtSplinePoint(i);
		FVector endScale = WaterSpline->GetScaleAtSplinePoint(i + 1);

		splineMesh->SetStartScale(FVector2D(startScale.X, startScale.Z));
		splineMesh->SetEndScale(FVector2D(endScale.Y, endScale.Z));

		splineMesh->SetStartAndEnd(startLoc, startTan, endLoc, endTan);

		splineMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		splineMesh->OnComponentBeginOverlap.AddDynamic(this, &AWater::OnBeginOverlap);

		splineMesh->ForwardAxis = ESplineMeshAxis::Y;
	}

	RegisterAllComponents();*/
}

void AWater::BeginPlay()
{
	Super::BeginPlay();

	LUNA_WARN(Water, "Debug: Subscribe to water event");
}

void AWater::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO: Refactor and do it better after prototype, or change it completely

	if (m_isRising)
	{
		float t = m_elapsedTime / 2.f;
		float newHeight = FMath::Lerp(m_currentTarget - m_deltaHeight, m_currentTarget, t);

		FVector loc = this->GetActorLocation();
		loc.Z = newHeight;

		this->SetActorLocation(loc);

		m_elapsedTime += DeltaTime;

		if (t >= 1)
		{
			m_isRising = false;
		}
	}

}

void AWater::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("ABC"));

	if (OtherActor->IsA(AWolf::StaticClass()))
	{
		LUNA_WARN(Water, "Debug: Wolf entered water");
		AWolfController *wolfController = Cast<AWolfController>((Cast<AWolf>(OtherActor))->GetController());
		//wolfController->reset();
	}
}

// TODO: Temporary, remove after prototype phase
void AWater::raiseWaterLevel()
{
	if (m_isRising) return;

	LUNA_WARN(Water, "Debug: Raise water level correctly executed");

	m_isRising = true;
	m_currentTarget = this->GetActorLocation().Z + m_deltaHeight;
	m_elapsedTime = 0.f;
}
