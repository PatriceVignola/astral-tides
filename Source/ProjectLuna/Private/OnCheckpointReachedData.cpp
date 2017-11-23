// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "OnCheckpointReachedData.h"

OnCheckpointReachedData::OnCheckpointReachedData(FVector location, FRotator rotation)
{
	m_location = location;
	m_rotation = rotation;
}

FVector OnCheckpointReachedData::getLocation()
{
	return m_location;
}

FRotator OnCheckpointReachedData::getRotation()
{
	return m_rotation;
}