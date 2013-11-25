#include "stdafx.h"
#include "RendererImp.h"

/*--------helper functions-------------*/
void CreateStaticVertexBuffer(const void *pSourceData ,unsigned int vertexSize,unsigned int numVertices, void **ppBuffer)
{
	*ppBuffer = malloc(sizeof(GLuint));
	glGenBuffers(1,(GLuint*)*ppBuffer);
	glBindBuffer(GL_ARRAY_BUFFER,*(GLuint*)*ppBuffer);
	glGetError();//reset error flag
    glBufferData(GL_ARRAY_BUFFER,numVertices*vertexSize,pSourceData,GL_STATIC_DRAW);
	if(GL_OUT_OF_MEMORY == glGetError())
	{
		glDeleteBuffers(1,(GLuint*)ppBuffer);
		free(*ppBuffer);
		*ppBuffer = 0;
		return ;
	}
}

void CreateVertexArray(const void *pSourceData ,unsigned int vertexSize,unsigned int numVertices,void **ppBuffer)
{
	*ppBuffer = malloc(vertexSize * numVertices);
	if(*ppBuffer)
		memcpy(*ppBuffer , pSourceData,vertexSize * numVertices);
}

void CreateStaticIndexBuffer(const unsigned short *pSourceData,unsigned int numIndices, void **ppBuffer)
{
	*ppBuffer = malloc(sizeof(GLuint));
	glGenBuffers(1,(GLuint*)*ppBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,*(GLuint*)*ppBuffer);
	glGetError();//reset error flag
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,numIndices*sizeof(unsigned short),pSourceData,GL_STATIC_DRAW);
	if(GL_OUT_OF_MEMORY == glGetError())
	{
		glDeleteBuffers(1,(GLuint*)*ppBuffer);
		free(*ppBuffer);
		*ppBuffer = 0;
		return ;
	}
}

void CreateIndexArray(const unsigned short *pSourceData ,unsigned int numIndices,void **ppBuffer)
{
	*ppBuffer = malloc(sizeof(unsigned short) * numIndices);
	if(*ppBuffer)
		memcpy(*ppBuffer , pSourceData,sizeof(unsigned short) * numIndices);
}


/*------Static Buffer classes---------*/
StaticBuffer::StaticBuffer(bool vbo,unsigned int numElements)
{
	this->vbo = vbo;
	pBufferData = NULL;
	this->numElements = numElements;
}
StaticBuffer::~StaticBuffer()
{
	if(vbo)//we are using vertex buffer object
	{
		if(pBufferData!=NULL)
		{
			glDeleteBuffers(1,(GLuint*)pBufferData);
			free(this->pBufferData);
		}
	}
	else //we are using vertex array
	{
		free(this->pBufferData);
	}
}
//vertex buffer
StaticVertexBuffer::StaticVertexBuffer(void* pSrcData,bool vbo,VertexType vType,unsigned int numVertices):StaticBuffer(vbo,numVertices)
{
	this->vType = vType;
	switch(vType)
	{
	case VT_P3:
		this->vertexSize = sizeof(VertexP3);
		break;
	case VT_P3_T2:
		this->vertexSize = sizeof(VertexP3T2);
		break;
	case VT_P3_T2_N3:
		this->vertexSize = sizeof(VertexP3T2N3);
		break;
	}

	if(this->vbo)//yay - vertex buffer object supported
		CreateStaticVertexBuffer(pSrcData,vertexSize,numVertices,&pBufferData);
	else
		CreateVertexArray(pSrcData,vertexSize,numVertices,&pBufferData);
}

void StaticVertexBuffer::Active()
{
	if(!pBufferData || !vbo)//this function only usable if vertex buffer object supported
		return;
	glBindBuffer(GL_ARRAY_BUFFER,*(GLuint*)pBufferData);
}

void StaticVertexBuffer::SetVertexArray(void *startAddress)
{
	switch(vType)
	{
	case VT_P3_T2_N3:
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT,vertexSize,(unsigned char*)startAddress + 5*sizeof(float));
	case VT_P3_T2:
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2,GL_FLOAT,vertexSize,(unsigned char*)startAddress + 3*sizeof(float));
	case VT_P3:
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3,GL_FLOAT,vertexSize,(unsigned char*)startAddress);
	}

}

