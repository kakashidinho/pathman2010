#ifndef _RENDERER_
#define _RENDERER_

#ifdef RENDERER_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else
#define RENDERER_API __declspec(dllimport)
#endif

#define R_OK 1
#define R_FAILED 0
#define R_FAILED_NOT_IN_2D_MODE 2
#define R_FAILED_NOT_IN_TEXT_MODE 3
#define R_FAILED_NOT_IN_3D_MODE 4
#define R_FAILED_NOT_BEGIN_RENDER 5
#define R_FAILED_FONT_NOT_EXISTED 6
#define R_FAILED_CANT_SWITCH_TEXT_TO_2D_MODE 7
#define R_FAILED_INVALID_BUFFER 8
#define TEXTURE_NOT_AVAIL 9
#define TEXTURE_CANT_LOAD_FROM_FILE_NOT_EXIST 10
#define TEXTURE_CANT_LOAD_FROM_INVALID_BYTE_STREAM 11
#define TEXTURE_CANT_LOAD_MEM_FAILED 12
#define TEXTURE_CANT_LOAD_FILE_NOT_SUPPORT 13

#define NOT_USE_ITEM 0xcdcdcdcd

enum RenderMode
{
	R_3D = 0,//3D mode
	R_2D = 1,//2D mode
	R_TEXT = 2//text mode
};

struct Rect
{
	int left;
	int right;
	int bottom;
	int top;
};

union Color
{
	struct
	{
		float r,g,b,a;
	};
	float c[4];
};

struct Material
{
	Color ambient;
	Color diffuse;
	Color specular;
	float shininess;
};

struct DirectionLight
{
	Color ambient;
	Color diffuse;
	Color specular;
	float direction[3];//vector direction in world space
};

#include "CommonVertexType.h"
/*---------------------------------
Interface Renderer
---------------------------------*/
class Renderer
{
protected:
	virtual ~Renderer(){};
public:
	Renderer(){};
	//release memory allocated for this object
	virtual int Release()=0;

	virtual void BeginRender()=0;//begin rendering .Basically ,it just clear the screen
	virtual void EndRender()=0;//finish rendering

	/*----------2D mode & text mode ----------*/
	//enter 2D mode, renderer can't draw 2D elements if this function not called before.Note: can't enter 2D mode if renderer already in text mode
	virtual int Enable2DMode()=0;
	virtual int	Disable2DMode()=0;//return to normal 3D mode

	//enter text mode, renderer can't draw text if this function not called before.
	//Note:If call this function after call Enable2DMode(),2D mode will switch to text mode,
	//call Disable2DMode() after that will take no effect unless text mode is disable by calling DisableTextMode() first
	virtual int EnableTextMode()=0;
	virtual int	DisableTextMode()=0;//return to normal mode

	virtual RenderMode GetCurrentMode()=0;//get current mode of renderer
	/*-----------------------------------------------*/
	virtual void SetViewport(Rect *rect)=0;//if <rect> is null ,viewport will be set to entire window
	virtual void SetViewport1(int left,int right,int bottom,int top)=0;
	virtual void SetViewport2(int x,int y,int width,int height)=0;//x,y is bottom left position
	
	virtual unsigned int GetWidth()=0;//get windows 's width
	virtual unsigned int GetHeight()=0;//get windows 's height

	virtual void EnableLighting()=0;//enable light shading
	virtual void DisableLighting()=0;//disable light shading

	virtual void EnableLightSource(unsigned int lightIndex)=0;//enable light source
	virtual void DisableLightSource(unsigned int lightIndex)=0;//disable light source

	virtual void SetupDirectionLight(unsigned int lightIndex,DirectionLight& dlight)=0;//setup directional light
	
	virtual void EnableAlphaBlend()=0;//enable alpha blend
	virtual void DisableAlphaBlend()=0;//disable alpha blend

	virtual void EnableDepthTest()=0;//enable depth test
	virtual void DisableDepthTest()=0;//disable depth test

	virtual void EnableCullBackFace()=0;//enable cull back face
	virtual void DisableCullBackFace()=0;//disable cull back face

	virtual bool SetViewMatrix(const float *matrix)=0;//set current view matrix in 3D mode.<matrix> is array of 16 float.this operation will fail if renderer is in 2D/text mode
	virtual bool SetWorldMatrix(const float *matrix)=0;//set current world transfrom matrix in 3D mode.<matrix> is array of 16 float.this operation will fail if renderer is in 2D/text mode.Passing NULL parameter will set world matrix to identity matrix
	virtual bool SetProjectionMatrix(const float *matrix)=0;//set current projection matrix in 3D mode.<matrix> is array of 16 float.this operation will fail if renderer is in 2D/text mode
	virtual void SetMaterial(Material &mat)=0;//set current active material

