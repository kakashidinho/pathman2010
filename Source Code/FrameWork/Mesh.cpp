#include "stdafx.h"
#include "../Mesh.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;
/*----axis aligned bounding box-----*/
bool AABB::IntersectAABB(const AABB& aabb)
{
	if(vMin.x>aabb.vMax.x||vMax.x<aabb.vMin.x)
		return false;
	if(vMin.y>aabb.vMax.y||vMax.y<aabb.vMin.y)
		return false;
	if(vMin.z>aabb.vMax.z||vMax.z<aabb.vMin.z)
		return false;
	return true;
}
/*-------------helper function--------------------*/
void DefaultMaterial(Material & mat)
{
	mat.ambient.r = mat.ambient.g = mat.ambient.b = mat.ambient.a = 1.0f;
	mat.diffuse.r = mat.diffuse.g = mat.diffuse.b = mat.diffuse.a = 1.0f;
	mat.specular.r = mat.specular.g = mat.specular.b = mat.specular.a = 1.0f;
	mat.shininess = 1.0f;
}
/*-------------Mesh-------------------------------*/
Mesh::~Mesh()
{
	SafeDeleteArray(vGroup);
}
/*-------------MeshManager------------------------*/
MeshManager::MeshManager(Renderer *pRenderer)
{
	this->pRenderer = pRenderer;currentFilePath = "";
}
MeshManager::~MeshManager()
{
	this->ReleasePIndexTable();
}

int MeshManager::CreateMeshFromObjFile(const char *file,unsigned int *pID)
{
	this->packMan = NULL; // not load it from package

	int lastIndex = strrchr(file,'/') - file;
	this->currentFilePath.assign(file,lastIndex + 1);

	if(this->LoadObjFromFile(file)==0)//read info from file
		return 0;
	this->SortGroup();
	/*---------create mesh-------------*/
	
	if(this->CreateNewMesh(pID) == 0)
		return 0;
	
	/*---------------------------------*/
	this->verGroupInfos.Clear();
	this->materialInfos.clear();
	this->vertices.Clear();
	this->indices.Clear();

	return 1;
}

int MeshManager::CreateMeshFromPackage(PackageManager *packMan,unsigned int packageBufferID,const char *objFileName,unsigned int *pID)
{
	if(!packMan)
		return 0;

	this->packMan = packMan;
	this->packageBufferID = packageBufferID;
	unsigned char *byteStream;
	unsigned int streamSize;

	byteStream = packMan->GetSubByteStream(this->packageBufferID ,objFileName, &streamSize);
	
	if(byteStream == NULL || streamSize == 0)
		return 0;

	if(this->LoadObjFromMem(byteStream,streamSize)==0)//read info from file
		return 0;
	this->SortGroup();
	/*---------create mesh-------------*/
	
	if(this->CreateNewMesh(pID) == 0)
		return 0;
	
	/*---------------------------------*/
	this->verGroupInfos.Clear();
	this->materialInfos.clear();
	this->vertices.Clear();
	this->indices.Clear();

	return 1;
}

