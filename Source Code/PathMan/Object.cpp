#include "Object.h"
#include <math.h>
MovableObject::MovableObject(float velocity, const Vector3 &position)
{
	this->velocity = velocity;
	this->position = position;
	this->direction = RIGHT;
	this->moving = false;
}

void MovableObject::Update()
{
	switch(this->direction)
	{
	case DOWN:
		if(moving)
		{
			this->position.z += this->velocity;
		}
		break;
	case UP:
		if(moving)
		{
			this->position.z -= this->velocity;
		}
		break;
	case RIGHT://default direction
		if(moving)
		{
			this->position.x += this->velocity;
		}
		break;
	case LEFT:
		if(moving)
		{
			this->position.x -= this->velocity;
		}
		break;
	}
	
	Matrix4Translate(position.x,position.y,position.z,&translation);

	this->transform = rotation * translation;
}
const AABB& MovableObject::GetUntranslateBoundingBox()
{
	if(direction == LEFT || direction == RIGHT)
		return this->horizonBox;
	return this->verticalBox;
}
void MovableObject::MoveRight()//default direction
{
	moving = true;
	if(this->direction != RIGHT)
	{
		direction = RIGHT;
		rotation.Identity();
	}
}
void MovableObject::MoveLeft()
{
	moving = true;
	if(this->direction != LEFT)
	{
		direction = LEFT;
		Matrix4RotateY(_PI,&rotation);
	}
}
void MovableObject::MoveUp()
{
	moving = true;
	if(this->direction != UP)
	{
		direction = UP;
		Matrix4RotateY(_PIOVER2,&rotation);
	}
}
void MovableObject::MoveDown()
{
	moving = true;
	if(this->direction != DOWN)
	{
		direction = DOWN;
		Matrix4RotateY(-_PIOVER2,&rotation);
	}
}

void MovableObject::Move(Direction direction)
{
	switch(direction)
	{
	case LEFT:
		MoveLeft();
		break;
	case RIGHT:
		MoveRight();
		break;
	case UP:
		MoveUp();
		break;
	case DOWN:
		MoveDown();
		break;
	}
}

void MovableObject::StopMoving()
{
	moving = false;
}

void MovableObject::SetPosition(const Vector3 &position)
{
	this->position = position;
}

void MovableObject::SetPosition(const Vector2 &position)
{
	this->position.x = position.x;
	this->position.z = position.y;
}

void MovableObject::SetPosition(float x,float z)
{
	this->position.x = x;
	this->position.z = z;
}

void MovableObject::ComputeObjectAABB(const AABB& meshAABB)
{
	this->horizonBox = meshAABB;
	this->verticalBox.vMax.x = this->horizonBox.vMax.z;
	this->verticalBox.vMax.z = this->horizonBox.vMax.x;
	this->verticalBox.vMax.w = 1.0f;
	this->verticalBox.vMin.x = this->horizonBox.vMin.z;
	this->verticalBox.vMin.z = this->horizonBox.vMin.x;
	this->verticalBox.vMin.w = 1.0f;
}

bool Collision(const MovableObject &obj1,const MovableObject &obj2,const Board &board)
{
	int row1,col1,row2,col2;
	board.GetTileInfo(obj1.position,row1,col1);
	board.GetTileInfo(obj2.position,row2,col2);

	if(row1==row2 && col1 == col2)
		return true;
	return false;
}
