#ifndef _OBJECT_
#define _OBJECT_
#include "GameFlowControl.h"
#include "Board.h"
enum Direction
{
	LEFT = 0,
	RIGHT =  1,
	DOWN = 2,
	UP  = 3
};

class MovableObject
{
private:
	Matrix4x4 transform;//transformation matrix
	Matrix4x4 translation;//translation matrix
	Matrix4x4 rotation;//rotation matrix
	AABB horizonBox;
	AABB verticalBox;
	Vector3 position;
	Direction direction;
	float velocity;
	bool moving;
public:
	MovableObject(float velocity,const Vector3 &position);
	void Update();
	void MoveRight();
	void MoveLeft();
	void MoveUp();
	void MoveDown();
	void Move(Direction direction);
	void StopMoving();
	bool IsMoving() {return moving;}
	Direction GetDirection() {return direction;}
	float GetVelocity() {return velocity;}
	const Matrix4x4& GetTransformMatrix()const {return transform;}
	const AABB& GetUntranslateBoundingBox();
	const Vector3 &GetPosition()const {return position;}
	void SetPosition(const Vector3 &position);
	void SetPosition(const Vector2 &position);
	void SetPosition(float x,float z);
	void ComputeObjectAABB(const AABB& meshAABB);//compute object's AABB from a mesh's AABB
	friend bool Collision(const MovableObject &obj1,const MovableObject &obj2,const Board &board);
};

struct StaticObject
{
	int row,col;
	Matrix4x4 translation;
};

#endif