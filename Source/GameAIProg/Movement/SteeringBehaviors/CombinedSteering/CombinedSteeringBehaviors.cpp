
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput BlendedSteering = {};
	float totalWeight = 0.f;

	for (const auto& weightedBehavior : WeightedBehaviors)
	{
		auto output = weightedBehavior.pBehavior->CalculateSteering(DeltaT, Agent);
		if (output.IsValid)
		{
			BlendedSteering.LinearVelocity += output.LinearVelocity * weightedBehavior.Weight;
			BlendedSteering.AngularVelocity += output.AngularVelocity * weightedBehavior.Weight;
			totalWeight += weightedBehavior.Weight;
		}
	}

	if (totalWeight > 0.f)
	{
		// Normaliseer het resultaat als het gewicht groter is dan 1
		float scale = 1.f / totalWeight;
		BlendedSteering.LinearVelocity *= scale;
	}

	return BlendedSteering;
}

float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
	auto it = find_if(WeightedBehaviors.begin(),
		WeightedBehaviors.end(),
		[SteeringBehavior](const WeightedBehavior& Elem)
		{
			return Elem.pBehavior == SteeringBehavior;
		}
	);

	if(it!= WeightedBehaviors.end())
		return &it->Weight;
	
	return nullptr;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return Steering;
}