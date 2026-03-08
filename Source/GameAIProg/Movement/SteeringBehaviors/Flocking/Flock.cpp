#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{ pWorld }
	, FlockSize{ FlockSize }
	, pAgentToEvade{ pAgentToEvade }
{
	pSeparationBehavior = std::make_unique<Separation>(this);
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pVelMatchBehavior = std::make_unique<VelocityMatch>(this);
	pSeekBehavior = std::make_unique<Seek>();
	pWanderBehavior = std::make_unique<Wander>();
	pEvadeBehavior = std::make_unique<Evade>();

	pBlendedSteering = std::make_unique<BlendedSteering>(std::vector<BlendedSteering::WeightedBehavior>{
		{ pSeparationBehavior.get(), 0.5f },
		{ pCohesionBehavior.get(), 0.3f },
		{ pVelMatchBehavior.get(), 0.2f },
		{ pWanderBehavior.get(), 0.1f }
	});

	pPrioritySteering = std::make_unique<PrioritySteering>(std::vector<ISteeringBehavior*>{
		pEvadeBehavior.get(),
			pBlendedSteering.get()
	});

	Agents.SetNum(FlockSize);
	for (int i = 0; i < FlockSize; i++)
	{
		Agents[i] = pWorld->SpawnActor<ASteeringAgent>(AgentClass);

		if (Agents[i])
		{
			Agents[i]->SetSteeringBehavior(pPrioritySteering.get());

			FVector2D randomPos = { FMath::FRandRange(-WorldSize, WorldSize), FMath::FRandRange(-WorldSize, WorldSize) };
			Agents[i]->SetActorLocation(FVector(randomPos, 90.f)); 
		}
	}
}

Flock::~Flock()
{
 // TODO: Cleanup any additional data
	for (auto pAgent : Agents)
	{
		if (pAgent) pAgent->Destroy();
	}
}

void Flock::Tick(float DeltaTime)
{
 // TODO: update the flock
 // TODO: for every agent:
  // TODO: register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  // TODO: update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  // TODO: trim the agent to the world
	for (auto pAgent : Agents)
	{
		if (!pAgent) continue;

		RegisterNeighbors(pAgent);

		pAgent->Tick(DeltaTime);
	}
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
	if (bShowNeighborhood)
	{
		RenderNeighborhood();
	}

	for (auto pAgent : Agents)
	{
		if (pAgent && bShowRadius)
		{
			DrawDebugCircle(pWorld, pAgent->GetActorLocation(), 10.f, 12, FColor::Blue, false, -1.f, 0, 1.f, FVector(0, 1, 0), FVector(1, 0, 0));
		}
	}
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();

  // TODO: implement ImGUI checkboxes for debug rendering here
		ImGui::Checkbox("Show Neighborhood Radius", &bShowNeighborhood);
		ImGui::Checkbox("Show Agent Body", &bShowRadius);
		ImGui::SliderFloat("Neighborhood Radius", &NeighborhoodRadius, 50.f, 1000.f);

		
		ImGui::Spacing();

  // TODO: implement ImGUI sliders for steering behavior weights here
		ImGui::Text("Behavior Weights");
		for (auto& weightedBehavior : pBlendedSteering->GetWeightedBehaviorsRef())
		{
			FString label = FString::Printf(TEXT("Weight ##%p"), weightedBehavior.pBehavior);
			ImGui::SliderFloat(TCHAR_TO_ANSI(*label), &weightedBehavior.Weight, 0.f, 1.f);
		}
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
 // TODO: Debugrender the neighbors for the first agent in the flock
	if (Agents.Num() == 0 || !Agents[0])
		return;

	ASteeringAgent* pFirstAgent = Agents[0];
	FVector agentPos = pFirstAgent->GetActorLocation();

	DrawDebugCircle(pWorld, agentPos, NeighborhoodRadius, 36, FColor::White, false, -1.f, 0, 5.f, FVector(0, 1, 0), FVector(1, 0, 0));

	RegisterNeighbors(pFirstAgent);

	for (auto pNeighbor : Neighbors)
	{
		if (pNeighbor)
		{
			DrawDebugLine(pWorld, agentPos, pNeighbor->GetActorLocation(), FColor::Green, false, -1.f, 0, 2.f);
		}
	}
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
 // TODO: Implement
	Neighbors.Empty(); 
	NrOfNeighbors = 0;

	for (auto pOther : Agents)
	{
		if (pOther == pAgent) continue;

		float distSq = FVector2D::DistSquared(pAgent->GetPosition(), pOther->GetPosition());
		if (distSq < (NeighborhoodRadius * NeighborhoodRadius))
		{
			Neighbors.Add(pOther);
			NrOfNeighbors++;
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;

	FVector2D sumPos = FVector2D::ZeroVector;
	for (auto pNeighbor : Neighbors)
	{
		sumPos += pNeighbor->GetPosition();
	}
	return sumPos / (float)NrOfNeighbors;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;

	FVector2D sumVel = FVector2D::ZeroVector;
	for (auto pNeighbor : Neighbors)
	{
		sumVel += pNeighbor->GetLinearVelocity();
	}
	return sumVel / (float)NrOfNeighbors;;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	// TODO: Implement
	pSeekBehavior->SetTarget(Target);
}

