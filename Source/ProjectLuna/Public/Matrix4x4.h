// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
class PROJECTLUNA_API Matrix4x4
{
public:
	Matrix4x4();
	Matrix4x4(const FVector4 &v1, const FVector4 &v2, const FVector4 &v3, const FVector4 &v4);
	Matrix4x4(const float(&m)[4][4]);
	Matrix4x4(const float(&row1)[4], const float(&row2)[4], const float(&row3)[4], const float(&row4)[4]);

	//float determinant() const;

	FVector4 operator*(const FVector4 &v) const;
	Matrix4x4 operator*(const Matrix4x4 &other) const;
	FVector4 operator[](int row) const;

	static Matrix4x4 translationMatrix(const FVector &trans);
	static Matrix4x4 scaleMatrix(const FVector &scale);
	static Matrix4x4 rotationMatrix(const FRotator &rot);
	Matrix4x4 transpose() const;
    
private:
    float m_matrix[4][4];// = { 0 };
};
