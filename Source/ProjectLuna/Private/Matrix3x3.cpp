// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "Matrix3x3.h"

const Matrix3x3 Matrix3x3::identity = Matrix3x3({ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 } });
const Matrix3x3 Matrix3x3::zero = Matrix3x3({ {0, 0, 0},{ 0, 0, 0 },{ 0, 0, 0 } });
const Matrix3x3 Matrix3x3::swapXYTransform = Matrix3x3({ { 0, 1, 0 },{ 1, 0, 0 },{ 0, 0, 1 } });
const Matrix3x3 Matrix3x3::swapXZTransform = Matrix3x3({ { 0, 0, 1 },{ 0, 1, 0 },{ 1, 0, 0 } });
const Matrix3x3 Matrix3x3::swapYZTransform = Matrix3x3({ { 1, 0, 0 },{ 0, 0, 1 },{ 0, 1, 0 } });

Matrix3x3::Matrix3x3() {
    for(int i = 0; i < 3; ++i) {
        for(int j = 0; j < 3; ++j) {
            m_matrix[i][j] = 0;
        }
    }
}

Matrix3x3::Matrix3x3(const FVector &v1, const FVector &v2, const FVector &v3)
{
	for (int j = 0; j < 3; j++)
	{
		m_matrix[0][j] = v1[j];
		m_matrix[1][j] = v2[j];
		m_matrix[2][j] = v3[j];
	}
}

Matrix3x3::Matrix3x3(const float(&m)[3][3])
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m_matrix[i][j] = m[i][j];
		}
	}
}

Matrix3x3::Matrix3x3(const float (&row1)[3], const float (&row2)[3], const float (&row3)[3])
{
	for (int j = 0; j < 3; j++)
	{
		m_matrix[0][j] = row1[j];
		m_matrix[1][j] = row2[j];
		m_matrix[2][j] = row3[j];
	}
}

Matrix3x3 Matrix3x3::getFirstQuadrantTransform(const FVector &v)
{
    float transformArray[3][3];
    for(int i = 0; i < 3; ++i) {
        for(int j = 0; j < 3; ++j) {
            transformArray[i][j] = 0;
        }
    }
    
	for (int i = 0; i < 3; i++)
	{
		transformArray[i][i] = FMath::Sign(v[i]);
	}

	return Matrix3x3(transformArray);
}

FVector Matrix3x3::operator*(const FVector &v) const
{
	FVector result = FVector::ZeroVector;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			result[i] += m_matrix[i][j] * v[j];
		}
	}

	return result;
}

Matrix3x3 Matrix3x3::operator*(const Matrix3x3 &other) const
{
    float result[3][3];
    for(int i = 0; i < 3; ++i) {
        for(int j = 0; j < 3; ++j) {
            result[i][j] = 0;
        }
    }

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			result[i][j / 3] += m_matrix[i][j % 3] * other[j % 3][j / 3];
		}
	}

	return Matrix3x3(result);
}

Matrix3x3 Matrix3x3::operator+(const Matrix3x3 &other) const
{
    float result[3][3];
    for(int i = 0; i < 3; ++i) {
        for(int j = 0; j < 3; ++j) {
            result[i][j] = 0;
        }
    }

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			result[i][j] = m_matrix[i][j] + other[i][j];
		}
	}

	return Matrix3x3(result);
}

Matrix3x3 Matrix3x3::operator*(float value) const
{
    float result[3][3];
    for(int i = 0; i < 3; ++i) {
        for(int j = 0; j < 3; ++j) {
            result[i][j] = 0;
        }
    }

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			result[i][j] = m_matrix[i][j] * value;
		}
	}

	return Matrix3x3(result);
}

FVector Matrix3x3::operator[](int row) const
{
	return FVector(m_matrix[row][0], m_matrix[row][1], m_matrix[row][2]);
}

Matrix3x3 Matrix3x3::transpose() const
{
    float transposeMatrix[3][3];
    for(int i = 0; i < 3; ++i) {
        for(int j = 0; j < 3; ++j) {
            transposeMatrix[i][j] = 0;
        }
    }

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			transposeMatrix[i][j] = m_matrix[j][i];
		}
	}

	return Matrix3x3(transposeMatrix);
}

Matrix3x3 Matrix3x3::inverse() const
{
	/*
	    |a b c|
	M = |d e f|
	    |g h i|

	A = (ei - fh)
	B = -(di - fg)
	C = (dh - eg)
	D = -(bi - ch)
	E = (ai - cg)
	F = -(ah - bg)
	G = (bf - ce)
	H = -(af - cd)
	I = (ae - bd)
	*/

	// Not the most performant way to do an inverse, but it's the fastest to code

	FVector x0 = FVector(m_matrix[0][0], m_matrix[1][0], m_matrix[2][0]);
	FVector x1 = FVector(m_matrix[0][1], m_matrix[1][1], m_matrix[2][1]);
	FVector x2 = FVector(m_matrix[0][2], m_matrix[1][2], m_matrix[2][2]);

	float detInverse = (FVector::DotProduct(x0, FVector::CrossProduct(x1, x2)));

	FVector x0Transpose = detInverse * FVector::CrossProduct(x1, x2);
	FVector x1Transpose = detInverse * FVector::CrossProduct(x2, x0);
	FVector x2Transpose = detInverse * FVector::CrossProduct(x0, x1);

	return Matrix3x3(x0Transpose, x1Transpose, x2Transpose);
}