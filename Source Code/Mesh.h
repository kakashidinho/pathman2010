#ifndef _MESH_
#define _MESG_
#include "Renderer.h"
#include "Util.h"

/*----axis aligned bounding box-----*/
struct AABB
{
	Vector4 vMax, vMin;
	bool IntersectAABB(const AABB& box2);
};
/*----------------------------------*/
struct VertexGroup
{
	Material material;
	unsigned int textureID;
	unsigned int startIndex;//starting element in index array buffer
	unsigned int endIndex;//last element in index array buffer
};

struct Mesh
{
	Mesh()
	{
		vGroup = NULL;
		nGroup = 0;
	}
	~Mesh();
	VertexGroup *vGroup;//list of vertices groups ,vertices in group use the same texture and material
	unsigned int nGroup;//number of vertices groups
	unsigned int vBufferID;//id of vertex buffer 
	unsigned int iBufferID;//id of index buffer
	AABB box;

};
/*------------------------------------------------------
classes & structs for building mesh from .obj file
------------------------------------------------------*/
/*-------generic vertex data type -----------*/
struct Vertex
{
	Vector3 position;
	Vector2 texcoord;
	Vector3 normal;
};
/*-------------texture info----------------*/
struct TextureInfo
{
	char textureName[256];//name of file image that texture is loaded from
	unsigned int textureID;//id of texture in renderer's texture manager
};

/*--------material info---------------------*/
struct MaterialInfo
{
	char materialName[256];//name defined in mtl file
	TextureInfo *texInfo;//name of file image for texture 
	Material material;
	bool specular;
};
/*-------------vertices group info------------------*/
struct VertGroupInfo
{
	unsigned int startIndex;//starting element in indices list
	MaterialInfo *materialInfo;//pointer to material info 
	unsigned int endIndex;//last element in index array
};
/*----------mesh manager--------------------*/
class MeshManager:private ItemManager<Mesh>
{
private:
	Renderer *pRenderer;
	GrowableArray<Vertex> vertices;//this will be filled in vertex buffer for newly created mesh
	GrowableArray<unsigned short> indices;//this will be filled in index buffer for newly created mesh
	
	GrowableArray<std::list<unsigned short> *> pIndexTable;//array of indices list of vertices that have the same position ,for checking duplicated vertex
	std::list<TextureInfo> texInfos;//for preventing texture load multiple times from the same file
	std::list<MaterialInfo> materialInfos;//for rearrange indices purpose

	GrowableArray<VertGroupInfo> verGroupInfos;
	
	std::string currentFilePath;//for load obj file from harddisk
	PackageManager *packMan;//for load obj file from package
	unsigned int packageBufferID;//id of buffer that holds bytestream of package file
	
	Vector3 vMax,vMin;//for bulding aabb of newly created mesh

	void SortGroup();
	int LoadObjFromFile(const char* filename);
	int LoadMtlFromFile(std::string& filename);
	int LoadObjFromMem(const unsigned char* byteStream,unsigned int streamSize);
	int LoadMtlFromMem(const unsigned char* byteStream,unsigned int streamSize);
	int CreateNewMesh(unsigned int *pMeshID);
	unsigned short AddVertex(unsigned int positionIndex,Vertex &vertex);//add vertex to <vertices> list if the list hasn't it yet , return value is index in list of newly added vertex or index of existing vertex
	void ReleasePIndexTable();
public:
	MeshManager(Renderer *pRenderer);
	~MeshManager();
	int CreateMeshFromObjFile(const char *fileName,unsigned int *pMeshID);//read file obj & create mesh.This function can not be called outside renderer's thread
	int CreateMeshFromPackage(PackageManager *packMan,unsigned int packageBufferID,const char *objFileName,unsigned int *pMeshID);//read file obj in packageFile & create mesh.This function can not be called outside renderer's thread
	int ReadObjFile(const char *fileName);//read info from obj file,if successful,it will ready for create new mesh with CreateMesh function.This function can be called outside renderer's thread
	int ReadObjFromPackage(PackageManager *packMan,unsigned int packageBufferID,const char *objFileName);//read info from obj file in package,if successful,it will ready for create new mesh with CreateMesh function.This function can be called outside renderer's thread
	int CreateMesh(unsigned int *pMeshID);//create mesh afrer call to ReadObjFile() or ReadObjFromPackage()
	AABB * GetMeshAABB(unsigned int meshID);
	void ReleaseTextures();//release all texture resources that associated with meshes
	void RemoveMesh(unsigned int meshID);
	void RemoveAllMeshes();
	void DrawMesh(unsigned int meshID);//renderer need to be in 3D mode
};

#endif