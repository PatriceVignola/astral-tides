// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "StreamEntity.h"
#include "LunaLevelScriptActor.h"

// Sets default values
AStreamEntity::AStreamEntity()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

AStreamEntity::~AStreamEntity()
{
	m_sections.Empty();
}

// Called when the game starts or when spawned
void AStreamEntity::BeginPlay()
{
	Super::BeginPlay();

	m_hasAlertedObjects = false;

	if (m_world) return;


	m_world = GetWorld();
	m_levelScript = Cast<ALunaLevelScriptActor>(m_world->GetLevelScriptActor());
	m_gameMode = (AProjectLunaGameMode*) m_world->GetAuthGameMode();

	m_gameMode->getEventManager().subscribe("OnWaterLevelChanged", this, &AStreamEntity::onWaterLevelChanged);

	TArray<USplineComponent*> splines;
	this->GetComponents(splines);
	if (splines.Num() > 0)
	{
		CreateSections(splines[0]);
	}

	this->GetComponents(m_meshes);


	//m_airMaterial = UMaterialInstanceDynamic::Create(m_levelScript->StreamAir, this);

	// Hack;
	m_initialWaterLevel = 70;

	currentWaterLevel = m_initialWaterLevel;
}

// Called every frame
void AStreamEntity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

		m_gameMode->getEventManager().raiseEvent("OnStreamSpawned", new OnStreamSpawnedData(this));
	}
}

void AStreamEntity::CreateSections(USplineComponent * spline)
{
	if (spline == NULL)
		return;

	m_spline = spline;

	//Split the current in a number of sections
	float length = spline->GetSplineLength();
	int numSections = FMath::FloorToInt(length / SectionLength);
	float divLength = length / numSections;

	//Needed variables
	FVector p1Pos, p2Pos;
	FVector p1Dir, p2Dir;
	FVector zP1, zP2, zP3, zP4;
	FVector leftVector;
	FVector oldLeftVector;
	FVector tangent;
	FVector normal;
	float angle;
	float radius;
	float dotP;

	StreamSection *section;
	/*TODO Calculate the width from the spline object to have variable widths*/

	//Get the properties of the first spline point
	p1Pos = spline->GetLocationAtDistanceAlongSpline(0, ESplineCoordinateSpace::Type::World);
	p1Pos.Z = 0;
	p1Dir = spline->GetTangentAtDistanceAlongSpline(0, ESplineCoordinateSpace::Type::World);
	p1Dir.Z = 0;
	leftVector = FVector::CrossProduct(p1Dir, FVector::UpVector);
	leftVector.Normalize();
	zP1 = p1Pos - (Width + 50) * leftVector;
	zP2 = p1Pos + (Width + 50) * leftVector;
	m_min = zP1.ComponentMin(zP2);
	m_max = zP1.ComponentMax(zP2);

	//UE_LOG(LogTemp, Warning, TEXT("New stream!"));

	for (int i = 1; i <= numSections; i++)
	{
		oldLeftVector = leftVector;

		//Take the new point
		p2Pos = spline->GetLocationAtDistanceAlongSpline(i * divLength, ESplineCoordinateSpace::Type::World);
		p2Pos.Z = 0;
		p2Dir = spline->GetTangentAtDistanceAlongSpline(i * divLength, ESplineCoordinateSpace::Type::World);
		p2Dir.Z = 0;
		leftVector = FVector::CrossProduct(p2Dir, FVector::UpVector);
		leftVector.Normalize();
		zP3 = p2Pos - (Width + 50) * leftVector;
		zP4 = p2Pos + (Width + 50) * leftVector;
		m_min = m_min.ComponentMin(zP3).ComponentMin(zP4);
		m_max = m_max.ComponentMax(zP3).ComponentMax(zP4);

		//Take the direction of the point in the middle of the section
		tangent = spline->GetTangentAtDistanceAlongSpline((i - 0.5) * divLength, ESplineCoordinateSpace::Type::World);
		tangent.Z = 0;
		tangent.Normalize();
		dotP = FVector::DotProduct(oldLeftVector, leftVector);

		//If both have the same direction, then no normal force
		if(FMath::IsNearlyEqual(dotP, 1, 0.0001f))
		{
			normal = FVector::ZeroVector;
			radius = 1.0;
		}
		else
		{
			//Compute the curvature radius for that section
			angle = FGenericPlatformMath::Acos(dotP);
			radius = divLength / angle;

			//Take the normal vector
			normal = FVector::CrossProduct(tangent, FVector::UpVector);
			normal.Normalize();

			normal *= FMath::Sign(FVector::CrossProduct(leftVector, oldLeftVector).Z);
		}

		//Create the current piece using the current and previous points
		section = new StreamSection(zP1, zP3, zP4, zP2, tangent, normal, radius);

		/*
		DrawDebugLine(GetWorld(), zP1, zP1 + FVector(0, 0, 300), FColor::Red, true);
		DrawDebugLine(GetWorld(), zP2, zP2 + FVector(0, 0, 300), FColor::Red, true);
		DrawDebugLine(GetWorld(), zP3, zP3 + FVector(0, 0, 300), FColor::Red, true);
		DrawDebugLine(GetWorld(), zP4, zP4 + FVector(0, 0, 300), FColor::Red, true);*/

		m_sections.Add(section);

		/*
		UE_LOG(LogTemp, Warning, TEXT(" Section %d"), i);
		UE_LOG(LogTemp, Warning, TEXT("  P1 - %f, %f"), zP1.X, zP1.Y);
		UE_LOG(LogTemp, Warning, TEXT("  P3 - %f, %f"), zP3.X, zP3.Y);
		UE_LOG(LogTemp, Warning, TEXT("  P4 - %f, %f"), zP4.X, zP4.Y);
		UE_LOG(LogTemp, Warning, TEXT("  P2 - %f, %f"), zP2.X, zP2.Y);

		UE_LOG(LogTemp, Warning, TEXT("  Radius:  %f"), radius);
		UE_LOG(LogTemp, Warning, TEXT("  Normal:  %f,  %f"), normal.X, normal.Y);*/

		FVector z = FVector::UpVector * MarksHeight;


		if (ShowMarks)
		{
			DrawDebugLine(GetWorld(),
				zP1 + z, zP3 + z,
				FColor(0, 0, 255),
				true, -1, 0, 2);

			DrawDebugLine(GetWorld(),
				zP2 + z, zP4 + z,
				FColor(0, 80, 175),
				true, -1, 0, 2);

			DrawDebugLine(GetWorld(),
				zP3 + z, zP4 + z,
				FColor(0, 120, 80),
				true, -1, 0, 1);
		}

		//Shift the current to the previous
		p1Pos = p2Pos;
		p1Dir = p2Dir;
		zP1 = zP3;
		zP2 = zP4;
	}
}

