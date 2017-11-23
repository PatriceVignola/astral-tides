/*

The buoyancy calculation technique was inspired from:
http://seawisphunter.com/blog/2015/11/24/simulating-buoyancy-part1/

Thanks to Shane Celis who assisted me generously by e-mail when I had questions

Twitter: @shanecelis

*/

#include "ProjectLuna.h"
#include "IceBlockRuntime.h"

DEFINE_LUNA_LOG(IceBlockRuntime);

AIceBlockRuntime::AIceBlockRuntime()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AIceBlockRuntime::BeginPlay()
{
	Super::BeginPlay();

	setRaycastReceiver();
}

void AIceBlockRuntime::setRaycastReceiver()
{
	/*
	FVector pos = GetActorLocation();
	FRotator rot = GetActorRotation();*/


	FVector origin, boxExtent;
	float radius;

	UKismetSystemLibrary::GetComponentBounds(Mesh, origin, boxExtent, radius);





	// We instantiate the bounding box
	UBoxComponent *boundingBox = NewObject<UBoxComponent>(this);

	boundingBox->SetBoxExtent(boxExtent);

	boundingBox->RegisterComponent();

	boundingBox->AttachTo(Mesh);
	boundingBox->SetRelativeLocation(origin - Mesh->GetComponentLocation());

	// We set the collider size and enable the physics for the collider

	
	boundingBox->SetCollisionProfileName(TEXT("IceBlockDetection"));
	boundingBox->SetSimulatePhysics(false);
	boundingBox->SetEnableGravity(false);


	/*
	// We relocate the components to keep their relative positions and rotations
	FVector originOffset = origin - pos;
	SetActorLocation(pos + originOffset);
	SetActorRotation(rot);
	Mesh->SetRelativeLocationAndRotation(-originOffset, FRotator::ZeroRotator);*/
}

void AIceBlockRuntime::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);
}

bool AIceBlockRuntime::isMovable()
{
	// If we meet collision problems in the future, we need to be more restrictive here
	return m_currentState == BuoyantObjectRuntimeState::Grounded || checkGround();
	//return m_boundingBox->GetPhysicsLinearVelocity() == FVector::ZeroVector && m_boundingBox->GetPhysicsAngularVelocity() == FVector::ZeroVector;
}

float AIceBlockRuntime::getWidth()
{
	return m_blockSize.X;
}

void AIceBlockRuntime::push(const FVector &velocity)
{
	BoxCollider->SetPhysicsLinearVelocity(velocity + FVector(0, 0, BoxCollider->GetPhysicsLinearVelocity().Z));
}