// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EventData.h"
#include "Runtime/Engine/Classes/Camera/CameraActor.h"

class SetActiveCameraData : public EventData
{
public:
	SetActiveCameraData(ACameraActor *actor);
	ACameraActor *getCamera();

private:
	ACameraActor *m_camera;
};