void AStreamEntity::onWaterLevelChanged(OnWaterLevelChangedData *eventData)
{
	currentWaterLevel = m_initialWaterLevel + eventData->getNewWaterDelta();
	float streamHeight = GetActorLocation().Z;

	currentState = StreamState::Surface;

	if (streamHeight + MaxHeight < currentWaterLevel)
	{
		currentState = StreamState::UnderWater;
	}
	else if (streamHeight + MinHeight > currentWaterLevel)
	{
		currentState = StreamState::InTheAir;
	}

	updateStreamMaterial();
}

StreamSection* AStreamEntity::IsInsideStream(FVector pos)
{
	float streamHeight = GetActorLocation().Z;

	if (currentState != StreamState::Surface)
	{
		return nullptr;
	}

	//Preliminary quick checks
	if (!FMath::IsWithinInclusive(pos.X, m_min.X, m_max.X))
		return nullptr;
	if (!FMath::IsWithinInclusive(pos.Y, m_min.Y, m_max.Y))
		return nullptr;

	StreamSection *section = nullptr;
	//Find the first stream section that applies
	for (int i = 0; i < m_sections.Num(); i++)
	{
		if (m_sections[i]->IsInside(pos))
		{
			section = m_sections[i];
			break;
		}
	}

	if (section)
	{
		bContainsBlock = true;
	}

	return section;
}

