#include "SteeringBehaviors.h"
#include "DrawDebugHelpers.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
// TODO: Do the Week01 assignment :^)
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& agent) {
	SteeringOutput Steering = {};
	
	Steering.LinearVelocity = Target.Position - agent.GetPosition(); //Desired Velocity
	// Class: show students this below doesn't affect the movement result right now, but with combined steering will
	Steering.LinearVelocity.Normalize(); //Normalize Desired Velocity

	//Debug Rendering
	if (agent.GetDebugRenderingEnabled())
	{

		DrawDebugDirectionalArrow(agent.GetWorld(),
			FVector{ agent.GetPosition(),0 },
			FVector{ agent.GetPosition() + Steering.LinearVelocity,0 },
			1.f,
			FColor::Green
		);
	}

	return Steering;
}

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& agent)
{
	SteeringOutput Steering = {};
	Steering.LinearVelocity = agent.GetPosition() - Target.Position;
	Steering.LinearVelocity.Normalize();

	if (agent.GetDebugRenderingEnabled())
	{
		DrawDebugDirectionalArrow(agent.GetWorld(), FVector{ agent.GetPosition(),0 }, FVector{ agent.GetPosition() - Steering.LinearVelocity,0 }, 1.f, FColor::Green);
	}

	return Steering;
}

SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& agent)
{
	SteeringOutput Steering = {};

	FVector2D toTarget = Target.Position - agent.GetPosition();
	float distance = toTarget.Size();

	const float SlowRadius = 250.f;
	const float TargetRadius = 100.f;

	if (distance < TargetRadius) {
		Steering.LinearVelocity = FVector2D::ZeroVector;
	}
	else if (distance < SlowRadius) {
		float speed = agent.max_LinearSpeed * (distance / SlowRadius);
		Steering.LinearVelocity = toTarget.GetSafeNormal() * speed;
	}
	else {
		Steering.LinearVelocity = toTarget.GetSafeNormal() * agent.max_LinearSpeed;
	}
	return Steering;
}


SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& agent) {
	WanderAngle += ((float)rand() / RAND_MAX * 2.f - 1.f) * MaxAngleChange;

	FVector2D agentDirection = agent.GetLinearVelocity().GetSafeNormal();
	if (agentDirection.IsNearlyZero()) agentDirection = FVector2D(1, 0);

	FVector2D circleCenter = agent.GetPosition() + agentDirection * Offset;

	FVector2D offsetPos = FVector2D(cos(WanderAngle), sin(WanderAngle)) * Radius;
	Target.Position = circleCenter + offsetPos;

	if (agent.GetDebugRenderingEnabled()) {
		DrawDebugCircle(agent.GetWorld(), FVector(circleCenter, 0), Radius, 24, FColor::White, false, -1.f, 0, 1.f, FVector(0, 1, 0), FVector(1, 0, 0));
		DrawDebugLine(agent.GetWorld(), FVector(agent.GetPosition(), 0), FVector(circleCenter, 0), FColor::White);
		DrawDebugSphere(agent.GetWorld(), FVector(offsetPos, 0), 8.f, 8, FColor::Green);
	}

	return Seek::CalculateSteering(DeltaT, agent);
}

SteeringOutput Evade::CalculateSteering(float DeltaT, ASteeringAgent& agent)
{
	Pursuit::CalculateSteering(DeltaT, agent);
	SteeringOutput Steering = {};
	Steering.LinearVelocity = agent.GetPosition() - Target.Position;
	Steering.LinearVelocity.Normalize();
	Steering.LinearVelocity *= agent.max_LinearSpeed;

	if (agent.GetDebugRenderingEnabled())
	{
		DrawDebugSphere(agent.GetWorld(), FVector(Target.Position, 0), 10.f, 8, FColor::Red);
		DrawDebugLine(agent.GetWorld(), FVector(agent.GetPosition(), 0), FVector(Target.Position, 0), FColor::Red);
	}

	return Steering;
}

SteeringOutput Pursuit::CalculateSteering(float DeltaT, ASteeringAgent& agent)
{
	float distance = (Target.Position - agent.GetPosition()).Size();
	float predictionTime = distance / agent.max_LinearSpeed;
	FVector2D predictedPos = Target.Position + (Target.LinearVelocity * predictionTime);

	if (agent.GetDebugRenderingEnabled()) {
		DrawDebugLine(agent.GetWorld(), FVector(Target.Position, 0), FVector(predictedPos, 0), FColor::Cyan);
		DrawDebugSphere(agent.GetWorld(), FVector(predictedPos, 0), 10.f, 8, FColor::Cyan);
	}

	SteeringOutput Steering = {};
	Steering.LinearVelocity = (predictedPos - agent.GetPosition()).GetSafeNormal();
	return Steering;
}


