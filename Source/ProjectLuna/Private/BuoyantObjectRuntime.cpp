/*

The buoyancy calculation technique was inspired from:
http://seawisphunter.com/blog/2015/11/24/simulating-buoyancy-part1/

Thanks to Shane Celis who assisted me generously by e-mail when I had questions

Twitter: @shanecelis

*/

#include "ProjectLuna.h"
#include "BuoyantObjectRuntime.h"
#include "GenericEventData.h"
#include "WaterPlane.h"
#include "Wolf.h"

DEFINE_LUNA_LOG(BuoyantObjectRuntime);


// Gravity force = Cube volume * density of cube * gravity
// Buoyancy force = Volume submerged * density of water * gravity
// Gravity: 980cm/s^2
// Ice cube density: 0.000917kg/cm^3

ABuoyantObjectRuntime::ABuoyantObjectRuntime()
{
	PrimaryActorTick.bCanEverTick = true;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	RootComponent = BoxCollider;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->AttachTo(RootComponent);
}

void ABuoyantObjectRuntime::BeginPlay()
{
	Super::BeginPlay();

	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode *)m_world->GetAuthGameMode();
	m_wasPushed = false;
}

void ABuoyantObjectRuntime::initSpawnValues(FVector meshLocation, FVector boxExtent, UStaticMesh *staticMesh)
{
	BoxCollider->SetBoxExtent(boxExtent);

	// Caching
	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode *)m_world->GetAuthGameMode();

	Mesh->SetStaticMesh(staticMesh);
	Mesh->SetRelativeLocation(meshLocation);

	FVector meshOrigin, meshBoxExtent;
	float sphereRadius;

	UKismetSystemLibrary::GetComponentBounds(Mesh, meshOrigin, meshBoxExtent, sphereRadius);

	computeBlockVolume(meshBoxExtent);

	FVector origin2, boxExtent2;
	float radius2;

	UKismetSystemLibrary::GetComponentBounds(Mesh, origin2, boxExtent2, radius2);

	FVector centerOfMassOffset = origin2 - Mesh->GetCenterOfMass();

	saveCheckpointData();

	// We need this because of the level streaming
	m_gameMode->getEventManager().subscribe("OnCheckpointReached", this, &ABuoyantObjectRuntime::onCheckpointReached);
	m_gameMode->getEventManager().subscribe("OnWolfRespawned", this, &ABuoyantObjectRuntime::onWolfRespawned);
	m_gameMode->getEventManager().subscribe("OnStreamSpawned", this, &ABuoyantObjectRuntime::onStreamSpawned);
	m_gameMode->getEventManager().subscribe("OnLevelUnloaded", this, &ABuoyantObjectRuntime::onLevelLoaded);
	m_gameMode->getEventManager().subscribe("OnWaterLevelChanged", this, &ABuoyantObjectRuntime::onWaterLevelChanged);

	findStreams();
}

void ABuoyantObjectRuntime::BeginDestroy()
{
	Super::BeginDestroy();

	if (m_gameMode)
	{
		// We need this because of the level streaming
		m_gameMode->getEventManager().unsubscribe("OnCheckpointReached", this, &ABuoyantObjectRuntime::onCheckpointReached);
		m_gameMode->getEventManager().unsubscribe("OnWolfRespawned", this, &ABuoyantObjectRuntime::onWolfRespawned);
		m_gameMode->getEventManager().unsubscribe("OnStreamSpawned", this, &ABuoyantObjectRuntime::onStreamSpawned);
		m_gameMode->getEventManager().unsubscribe("OnLevelUnloaded", this, &ABuoyantObjectRuntime::onLevelLoaded);
		m_gameMode->getEventManager().unsubscribe("OnWaterLevelChanged", this, &ABuoyantObjectRuntime::onWaterLevelChanged);
	}
}

void ABuoyantObjectRuntime::initStreamValues(float acceleration, float maxSpeed, float radius, float normalMod)
{
	m_acceleration = acceleration;
	m_maxSpeed = maxSpeed;
	m_radius = radius;
	m_normalMod = normalMod;
}

void ABuoyantObjectRuntime::computeBlockVolume(const FVector &boxExtent)
{
	m_blockSize = boxExtent * 2;
	m_blockVolume = m_blockSize.X * m_blockSize.Y * m_blockSize.Z;

	BoxCollider->SetMassOverrideInKg(NAME_None, m_blockVolume * M_ICE_BLOCK_DENSITY);
	Mesh->SetMassOverrideInKg(NAME_None, m_blockVolume * M_ICE_BLOCK_DENSITY);

	FVector tableInit[] = { FVector(1, 0, 0), FVector(0, 1, 0), FVector(0, 0, 1) };
	m_unitVectors.Append(tableInit, 3);
}

