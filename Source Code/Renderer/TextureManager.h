#ifndef _TEXTURE_MANAGER_
#define _TEXTURE_MANAGER_

#include "GL/gl.h"
#include "TgaLoader.h"
#include "../ItemManager.h"
struct Texture
{
	GLuint texture;
	/*----------info of image that texture loaded from------*/
	unsigned int width;//image's width
	unsigned int height;//image's height
	~Texture();
};

/*--------------------------------------------------
class TextureManager - take responsibility for loading
,storing,managing texture objects
--------------------------------------------------*/

class TextureManager:public ItemManager<Texture>
{
	friend class RendererImp;
private:
	bool npot;//support non power of 2 texture

	TgaLoader tgaLoader;
	unsigned int currentActiveTexture ; //id of current active texture
public:
	TextureManager();
	~TextureManager();

	int LoadTextureFromFile(const char* imgFile,unsigned int* pTextureID);//load texture from file,only support tga image
	int LoadTextureFromMemory(const unsigned char* byteStream,unsigned int streamSize,unsigned int* pTextureID);//load texture from byte stream,only support tga image
	int ReleaseAllTexture();//release all textures
	int ReleaseTexture(unsigned int textureID);//release texture that has ID <textureID>
	
	int SetTexture(unsigned int textureID);//set current active texture for rendering
	void ReActiveTexture();//re active current active texture after renderer returns to normal mode from text mode or 2d mode.

	SharedPtr<Texture> GetTexture(unsigned int textureID);//get texture object's pointer,given its ID
};

#endif