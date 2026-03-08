#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	// TODO create the cells
	for (int r = 0; r < Rows; ++r)
	{
		for (int c = 0; c < Cols; ++c)
		{
			float left = c * CellWidth;
			float bottom = r * CellHeight;
			Cells.push_back(Cell(left, bottom, CellWidth, CellHeight));
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	// TODO Add the agent to the correct cell
	int index = PositionToIndex(Agent.GetPosition());
	Cells[index].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	//TODO Check if the agent needs to be moved to another cell.
	//TODO Use the calculated index for oldPos and currentPos for this
	int oldIdx = PositionToIndex(OldPos);
	int newIdx = PositionToIndex(Agent.GetPosition());

	if (oldIdx != newIdx)
	{
		Cells[oldIdx].Agents.remove(&Agent);
		Cells[newIdx].Agents.push_back(&Agent);
	}
}


void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	NrOfNeighbors = 0;
	FRect queryBox;
	queryBox.Min = Agent.GetPosition() - FVector2D(QueryRadius, QueryRadius);
	queryBox.Max = Agent.GetPosition() + FVector2D(QueryRadius, QueryRadius);

	for (const Cell& cell : Cells)
	{
		if (DoRectsOverlap(cell.BoundingBox, queryBox))
		{
			for (ASteeringAgent* pOtherAgent : cell.Agents)
			{
				if (pOtherAgent == &Agent)
					continue;

				float distSq = FVector2D::DistSquared(Agent.GetPosition(), pOtherAgent->GetPosition());
				if (distSq < (QueryRadius * QueryRadius))
				{
					if (NrOfNeighbors < Neighbors.Num())
					{
						Neighbors[NrOfNeighbors] = pOtherAgent;
						NrOfNeighbors++;
					}
				}
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	// TODO Render the cells with the number of agents inside of it
	if (!pWorld) return;

	for (const Cell& cell : Cells)
	{
		TArray<FVector> points;
		auto rectPoints = cell.GetRectPoints();

		for (const auto& pt : rectPoints)
			points.Add(FVector(pt.X, pt.Y, 0.f));

		for (int i = 0; i < 4; ++i)
		{
			DrawDebugLine(pWorld, points[i], points[(i + 1) % 4], FColor::Cyan, false, -1.f, 0, 2.f);
		}

		FVector cellCenter = FVector(cell.BoundingBox.Min.X + (CellWidth * 0.5f),
			cell.BoundingBox.Min.Y + (CellHeight * 0.5f),
			10.f);

		FString agentCount = FString::FromInt(cell.Agents.size());
		DrawDebugString(pWorld, cellCenter, agentCount, nullptr, FColor::White, 0.01f, false);
	}
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	// TODO Calculate the index of the cell based on the position
	int col = static_cast<int>(Pos.X / CellWidth);
	int row = static_cast<int>(Pos.Y / CellHeight);

	if (col < 0) col = 0; if (col >= NrOfCols) col = NrOfCols - 1;
	if (row < 0) row = 0; if (row >= NrOfRows) row = NrOfRows - 1;

	return col + (row * NrOfCols);
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}