	/*--------------------------------*/
	virtual int LoadTextureFromFile(const char* imgFile,unsigned int* pTextureID)=0;//load texture from file,only support tga image
	virtual int LoadTextureFromMemory(const unsigned char* byteStream,unsigned int streamSize,unsigned int* pTextureID)=0;//load texture from byte stream,only support tga image
	virtual int ReleaseAllTexture()=0;//release all textures
	virtual int ReleaseTexture(unsigned int textureID)=0;//release texture that has ID <textureID>
	
	virtual int	GetTextureImageRect(unsigned int textureID,Rect &rect)=0;//get size of image that texture loaded from

	virtual int SetTexture(unsigned int textureID)=0;//set current active texture for rendering.if <texureID> = 0,renderer will not use any texture

	/*---------create font---------*/
	//create Font object from init text file & image file
	virtual int CreateFontFromFile(const char* iniFile,const char * imageFile,unsigned int *pFontID)=0;
	//create Font object from init info byte stream & image data
	virtual int CreateFontFromMemory(const unsigned char* initInfoByteStream,
									 unsigned int initInfoStreamSize,
									 const unsigned char * imageDataByteStream,
									 unsigned int imageDataSize,
									 unsigned int *pFontID)=0;

	virtual float GetStringLength(const char* String,unsigned int fontID,float spaceBetweenChar=2.0f)=0;//get length of string if it is displayed to screen using font object <fontID>.This string must not have line breaking char such as '\n'
	virtual int GetFontLineDistance(unsigned int fontID)=0;//get distance between lines if text is drawn to screen with font <fontID>

	/*----------------------------------*/
	//create static vertex buffer and fill data<pVerticesData> into it
	virtual int CreateStaticVertexBuffer(void *pVerticesData,unsigned int numVertices , VertexType vType,unsigned int *pBufferID)=0;
	//create static index buffer and fill data<pIndicesData> into it
	virtual int CreateStaticIndexBuffer(unsigned short *pIndicesData,unsigned int numIndices,unsigned int *pBufferID)=0;
	
	virtual int ReleaseStaticVertexBuffer(unsigned int bufferID)=0;
	virtual int ReleaseStaticIndexBuffer(unsigned int bufferID)=0;

	virtual int SetStaticBuffer(unsigned int vertexBufferID,unsigned int indexBufferID)=0;//set vertex buffer <vetexBufferID> as current active vertex buffer and index buffer <indexbufferID> as current active index buffer for rendering 

	/*-----------------------------------
	copy texture to entire window screen or viewport of screen,
		<depth> - depth value for sorting purpose, value is clamped in range [0-1] ,which 1 denotes furthest and 0 denotes nearest.Pixels in area that texture will map to will have this <depth> value in depth buffer
		<pTexRect> - pointer to rectangle area in image space of texture that will be used,NULL denotes enrire texture
		<pDestRect> - pointer to rectangle area in window space that will be mapped texture to,NULL denotes enrire window
	image space - origin is in top left corner
	window space - origin is in bottom left corner
	this operation will be failed if renderer not in 2D mode
	-----------------------------------*/
	virtual int BlitTextureToScreen(unsigned int textureID,float depth=0.0f,const Rect *pTexRect = NULL ,const Rect *pDestRect=NULL)=0;
	
	virtual int DrawString(const char *String,unsigned int fontID,float X,float Y,float spaceBetweenChar=2.0f)=0;//(X,Y) is starting position of text's line in window space
	virtual int FadingScreen(float fadeLevel)=0;//draw fading effect , <fadeLevel> is in range [0-1], if <fadeLevel> = 1  the screen will totally in black .This operation will be failed if renderer in text mode
	
	//draw object that is constructed from vertex data stored in current active vertex & index buffer.Geometric primitives must be triangle primitives.
	//<numTriangles> - number of triangles that form this object
	//<startIndex> is starting element in index array buffer that you want to use
	virtual int DrawObject(unsigned int startIndex,unsigned int numTriangles)=0;

	//draw object that is constructed from vertex data stored in current active vertex & index buffer.Geometric primitives must be triangle primitives.
	//<startIndex> is starting element in index array buffer that you want to use
	//<startIndex> is last element in index array buffer that you want to use
	virtual int DrawObject2(unsigned int startIndex,unsigned int endIndex)=0;

	//draw flat face with current active vertex & index buffer.
	//<numElems> - number of triangles primitives if this face is constructed from a set of triangles,or number of vertices in face if this face's primitive is single polygon rather than triangle
	//<isTriPrimitive> - is this face constructed from a set of triangles?
	//<startIndex> is starting element in index array buffer that you want to use
	virtual int DrawFace(unsigned int startIndex,unsigned int numElems,bool isTriPrimitive = false)=0;

};


extern "C" RENDERER_API Renderer* CreateRenderer(unsigned int sWidth,unsigned sHeight);//create renderer object and get interface pointer to it
typedef Renderer* (*pCreateRendererProc)(unsigned int sWidth,unsigned sHeight);
#endif