/*

The buoyancy calculation technique was inspired from:
http://seawisphunter.com/blog/2015/11/24/simulating-buoyancy-part1/

Thanks to Shane Celis who assisted me generously by e-mail when I had questions

Twitter: @shanecelis

*/

#include "ProjectLuna.h"
#include "IceBlock.h"

DEFINE_LUNA_LOG(IceBlock);

AIceBlock::AIceBlock()
{
	PrimaryActorTick.bCanEverTick = false;
}

float AIceBlock::getWidth()
{
	return m_blockSize.X;
}