void ABuoyantObjectRuntime::Tick(float deltaSeconds)
{
	// TODO: Improve code after prototype has been tested, all temporary for now

	Super::Tick(deltaSeconds);

	if (m_streams.Num() == 0)
	{
		//findStreams();
	}

	// Workaround because level streaming is STUPID
	// https://answers.unrealengine.com/questions/294580/get-all-actors-of-class-with-level-streaming-broke.html

	/*
	"No, and we do not expect it to be fixed for any hotfixes. As mentioned above,
	there has been no update on the report itself, including a fix, but we will post
	here once there is. Thank you."
	*/
	if (!m_hasAlertedObjects)
	{
		m_hasAlertedObjects = true;

		m_gameMode->getEventManager().raiseEvent("OnBuoyantObjectRuntimeSpawned", new OnBuoyantObjectSpawnedData(this));
	}












	bool grounded = checkGround();

	// TODO: Put in individual private methods
	switch (m_currentState)
	{
	case BuoyantObjectRuntimeState::Grounded:
		if (!grounded)
		{
			m_currentState = BuoyantObjectRuntimeState::FreeFalling;
		}
		break;

	case BuoyantObjectRuntimeState::FreeFalling:
		if (grounded)
		{
			m_currentState = BuoyantObjectRuntimeState::Grounded;
		}
		break;

	case BuoyantObjectRuntimeState::Floating:
		// If we reset to a checkpoint, we may tick before we have the chance to set the water plane
		if (m_waterPlane)
		{
			floatOnWater();
			applyStreams();
		}

		break;
	}
}

// When the block is entering the water, we simulate a force to push it back afloat
void ABuoyantObjectRuntime::floatOnWater()
{
	applyBuoyancyForces();
}

void ABuoyantObjectRuntime::applyBuoyancyForces()
{
	// For our calculations, we rotate the system so that the block in lying flat and the water plane is skewed
	FRotator cuboidRot = BoxCollider->GetComponentRotation();
	FVector cuboidPlaneNormal = m_waterPlane->GetActorUpVector();
	FVector cuboidPlanePoint = m_waterPlane->GetActorLocation();
	FVector cuboidLocation = GetActorLocation();

	cuboidPlanePoint.X = cuboidLocation.X;
	cuboidPlanePoint.Y = cuboidLocation.Y;

	Matrix4x4 cuboidToUnitTrans = Matrix4x4::translationMatrix(-cuboidLocation);
	Matrix4x4 cuboidToUnitRot = Matrix4x4::rotationMatrix(cuboidRot.GetInverse());
	Matrix4x4 cuboidToUnitScale = Matrix4x4::scaleMatrix(FVector(1 / m_blockSize.X, 1 / m_blockSize.Y, 1 / m_blockSize.Z));

	/*
	A normal vector needs a different scale transform than a point since it's only a direction without a location,
	and it actually scales inversely proportional to the point. So, we needs to get the transpose of the inverse
	for non-uniform scaling. This is not a problem for rotation or translation transforms since there is not "stretching",
	i.e. proportions are met.
	Reference: http://oldwww.acm.org/tog/resources/RTNews/html/rtnews1a.html#art4
	*/
	Matrix4x4 cuboidToUnitNormalScale = Matrix4x4::scaleMatrix(m_blockSize).transpose();

	Matrix4x4 cuboidToUnitTransform = cuboidToUnitScale * cuboidToUnitRot * cuboidToUnitTrans;
	Matrix4x4 cuboidToUnitNormalTransform = cuboidToUnitNormalScale * cuboidToUnitRot;

	FVector unitPlaneNormal = FVector(cuboidToUnitNormalTransform * FVector4(cuboidPlaneNormal, 0));
	unitPlaneNormal.Normalize(); // The scale may un-normalize it

	FVector unitPlanePoint = FVector(cuboidToUnitTransform * FVector4(cuboidPlanePoint, 1));

	FVector prevUnitPlanePoint = unitPlanePoint;
	FVector prevUnitPlaneNormal = unitPlaneNormal;

	Matrix3x3 swapTransform = getSwapTransform(unitPlaneNormal);
	Matrix3x3 firstQuadrantTransform = Matrix3x3::getFirstQuadrantTransform(unitPlaneNormal);
	Matrix3x3 reorderTransform = swapTransform * firstQuadrantTransform;
	unitPlaneNormal = reorderTransform * unitPlaneNormal;
	unitPlanePoint = reorderTransform * unitPlanePoint;

	unitPlanePoint += FVector(0.5, 0.5, 0.5);

	// Ugly patch because, apparently, the buoyancy method doesn't work when one element of the normal is zero
	if (FMath::IsNearlyZero(unitPlaneNormal.X, 0.001f))
	{
		unitPlaneNormal.X = 0.001;
	}

	if (FMath::IsNearlyZero(unitPlaneNormal.Y, 0.001f))
	{
		unitPlaneNormal.Y = 0.001;
	}

	if (FMath::IsNearlyZero(unitPlaneNormal.Z, 0.001f))
	{
		unitPlaneNormal.Z = 0.001;
	}

	float submergedVolume = getSubmergedUnitBlockVolume(unitPlaneNormal, unitPlanePoint) * m_blockVolume;

	// This guard is necessary since the overlap event is triggered the frame before the actual overlap
	if (submergedVolume > 0)
	{
		FVector totalForce = FVector(0, 0, getBuoyancyZ(submergedVolume) + getViscosityZ() + getDragZ());

		FVector centerOfBuoyancy = getCenterOfBuoyancy(unitPlaneNormal, unitPlanePoint, swapTransform, firstQuadrantTransform);

		BoxCollider->AddForceAtLocation(totalForce, centerOfBuoyancy);
	}
}

