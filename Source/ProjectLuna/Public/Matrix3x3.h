// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class Matrix3x3
{
public:
	static const Matrix3x3 identity;
	static const Matrix3x3 zero;
	static const Matrix3x3 swapXYTransform;
	static const Matrix3x3 swapXZTransform;
	static const Matrix3x3 swapYZTransform;

	static Matrix3x3 getFirstQuadrantTransform(const FVector &v);

	Matrix3x3();
	Matrix3x3(const FVector &v1, const FVector &v2, const FVector &v3);
	Matrix3x3(const float (&m)[3][3]);
	Matrix3x3(const float(&row1)[3], const float(&row2)[3], const float(&row3)[3]);

	FVector operator*(const FVector &v) const;
	Matrix3x3 operator*(const Matrix3x3 &other) const;
	Matrix3x3 operator+(const Matrix3x3 &other) const;
	Matrix3x3 operator*(float value) const;

	FVector operator[](int row) const;

	Matrix3x3 transpose() const;
	Matrix3x3 inverse() const;

private:
    float m_matrix[3][3];// = {0};
};