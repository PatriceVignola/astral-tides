// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "StreamSection.h"

StreamSection::StreamSection(FVector p1, FVector p2, FVector p3, FVector p4, 
	FVector tangent, FVector normal, float radius)
{
	m_tangent = tangent;
	m_normal = normal;

	m_radius = radius;

	m_p1 = p1;
	m_p2 = p2;
	m_p3 = p3;
	m_p4 = p4;

	// Calculate minimum and maximum
	m_min = p1.ComponentMin(p2).ComponentMin(p3).ComponentMin(p4);
	m_max = p1.ComponentMax(p2).ComponentMax(p3).ComponentMax(p4);
	m_centre = (p1 + p2 + p3 + p4) * 0.25;

	//Area of a quad is the sum of areas of two triangles making it
	m_area = (GetArea(p1, p3, p2) + GetArea(p1, p4, p3)) * TOLERANCE;
}

StreamSection::~StreamSection()
{
}

bool StreamSection::IsInside(FVector pos)
{
	//Preliminary checks
	if (!FMath::IsWithinInclusive(pos.X, m_min.X, m_max.X))
		return false;
	if (!FMath::IsWithinInclusive(pos.Y, m_min.Y, m_max.Y))
		return false;

	
	//Verify with area
	if (GetArea(pos, m_p1, m_p2) + GetArea(pos, m_p2, m_p3) +
		GetArea(pos, m_p3, m_p4) + GetArea(pos, m_p4, m_p1) > m_area)
		return false;

	//Here, we know we're inside

	return true;
}

bool StreamSection::IsInside2(FVector pos) {
	//Preliminary checks
	if(!FMath::IsWithinInclusive(pos.X, m_min.X, m_max.X)) {
		UE_LOG(LogTemp, Warning, TEXT("NOT WITHIN X OF SECTION"));
		return false;
	}

	if(!FMath::IsWithinInclusive(pos.Y, m_min.Y, m_max.Y)) {
		UE_LOG(LogTemp, Warning, TEXT("NOT WITHIN Y OF SECTION"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("PASSED THE WALL!!!"));


	//Verify with area
	if(GetArea(pos, m_p1, m_p2) + GetArea(pos, m_p2, m_p3) +
	   GetArea(pos, m_p3, m_p4) + GetArea(pos, m_p4, m_p1) > m_area)
		return false;

	//Here, we know we're inside


	return true;
}

FVector StreamSection::GetTangent()
{
	return m_tangent;
}

FVector StreamSection::GetNormal()
{
	return m_normal;
}

float StreamSection::GetRadius()
{
	return m_radius;
}

FVector StreamSection::GetCentre()
{
	return m_centre;
}

FVector StreamSection::GetMin()
{
	return m_min;
}

FVector StreamSection::GetMax()
{
	return m_max;
}

float StreamSection::GetArea(FVector pt1, FVector pt2, FVector pt3)
{
	//Cross product in 2D space
	return 0.5 * ((pt1.X - pt3.X) * (pt2.Y - pt3.Y) - (pt2.X - pt3.X) * (pt1.Y - pt3.Y));
}

/*void StreamSection::GetPoints(const FVector& p1, const FVector& p2, 
	const FVector& p3, const FVector& p4)
{
	p1 = m_p1;
	p2 = m_p2;
	p3 = m_p3;
	p4 = m_p4;
}*/