FVector ABuoyantObjectRuntime::getCenterOfBuoyancy(FVector unitPlaneNormal, FVector unitPlanePoint, Matrix3x3 swapTransform, Matrix3x3 firstQuadrantTransform)
{
	float d = FMath::Abs(FVector::DotProduct(unitPlaneNormal, unitPlanePoint));

	if (d < 0 || d > FVector::DotProduct(unitPlaneNormal, FVector(1, 1, 1))) return GetActorLocation();


	FVector baseCenter = d / 4 * unitPlaneNormal; // The center of the big tetrahedron without corrections

	FVector otherCenters = FVector::ZeroVector;

	for (int i = 0; i < 3; i++)
	{
		// We check if there are any overcounted tetrahedra
		if (unitPlaneNormal[i] != 0 && d / unitPlaneNormal[i] - 1 >= 0)
		{
			otherCenters[i] -= getTetrahedronCenterOfMassElement(unitPlaneNormal[i], unitPlaneNormal, unitPlanePoint - m_unitVectors[i]);
		}

		// We check if there are any undercounted tetrahedra
		if (unitPlaneNormal[(i + 1) % 3] != 0 && (d - unitPlaneNormal[i]) / unitPlaneNormal[(i + 1) % 3] - 1 >= 0)
		{
			otherCenters[i] += getTetrahedronCenterOfMassElement(unitPlaneNormal[i], unitPlaneNormal, unitPlanePoint - (m_unitVectors[i] + m_unitVectors[(i + 1) % 3]));
		}
	}

	FVector unitCenterOfBuoyancy = baseCenter + otherCenters;

	FVector centerOfBuoyancy = unitCenterOfBuoyancy;// -FVector(0.5, 0.5, 0.5);

	Matrix3x3 reorderTransform = firstQuadrantTransform * swapTransform;
	centerOfBuoyancy = reorderTransform * centerOfBuoyancy;


	centerOfBuoyancy.X *= -1;
	centerOfBuoyancy.Y *= -1;
	centerOfBuoyancy.Z *= -1;

	Matrix4x4 unitToCuboidTrans = Matrix4x4::translationMatrix(GetActorLocation());
	Matrix4x4 unitToCuboidRot = Matrix4x4::rotationMatrix(BoxCollider->GetComponentRotation());
	Matrix4x4 unitToCuboidScale = Matrix4x4::scaleMatrix(m_blockSize);

	Matrix4x4 unitToCuboidTransform = unitToCuboidTrans * unitToCuboidRot * unitToCuboidScale;
	centerOfBuoyancy = FVector(unitToCuboidTransform * FVector4(centerOfBuoyancy, 1));

	return centerOfBuoyancy;
}

