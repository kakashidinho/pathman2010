#ifndef _RENDERER_IMPLEMENT_
#define _RENDERER_IMPLEMENT_

#include <GL/gl.h>
#include <GL/glu.h>
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"opengl32.lib")

#include "../Util.h"
#if defined (_DEBUG)||defined(DEBUG)
#pragma comment(lib,"../Debug/Util.lib")
#else
#pragma comment(lib,"../Release/Util.lib")
#endif

#include "../Renderer.h"
#include "TextureManager.h"
#include "Font.h"
#include "StaticBuffer.h"
/*----------------
macro
----------------*/
#define RENDERING 0x1
#define TEXT_MODE 0x2
#define _2D_MODE 0x4
#define LIGHTING 0x8
#define ALPHA_BLEND 0x10
#define DEPTH_TEST 0x20
#define CULL 0x40
#define VBO 0x100 //vertex buffer object supported
#define SWAP_CONTROL 0x1000

//*****************
//constants
//*****************
const Vector4 XaxisPos =Vector4(1,0,0,0);//positive X axis
const Vector4 YaxisPos =Vector4(0,1,0,0);//positive Y axis
const Vector4 ZaxisPos =Vector4(0,0,1,0);//positive Z axis
const Vector4 XaxisNeg =Vector4(-1,0,0,0);//negative X axis
const Vector4 YaxisNeg =Vector4(0,-1,0,0);//negative Y axis
const Vector4 ZaxisNeg =Vector4(0,0,-1,0);//negative Z axis
const Vector4 OriginPos =Vector4(0,0,0,1);//world's origin


/*---------------------------------------------
class RendererImp implements interface Renderer
----------------------------------------------*/
class RendererImp:public Renderer
{
private:
	unsigned int flags;
	unsigned int sWidth,sHeight;//window width, height in screen space
	TextureManager textureMan;//texture manager
	ItemManager<Font> fontMan;//font manager
	ItemManager<StaticVertexBuffer> vMan;//vertex buffer manager
	ItemManager<StaticIndexBuffer> iMan;//index buffer manager
	unsigned int currentVBuffer;//id of current active vetex buffer
	unsigned int currentIBuffer;//id of current active index buffer
	Matrix4x4 ortho2DMatrix;//ortho projection matrix for 2D rendering
	Matrix4x4 view2DMatrix;//view matrix for 2D rendering
	Matrix4x4 projMatrix;//main 3D projection matrix
	Matrix4x4 modelViewMatrix;//main 3D model view matrix
	Matrix4x4 viewMatrix;//main 3D view matrix
	Matrix4x4 worldMatrix;//main 3D world transform matrix
	
	void Init2DMatrices();//initialize 2D projection , modelview matrices
	void Init();//init default openGL state
	void CalModelViewMatrix();
	void ReActiveVertexBuffer();
	

	~RendererImp();//destructor
public:
	RendererImp(unsigned int sWidth,unsigned sHeight);//contructor
	int Release();

	void BeginRender();
	void EndRender();
	/*----------2D mode & text mode ----------*/
	int Enable2DMode();//enter 2D mode, renderer can't draw 2D elements if this function not called before.Note: can't enter 2D mode if renderer already in text mode
	int	Disable2DMode();//return to normal 3D mode

	int EnableTextMode();//enter text mode, renderer can't draw text if this function not called before
	int	DisableTextMode();//return to normal mode

	RenderMode GetCurrentMode();//get current mode of renderer
	/*-----------------------------------------------*/
	void SetViewport(Rect *rect);
	void SetViewport1(int left,int right,int bottom,int top);
	void SetViewport2(int x,int y,int width,int height);//x,y is bottom left position

	unsigned int GetWidth() {return sWidth;};//get windows 's width
	unsigned int GetHeight() {return sHeight;};//get windows 's height

	unsigned int GetFlags(){return flags;};

	void EnableLighting();
	void DisableLighting();

	void EnableLightSource(unsigned int sourceIndex);//enable light source
	void DisableLightSource(unsigned int sourceIndex);//disable light source

	void SetupDirectionLight(unsigned int lightIndex,DirectionLight& dlight);

	void EnableAlphaBlend();
	void DisableAlphaBlend();

	void EnableDepthTest();
	void DisableDepthTest();

	virtual void EnableCullBackFace();//enable cull back face
	virtual void DisableCullBackFace();//disable cull back face

	bool SetViewMatrix(const float *matrix);//set current view matrix in 3D mode.<matrix> is array of 16 float
	bool SetWorldMatrix(const float *matrix);//set current world transfrom matrix in 3D mode.<matrix> is array of 16 float
	bool SetProjectionMatrix(const float *matrix);//set current projection matrix in 3D mode.<matrix> is array of 16 float
	
