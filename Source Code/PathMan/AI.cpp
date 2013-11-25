#include "AI.h"
#include <math.h>
#include <float.h>
#include <set>
#include <list>
#ifndef _DEBUG
#define NDEBUG 
#endif
#include <assert.h>
//node structure for path finding
struct Node
{
	Node *parent;
	Tile tile;
	CostType fcost;
	CostType gcost;
	CostType hcost;
};

struct NodeLess
{
	bool operator() (const Node* const a,const Node* const b)
	{
		return a->fcost < b->fcost;
	}
};

const Direction directions[4] = {LEFT,RIGHT,UP,DOWN};
const int offset[4][2] = {{0,-1},{0,1},{-1,0},{1,0}};

#define FLOAT_DELTA 0.00001f

/*-------AI controller----------------*/
AIController::AIController(float objectVelocity,const Vector3 &objectPosition,float ros)
{
	this->object = new MovableObject(objectVelocity,objectPosition);
	
	int index = rand()%4;
	this->object->Move(directions[index]);//random moving direction

	this->rosSqr = ros * ros;
	this->destination.x = objectPosition.x;
	this->destination.y = objectPosition.z;

	this->posOffset = 0.0f;
}
AIController::~AIController()
{
	delete object;
}
void AIController::Control(const Board *board,const MovableObject *player,bool playerInv)
{
	const Vector3 & pos1 = object->GetPosition();//ghost position in world space
	float velocity = object->GetVelocity();
	//get ghost location in board
	board->GetTileInfo(pos1,currentTile.row,currentTile.col);
	
	float diff;
	switch (object->GetDirection())
	{
	case LEFT:
		diff = pos1.x - destination.x;
		if(diff <= FLOAT_DELTA && diff >=- FLOAT_DELTA)
			break;
		if (diff > velocity + FLOAT_DELTA)
		{
			this->object->Update();
			return;
		}
		else
		{
			posOffset = velocity - diff;
			this->object->SetPosition(destination);
			return;
		}
		break;
	case RIGHT:
		diff = destination.x - pos1.x;
		if(diff <= FLOAT_DELTA && diff >=- FLOAT_DELTA)
			break;
		if (diff > velocity + FLOAT_DELTA)
		{
			this->object->Update();
			return;
		}
		else
		{
			posOffset = velocity - diff;
			this->object->SetPosition(destination);
			return;
		}
		break;
	case UP:
		diff = pos1.z - destination.y;
		if(diff <= FLOAT_DELTA && diff >=- FLOAT_DELTA)
			break;
		if (diff > velocity + FLOAT_DELTA)
		{
			this->object->Update();
			return;
		}
		else
		{
			posOffset = velocity - diff;
			this->object->SetPosition(destination);
			return;
		}
		break;
	case DOWN:
		diff = destination.y - pos1.z;
		if(diff <= FLOAT_DELTA && diff >=- FLOAT_DELTA)
			break;
		if (diff > velocity + FLOAT_DELTA)
		{
			this->object->Update();
			return;
		}
		else
		{
			posOffset = velocity - diff;
			this->object->SetPosition(destination);
			return;
		}
		break;
	}
	if(playerInv)//player is invisible
	{
		Wandering(board);
		return;
	}
	Vector2 distance ;//since the board gemeotry is mostly flat,we can use 2D vector
	float distSqr;

	const Vector3 & pos2 = player->GetPosition();
	
	distance.x = pos1.x - pos2.x;
	distance.y = pos1.z - pos2.z;

	distSqr = distance.LengthSqr();
	if (distSqr <= rosSqr)//player is in ghost's range of sight
		Chasing(board,player);
	else
	{
		Wandering(board);
	}
}