// If there are 0 elements in the vector (except for Y), we have to swap them since the integration order
// for the volume is "dz dx dy". So we order the zeros at the start of the vector when possible
Matrix3x3 ABuoyantObjectRuntime::getSwapTransform(FVector normal)
{
	Matrix3x3 swapTransform = Matrix3x3::identity;

	if (normal.X == 0 && normal.Y != 0 && normal.Z != 0)
	{
		// We want to have the Y as 0

		swapTransform = Matrix3x3::swapXYTransform;
	}
	else if (normal.X != 0 && normal.Y != 0 && normal.Z == 0)
	{
		// We want to have the Y as 0

		swapTransform = Matrix3x3::swapYZTransform;
	}
	else if (normal.Z == 0 && normal.X != 0 && normal.Y == 0)
	{
		// We want to have the X, Y as 0

		swapTransform = Matrix3x3::swapXZTransform;
	}
	else if (normal.Z == 0 && normal.X == 0 && normal.Y != 0)
	{
		// We want to have the X, Y as 0

		swapTransform = Matrix3x3::swapYZTransform;
	}

	return swapTransform;
}

float ABuoyantObjectRuntime::getSubmergedUnitBlockVolume(FVector planeNormal, FVector p0)
{
	Matrix3x3 swapTransform = getSwapTransform(planeNormal);
	Matrix3x3 firstQuadrantTransform = Matrix3x3::getFirstQuadrantTransform(planeNormal);
	Matrix3x3 reorderTransform = swapTransform * firstQuadrantTransform;
	planeNormal = reorderTransform * planeNormal;
	p0 = reorderTransform * p0;

	float d = FMath::Abs(FVector::DotProduct(planeNormal, p0));

	// If the block is not in the water yet
	if (d < 0) return 0;

	// If the block is completely under water
	if (d > FVector::DotProduct(planeNormal, FVector(1, 1, 1))) return 1;

	float baseVol = getTetrahedronVolume(planeNormal, p0); // The volume without corrections
	float volCorrection = 0; // The overcounted or undercounted volumes of tetrahedra

	for (int i = 0; i < 3; i++)
	{
		// We check if there are any overcounted tetrahedra
		if (planeNormal[i] != 0 && d / planeNormal[i] - 1 >= 0)
		{
			volCorrection -= getTetrahedronVolume(planeNormal, p0 - m_unitVectors[i]);
		}

		// We check if there are any undercounted tetrahedra
		if (planeNormal[(i + 1) % 3] != 0 && (d - planeNormal[i]) / planeNormal[(i + 1) % 3] - 1 >= 0)
		{
			volCorrection += getTetrahedronVolume(planeNormal, p0 - (m_unitVectors[i] + m_unitVectors[(i + 1) % 3]));
		}
	}

	float unitCubeSubmergedVolume = baseVol + volCorrection;

	return unitCubeSubmergedVolume;
}

float ABuoyantObjectRuntime::getTetrahedronVolume(FVector planeNormal, FVector planePoint)
{
	Matrix3x3 swapTransform = getSwapTransform(planeNormal);
	Matrix3x3 firstQuadrantTransform = Matrix3x3::getFirstQuadrantTransform(planeNormal);
	Matrix3x3 reorderTransform = swapTransform * firstQuadrantTransform;
	planeNormal = reorderTransform * planeNormal;
	planePoint = reorderTransform * planePoint;

	float d = FVector::DotProduct(planeNormal, planePoint);
	float a = planeNormal.X;
	float b = planeNormal.Y;
	float c = planeNormal.Z;

	// Since we reordered the normal elements earlier, we only have to do 3 cases instead of 7

	float vol = 0;

	if (a != 0 && b != 0 && c != 0)
	{
		vol = (d*d*d) / (6 * a*b*c);
	}
	else if (a == 0 && b == 0)
	{
		vol = d / c;
	}
	else if (b == 0)
	{
		vol = (d*d) / (2 * a*c);
	}

	return vol;
}

float ABuoyantObjectRuntime::getTetrahedronCenterOfMassElement(float element, FVector planeNormal, FVector planePoint)
{
	float d = FVector::DotProduct(planeNormal, planePoint);

	return d / 4 * element;
}

float ABuoyantObjectRuntime::getBuoyancyZ(float submergedVolume)
{
	// Buoyancy force = Volume submerged * density of water * gravity

	return submergedVolume * M_SEA_WATER_DENSITY * -m_world->GetGravityZ();
}


