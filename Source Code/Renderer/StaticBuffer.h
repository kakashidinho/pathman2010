#ifndef _STATIC_BUFFER_
#define _STATIC_BUFFER_
#include "../CommonVertexType.h"
#include "../ItemManager.h"
/*-----------static buffer - for holding data that is filled into buffer once at creation time and never changes after that-------------------*/
struct StaticBuffer
{
	StaticBuffer(bool vbo,unsigned int numElements);
	virtual ~StaticBuffer();
	
	bool vbo;//is vertex buffer object supported
	void *pBufferData;
	unsigned int numElements;
};

struct StaticVertexBuffer:public StaticBuffer
{
private:
	void SetVertexArray(void *startAddress);//worker function
public:
	StaticVertexBuffer(void* pSrcData,bool vbo,VertexType vType,unsigned int numVertices);
	~StaticVertexBuffer(){};
	
	void Active();//active this buffer
	void BeginUseBuffer();//begin using this buffer before rendering
	void EndUseBuffer();//end using this buffer after rendering
	VertexType vType;
	unsigned int vertexSize;
};

struct StaticIndexBuffer:public StaticBuffer
{
	StaticIndexBuffer(unsigned short* pSrcData,bool vbo,unsigned int numIndices);
	~StaticIndexBuffer(){};
	void Active();//active this buffer

	void *GetArrayAddress();
};


#endif