int MeshManager::ReadObjFile(const char *fileName)
{
	this->packMan = NULL; // not load it from package

	int lastIndex = strrchr(fileName,'/') - fileName;
	this->currentFilePath.assign(fileName,lastIndex + 1);

	if(this->LoadObjFromFile(fileName)==0)//read info from file
		return 0;
	this->SortGroup();

	return 1;
}
int MeshManager::ReadObjFromPackage(PackageManager *packMan,unsigned int packageBufferID,const char *objFileName)
{
	if(!packMan)
		return 0;

	this->packMan = packMan;
	this->packageBufferID = packageBufferID;
	unsigned char *byteStream;
	unsigned int streamSize;

	byteStream = packMan->GetSubByteStream(this->packageBufferID ,objFileName, &streamSize);
	
	if(byteStream == NULL || streamSize == 0)
		return 0;

	if(this->LoadObjFromMem(byteStream,streamSize) == 0)
		return 0;
	this->SortGroup();
	return 1;
}
int MeshManager::CreateMesh(unsigned int *pMeshID)
{
	if(this->vertices.Size() == 0)
		return 0;
	if(this->CreateNewMesh(pMeshID) == 0)
		return 0;

	this->verGroupInfos.Clear();
	this->materialInfos.clear();
	this->vertices.Clear();
	this->indices.Clear();

	return 1;
}
int MeshManager::CreateNewMesh(unsigned int *pMeshID)
{
	//load all texture
	if(this->packMan == NULL)//not load from package
	{
		for(std::list<TextureInfo>::iterator i = texInfos.begin();
			i !=this->texInfos.end() ;++i)
		{
			string file = this->currentFilePath + i->textureName;
			if(pRenderer->LoadTextureFromFile(file.c_str(),&i->textureID)!=R_OK)
				return 0;
		}
	}
	else
	{
		unsigned char *byteStream;unsigned int streamSize;
		for(std::list<TextureInfo>::iterator i = texInfos.begin();
			i !=this->texInfos.end() ;++i)
		{
			byteStream = packMan->GetSubByteStream(this->packageBufferID,i->textureName,&streamSize);
			if(byteStream == NULL || streamSize == 0 ||
				pRenderer->LoadTextureFromMemory(byteStream,streamSize,&i->textureID)!=R_OK)
				return 0;
		}
	}

	Mesh *pMesh = new Mesh();
	
	pRenderer->CreateStaticVertexBuffer(this->vertices,this->vertices.Size(),
										VT_P3_T2_N3,&pMesh->vBufferID);
	pRenderer->CreateStaticIndexBuffer(this->indices,this->indices.Size(),
										&pMesh->iBufferID);

	if(this->verGroupInfos.Size())//multiple groups
	{
		pMesh->nGroup = this->verGroupInfos.Size();
		pMesh->vGroup = new VertexGroup[pMesh->nGroup];
		for(unsigned int i = 0;i < pMesh->nGroup;++i)
		{
			if(this->verGroupInfos[i].materialInfo)
			{
				pMesh->vGroup[i].material = this->verGroupInfos[i].materialInfo->material;
				if(this->verGroupInfos[i].materialInfo->texInfo)
					pMesh->vGroup[i].textureID = this->verGroupInfos[i].materialInfo->texInfo->textureID;
				else 
					pMesh->vGroup[i].textureID = 0;
			}
			else 
			{
				DefaultMaterial(pMesh->vGroup[i].material);
				pMesh->vGroup[i].textureID = 0;
				
			}
			if(i==this->verGroupInfos.Size() - 1)
				pMesh->vGroup[i].endIndex = this->indices.Size() - 1;
			else
				pMesh->vGroup[i].endIndex = this->verGroupInfos[i].endIndex;
			pMesh->vGroup[i].startIndex = this->verGroupInfos[i].startIndex;
			
		}
	}
	else//single group
	{
		pMesh->nGroup = 1;
		pMesh->vGroup = new VertexGroup[1];
		
	
		DefaultMaterial(pMesh->vGroup->material);
		pMesh->vGroup->textureID = 0;
				
		pMesh->vGroup->endIndex = 0;
		pMesh->vGroup->startIndex = this->indices.Size() - 1;
			
	}
	pMesh->box.vMax.Set(vMax,1.0f);
	pMesh->box.vMin.Set(vMin,1.0f);
	if(!this->AddItem(pMesh,pMeshID))
		return 0;

	return 1;
}
int MeshManager::LoadObjFromFile(const char* filename)
{
	vMax.Set(-99999.0f,-99999.0f,-99999.0f);
	vMin.Set(99999.0f,99999.0f,99999.0f);

	this->verGroupInfos.Clear();
	this->materialInfos.clear();
	this->vertices.Clear();
	this->indices.Clear();

	char mtlFileName[512];
	mtlFileName[0] = '\0';
	ifstream fstream;
	istringstream stream;
	fstream.open(filename);

	if(!fstream.good())
		return 0;
	
	GrowableArray<Vector3> positions;//position list
	GrowableArray<Vector3> normals;//normal list
	GrowableArray<Vector2> texcoords;//texcoord list
	
	char keyword[256];
	
	

	while(fstream.good())
	{
		string infoString;
		string line;
		while(1)
		{
			getline(fstream,line);//get line
			if(line.size() == 0)
				break;
			if(line[line.size()-1] == '\\')//info also contains in next line
			{
				infoString.append(line,0,line.size()-1);
			}
			else
			{
				infoString.append(line); 
				break;
			}
				
		}
		stream.clear();
		stream.str(infoString);

		stream >> keyword;
		if(stream.fail())
			continue;
		if(!strncmp(keyword,"#",1))//comment line
		{
			continue;
		}
		if(!strcmp(keyword,"v"))//vertex position
		{
			Vector3 v;
			stream >> v.x >> v.y >> v.z;
			positions.Add(v);

			if(vMin.x > v.x)
				vMin.x = v.x;
			if(vMin.z > v.z)
				vMin.z = v.z;
			if(vMin.y > v.y)
				vMin.y = v.y;
			if(vMax.x < v.x)
				vMax.x = v.x;
			if(vMax.y < v.y)
				vMax.y = v.y;
			if(vMax.z < v.z)
				vMax.z = v.z;
		}
		else if(!strcmp(keyword,"vn"))//vertex normal
		{
			Vector3 v;
			stream >> v.x >> v.y >> v.z;
			v.Normalize();
			normals.Add(v);
		}
		else if(!strcmp(keyword,"vt"))//vertex texcoord
		{
			Vector2 v;
			stream >> v.x >> v.y;
			texcoords.Add(v);
		}
		else if(!strcmp(keyword,"f"))//face info
		{
			unsigned int  pIndex,nIndex,tIndex;//position ,normal,texcoord index
			unsigned int nVertices;//number of vertices in this face
			GrowableArray<unsigned short> findices;//list of index of each vertex in face
			Vertex vertex;

			memset(&vertex,0,sizeof(Vertex));
			while(1)
			{
				stream >> pIndex;
				if(stream.fail())
					break;
				vertex.position = positions[pIndex - 1];//index in obj file is 1-based
				if('/'==stream.peek())
				{
					stream.ignore();
					if('/'!=stream.peek())
					{
						stream >> tIndex;
						vertex.texcoord = texcoords[tIndex - 1];
					}
					if('/'==stream.peek())
					{
						stream.ignore();
						stream >> nIndex;
						vertex.normal = normals[nIndex - 1];
					}
				}
				findices.Add(this->AddVertex(pIndex,vertex));
			}//get all vertices in face
			
			nVertices = findices.Size();
			//construct indices so that this face will be drawn by a list of triangles for best performance
			//the indices will be like this {i,i+1,i+2 , i,i+2,i+3 , ..... ,i,i+n-2,i+n-1} which i is the index of first vertex and n is the number of vertices in face
			for(unsigned int i = 0; i< nVertices - 2 ; ++i)
			{
				this->indices.Add(findices[0]);
				this->indices.Add(findices[i+1]);
				this->indices.Add(findices[i+2]);
			}

		} //face info
		else if(!strcmp(keyword,"mtllib"))//mtl file name
		{
			stream >> mtlFileName;
		}
		else if(!strcmp(keyword,"usemtl"))
		{
			char name[100];
			stream >> name;
			
			if(this->verGroupInfos.Size() >= 1)
				this->verGroupInfos[this->verGroupInfos.Size() - 1].endIndex = this->indices.Size() - 1;
			//new vertices group
			
			VertGroupInfo vGroupInfo;
			vGroupInfo.startIndex = this->indices.Size() ;

			bool found = false;//found out that this material exists
			for(std::list<MaterialInfo>::iterator ite = materialInfos.begin();
				ite != materialInfos.end();
				++ite)
			{
				if(!strcmp(ite->materialName,name))//material exists
				{
					found = true;
					vGroupInfo.materialInfo = &(*ite);
					break;
				}
			}
			if(!found)//not found,add new material info to list
			{
				MaterialInfo matInfo;
				strcpy(matInfo.materialName,name);
				matInfo.material.ambient.a = 1.0f;
				matInfo.material.diffuse.a = 1.0f;
				matInfo.material.specular.a = 1.0f;

				matInfo.texInfo = NULL;

				this->materialInfos.push_back(matInfo);
				vGroupInfo.materialInfo = &(this->materialInfos.back());
			}
			this->verGroupInfos.Add(vGroupInfo);//add new group info
		}
	}
	this->ReleasePIndexTable();
	fstream.close();

	if(mtlFileName[0]!='\0')
	{
		if(this->LoadMtlFromFile(this->currentFilePath + mtlFileName)==0)
			return 0;
	}

	return 1;
}