float ABuoyantObjectRuntime::getViscosityZ()
{
	// Since this force is negligible for large objects, we can remove it if there are performance issues or too many blocks in the level

	// Viscosity force = 6 * Pi * dynamic viscosity * radius * velocity
	float viscosityZ = 6 * PI * M_COLD_WATER_VISCOSITY * m_blockSize.Z / 2 * BoxCollider->GetPhysicsLinearVelocity().Z;

	// The formula gives us Pa*cm^2 (since Unreal units are centimeters), so we need to convert it
	// to Pa*m^2 (which are equal to newton or kg*m*s^-2), and then to kg*cm*s^-2
	viscosityZ /= m_blockSize.Z;

	return viscosityZ;
}

float ABuoyantObjectRuntime::getDragZ()
{
	// Drag force = 0.5 * drag coefficient * density of liquid * cross sectional area (i.e. area of bottom face) * velocity^2

	float velocityZ = BoxCollider->GetPhysicsLinearVelocity().Z;

	float dragZ = 0.5 * M_CUBE_DRAG_COEFFICIENT * M_SEA_WATER_DENSITY * m_blockSize.X * m_blockSize.Y * -FMath::Sign(velocityZ) * velocityZ * velocityZ;

	return dragZ;
}

// Check if we are touching the ground in order to enable/disable the physics
// When the block is on the ground, we currently disable the physics in order to push/pull it correctly
bool ABuoyantObjectRuntime::checkGround()
{
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	FVector startTrace = GetActorLocation() - FVector(0, 0, m_blockSize.Z / 2);
	FVector endTrace = startTrace + FVector(0.f, 0.f, -5.0f);

	FCollisionShape boxShape = FCollisionShape();
	boxShape.SetBox(FVector(8, 8, 8));

	return m_world->SweepTestByObjectType(startTrace, endTrace, FQuat::Identity, FCollisionObjectQueryParams::DefaultObjectQueryParam, boxShape, TraceParams);
}

// TODO: Temporary
void ABuoyantObjectRuntime::onCheckpointReached(OnCheckpointReachedData *eventData)
{
	saveCheckpointData();
}

// TODO: Temporary
void ABuoyantObjectRuntime::onWolfRespawned(OnWolfRespawnedData *eventData)
{
	loadCheckpointData();
}


// TODO: Temporary
// We could use a state object to load/save easily
void ABuoyantObjectRuntime::saveCheckpointData()
{
	m_lastCheckpointState = m_currentState;
	m_lastCheckpointPosition = GetActorLocation();
	m_lastCheckpointRotation = GetActorRotation();
	m_lastCheckpointVelocity = BoxCollider->GetPhysicsLinearVelocity();
	m_lastCheckpointAngularVelocity = BoxCollider->GetPhysicsAngularVelocity();
}

// TODO: Temporary
// We could use a state object to load/save easily
void ABuoyantObjectRuntime::loadCheckpointData()
{
	LUNA_WARN(BuoyantObjectRuntime, "Resetting block");

	m_currentState = m_lastCheckpointState;

	SetActorLocation(m_lastCheckpointPosition, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorRotation(m_lastCheckpointRotation);

	BoxCollider->SetPhysicsLinearVelocity(m_lastCheckpointVelocity);
	BoxCollider->SetPhysicsAngularVelocity(m_lastCheckpointAngularVelocity);
}

void ABuoyantObjectRuntime::beginWaterOverlap(AWaterPlane *waterPlane)
{
	//UE_LOG(LogTemp, Warning, TEXT("BeginWaterOverlap"));

	m_waterPlane = Cast<AWaterPlane>(waterPlane);

	if (m_currentState == BuoyantObjectRuntimeState::FreeFalling)
	{
		//UE_LOG(LogTemp, Warning, TEXT("BeginWaterOverlap"));

		for(TActorIterator<AActor> ActorItr(m_world); ActorItr; ++ActorItr) {
			if(ActorItr->IsA(AWolf::StaticClass())) {
				AWolf *wolf = Cast<AWolf>(*ActorItr);

				float distance = (wolf->GetActorLocation() - GetActorLocation()).Size();

				if(distance < 300) {
					if(OnBlockSplash.IsBound()) {
						UE_LOG(LogTemp, Warning, TEXT("Hitting Water"));
						OnBlockSplash.Broadcast();
					}
				}

				break;
			}
		}
		

		//TODO SR improve
		m_gameMode->getEventManager().raiseEvent("OnSplash", new GenericEventData());
	}
	
	m_currentState = BuoyantObjectRuntimeState::Floating;

	BoxCollider->SetPhysicsLinearVelocity(FVector(0, 0, BoxCollider->GetPhysicsLinearVelocity().Z));
}

void ABuoyantObjectRuntime::endWaterOverlap(AWaterPlane *waterPlane)
{
	if (m_waterPlane && m_waterPlane == Cast<AWaterPlane>(waterPlane))
	{
		m_currentState = BuoyantObjectRuntimeState::FreeFalling;

		m_waterPlane = nullptr;
	}
}

bool ABuoyantObjectRuntime::isInWater()
{
	return m_currentState == BuoyantObjectRuntimeState::Floating;
}

void ABuoyantObjectRuntime::findStreams()
{
	// COMMENTED OUT UNTIL EPIC HAS FIXED THIS LEVEL STREAMING BUG
	// https://answers.unrealengine.com/questions/294580/get-all-actors-of-class-with-level-streaming-broke.html

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStreamEntity::StaticClass(), FoundActors);

	for (AActor* TActor : FoundActors)
	{
		AStreamEntity* MyActor = (AStreamEntity*)TActor;

		if (MyActor != nullptr)
		{
			m_streams.Add(MyActor);
		}
	}
}

