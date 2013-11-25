#ifndef _AI_H_
#define _AI_H_
#include "Object.h"
#include "Board.h"
#include "../Util.h"

typedef unsigned int CostType;

struct Tile
{
	int row,col;
	bool operator == (const Tile& lhs)
	{
		return (row == lhs.row && col == lhs.col);
	} 
	bool operator !=(const Tile& lhs)
	{
		return (row != lhs.row || col != lhs.col);
	}
};

struct NextTile//next tile to move to
{
	Tile tile;//location of tile in board
	Vector3 position;//position in world space
	Direction direction;//direction to follow in order to reach this tile
};


class AIController
{
private:
	MovableObject *object;
	Tile currentTile;//current location of object in board
	Vector2	destination;//position of next tile's center point
	float rosSqr;//square of range of sight
	float posOffset;
	float GetDistanceSqr(const Vector3 &pos1,const Vector3 &pos2)const;
	void Chasing(const Board *board,const MovableObject *player);
	void Wandering(const Board *board);
	void RandomNextTile(const Board *board,NextTile & nextTile);
	CostType Cost(Tile &tile,Tile & destTile);//heuristic estimated cost for moving from this tile to destination tile
public:
	AIController(float objectVelocity,const Vector3 &objectPosition,float ros);
	~AIController();
	const MovableObject * GetControlledObject() {return object;}
	void Control(const Board *board,const MovableObject *player,bool playerInv);
};

#endif