int MeshManager::LoadMtlFromFile(string& filename)
{
	ifstream stream(filename.c_str());
	if(!stream.good())
		return 0;
	char keyword[256];
	MaterialInfo *pMat = NULL;
	while (stream.good())
	{
		stream >> keyword;
		if(!strcmp(keyword,"newmtl"))
		{
			char name[100];
			stream >> name;
			pMat = NULL;
			for(std::list<MaterialInfo>::iterator ite = materialInfos.begin();
				ite != materialInfos.end();
				++ite)
			{
				if(!strcmp(ite->materialName,name))//material exists
				{
					pMat = &(*ite);
					break;
				}
			}

		}
		if(pMat == NULL)
			continue;

		if(!strncmp(keyword,"#",1))//comment line
		{

		}
		else if(!strcmp(keyword,"Ka"))//ambient color
		{
			float r,g,b;
			stream >> r >> g >> b;
			pMat->material.ambient.r = r;
			pMat->material.ambient.g = g;
			pMat->material.ambient.b = b;
		}
		else if(!strcmp(keyword,"Kd"))//diffuse color
		{
			float r,g,b;
			stream >> r >> g >> b;
			pMat->material.diffuse.r = r;
			pMat->material.diffuse.g = g;
			pMat->material.diffuse.b = b;
		}
		else if(!strcmp(keyword,"Ks"))//specular color
		{
			float r,g,b;
			stream >> r >> g >> b;
			pMat->material.specular.r = r;
			pMat->material.specular.g = g;
			pMat->material.specular.b = b;
		}
		else if(strcmp(keyword,"d") == 0 || strcmp(keyword,"Tr") == 0)//alpha of material
		{
			//what is this alpha value used for?
		}
		else if(!strcmp(keyword,"Ns"))//shininess
		{
			stream >> pMat->material.shininess ;
		}
		else if(!strcmp(keyword,"illum"))//illumination
		{
			int illum;
			stream >> illum;
			pMat->specular = (illum == 2);
		}
		else if(strcmp(keyword,"map_Ka") == 0 || strcmp(keyword,"map_Kd") == 0)//texture file
		{
			if(pMat->texInfo == NULL)
			{
				char name[100];
				stream >> name;
				bool found = false;
				for(std::list<TextureInfo>::iterator i = texInfos.begin();
					i !=this->texInfos.end() ;++i)//checking for duplicated texture
				{
					if(!strcmp(i->textureName,name))//found
					{
						pMat->texInfo = &(*i);//save pointer to this texture info object 
						found = true;
						break;
					}
				}
				if(!found)//not found
				{
					TextureInfo texInfo;
					strcpy(texInfo.textureName,name);
					this->texInfos.push_back(texInfo);
					pMat->texInfo = &(this->texInfos.back());
				}
			}
		}
		stream.ignore(1000,'\n');
	}
	stream.close();
	
	
	for(std::list<MaterialInfo>::iterator ite = materialInfos.begin();
		ite != materialInfos.end();
		++ite)
	{
		if(ite->specular == false)
		{
			ite->material.specular.r = 0;
			ite->material.specular.g = 0;
			ite->material.specular.b = 0;
			break;
		}
	}

	return 1;
}