void StaticVertexBuffer::BeginUseBuffer()
{
	if(vbo)//using vertex buffer object
		SetVertexArray(0);
	else//using vertex array
		SetVertexArray(pBufferData);
}
void StaticVertexBuffer::EndUseBuffer()
{
	switch(vType)
	{
	case VT_P3_T2_N3:
		glDisableClientState(GL_NORMAL_ARRAY);
	case VT_P3_T2:
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	case VT_P3:
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

//index buffer
StaticIndexBuffer::StaticIndexBuffer(unsigned short* pSrcData,bool vbo,unsigned int numIndices):StaticBuffer(vbo,numIndices)
{
	if(this->vbo)//yay - vertex buffer object supported
		CreateStaticIndexBuffer(pSrcData,numIndices,&pBufferData);
	else
		CreateIndexArray(pSrcData,numIndices,&pBufferData);
}

void StaticIndexBuffer::Active()
{
	if(!pBufferData || !vbo)//this function only usable if vertex buffer object supported
		return;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,*(GLuint*)pBufferData);
}

void *StaticIndexBuffer::GetArrayAddress()
{
	if(vbo)//using vertex buffer object
		return 0;
	return pBufferData;
}
/*-----------renderer's buffer management----------------*/
int RendererImp::CreateStaticVertexBuffer(void *pVerticesData,unsigned int numVertices , VertexType vType,unsigned int *pBufferID)
{
	StaticVertexBuffer* pBuff = new StaticVertexBuffer(pVerticesData,(flags & VBO)!=0,vType, numVertices);
	if(pBuff->pBufferData == NULL || !vMan.AddItem(pBuff,pBufferID))
	{
		delete pBuff;
		return R_FAILED;
	}
	return R_OK;
}
int RendererImp::CreateStaticIndexBuffer(unsigned short *pIndicesData,unsigned int numIndices,unsigned int *pBufferID)
{
	StaticIndexBuffer* pBuff = new StaticIndexBuffer(pIndicesData,(flags & VBO)!=0, numIndices);
	if(pBuff->pBufferData == NULL || !iMan.AddItem(pBuff,pBufferID))
	{
		delete pBuff;
		return R_FAILED;
	}
	return R_OK;
}
int RendererImp::ReleaseStaticVertexBuffer(unsigned int bufferID)
{
	return vMan.ReleaseSlot(bufferID);
}
int RendererImp::ReleaseStaticIndexBuffer(unsigned int bufferID)
{
	return iMan.ReleaseSlot(bufferID);
}

int RendererImp::SetStaticBuffer(unsigned int vertexBufferID,unsigned int indexBufferID)
{
	if(this->currentVBuffer != vertexBufferID)
	{
		SharedPtr<StaticVertexBuffer> ptr = vMan.GetItemPointer(vertexBufferID);
		if(ptr!= NULL)
		{
			ptr->Active();
			this->currentVBuffer = vertexBufferID;
		}
	}
	if(this->currentIBuffer != indexBufferID)
	{
		SharedPtr<StaticIndexBuffer> ptr = iMan.GetItemPointer(indexBufferID);
		if(ptr!= NULL)
		{
			ptr->Active();
			this->currentIBuffer = indexBufferID;
		}
	}
	return R_OK;
}

void RendererImp::ReActiveVertexBuffer()
{
	SharedPtr<StaticVertexBuffer> pV = vMan.GetItemPointer(currentVBuffer);
	if(pV!= NULL)
	{
		pV->Active();
	}
	SharedPtr<StaticIndexBuffer> pI = iMan.GetItemPointer(currentIBuffer);
	if(pI!= NULL)
	{
		pI->Active();
	}
}

int RendererImp::DrawObject(unsigned int startIndex,unsigned int numTriangles)
{
	if(!(flags & RENDERING))
		return R_FAILED_NOT_BEGIN_RENDER;
	if(flags & _2D_MODE || flags & TEXT_MODE)
		return R_FAILED_NOT_IN_3D_MODE;

	SharedPtr<StaticVertexBuffer> pV = vMan.GetItemPointer(currentVBuffer);
	SharedPtr<StaticIndexBuffer> pI = iMan.GetItemPointer(currentIBuffer);
	if(pV== NULL || pI==NULL)
		return R_FAILED_INVALID_BUFFER;
	
	pV->BeginUseBuffer();
	
	glDrawElements(GL_TRIANGLES,numTriangles * 3,GL_UNSIGNED_SHORT,(unsigned short*)pI->GetArrayAddress() + startIndex);

	pV->EndUseBuffer();
	
	return R_OK;
}

int RendererImp::DrawObject2(unsigned int startIndex,unsigned int endIndex)
{
	if(!(flags & RENDERING))
		return R_FAILED_NOT_BEGIN_RENDER;
	if(flags & _2D_MODE || flags & TEXT_MODE)
		return R_FAILED_NOT_IN_3D_MODE;

	SharedPtr<StaticVertexBuffer> pV = vMan.GetItemPointer(currentVBuffer);
	SharedPtr<StaticIndexBuffer> pI = iMan.GetItemPointer(currentIBuffer);
	if(pV== NULL || pI==NULL)
		return R_FAILED_INVALID_BUFFER;
	
	pV->BeginUseBuffer();
	
	glDrawElements(GL_TRIANGLES,endIndex - startIndex + 1,GL_UNSIGNED_SHORT,(unsigned short*)pI->GetArrayAddress() + startIndex);

	pV->EndUseBuffer();
	
	return R_OK;
}

int RendererImp::DrawFace(unsigned int startIndex,unsigned int numElems,bool isTriPrimitive)
{
	if(!(flags & RENDERING))
		return R_FAILED_NOT_BEGIN_RENDER;
	if(flags & _2D_MODE || flags & TEXT_MODE)
		return R_FAILED_NOT_IN_3D_MODE;

	SharedPtr<StaticVertexBuffer> pV = vMan.GetItemPointer(currentVBuffer);
	SharedPtr<StaticIndexBuffer> pI = iMan.GetItemPointer(currentIBuffer);
	if(pV== NULL || pI==NULL)
		return R_FAILED_INVALID_BUFFER;
	
	pV->BeginUseBuffer();
	
	if(isTriPrimitive)
		glDrawElements(GL_TRIANGLES,numElems * 3,GL_UNSIGNED_SHORT,(unsigned short*)pI->GetArrayAddress() + startIndex);
	else
		glDrawElements(GL_POLYGON,numElems,GL_UNSIGNED_SHORT,(unsigned short*)pI->GetArrayAddress() + startIndex);
	pV->EndUseBuffer();
	
	return R_OK;
}
