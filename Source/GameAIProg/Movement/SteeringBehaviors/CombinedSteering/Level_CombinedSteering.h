#pragma once

#include "CoreMinimal.h"
#include "GameAIProg/Shared/Level_Base.h"
#include "CombinedSteeringBehaviors.h"
#include <memory>
#include "Level_CombinedSteering.generated.h"

// Forward declarations
class ASteeringAgent;
class Seek;
class Wander;
class Evade;

UCLASS()
class GAMEAIPROG_API ALevel_CombinedSteering : public ALevel_Base
{
	GENERATED_BODY()

public:
	ALevel_CombinedSteering();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy();

private:
	// Agents
	ASteeringAgent* pSeekerAgent = nullptr;
	ASteeringAgent* pWandererAgent = nullptr;

	// Behaviors
	std::unique_ptr<Seek> pSeekBehavior;
	std::unique_ptr<Wander> pWanderBehavior;
	std::unique_ptr<Evade> pEvadeBehavior;
	std::unique_ptr<BlendedSteering> pBlendedSteering;
	std::unique_ptr<PrioritySteering> pPrioritySteering;

	bool CanDebugRender = false;
};