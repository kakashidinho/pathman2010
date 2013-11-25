#include "stdafx.h"
#include "TextureManager.h"
/*--------------------------------------*/
Texture::~Texture()
{
	glDeleteTextures(1,&texture);
}
/*------------------------------------------
check if integer <d> is power of two,
<pExponent> - exponent of nearest power of 2
integer that larger than or equal to <d>
----------------------------------------*/

bool IsPowerOfTwo(const unsigned int d,unsigned int *pExponent)
{
	unsigned int i=d;
	if(i==0)//zero case
	{
		if(pExponent)
			*pExponent=0;
		return false;
	}
	unsigned int result=31;
	if((i & 0xffffff00) == 0)//this integer only has max 8 bits
	{
		i <<= 24;
		result=7;
	}
	else if ((i & 0xffff0000) == 0)//this integer only has max 16 bits
	{
		i <<=16;
		result=15;
	}
	else if ((i & 0xff000000) == 0)//this integer only has max 24 bits
	{
		i <<=8;
		result=23;
	}

	if((i & 0xf0000000)==0)//this integer only has max 28 bits
	{
		i <<=4;
		result-=4;
	}
	while ((i & 0x80000000) == 0)
	{
		i <<=1;
		result-=1;
	}
	if( i & 0x7fffffff )
	{
		if(pExponent)
			*pExponent=result+1;//this is exponent of nearest power of 2 integer that larger than d
		return false;
	}
	if(pExponent)
		*pExponent=result;//same as log2(d);
	return true;
}
/*------------------------------------*/
TextureManager::TextureManager():ItemManager()
{
	this->npot=false;
	currentActiveTexture = 0;

	//create dummy texture
	unsigned char dummyPixel[] = {255,255,255,255};//white color
	
	Texture *pNewTexture=new Texture();
	pNewTexture->width = 0;
	pNewTexture->height = 0;
	

	glGenTextures(1,&pNewTexture->texture);      
	glBindTexture(GL_TEXTURE_2D,pNewTexture->texture); 
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_POINT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_POINT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
	
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1,1,0
		,GL_RGBA,GL_UNSIGNED_BYTE,dummyPixel);

	this->AddItem(pNewTexture,0);

}

TextureManager::~TextureManager()
{
}

int TextureManager::ReleaseAllTexture()
{
	//release all texture except dummy texture
	for(unsigned int id = 1; id < allocSlots;++id)
		this->ReleaseSlot(id);
	return R_OK;
}

int TextureManager::LoadTextureFromFile(const char *imgFile, unsigned int *pTextureID)
{
	int re=tgaLoader.LoadFromFile(imgFile);//load pixel data from image file
	tgaLoader.FlipRGB();

	//error
	if (re==IMG_FAIL_FILE_NOT_EXIST)
		return TEXTURE_CANT_LOAD_FROM_FILE_NOT_EXIST;
	if(re==IMG_FAIL_MEM_ALLOC)
		return TEXTURE_CANT_LOAD_MEM_FAILED;
	if(re==IMG_FAIL_BAD_FORMAT || re==IMG_FAIL_NOT_SUPPORTED)
		return TEXTURE_CANT_LOAD_FILE_NOT_SUPPORT;

	//create texture object
	Texture *pNewTexture=new Texture();

	/*----------save image original size--------*/
	unsigned int w=pNewTexture->width=tgaLoader.GetWidth();
	unsigned int h=pNewTexture->height=tgaLoader.GetHeight();
	
	
	if(!npot)//not support non power of 2
	{
		unsigned int exp;
		bool needResize=false;
		if(!IsPowerOfTwo(w,&exp))//width is not power of 2
		{
			needResize=true;
			w=0x1 << exp;//2^exp
		}
		if(!IsPowerOfTwo(h,&exp))//height is not power of 2
		{
			needResize=true;
			h=0x1 << exp;//2^exp
		}
		if(needResize)//need to resize to nearest power of 2 size
		{
			//scale image
			if(tgaLoader.Scalei(w,h)!=IMG_OK)
			{
				delete pNewTexture;
				return TEXTURE_CANT_LOAD_FILE_NOT_SUPPORT;
			}
		}
	}//if (power of two)

	glGenTextures(1,&pNewTexture->texture);      
	glBindTexture(GL_TEXTURE_2D,pNewTexture->texture); 
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
	switch(tgaLoader.GetBpp())//bits per pixel
	{
	case 24:// R8G8B8
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w,h,0
			,GL_RGB,GL_UNSIGNED_BYTE,tgaLoader.GetData());
		break;
	case 32://  R8G8B8A8
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0
			,GL_RGBA,GL_UNSIGNED_BYTE,tgaLoader.GetData());
		break;
	default:
		delete pNewTexture;
		return TEXTURE_CANT_LOAD_FILE_NOT_SUPPORT;//not support formats other than 2 above
	}
	
	if(!this->AddItem(pNewTexture,pTextureID))
	{
		delete pNewTexture;
		return TEXTURE_CANT_LOAD_MEM_FAILED;
	}

	return R_OK;
}

