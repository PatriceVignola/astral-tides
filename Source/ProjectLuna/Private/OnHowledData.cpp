// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "OnHowledData.h"

OnHowledData::OnHowledData(float speed, float axisValue)
{
	m_speed = speed;
	m_axisValue = axisValue;
}

float OnHowledData::getSpeed()
{
	return m_speed;
}

float OnHowledData::getAxisValue()
{
	return m_axisValue;
}