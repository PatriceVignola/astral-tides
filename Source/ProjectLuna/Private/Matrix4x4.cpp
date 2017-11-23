// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "Matrix4x4.h"

Matrix4x4::Matrix4x4() {
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            m_matrix[i][j] = 0;
        }
    }
}

Matrix4x4::Matrix4x4(const FVector4 &v1, const FVector4 &v2, const FVector4 &v3, const FVector4 &v4)
{
	for (int j = 0; j < 4; j++)
	{
		m_matrix[0][j] = v1[j];
		m_matrix[1][j] = v2[j];
		m_matrix[2][j] = v3[j];
		m_matrix[3][j] = v4[j];
	}
}

Matrix4x4::Matrix4x4(const float(&m)[4][4])
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m_matrix[i][j] = m[i][j];
		}
	}
}

Matrix4x4::Matrix4x4(const float(&row1)[4], const float(&row2)[4], const float(&row3)[4], const float(&row4)[4])
{
	for (int j = 0; j < 3; j++)
	{
		m_matrix[0][j] = row1[j];
		m_matrix[1][j] = row2[j];
		m_matrix[2][j] = row3[j];
		m_matrix[3][j] = row4[j];
	}
}

FVector4 Matrix4x4::operator*(const FVector4 &v) const
{
	FVector4 result = FVector4(0, 0, 0, 0);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result[i] += m_matrix[i][j] * v[j];
		}
	}

	return result;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &other) const
{
    float result[4][4];
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            result[i][j] = 0;
        }
    }
    
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			result[i][j / 4] += m_matrix[i][j % 4] * other[j % 4][j / 4];
		}
	}

	return Matrix4x4(result);
}

FVector4 Matrix4x4::operator[](int row) const
{
	return FVector4(m_matrix[row][0], m_matrix[row][1], m_matrix[row][2], m_matrix[row][3]);
}

Matrix4x4 Matrix4x4::translationMatrix(const FVector &trans)
{
	float result[4][4] = { { 1, 0, 0, trans.X },{ 0, 1, 0, trans.Y },{ 0, 0, 1, trans.Z },{ 0, 0, 0, 1 } };

	return Matrix4x4(result);
}

Matrix4x4 Matrix4x4::scaleMatrix(const FVector &scale)
{
	float result[4][4] = { { scale.X, 0, 0, 0 },{ 0, scale.Y, 0, 0 },{ 0, 0, scale.Z, 0 },{ 0, 0, 0, 1 } };

	return Matrix4x4(result);
}

Matrix4x4 Matrix4x4::rotationMatrix(const FRotator &rot)
{
	// Since the x axis is reversed in Unreal and the rotation is clockwise, we have to use the inverse rotation for this axis
	float x = FMath::DegreesToRadians(rot.Roll);
	float cosX = FMath::Cos(x);
	float sinX = FMath::Sin(x);

	float xRot[4][4] = { { 1, 0, 0, 0 },{ 0, cosX, sinX, 0 },{ 0, -sinX, cosX, 0 },{ 0, 0, 0, 1 } };


	// Since the x axis is reversed in Unreal and the rotation is clockwise, we have to use the inverse rotation for this axis
	float y = FMath::DegreesToRadians(rot.Pitch);
	float cosY = FMath::Cos(y);
	float sinY = FMath::Sin(y);

	float yRot[4][4] = { { cosY, 0, -sinY, 0 },{ 0, 1, 0, 0 },{ sinY, 0, cosY, 0 },{ 0, 0, 0, 1 } };


	float z = FMath::DegreesToRadians(rot.Yaw);
	float cosZ = FMath::Cos(z);
	float sinZ = FMath::Sin(z);

	float zRot[4][4] = { { cosZ, -sinZ, 0, 0 },{ sinZ, cosZ, 0, 0 },{ 0, 0, 1, 0 },{ 0, 0, 0, 1 } };

	return Matrix4x4(zRot) * Matrix4x4(yRot) * Matrix4x4(xRot);
}

Matrix4x4 Matrix4x4::transpose() const
{
    float transposeMatrix[4][4];
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            transposeMatrix[i][j] = 0;
        }
    }

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			transposeMatrix[i][j] = m_matrix[j][i];
		}
	}

	return Matrix4x4(transposeMatrix);
}