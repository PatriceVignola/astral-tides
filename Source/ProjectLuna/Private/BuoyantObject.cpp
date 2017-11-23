/*

The buoyancy calculation technique was inspired from:
http://seawisphunter.com/blog/2015/11/24/simulating-buoyancy-part1/

Thanks to Shane Celis who assisted me generously by e-mail when I had questions

Twitter: @shanecelis

*/

/******************************************************************************************************
MOST OF THE CODE IN THIS FILE IS FOR AN OLD SOLUTION, BUT WE WILL KEEP IT IN CASE THE NEW SOLUTION
 (LOCATED IN BuoyantObjectRuntime.cpp) DOESN'T WORK OUT LIKE WE WANT
*******************************************************************************************************/

#include "ProjectLuna.h"
#include "BuoyantObject.h"
#include "IceBlock.h"
#include "WaterPlane.h"

DEFINE_LUNA_LOG(BuoyantObject);

// Gravity force = Cube volume * density of cube * gravity
// Buoyancy force = Volume submerged * density of water * gravity
// Gravity: 980cm/s^2
// Ice cube density: 0.000917kg/cm^3

/*
WARNING: THIS FILE IS A WORK IN PROGRESS TO TEST THE BASIC PHYSICS. EVENTUALLY (SOON), MOST OF THE BUOYANCY
LOGIC WILL BE TRANSFERRED TO A GENERIC "FLOATING OBJECT" SUPER CLASS.
*/

ABuoyantObject::ABuoyantObject()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
}

void ABuoyantObject::BeginPlay()
{
	Super::BeginPlay();

	// Caching
	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode *)m_world->GetAuthGameMode();

	FVector origin, boxExtent;
	GetActorBounds(false, origin, boxExtent);

	instantiateBoundingBoxCollider(boxExtent, origin);

	//Destroy();
}

void ABuoyantObject::instantiateBoundingBoxCollider(const FVector &boxExtent, const FVector &origin)
{
	FVector pos = GetActorLocation();
	FRotator rot = GetActorRotation();

	// We instantiate the bounding box

	if (!m_boundingBox)
	{
		m_boundingBox = NewObject<UBoxComponent>(this);

		m_boundingBox->SetBoxExtent(boxExtent);

		m_boundingBox->RegisterComponent();

		RootComponent = m_boundingBox;

		// We relocate the components to keep their relative positions and rotations
		FVector originOffset = origin - pos;
		SetActorLocation(pos + originOffset);
		SetActorRotation(rot);
		Mesh->SetRelativeLocationAndRotation(-originOffset, FRotator::ZeroRotator);

		Mesh->AttachTo(RootComponent);
	}

	
	if (IsA(AIceBlock::StaticClass()) && IceBlockRuntimeClass)
	{
		AIceBlockRuntime *iceBlockRuntime = m_world->SpawnActor<AIceBlockRuntime>(IceBlockRuntimeClass, GetActorLocation(), GetActorRotation());
		iceBlockRuntime->initSpawnValues(Mesh->GetRelativeTransform().GetTranslation(), boxExtent, Mesh->StaticMesh);
		iceBlockRuntime->initStreamValues(Acceleration, MaxSpeed, Radius, NormalMod);
	}
	else if (BuoyantObjectRuntimeClass)
	{
		ABuoyantObjectRuntime *buoyantObjectRuntime = m_world->SpawnActor<ABuoyantObjectRuntime>(BuoyantObjectRuntimeClass, GetActorLocation(), GetActorRotation());
		buoyantObjectRuntime->initSpawnValues(Mesh->GetRelativeTransform().GetTranslation(), boxExtent, Mesh->StaticMesh);
		buoyantObjectRuntime->initStreamValues(Acceleration, MaxSpeed, Radius, NormalMod);
	}
}