	void SetMaterial(Material &mat);

	/*-----------------------------------------*/
	int LoadTextureFromFile(const char* imgFile,unsigned int* pTextureID);//load texture from file,only support tga image
	int LoadTextureFromMemory(const unsigned char* byteStream,unsigned int streamSize,unsigned int* pTextureID);//load texture from byte stream,only support tga image
	int ReleaseAllTexture();//release all textures
	int ReleaseTexture(unsigned int textureID);//release texture that has ID <textureID>
	
	int	GetTextureImageRect(unsigned int textureID,Rect &rect);//get size of image that texture loaded from
	
	int SetTexture(unsigned int textureID);//set current active texture for rendering
	/*---------create font---------*/
	//create Font object from init text file & image file
	int CreateFontFromFile(const char* iniFile,const char * imageFile,unsigned int *pFontID);
	//create Font object from init info byte stream & image data
	int CreateFontFromMemory(const unsigned char* initInfoByteStream,
									 unsigned int initInfoStreamSize,
									 const unsigned char * imageDataByteStream,
									 unsigned int imageDataSize,
									 unsigned int *pFontID);
	
	float GetStringLength(const char* String,unsigned int fontID,float spaceBetweenChar=2.0f);
	int GetFontLineDistance(unsigned int fontID);

	/*----------------------------------*/
	//create static vertex buffer and fill data<pVerticesData> into it
	int CreateStaticVertexBuffer(void *pVerticesData,unsigned int numVertices , VertexType vType,unsigned int *pBufferID);
	//create static index buffer and fill data<pIndicesData> into it
	int CreateStaticIndexBuffer(unsigned short *pIndicesData,unsigned int numIndices,unsigned int *pBufferID);
	
	int ReleaseStaticVertexBuffer(unsigned int bufferID);
	int ReleaseStaticIndexBuffer(unsigned int bufferID);

	int SetStaticBuffer(unsigned int vertexBufferID,unsigned int indexBufferID);//set vertex buffer <vetexBufferID> as current active vertex buffer and index buffer <indexbufferID> as current active index buffer for rendering 
	/*-----------------------------------
	copy texture to entire window screen or viewport of screen,
		<depth> - depth value for sorting purpose, value is clamped in range [0-1] ,which 1 denotes furthest and 0 denotes nearest.Pixels in area that texture will map to will have this <depth> value in depth buffer
		<pTexRect> - pointer to rectangle area in image space of texture that will be used,NULL denotes enrire texture
		<pDestRect> - pointer to rectangle area in window space that will be mapped texture to,NULL denotes enrire window
	image space - origin is in top left corner
	window space - origin is in bottom left corner
	this operation will be failed if renderer not in 2D mode
	-----------------------------------*/
	int BlitTextureToScreen(unsigned int textureID,float depth,const Rect *pTexRect = NULL ,const Rect *pDestRect=NULL);
	
	int DrawString(const char *String,unsigned int fontID,float X,float Y,float spaceBetweenChar);
	int FadingScreen(float fadeLevel);//draw fading effect

	//draw object that is constructed from vertex data stored in current active vertex & index buffer.Geometric primitives must be triangle primitives.
	//<numTriangles> - number of triangles that form this object
	//<startIndex> is starting element in index array buffer that you want to use
	int DrawObject(unsigned int startIndex,unsigned int numTriangles);
	
	//draw object that is constructed from vertex data stored in current active vertex & index buffer.Geometric primitives must be triangle primitives.
	//<startIndex> is starting element in index array buffer that you want to use
	//<startIndex> is last element in index array buffer that you want to use
	int DrawObject2(unsigned int startIndex,unsigned int endIndex);

	//draw flat face with current active vertex & index buffer.
	//<numElems> - number of triangles primitives if this face is constructed from a set of triangles,or number of vertices in face if this face's primitive is single polygon rather than triangle
	//<isTriPrimitive> - is this face constructed from a set of triangles?
	//<startIndex> is starting element in index array buffer that you want to use
	int DrawFace(unsigned int startIndex,unsigned int numTriangles ,bool isTriPrimitive);
};

/*----------openGL stuffs---------------*/
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_STREAM_DRAW 0x88E0
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_STATIC_DRAW 0x88E4

typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);

typedef void (APIENTRY * PFNGLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
typedef void (APIENTRY * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
typedef void (APIENTRY * PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
typedef void (APIENTRY * PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);
typedef GLvoid* (APIENTRY * PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
typedef GLboolean (APIENTRY * PFNGLUNMAPBUFFERPROC) (GLenum target);

extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLMAPBUFFERPROC glMapBuffer;
extern PFNGLUNMAPBUFFERPROC glUnmapBuffer;

extern PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

#endif