void ABuoyantObjectRuntime::applyStreams()
{
	for (AStreamEntity* stream : m_streams)
	{
		stream->ApplyForce(BoxCollider);
	}
}

void ABuoyantObjectRuntime::testNormals()
{
	float radius = 300;

	FVector velocity = FVector(BoxCollider->GetComponentVelocity());
	velocity.Z = 0;
	float tangSpeed = velocity.Size2D();
	UE_LOG(LogTemp, Warning, TEXT("Speed:  %f"), tangSpeed);
	velocity.Normalize();

	FVector forward = FVector(velocity);

	if (FMath::Abs(tangSpeed) < 0.0001f)
	{
		forward = FVector::ForwardVector;
	}

	FVector left = FVector::CrossProduct(forward, FVector::UpVector);

	float targetAcceleration = FMath::Min(m_maxSpeed - tangSpeed * 1.1f, m_acceleration);

	BoxCollider->AddForce(forward * targetAcceleration, NAME_None, true);

	DrawDebugLine(GetWorld(),
		BoxCollider->GetComponentLocation(),
		BoxCollider->GetComponentLocation() + forward * 300 * targetAcceleration / m_acceleration + FVector::UpVector * 20,
		FColor(255, 0, 0),
		false, -1, 0, 15);

	float acc = tangSpeed * tangSpeed / m_radius;
	BoxCollider->AddForce(left * acc * m_normalMod, NAME_None, true);

	DrawDebugLine(GetWorld(),
		BoxCollider->GetComponentLocation(),
		BoxCollider->GetComponentLocation() + left * 300 + FVector::UpVector * 20,
		FColor(0, 200, 0),
		false, -1, 0, 15);
}

void ABuoyantObjectRuntime::onStreamSpawned(OnStreamSpawnedData *eventData)
{
	// If the BuoyantObjectRuntime is spawned before the streams, we need to add them manually afterwards
	m_streams.Add(eventData->getStream());
}

void ABuoyantObjectRuntime::onLevelLoaded(OnLevelLoadedData *eventData)
{
	// We need to destroy ourselves when we load another level since the level streaming doesn't know us
	Destroy();
}

void ABuoyantObjectRuntime::onWaterLevelChanged(OnWaterLevelChangedData *eventData)
{
	bool isStillInStream = false;

	for(AStreamEntity* stream : m_streams) {
		// Ewww, wtf is that code

		if(m_currentState == BuoyantObjectRuntimeState::Floating && !isStillInStream) {
			StreamSection *section = stream->IsInsideStream(BoxCollider->GetComponentLocation());

			if(section) {
				isStillInStream = true;

				if (!m_isInStream)
				{
					//stream->applyImpulse(BoxCollider);
				}

				break;
			}
		}
	}
	
	if(m_isInStream && !isStillInStream) {
		// The block is no longer in the stream
		BoxCollider->SetPhysicsLinearVelocity(FVector(0, 0, BoxCollider->GetPhysicsLinearVelocity().Z));
		m_isInStream = false;

		UE_LOG(LogTemp, Warning, TEXT("Left Stream!!!"));

		for(AStreamEntity* stream : m_streams) {
			StreamSection *section = stream->IsInsideStream2(BoxCollider->GetComponentLocation());
		}
	} else if(!m_isInStream && isStillInStream) {
		// The block just entered a stream
		m_isInStream = true;


		//UE_LOG(LogTemp, Warning, TEXT("Entered Stream!!!"));
	}
}