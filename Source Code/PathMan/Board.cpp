#include "Board.h"
#include <math.h>
Board::~Board()
{
	if(tiles)
	{
		for(int i = 0;i<this->numRowTiles;++i)
		{
			delete[] tiles[i];
		}
		delete[] tiles;
	}
	pRenderer->ReleaseStaticVertexBuffer(this->vBufferID);
	pRenderer->ReleaseStaticIndexBuffer(this->iBufferID);
}

Board::Board(const char *fileName,const Vector3& topleftCorner,int numRowTiles,float boardWidth)
{
	tiles = NULL;
	FILE *f = fopen(fileName,"r");
	if(!f)
		return;
	this->numRowTiles = numRowTiles;
	this->topleftCorner = topleftCorner;
	
	this->tileWidth = boardWidth / numRowTiles;
	
	fscanf(f,"//board grid 0=no tile 1=tile 2=mc 3=coin 4=ghost 5=box\n");

	//setup tiles
	tiles = new char*[numRowTiles];
	for(int i = 0;i<this->numRowTiles;++i)
	{
		tiles[i] = new char[numRowTiles];
		fread(tiles[i],1,numRowTiles,f);
		fscanf(f,"\n");
	}
	fclose(f);
}

Board::Board(const unsigned char*byteStream,unsigned int streamSize,const Vector3& topleftCorner,int numRowTiles,float boardWidth)
{
	tiles = NULL;
	if(!byteStream)
		return;
	this->numRowTiles = numRowTiles;
	this->topleftCorner = topleftCorner;
	
	this->tileWidth = boardWidth / numRowTiles;
	CStringStream stream((char*)byteStream,streamSize);
	
	//setup tiles
	char line[100];
	stream.GetLine(line,100);//nothing to do with this line

	tiles = new char*[numRowTiles];
	for(int i = 0;i<this->numRowTiles;++i)
	{
		tiles[i] = new char[numRowTiles];
		stream.GetLine(line,100);
		strncpy(tiles[i],line,numRowTiles);
	}
}


void Board::SetupVertexBuffer()
{
	GrowableArray<unsigned short> indices;
	Vertex *vertices = new Vertex[(numRowTiles + 1) * (numRowTiles + 1)];
	float du = 1.0f;

	for(int i = 0;i<this->numRowTiles + 1 ;++i)//each row of tiles
	{
		int j=0;
		for(;j < this->numRowTiles + 1 ;++j)//each col of tiles
		{
			unsigned short index = i * (numRowTiles + 1) + j;
			vertices[index].normal = Vector3(0,1,0);
			vertices[index].position.z = this->topleftCorner.z + i*tileWidth;
			vertices[index].position.x = this->topleftCorner.x + j*tileWidth;
			vertices[index].texcoord = Vector2(du*j,du *i);

			if(this->GetTileInfo(i,j)!='0')//tile is visibles
			{
				//indices of two triangles for drawing this tile
				indices.Add( index);
				indices.Add( index + numRowTiles + 1);
				indices.Add( index + numRowTiles + 2);

				indices.Add(index + numRowTiles + 2);
				indices.Add(index + 1);
				indices.Add(index);
			}

		}


	}


	pRenderer->CreateStaticVertexBuffer(vertices,(numRowTiles + 1) * (numRowTiles + 1),
										VT_P3_T2_N3,&this->vBufferID);

	pRenderer->CreateStaticIndexBuffer(indices,indices.Size(),&this->iBufferID);
	
	this->endIndex = indices.Size() - 1;

	delete []vertices;
}

char Board::GetTileInfo(const Vector3 &pos,int &row,int &col)const
{
	Vector3 dv = pos - this->topleftCorner;
	row = floor(dv.z / tileWidth);
	col = floor(dv.x / tileWidth);
	if(row >= this->numRowTiles || row < 0 ||
		col >= this->numRowTiles || col < 0 ||
		tiles == NULL)
		return '0';
	else return tiles[row][col];
}

char Board::GetTileInfo(int row,int col)const
{
	if(row >= this->numRowTiles || row < 0 ||
		col >= this->numRowTiles || col < 0 ||
		tiles == NULL)
		return '0';
	else return tiles[row][col];

}
void Board::GetCenterPoint(Vector3& centerPoint)const
{
	centerPoint.y = this->topleftCorner.y;
	centerPoint.x = this->topleftCorner.x + this->numRowTiles/2.0f * this->tileWidth;
	centerPoint.z = this->topleftCorner.z + this->numRowTiles/2.0f * this->tileWidth;
}
void Board::GetTileCenterPoint(int row,int col,Vector3& pos)const
{
	if(row >= this->numRowTiles || row < 0 ||
		col >= this->numRowTiles || col < 0 ||
		tiles == NULL)
		return;
	
	pos.y = this->topleftCorner.y;
	pos.x = this->topleftCorner.x + (col + 0.5f ) * this->tileWidth;
	pos.z = this->topleftCorner.z + (row + 0.5f ) * this->tileWidth;
}
void Board::Draw()
{
	pRenderer->SetStaticBuffer(this->vBufferID,this->iBufferID);
	pRenderer->SetTexture(this->textureID);
	pRenderer->SetMaterial(material);
	pRenderer->DrawObject2(0,this->endIndex);
}

void Board::InitMaterial()
{
	material.ambient.r = material.ambient.g = material.ambient.b = material.ambient.a = 1.0f;
	material.diffuse.r = material.diffuse.g = material.diffuse.b = material.diffuse.a = 1.0f;
	material.specular.r = material.specular.g = material.specular.b = material.specular.a = 1.0f;
	material.shininess = 1.0f;
}

void Board::InitGraphicsResource(unsigned int textureID)
{
	if(!tiles)
		return;
	this->InitMaterial();
	this->SetupVertexBuffer();
	this->textureID = textureID;
}

bool Board::IsBoxStandingAtTile(const AABB &box, int row,int col)const
{
	if(row >= this->numRowTiles || row < 0 ||
		col >= this->numRowTiles || col < 0 ||
		tiles == NULL)
		return false;
	float minX = this->topleftCorner.x + this->tileWidth*col;
	float minZ = this->topleftCorner.z + this->tileWidth*row;
	float maxX = minX + this->tileWidth;
	float maxZ = minZ + this->tileWidth;

	if(box.vMax.x < minX || maxX <box.vMin.x || 
		box.vMax.z < minZ || maxZ <box.vMin.z )
		return false;

	return true;
}
void Board::SetInfo(int row,int col,char info)
{
	if(row >= this->numRowTiles || row < 0 ||
		col >= this->numRowTiles || col < 0 ||
		tiles == NULL)
		return ;

	tiles[row][col] = info;
}