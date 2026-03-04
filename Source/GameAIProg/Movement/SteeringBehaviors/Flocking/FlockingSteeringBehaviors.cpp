#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering = {};
	FVector2D avgPos = pFlock->GetAverageNeighborPos();

	if (pFlock->GetNrOfNeighbors() > 0)
	{
		this->SetTarget(FTargetData(avgPos));
		return Seek::CalculateSteering(deltaT, pAgent);
	}

	return steering;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering{};
	FVector2D neighborCenter = pFlock->GetAverageNeighborPos();

	// Vector van buren naar agent (er vandaan)
	steering.LinearVelocity = pAgent.GetPosition() - neighborCenter;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent.max_LinearSpeed;

	return steering;
}
//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering{};
	steering.LinearVelocity = pFlock->GetAverageNeighborVelocity();

	return steering;
}