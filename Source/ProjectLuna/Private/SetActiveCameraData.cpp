#include "ProjectLuna.h"
#include "SetActiveCameraData.h"

SetActiveCameraData::SetActiveCameraData(ACameraActor *actor)
{
	m_camera = actor;
}

ACameraActor *SetActiveCameraData::getCamera()
{
	return m_camera;
}