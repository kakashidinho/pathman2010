#ifndef _BOARD_
#define _BOARD_
#include "GameFlowControl.h"
class Board
{
private:
	int numRowTiles;//number of tiles in 1 row
	//list of tiles . 
	//If value of tiles[i][j] = 0 ->means no tile
	//							1 ->normal tile
	//							3 ->coin
	//							5 ->box
	char **tiles;
	unsigned int vBufferID;//id of vertex buffer for rendering board
	unsigned int iBufferID;//id of index buffer for rendering board
	unsigned int endIndex;//last index in index array buffer
	unsigned int textureID;//id of texture
	Material material;
	Vector3 topleftCorner;//position of top left corner
	float tileWidth;//size of 1 tile in world space
	void SetupVertexBuffer();
	void InitMaterial();
public:
	//create board object with  infos: 
	//<fileName> - name of file contains info about the grid 
	//<topleftCorner> - position of top left tile
	//<numRowTiles> - number of tiles per row
	//<boardWidth> - width of board in world space
	//<textureID> - id of texture
	Board(const char *fileName,const Vector3& topleftCorner,int numRowTiles,float boardWidth);
	Board(const unsigned char*byteStream,unsigned int streamSize,const Vector3& topleftCorner,int numRowTiles,float boardWidth);
	~Board();
	
	void InitGraphicsResource(unsigned int textureID);
	int GetTilesPerRow()const {return numRowTiles;}
	void GetCenterPoint(Vector3& centerPoint)const;//get position of center point in board
	char GetTileInfo(const Vector3 &pos,int &row,int &col)const;//get info of the tile where the point <pos> is standing at,also get the row and col index of that tile
	char GetTileInfo(int row,int col)const ;
	void  GetTileCenterPoint(int row,int col,Vector3& pos)const;//get position of center point of tile at <row> & <col> ,store in parameter <pos>
	bool IsBoxStandingAtTile(const AABB &box, int row,int col)const;
	void SetInfo(int row,int col,char info);//set <info> val to info value stored in tile [row][col] of board
	void Draw();
};

#endif