int MeshManager::LoadObjFromMem(const unsigned char* byteStream,unsigned int streamSize)
{
	this->verGroupInfos.Clear();
	this->materialInfos.clear();
	this->vertices.Clear();
	this->indices.Clear();

	char mtlFileName[512];
	mtlFileName[0] = '\0';
	CStringStream csstream((char*)byteStream,streamSize);
	istringstream stream;

	
	GrowableArray<Vector3> positions;//position list
	GrowableArray<Vector3> normals;//normal list
	GrowableArray<Vector2> texcoords;//texcoord list
	
	char keyword[256];
	
	

	while(!csstream.isAtEnd())
	{
		string infoString;
		char line[256];
		while(1)
		{
			csstream.GetLine(line,256);//get line
			int lineLen = strlen(line);
			if(lineLen == 0)
				break;
			if(line[lineLen-1] == '\\' || (line[lineLen-1] == '\r' && line[lineLen-2] == '\\'))//info also contains in next line
			{
				infoString.append(line,lineLen-1);
			}
			else
			{
				infoString.append(line); 
				break;
			}
				
		}
		stream.clear();
		stream.str(infoString);

		stream >> keyword;
		if(stream.fail())
			continue;
		if(!strncmp(keyword,"#",1))//comment line
		{
			continue;
		}
		if(!strcmp(keyword,"v"))//vertex position
		{
			Vector3 v;
			stream >> v.x >> v.y >> v.z;
			positions.Add(v);

			if(vMin.x > v.x)
				vMin.x = v.x;
			if(vMin.z > v.z)
				vMin.z = v.z;
			if(vMin.y > v.y)
				vMin.y = v.y;
			if(vMax.x < v.x)
				vMax.x = v.x;
			if(vMax.y < v.y)
				vMax.y = v.y;
			if(vMax.z < v.z)
				vMax.z = v.z;
		}
		else if(!strcmp(keyword,"vn"))//vertex normal
		{
			Vector3 v;
			stream >> v.x >> v.y >> v.z;
			v.Normalize();
			normals.Add(v);
		}
		else if(!strcmp(keyword,"vt"))//vertex texcoord
		{
			Vector2 v;
			stream >> v.x >> v.y;
			texcoords.Add(v);
		}
		else if(!strcmp(keyword,"f"))//face info
		{
			unsigned int  pIndex,nIndex,tIndex;//position ,normal,texcoord index
			unsigned int nVertices;//number of vertices in this face
			GrowableArray<unsigned short> findices;//list of index of each vertex in face
			Vertex vertex;

			memset(&vertex,0,sizeof(Vertex));
			while(1)
			{
				stream >> pIndex;
				if(stream.fail())
					break;
				vertex.position = positions[pIndex - 1];//index in obj file is 1-based
				if('/'==stream.peek())
				{
					stream.ignore();
					if('/'!=stream.peek())
					{
						stream >> tIndex;
						vertex.texcoord = texcoords[tIndex - 1];
					}
					if('/'==stream.peek())
					{
						stream.ignore();
						stream >> nIndex;
						vertex.normal = normals[nIndex - 1];
					}
				}
				findices.Add(this->AddVertex(pIndex,vertex));
			}//get all vertices in face
			
			nVertices = findices.Size();
			//construct indices so that this face will be drawn by a list of triangles for best performance
			//the indices will be like this {i,i+1,i+2 , i,i+2,i+3 , ..... ,i,i+n-2,i+n-1} which i is the index of first vertex and n is the number of vertices in face
			for(unsigned int i = 0; i< nVertices - 2 ; ++i)
			{
				this->indices.Add(findices[0]);
				this->indices.Add(findices[i+1]);
				this->indices.Add(findices[i+2]);
			}

		} //face info
		else if(!strcmp(keyword,"mtllib"))//mtl file name
		{
			stream >> mtlFileName;
		}
		else if(!strcmp(keyword,"usemtl"))
		{
			char name[100];
			stream >> name;
			
			if(this->verGroupInfos.Size() >= 1)
				this->verGroupInfos[this->verGroupInfos.Size() - 1].endIndex = this->indices.Size() - 1;
			//new vertices group
			
			VertGroupInfo vGroupInfo;
			vGroupInfo.startIndex = this->indices.Size() ;

			bool found = false;//found out that this material exists
			for(std::list<MaterialInfo>::iterator ite = materialInfos.begin();
				ite != materialInfos.end();
				++ite)
			{
				if(!strcmp(ite->materialName,name))//material exists
				{
					found = true;
					vGroupInfo.materialInfo = &(*ite);
					break;
				}
			}
			if(!found)//not found,add new material info to list
			{
				MaterialInfo matInfo;
				strcpy(matInfo.materialName,name);
				matInfo.material.ambient.a = 1.0f;
				matInfo.material.diffuse.a = 1.0f;
				matInfo.material.specular.a = 1.0f;

				matInfo.texInfo = NULL;

				this->materialInfos.push_back(matInfo);
				vGroupInfo.materialInfo = &(this->materialInfos.back());
			}
			this->verGroupInfos.Add(vGroupInfo);//add new group info
		}
	}
	this->ReleasePIndexTable();

	if(mtlFileName[0]!='\0')
	{
		unsigned char *byteStream;
		unsigned int streamSize;
		byteStream = packMan->GetSubByteStream(this->packageBufferID , mtlFileName,&streamSize);
		if(byteStream == 0 || streamSize == 0)
			return 0;

		if(this->LoadMtlFromMem(byteStream,streamSize)==0)
			return 0;
	}

	return 1;
}