StreamSection* AStreamEntity::IsInsideStream2(FVector pos) {
	float streamHeight = GetActorLocation().Z;

	if(currentState != StreamState::Surface) {
		UE_LOG(LogTemp, Warning, TEXT("NOT AT SURFACE"));
		return nullptr;
	}

	//Preliminary quick checks
	if(!FMath::IsWithinInclusive(pos.X, m_min.X, m_max.X))
	{
		UE_LOG(LogTemp, Warning, TEXT("NOT WITHIN X"));
	}
		
	if(!FMath::IsWithinInclusive(pos.Y, m_min.Y, m_max.Y))
	{
		UE_LOG(LogTemp, Warning, TEXT("NOT WITHIN Y"));
	}
		

	StreamSection *section = nullptr;
	//Find the first stream section that applies
	for(int i = 0; i < m_sections.Num(); i++) {
		if(m_sections[i]->IsInside2(pos)) {
			section = m_sections[i];
			break;
		}
	}

	return section;
}

void AStreamEntity::ApplyForce(UPrimitiveComponent *comp)
{
	// Ewww, wtf is that code

	FVector boxExtent, actorPos;
	float sphereRadius;
	UKismetSystemLibrary::GetComponentBounds(comp, actorPos, boxExtent, sphereRadius);

	if (actorPos.Z - boxExtent.Z > currentWaterLevel || actorPos.Z + boxExtent.Z < currentWaterLevel)
	{
		return;
	}

	StreamSection *section = IsInsideStream(actorPos);
	if (section == NULL) return;
	

	//Object properties
	FVector vVelocity = comp->GetComponentVelocity();
	FVector pPos = comp->GetComponentLocation();

	//Section properties
	FVector uTangent = (m_isReversed ? -1 : 1) * section->GetTangent();







	// Position of the block in the spline's space
	FVector relativePos = m_spline->ComponentToWorld.InverseTransformPosition(actorPos);
	float dummy;
	float inputKey = m_spline->SplineInfo.InaccurateFindNearest(relativePos, dummy);

	// Tangent closest to the block
	uTangent = (m_isReversed ? -1 : 1) * m_spline->GetTangentAtSplineInputKey(inputKey, ESplineCoordinateSpace::World).GetSafeNormal();

	//DrawDebugLine(GetWorld(), actorPos, actorPos + uTangent * 400, FColor::Red);


	// Spline position closest to the block
	FVector closestPoint = m_spline->SplineInfo.Eval(inputKey, FVector::ZeroVector);
	closestPoint = m_spline->ComponentToWorld.TransformPosition(closestPoint);

	//DrawDebugLine(GetWorld(), closestPoint, closestPoint + FVector(0, 0, 1000), FColor::Black);

	// Distance between the block and the closest spline point
	float blockDistance = (actorPos - closestPoint).Size();
	//UE_LOG(LogTemp, Warning, TEXT("%f"), blockDistance);

	// Normal to push the block towards the center (don't forget, we need to use the left hand rule with Unreal Engine, so the product order is inversed)
	FVector normal = FVector::CrossProduct(uTangent, FVector::UpVector);
	
	FVector streamOrigin, streamExtents;
	GetActorBounds(false, streamOrigin, streamExtents);


	//DrawDebugLine(GetWorld(), streamOrigin, streamOrigin + FVector(0, 0, 2000), FColor::Red);

	float streamDirection = FMath::Sign(FVector::CrossProduct((streamOrigin - closestPoint), uTangent).Z);
	


	// If the block is more on the outer side of the stream, we push it inward
	if ((streamOrigin - actorPos).Size2D() > (streamOrigin - closestPoint).Size2D())
	{
		comp->AddForce(normal * blockDistance * 8000 * streamDirection);//, NAME_None, true);
		//DrawDebugLine(GetWorld(), actorPos, actorPos + normal * 400 * streamDirection, FColor::Blue);
	}
	// Otherwise, we push it outward
	else
	{
		comp->AddForce(-normal * blockDistance * 8000 * streamDirection);//, NAME_None, true);
		//DrawDebugLine(GetWorld(), actorPos, actorPos - normal * 400 * streamDirection, FColor::Blue);
	}
	

	FVector uNormal = section->GetNormal();
	float radius = section->GetRadius();

	//Current tangent speed
	float tgSpeed = FVector::DotProduct(vVelocity, uTangent);
	float nmSpeed = FVector::DotProduct(vVelocity, uNormal);

	//Add the tangent force
	float tgForce = FMath::Min(TargetSpeed - tgSpeed * 1.1f, 2000.f);
	comp->AddForce(uTangent * tgForce, NAME_None, true);

	//Add the normal force
	float nmForce = (tgSpeed * tgSpeed / radius) * NormalMod  ;
	comp->AddForce(uNormal * nmForce, NAME_None, true);

	float addNmForce = -nmSpeed * NormalSpeedMod;
	comp->AddForce(uNormal * addNmForce, NAME_None, true);

	//UE_LOG(LogTemp, Warning, TEXT("%f"), tgForce);

	if (ShowMarks)
	{
		DrawDebugLine(GetWorld(),
			pPos,
			pPos + uTangent * tgForce * LineWidth + FVector::UpVector * 20,
			FColor(255, 0, 0),
			false, 0.5f, 0, 15);

		DrawDebugLine(GetWorld(),
			pPos,
			pPos + uNormal * nmForce * LineWidth + FVector::UpVector * 15,
			FColor(0, 250, 0),
			false, 0.5f, 0, 6);

		DrawDebugLine(GetWorld(),
			pPos,
			pPos + uNormal * addNmForce * LineWidth + FVector::UpVector * 25,
			FColor(50, 50, 150),
			false, 0.5f, 0, 7);
	}
}

