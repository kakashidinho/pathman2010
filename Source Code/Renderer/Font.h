#ifndef _FONT_
#define _FONT_

#include "../Renderer.h"
#include "../Util.h"
#include "TextureManager.h"

struct Glyph
{
	/*--------------the following values are used in normalize texture coordinate space--*/ 
	float leftU;//leftmost texture coordinate
	float rightU;//rightmost texture coordinate
	float bottomV;//bottommost texture coordinate
	float topV;//topmost texture coordinate
	
	/*------------the following values are used in image space------*/
	/*------offsets of glypth quad's origin (bottom left)---------------------------------*/ 
	float xOffset;//signed distance from location that will insert this glyph
	float yOffset;//signed distance from main line of the text that contains this glyph
	//width of character
	int width;
	//height of character
	int height;
};


/*--------------------------------------
basic font class - handles 128 ASCII
character set
--------------------------------------*/
class Font
{
private :
	bool bBufferSupport;//support vertex buffer object?
	
	unsigned int numIndices;
	unsigned int maxVertices;//for batched dynamic rendering
	unsigned int maxInBatch;//for batched dynamic rendering
	unsigned int offset;//for batched dynamic rendering
	
	void *pvBuffer;//pointer to a vertex buffer (if supported) or simply a vertex array (if not support vertex buffer object)
	void *piBuffer;//pointer to a index buffer (if vertex buffer supported) or simply a index array (if not support vertex buffer object)

	TextureManager* texMan;
	unsigned int textureID;//texture ID
	Glyph glyphs[128];//128 glyphs
	int lineDistance;//distance between 2 lines (in image space)
	
	void SetupVertexBuffer();
	void DrawUsingVertexBuffer(const char *String,float X,float Y,float spaceBetweenChar);
	void DrawUsingVertexArray(const char *String,float X,float Y,float spaceBetweenChar);//in case vertex buffer object not supported in openGL
public:
	Font(const char *iniFile,const char* imageFile,TextureManager* texMan,bool vbo);
	Font(const unsigned char* fontInfoByteStream,//font info byte stream
		 unsigned int fontInfoStreamSize,//size of info byte stream
		 const unsigned char *imageDataByteStream,//image byte stream used for load texture
		 unsigned int imageDataSize,//size of image byte stream
		 TextureManager* texMan,//texture manager
		 bool vbo);//support vertex buffer object ?
	~Font();
	
	float GetStringLength(const char* String,float spaceBetweenChar=2.0f);//get string length of it is displayed to screen.This string must not have line breaking char such as '\n'
	int GetFontLineDistance(){return lineDistance;};
	void DrawString(const char *String,float X,float Y,float spaceBetweenChar = 2.0f);//draw a string ,(X,Y) is bottom left location ,redender must already set to 2D mode before this call
};


#endif