int MeshManager::LoadMtlFromMem(const unsigned char* byteStream,unsigned int streamSize)
{
	stringstream stream;
	stringbuf *rdbuf = stream.rdbuf();
	rdbuf->sputn((char*)byteStream,streamSize);
	
	if(!stream.good())
		return 0;
	char keyword[256];
	MaterialInfo *pMat = NULL;
	while (stream.good())
	{
		stream >> keyword;
		if(!strcmp(keyword,"newmtl"))
		{
			char name[100];
			stream >> name;
			pMat = NULL;
			for(std::list<MaterialInfo>::iterator ite = materialInfos.begin();
				ite != materialInfos.end();
				++ite)
			{
				if(!strcmp(ite->materialName,name))//material exists
				{
					pMat = &(*ite);
					break;
				}
			}

		}
		if(pMat == NULL)
			continue;

		if(!strncmp(keyword,"#",1))//comment line
		{

		}
		else if(!strcmp(keyword,"Ka"))//ambient color
		{
			float r,g,b;
			stream >> r >> g >> b;
			pMat->material.ambient.r = r;
			pMat->material.ambient.g = g;
			pMat->material.ambient.b = b;
		}
		else if(!strcmp(keyword,"Kd"))//diffuse color
		{
			float r,g,b;
			stream >> r >> g >> b;
			pMat->material.diffuse.r = r;
			pMat->material.diffuse.g = g;
			pMat->material.diffuse.b = b;
		}
		else if(!strcmp(keyword,"Ks"))//specular color
		{
			float r,g,b;
			stream >> r >> g >> b;
			pMat->material.specular.r = r;
			pMat->material.specular.g = g;
			pMat->material.specular.b = b;
		}
		else if(strcmp(keyword,"d") == 0 || strcmp(keyword,"Tr") == 0)//alpha of material
		{
			//what is this alpha value used for?
		}
		else if(!strcmp(keyword,"Ns"))//shininess
		{
			stream >> pMat->material.shininess ;
		}
		else if(!strcmp(keyword,"illum"))//illumination
		{
			int illum;
			stream >> illum;
			pMat->specular = (illum == 2);
		}
		else if(strcmp(keyword,"map_Ka") == 0 || strcmp(keyword,"map_Kd") == 0)//texture file
		{
			if(pMat->texInfo == NULL)
			{
				char name[100];
				stream >> name;
				bool found = false;
				for(std::list<TextureInfo>::iterator i = texInfos.begin();
					i !=this->texInfos.end() ;++i)//checking for duplicated texture
				{
					if(!strcmp(i->textureName,name))//found
					{
						pMat->texInfo = &(*i);//save pointer to this texture info object 
						found = true;
						break;
					}
				}
				if(!found)//not found
				{
					TextureInfo texInfo;
					strcpy(texInfo.textureName,name);
					this->texInfos.push_back(texInfo);
					pMat->texInfo = &(this->texInfos.back());
				}
			}
		}
		stream.ignore(1000,'\n');
	}
	
	
	for(std::list<MaterialInfo>::iterator ite = materialInfos.begin();
		ite != materialInfos.end();
		++ite)
	{
		if(ite->specular == false)
		{
			ite->material.specular.r = 0;
			ite->material.specular.g = 0;
			ite->material.specular.b = 0;
			break;
		}
	}

	return 1;
}