void AIController::Chasing(const Board *board,const MovableObject *player)
{
	NextTile* pNextTile = NULL;

	std::list<Node> nodes;
	
	std::multiset<Node*,NodeLess> open;
	std::list<Tile*> closed;
	std::multiset<Node*,NodeLess>::iterator openIte;
	std::list<Tile*>::iterator closeIte;

	Tile dest;//player's tile
	board->GetTileInfo(player->GetPosition(),dest.row,dest.col);
	
	//first node
	Node node;
	node.parent = NULL;
	node.tile = currentTile;
	node.fcost  = node.hcost = this->Cost(currentTile,dest);//heuristic estimated cost for moving from this tile to destination tile(player's tile)
	node.gcost = 0;
	nodes.push_back(node);
	open.insert(&(nodes.back()));

	while(!open.empty())
	{
		openIte = open.begin();//node with lowest fcost is at the beginning of open list
		Node *pNode = (*openIte);
		assert((unsigned int)pNode != 0xfeeefeee);
		if ((*openIte)->tile == dest)//this is goal tile
		{
			if(pNode->parent == NULL)//already in same tile with player
			{
				break;
			}
			pNextTile = new NextTile();
			while(pNode->parent->tile != currentTile)
			{
				pNode = pNode->parent;
			}
			pNextTile->tile = pNode->tile;
			board->GetTileCenterPoint(pNextTile->tile.row , pNextTile->tile.col ,pNextTile->position);
			if (pNextTile->tile.col > currentTile.col)
				pNextTile->direction = RIGHT;
			else if (pNextTile->tile.col < currentTile.col)
				pNextTile->direction = LEFT;
			else if (pNextTile->tile.row > currentTile.row)
				pNextTile->direction = DOWN;
			else
				pNextTile->direction = UP;
			break;
		}
		closed.push_back(&((*openIte)->tile));
		open.erase(openIte);

		Node checkingNode;
		char tileInfo;
		for(int i =0;i<4 ;++i)//check each adjacent tile
		{
			checkingNode.tile.row = pNode->tile.row + offset[i][0];
			checkingNode.tile.col = pNode->tile.col + offset[i][1];

			tileInfo = board->GetTileInfo(checkingNode.tile.row,checkingNode.tile.col);
			if(tileInfo != '0' && tileInfo!= '5')//this tile must doesn't have box
			{
				bool inClosed = false;//in closed list
				//this tile must not in closed list
				for(closeIte = closed.begin(); closeIte!= closed.end();++closeIte)
				{
					if (*(*closeIte) == checkingNode.tile)
					{
						inClosed = true;
						break;
					}
				}
				if (inClosed)
					continue;
				for(openIte = open.begin(); openIte!= open.end();++openIte)
				{
					if((*openIte)->tile == checkingNode.tile)
					{
						break;
					}
				}
				if (openIte == open.end())//this tile not in open list
				{
					checkingNode.hcost = this->Cost(checkingNode.tile,dest);//caculate heuristic cost
					checkingNode.gcost = (pNode->gcost + 1);
					checkingNode.fcost = checkingNode.gcost + checkingNode.hcost;
					checkingNode.parent = pNode;
					nodes.push_back(checkingNode);
					open.insert(&(nodes.back()));//move to open list
				}
				else if ((*openIte)->gcost > (pNode->gcost + 1))
				{
					(*openIte)->gcost = (pNode->gcost + 1);
					(*openIte)->fcost = (*openIte)->gcost + (*openIte)->hcost;
					(*openIte)->parent = pNode;
				}
			}
		}
	}

	if (pNextTile == NULL)//couldnt find path to player
		return this->Wandering(board);//wandering

	switch(pNextTile->direction)
	{
	case LEFT:
		this->object->SetPosition(destination.x - posOffset,destination.y);
		break;
	case RIGHT:
		this->object->SetPosition(destination.x + posOffset,destination.y);
		break;
	case UP:
		this->object->SetPosition(destination.x,destination.y - posOffset);
		break;
	case DOWN:
		this->object->SetPosition(destination.x ,destination.y + posOffset);
		break;
	}

	this->destination.x = pNextTile->position.x;
	this->destination.y = pNextTile->position.z;

	this->object->Move(pNextTile->direction);

	this->object->Update();

	delete pNextTile;
}
void AIController::Wandering(const Board *board)
{
	NextTile nextTile;
	this->RandomNextTile(board,nextTile);
	switch(nextTile.direction)
	{
	case LEFT:
		this->object->SetPosition(destination.x - posOffset,destination.y);
		break;
	case RIGHT:
		this->object->SetPosition(destination.x + posOffset,destination.y);
		break;
	case UP:
		this->object->SetPosition(destination.x,destination.y - posOffset);
		break;
	case DOWN:
		this->object->SetPosition(destination.x ,destination.y + posOffset);
		break;
	}

	this->destination.x = nextTile.position.x;
	this->destination.y = nextTile.position.z;

	this->object->Move(nextTile.direction);

	this->object->Update();
}

float AIController::GetDistanceSqr(const Vector3 &pos1,const Vector3 &pos2)const
{
	Vector2 distance ;//since the board gemeotry is mostly flat,we can use 2D vector
	distance.x = pos1.x - pos2.x;
	distance.y = pos1.z - pos2.z;

	return distance.LengthSqr();
}

void AIController::RandomNextTile(const Board *board,NextTile & nextTile)
{
	bool checked[4] = {false,false,false,false};
	int index;
	char tileInfo;
	do
	{
		do{
			index = rand() % 4;
		}while(checked[index]);

		checked[index] = true;
		nextTile.tile.row = currentTile.row + offset[index][0];
		nextTile.tile.col = currentTile.col + offset[index][1];
		tileInfo = board->GetTileInfo(nextTile.tile.row,nextTile.tile.col);
	}while( tileInfo == '0' || tileInfo == '5');

	nextTile.direction = directions[index];

	board->GetTileCenterPoint(nextTile.tile.row,nextTile.tile.col,nextTile.position);

}
//heuristic estimated cost for moving from this tile to destination tile
CostType AIController::Cost(Tile &tile,Tile & destTile)
{
	return (abs(destTile.row - tile.row) + abs(destTile.col - tile.col));
}