int TextureManager::LoadTextureFromMemory(const unsigned char* byteStream,unsigned int streamSize,unsigned int* pTextureID)
{
	int re=tgaLoader.LoadFromMemory(byteStream,streamSize);//load pixel data from byte stream
	tgaLoader.FlipRGB();

	//error
	if (re==IMG_FAIL_FILE_NOT_EXIST)
		return TEXTURE_CANT_LOAD_FROM_INVALID_BYTE_STREAM;
	if(re==IMG_FAIL_MEM_ALLOC)
		return TEXTURE_CANT_LOAD_MEM_FAILED;
	if(re==IMG_FAIL_BAD_FORMAT || re==IMG_FAIL_NOT_SUPPORTED)
		return TEXTURE_CANT_LOAD_FILE_NOT_SUPPORT;


	//create texture object
	Texture *pNewTexture=new Texture();
	
	/*----------save image original size--------*/
	unsigned int w=pNewTexture->width=tgaLoader.GetWidth();
	unsigned int h=pNewTexture->height=tgaLoader.GetHeight();
	
	
	if(!npot)//not support non power of 2
	{
		unsigned int exp;
		bool needResize=false;
		if(!IsPowerOfTwo(w,&exp))//width is not power of 2
		{
			needResize=true;
			w=0x1 << exp;//2^exp
		}
		if(!IsPowerOfTwo(h,&exp))//height is not power of 2
		{
			needResize=true;
			h=0x1 << exp;//2^exp
		}
		if(needResize)//need to resize to nearest power of 2 size
		{
			//scale image
			if(tgaLoader.Scalei(w,h)!=IMG_OK)
			{
				delete pNewTexture;
				return TEXTURE_CANT_LOAD_FILE_NOT_SUPPORT;
			}
		}
	}//if (power of two)

	glGenTextures(1,&pNewTexture->texture);      
	glBindTexture(GL_TEXTURE_2D,pNewTexture->texture); 
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
	switch(tgaLoader.GetBpp())//bits per pixel
	{
	case 24:// R8G8B8
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w,h,0
			,GL_RGB,GL_UNSIGNED_BYTE,tgaLoader.GetData());
		break;
	case 32://  R8G8B8A8
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0
			,GL_RGBA,GL_UNSIGNED_BYTE,tgaLoader.GetData());
		break;
	default:
		delete pNewTexture;
		return TEXTURE_CANT_LOAD_FROM_INVALID_BYTE_STREAM;//not support formats other than 2 above
	}
	if(!this->AddItem(pNewTexture,pTextureID))
	{
		delete pNewTexture;
		return TEXTURE_CANT_LOAD_MEM_FAILED;
	}

	return R_OK;
}

int TextureManager::ReleaseTexture(unsigned int ID)
{
	if(this->ReleaseSlot(ID) == ID_NOT_FOUND)
		return TEXTURE_NOT_AVAIL;

	return R_OK;
}


SharedPtr<Texture> TextureManager::GetTexture(unsigned int textureID)
{
	return this->GetItemPointer(textureID);
}

int TextureManager::SetTexture(unsigned int textureID)//set current active texture for rendering
{
	if(textureID == currentActiveTexture)
		return R_OK;
	SharedPtr<Texture> ptr = this->GetItemPointer(textureID);
	if(ptr == NULL)
		return TEXTURE_NOT_AVAIL;
	
	currentActiveTexture = textureID;
	glBindTexture(GL_TEXTURE_2D,ptr->texture);
	
	return R_OK;
}

void TextureManager::ReActiveTexture()
{
	SharedPtr<Texture> ptr = this->GetItemPointer(currentActiveTexture);
	if(ptr == NULL)
		return ;
	
	glBindTexture(GL_TEXTURE_2D,ptr->texture);
}