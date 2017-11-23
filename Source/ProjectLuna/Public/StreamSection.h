// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
class PROJECTLUNA_API StreamSection
{
public:
	StreamSection(FVector p1, FVector p2, FVector p3, FVector p4, FVector tangent,
		FVector normal, float radius);
	~StreamSection();

	/*
	Applies the current force to the object if inside
	*/
	bool IsInside(FVector pos);
	bool IsInside2(FVector pos);
	FVector GetTangent();
	FVector GetNormal();
	float GetRadius();
	FVector GetCentre();
	FVector GetMin();
	FVector GetMax();
	/*void GetPoints(const FVector& p1, const FVector& p2,
		const FVector& p3, const FVector& p4);*/

private:

	FVector m_normal;
	FVector m_tangent;
	FVector m_p1, m_p2, m_p3, m_p4;
	float m_area;
	float m_radius;
	FVector m_min;
	FVector m_max;
	FVector m_centre;

	const float TOLERANCE = 1.001;

	float GetArea(FVector pt1, FVector pt2, FVector pt3);
};