void AStreamEntity::applyImpulse(UPrimitiveComponent *comp) {
	StreamSection *section = IsInsideStream(comp->GetComponentLocation());
	if(section == NULL) return;

	FVector uTangent = (m_isReversed ? -1 : 1) * section->GetTangent();

	comp->AddImpulse(uTangent * Force);
}

bool AStreamEntity::IsReversed()
{
	return m_isReversed;
}

void AStreamEntity::Reverse()
{
	SetReversed(!m_isReversed);
}

void AStreamEntity::SetReversed(bool isReversed)
{
	m_isReversed = isReversed;

	UMaterialInstanceDynamic* mat = NULL;

	//Load the right material
	if (!isReversed)
	{
		mat = UMaterialInstanceDynamic::Create(m_levelScript->StreamSurface, this);
	}
	else
	{
		mat = UMaterialInstanceDynamic::Create(m_levelScript->StreamReversed, this);
	}

	for (USplineMeshComponent* comp : m_meshes)
	{
		comp->SetMaterial(0, mat);
	}

	updateStreamMaterial();
}

const TArray<StreamSection*>& AStreamEntity::getSections() const {
	return m_sections;
}

void AStreamEntity::updateStreamMaterial()
{
	// Material values for the active stream
	float matWind = 0;
	float matType = 1;

	switch (currentState)
	{
	case StreamState::UnderWater:
		matType = 0;
		break;
	case StreamState::InTheAir:
		matWind = 1;
		break;
	}

	for(USplineMeshComponent* comp : m_meshes) {
		UMaterialInstanceDynamic *mat = Cast<UMaterialInstanceDynamic>(comp->GetMaterial(0));

		if (mat)
		{
			mat->SetScalarParameterValue("mat_type", matType);
			mat->SetScalarParameterValue("mat_wind", matWind);
		}
	}
}

FVector AStreamEntity::getClosestPointOnSpline(FVector otherPoint)
{
	FVector relativePos = m_spline->ComponentToWorld.InverseTransformPosition(otherPoint);
	float dummy;
	float inputKey = m_spline->SplineInfo.InaccurateFindNearest(relativePos, dummy);

	// Spline position closest to the block
	FVector closestPoint = m_spline->SplineInfo.Eval(inputKey, FVector::ZeroVector);
	closestPoint = m_spline->ComponentToWorld.TransformPosition(closestPoint);

	return closestPoint;
}

bool AStreamEntity::containsBlock()
{
	// Ugly hack
	bool containedBlock = bContainsBlock;
	bContainsBlock = false;

	return containedBlock;
}