unsigned short MeshManager::AddVertex(unsigned int positionIndex, Vertex &vertex)
{
	bool found = false;//found that this vertex already exists?
	unsigned short index = 0;//the return index of newly added vertex or existing vertex

	if(positionIndex < this->pIndexTable.Size())
	{
		list<unsigned short> *pList = this->pIndexTable[positionIndex];
		if(pList != NULL)//found vertices that have same position with waiting vertex
		{
			Vertex *pVertex;
			list<unsigned short>::iterator ite;
			for(ite = pList->begin() ; ite != pList->end(); ++ite)
			{
				pVertex = this->vertices.GetArray() + *ite;
				if(!memcmp(pVertex,&vertex,sizeof(Vertex)))//we found duplicated vertex
				{
					index = *ite;
					found = true;
					break;
				}
			}
		}
	}

	if(!found)//this vertex will be added to vertices list
	{
		index = this->vertices.Size();
		this->vertices.Add(vertex);

		while(this->pIndexTable.Size() <= positionIndex)//grow pIndextable's size to the point that <positionIndex> will not out of range
		{
			this->pIndexTable.Add(NULL);
		}
		list<unsigned short> *&pList = this->pIndexTable[positionIndex];
		if( pList == NULL )//doesn't has the list of vertices that have same position with newly added vertex yet
		{
			pList = new list<unsigned short>();
		}
		pList->push_back(index);

	}
	return index;
}

