#include "ProjectLuna.h"
#include "OnWolfNearIceBlockData.h"

OnWolfNearIceBlockData::OnWolfNearIceBlockData(bool isEntering, FVector location)
{
	m_isEntering = isEntering;
	m_location = location;
}

bool OnWolfNearIceBlockData::getIsEntering()
{
	return m_isEntering;
}

FVector OnWolfNearIceBlockData::getLocation()
{
	return m_location;
}