void MeshManager::SortGroup()
{

}

void MeshManager::ReleasePIndexTable()
{
	for(unsigned int i = 0;i < this->pIndexTable.Size();++i)
	{
		list<unsigned short> *&pList = this->pIndexTable[i];
		if(pList != NULL)
		{
			pList->clear();
			delete pList;
			pList = NULL;
		}
	}
	this->pIndexTable.Clear();

}

void MeshManager::ReleaseTextures()
{
	for(std::list<TextureInfo>::iterator i = texInfos.begin();
		i !=this->texInfos.end() ;++i)
	{
		this->pRenderer->ReleaseTexture(i->textureID);
	}
	this->texInfos.clear();
}

void MeshManager::RemoveMesh(unsigned int meshID)
{
	SharedPtr<Mesh> ptr = this->GetItemPointer(meshID);
	if(ptr == NULL)
		return;
	pRenderer->ReleaseStaticVertexBuffer(ptr->vBufferID);
	pRenderer->ReleaseStaticIndexBuffer(ptr->iBufferID);
	this->ReleaseSlot(meshID);
}

void MeshManager::RemoveAllMeshes()
{
	for(unsigned int i = 0;i < this->allocSlots ; ++i)
		RemoveMesh(i);
}

void MeshManager::DrawMesh(unsigned int meshID)
{
	SharedPtr<Mesh> ptr = this->GetItemPointer(meshID);
	if(ptr == NULL)
		return;
	pRenderer->SetStaticBuffer(ptr->vBufferID,ptr->iBufferID);
	for(unsigned int i=0;i<ptr->nGroup;++i)
	{
		pRenderer->SetMaterial(ptr->vGroup[i].material);
		pRenderer->SetTexture(ptr->vGroup[i].textureID);
		pRenderer->DrawObject2(ptr->vGroup[i].startIndex,ptr->vGroup[i].endIndex);
	}
}


AABB * MeshManager::GetMeshAABB(unsigned int meshID)
{
	SharedPtr<Mesh> ptr = this->GetItemPointer(meshID);
	if(ptr == NULL)
		return NULL;
	return &ptr->box;
}