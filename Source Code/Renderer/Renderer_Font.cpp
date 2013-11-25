#include "stdafx.h"
#include "RendererImp.h"
#include <iostream>
#include <fstream>
/*-------FontVertex---------------*/
struct FontVertex
{
	float x,y;//position
	float u,v;//texcoord
};
/*-------Font class---------------*/
Font::Font(const char *iniFile,const char* imageFile,TextureManager* texMan,bool vbo)
{
	memset(glyphs,0,128 * sizeof(Glyph));

	this->offset=0;

	this->lineDistance=0;

	bBufferSupport = vbo;
	
	this->texMan=texMan;
	
	//load texture
	texMan->LoadTextureFromFile(imageFile,&textureID);
	SharedPtr<Texture> pTex = texMan->GetTexture(textureID);

	char line[100];
	
	std::ifstream stream;
	stream.open(iniFile);
	/*------parse file-----------*/
	stream.getline(line,100);
	sscanf(line ,"maxVertices %u",&maxVertices);//max vertices in vertex buffer

	stream.getline(line,100);
	sscanf(line ,"Batch %u",&maxInBatch);//max vertices in batch that will be sent to vertex buffer
	
	//get line distance
	stream.getline(line,100);
	sscanf(line,"lineDistance %d",&lineDistance);

	stream.getline(line,100);//do nothing with this line
	
	char c;
	int left,right,bottom,top,xOffset,yOffset;
	//begin parse character info
	bool specialChar = false;
	
	while(!stream.eof())
	{
		stream.getline(line,100);
		if(!specialChar)
		{
			if(!strncmp(line,"special char	width	comment",26))//from now on we will parse special character
			{
				specialChar=true;
				continue;
			}
			sscanf(line,"%c %d %d %d %d %d %d",&c,&left,&right,&bottom,&top,&xOffset,&yOffset);
			glyphs[c].xOffset=xOffset;
			glyphs[c].yOffset=yOffset;
			glyphs[c].leftU = ((float)left/pTex->width);
			glyphs[c].rightU = ((float)right/pTex->width);
			glyphs[c].bottomV = 1.0f -((float)bottom/pTex->height);
			glyphs[c].topV = 1.0f - ((float)top/pTex->height);
			glyphs[c].width = right - left;
			glyphs[c].height = bottom - top;

		}
		else//special character is character that doesn't has image ,such as space ,tab ...v.v.v
		{
			/*--------------------------------------------------------------------------------
			in this case <left> & <right> variables not mean left & right ,
			they're used for holding character ASCII code & width  value of special character
			--------------------------------------------------------------------------------*/
			sscanf(line,"%d %d",&left,&right);
			glyphs[left].width = right;
			glyphs[left].leftU= -1.0f;//special mark for special char
		}
	}
	
	/*----done parsing--------------------*/
	stream.close();

	this->SetupVertexBuffer();
}
Font::Font(const unsigned char* fontInfoByteStream,//font info byte stream
		   unsigned int fontInfoStreamSize,//size of info byte stream
		   const unsigned char *imageDataByteStream,//image byte stream used for load texture
		   unsigned int imageDataSize,//size of image byte stream
		   TextureManager* texMan,//texture manager
		   bool vbo)//support vertex buffer object ?
{
	memset(glyphs,0,128 * sizeof(Glyph));

	this->offset=0;

	this->lineDistance=0;

	bBufferSupport = vbo;
	
	this->texMan=texMan;
	
	//load texture
	texMan->LoadTextureFromMemory(imageDataByteStream,imageDataSize,&textureID);
	SharedPtr<Texture> pTex = texMan->GetTexture(textureID);

	char line[100];

	CStringStream stream((char*)fontInfoByteStream,fontInfoStreamSize);
	/*------parse string stream-----------*/
	stream.GetLine(line,100);
	sscanf(line ,"maxVertices %u",&maxVertices);//max vertices in vertex buffer

	stream.GetLine(line,100);
	sscanf(line ,"Batch %u",&maxInBatch);//max vertices in batch that will be sent to vertex buffer
	
	//get line distance
	stream.GetLine(line,100);
	sscanf(line,"lineDistance %d",&lineDistance);

	stream.GetLine(line,100);//do nothing with this line
	
	char c;
	int left,right,bottom,top,xOffset,yOffset;
	//begin parse character info
	bool specialChar = false;
	
	while(stream.GetLine(line,100)!=END_OF_STREAM)
	{
		if(!specialChar)
		{
			if(!strncmp(line,"special char	width	comment",26))//from now on we will parse special character
			{
				specialChar=true;
				continue;
			}
			sscanf(line,"%c %d %d %d %d %d %d",&c,&left,&right,&bottom,&top,&xOffset,&yOffset);
			glyphs[c].xOffset=xOffset;
			glyphs[c].yOffset=yOffset;
			glyphs[c].leftU = ((float)left/pTex->width);
			glyphs[c].rightU = ((float)right/pTex->width);
			glyphs[c].bottomV =1.0f - ((float)bottom/pTex->height);
			glyphs[c].topV =1.0f - ((float)top/pTex->height);
			glyphs[c].width = right - left;
			glyphs[c].height = bottom - top;

		}
		else//special character is character that doesn't has image ,such as space ,tab ...v.v.v
		{
			/*--------------------------------------------------------------------------------
			in this case <left> & <right> variables not mean left & right ,
			they're used for holding character ASCII code & width  value of special character
			--------------------------------------------------------------------------------*/
			sscanf(line,"%d %d",&left,&right);
			glyphs[left].width = right;
			glyphs[left].leftU= -1.0f;//special mark for special char
		}
	}
	
	/*----done parsing--------------------*/

	this->SetupVertexBuffer();
}

Font::~Font()
{
	if(bBufferSupport)//we are using vertex buffer object
	{
		if(pvBuffer!=NULL)
		{
			glDeleteBuffers(1,(GLuint*)pvBuffer);
			delete (GLuint*)this->pvBuffer;
			this->pvBuffer=NULL;
		}
		if(piBuffer!=NULL)
		{
			glDeleteBuffers(1,(GLuint*)piBuffer);
			delete (GLuint*)this->piBuffer;
			this->piBuffer=NULL;
		}
	}
	else //we are using vertex array
	{
		delete[] (FontVertex*)pvBuffer;
        delete[] (unsigned short*)piBuffer;
	}
}

void Font::SetupVertexBuffer()
{
	numIndices = (maxInBatch/4)*6;//max number of indices in 1 batch
	unsigned short *pIndices=NULL;
	if(bBufferSupport)//vertex buffer object supported
	{
		
		//create vertex buffer object
		this->pvBuffer=new GLuint();

		glGenBuffers(1,(GLuint*)pvBuffer);
		glBindBuffer(GL_ARRAY_BUFFER,*(GLuint*)pvBuffer);
        glGetError();//reset error flag
		glBufferData(GL_ARRAY_BUFFER,maxVertices*sizeof(FontVertex),NULL,GL_DYNAMIC_DRAW);
		if(GL_OUT_OF_MEMORY == glGetError())
		{
			glDeleteBuffers(1,(GLuint*)pvBuffer);
			delete (GLuint*)pvBuffer;
			pvBuffer=0;
			return ;
		}
		//create index buffer object
		this->piBuffer=new GLuint();

		glGenBuffers(1,(GLuint*)piBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,*(GLuint*)piBuffer);
        glGetError();//reset error flag

		glBufferData(GL_ELEMENT_ARRAY_BUFFER,numIndices*sizeof(unsigned short),NULL,GL_STATIC_DRAW);
		if(GL_OUT_OF_MEMORY == glGetError())
		{
			glDeleteBuffers(1,(GLuint*)pvBuffer);
			delete (GLuint*)pvBuffer;
			pvBuffer=0;
			glDeleteBuffers(1,(GLuint*)piBuffer);
			delete (GLuint*)piBuffer;
			piBuffer=0;
		}
		pIndices = (unsigned short*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,GL_WRITE_ONLY);
	}//vertex buffer object supported
	else//not support vertex buffer
	{
		//create vertex array
		pvBuffer = new FontVertex[maxVertices];
		//create index array
		if(pvBuffer!=NULL)
		{
			piBuffer=new unsigned short[numIndices];
			if(piBuffer==NULL)
			{
                 delete[] (FontVertex*)pvBuffer;
				 pvBuffer = NULL;
			}
			pIndices = (unsigned short *)piBuffer;
		}
	}
	if(pIndices == NULL)
		return;
	//fill index buffer
	/*--------------------------------------

		i------i+1
		|\    |
		| \   |
		|  \  |		=>this quad has indices {i , i+1 , i+2 , //first triangle
		|   \ |								 i+2 , i+3 , i} //second triangle
	 i+3|____\|i+2

	---------------------------------------*/
	unsigned int i=0;unsigned int j=0;
	for(;j<numIndices;i+=4)
	{
		//first triangle
		pIndices[j] = i;
		pIndices[++j] = i+1;
		pIndices[++j] = i+2;
		//second triangle
		pIndices[++j] = i+2;
		pIndices[++j] = i+3;
		pIndices[++j] = i;
		j++;
	}

	if(bBufferSupport)//we are using index buffer object and mapped buffer before so we need to unmap it
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

float Font::GetStringLength(const char* String,float spaceBetweenChar)
{
	if(String == NULL)
		return 0.0f;
	float length = 0;
	char c;
	for(unsigned int i=0;String[i]!='\0';++i)
	{
		c=String[i];
		if(c >=128)
			continue;

		if(glyphs[c].width == 0)
			continue;

		length += glyphs[c].width + spaceBetweenChar;
	}
	return length;
}

void Font::DrawUsingVertexBuffer(const char *String, float X, float Y, float spaceBetweenChar)
{
	unsigned char *pV;
	unsigned int sizeofQuad = 4*sizeof(FontVertex);
	glBindBuffer(GL_ARRAY_BUFFER,*(GLuint*)pvBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,*(GLuint*)piBuffer);
	if(offset == 0)//offset at the beginning of buffer
		glBufferData(GL_ARRAY_BUFFER,maxVertices*sizeof(FontVertex),NULL,GL_DYNAMIC_DRAW);//discard old data in buffer

	FontVertex v[4];
	//current location
	float x=X;
	float y=Y;
	//current char in string
	char c;
	//current number of vertices in batch
	unsigned int num=0;
	for(unsigned int i=0;String[i]!= '\0';++i)//for each char in string
	{
		c=String[i];
		if(c >=128)
			continue;
		if(c == '\n')//newline
		{
			x=X;
			y=y - this->lineDistance;//next height
		}
		if(glyphs[c].leftU == -1.0f)//this is special char
		{
			x+=glyphs[c].width + spaceBetweenChar;
			continue;
		}
		if(glyphs[c].width == 0)
			continue;

		/*-----------------
			0   1
			3	2
		-----------------*/

		float originX = x + glyphs[c].xOffset;
		float originY = y - glyphs[c].yOffset; 

		v[0].x=originX; 
		v[0].y=originY + glyphs[c].height; 
		v[0].u=glyphs[c].leftU; 
		v[0].v= glyphs[c].topV; 

		v[1].x=originX + glyphs[c].width; 
		v[1].y=originY + glyphs[c].height; 
		v[1].u=glyphs[c].rightU; 
		v[1].v= glyphs[c].topV; 

		v[2].x=originX + glyphs[c].width; 
		v[2].y=originY; 
		v[2].u=glyphs[c].rightU; 
		v[2].v= glyphs[c].bottomV ; 

		v[3].x=originX; 
		v[3].y=originY; 
		v[3].u=glyphs[c].leftU; 
		v[3].v= glyphs[c].bottomV;

		//v+=4;
		
		glBufferSubData(GL_ARRAY_BUFFER,(offset + num)*sizeof(FontVertex),sizeofQuad,v);

		x+=glyphs[c].width + spaceBetweenChar;

		num+=4;
		if(num == maxInBatch)//batch full ,so draw
		{
			pV = (unsigned char*)0 + sizeof(FontVertex)*offset;
			glVertexPointer(2,GL_FLOAT,sizeof(FontVertex),pV);
			glTexCoordPointer(2,GL_FLOAT,sizeof(FontVertex),pV + 2*sizeof(float));

			glDrawElements(GL_TRIANGLES,numIndices,GL_UNSIGNED_SHORT,0);

			num=0;//reset number of vertices in batch
			offset+=maxInBatch;//next batch
			if(offset >= maxVertices)//used all capacity
				offset = 0;//return to beginning batch
		}
	}

	if(num > 0)//last vertices in batch that are not rendered yet
	{
		pV = (unsigned char*)0 + sizeof(FontVertex)*offset;
		glVertexPointer(2,GL_FLOAT,sizeof(FontVertex),pV);
		glTexCoordPointer(2,GL_FLOAT,sizeof(FontVertex),pV + 2*sizeof(float));

		glDrawElements(GL_TRIANGLES,(num/4) * 6,GL_UNSIGNED_SHORT,0);		
		offset +=maxInBatch;
	}
}
void Font::DrawUsingVertexArray(const char *String, float X, float Y, float spaceBetweenChar)
{
	FontVertex * v = (FontVertex*)pvBuffer + offset;
	unsigned char* pV;
	//current location
	float x=X;
	float y=Y;
	//current char in string
	char c;
	//current number of vertices in batch
	unsigned int num=0;
	for(unsigned int i=0;String[i]!= '\0';++i)//for each char in string
	{
		c=String[i];
		if(c >=128)
			continue;
		if(c == '\n')//newline
		{
			x=X;
			y=y - this->lineDistance;//next height
		}
		if(glyphs[c].leftU == -1.0f)//this is special char
		{
			x+=glyphs[c].width + spaceBetweenChar;
			continue;
		}
		if(glyphs[c].width == 0)
			continue;

		/*-----------------
			0   1
			3	2
		-----------------*/

		float originX = x + glyphs[c].xOffset;
		float originY = y - glyphs[c].yOffset; 

		v[0].x=originX; 
		v[0].y=originY + glyphs[c].height; 
		v[0].u=glyphs[c].leftU; 
		v[0].v= glyphs[c].topV; 

		v[1].x=originX + glyphs[c].width; 
		v[1].y=originY + glyphs[c].height; 
		v[1].u=glyphs[c].rightU; 
		v[1].v= glyphs[c].topV; 

		v[2].x=originX + glyphs[c].width; 
		v[2].y=originY; 
		v[2].u=glyphs[c].rightU; 
		v[2].v= glyphs[c].bottomV ; 

		v[3].x=originX; 
		v[3].y=originY; 
		v[3].u=glyphs[c].leftU; 
		v[3].v= glyphs[c].bottomV;

		v+=4;
		
		x+=glyphs[c].width + spaceBetweenChar;

		num+=4;
		if(num == maxInBatch)//batch full ,so draw
		{
			pV = (unsigned char*)pvBuffer + sizeof(FontVertex)*offset;
			glVertexPointer(2,GL_FLOAT,sizeof(FontVertex),pV);
			glTexCoordPointer(2,GL_FLOAT,sizeof(FontVertex),pV + 2*sizeof(float));

			glDrawElements(GL_TRIANGLES,numIndices,GL_UNSIGNED_SHORT,piBuffer);

			num=0;//reset number of vertices in batch
			offset+=maxInBatch;//next batch
			if(offset >= maxVertices)//full
			{
				//return to the beginning of buffer
				offset = 0;
				v = (FontVertex*)pvBuffer;
			}
		}
	}

	if(num > 0)//we have some last vertices that are not rendered yet
	{
		pV = (unsigned char*)pvBuffer + sizeof(FontVertex)*offset;
		glVertexPointer(2,GL_FLOAT,sizeof(FontVertex),pV);
		glTexCoordPointer(2,GL_FLOAT,sizeof(FontVertex),pV + 2*sizeof(float));

		glDrawElements(GL_TRIANGLES,(num/4) * 6,GL_UNSIGNED_SHORT,piBuffer);		
		offset +=maxInBatch;
	}
}

void Font::DrawString(const char *String, float X, float Y,float spaceBetweenChar)
{
	if(!pvBuffer || !piBuffer)
		return;

	if(offset >= maxVertices)
	{
		offset=0;
	}
	
	glBindTexture(GL_TEXTURE_2D,texMan->GetTexture(textureID)->texture);

	if(bBufferSupport)
		this->DrawUsingVertexBuffer(String,X,Y,spaceBetweenChar);
	else 
		this->DrawUsingVertexArray(String,X,Y,spaceBetweenChar);
}

/*-------------font manager--------------*/
int RendererImp::CreateFontFromFile(const char* iniFile,const char * imageFile,unsigned int *pFontID)
{
	Font *pNewFont = new Font(iniFile,imageFile,&textureMan,(flags & VBO)!=0);

	if(!fontMan.AddItem(pNewFont,pFontID))
	{
		delete pNewFont;
		return R_FAILED;
	}
	
	return R_OK;
}
int RendererImp::CreateFontFromMemory(const unsigned char* initInfoByteStream,
									 unsigned int initInfoStreamSize,
									 const unsigned char * imageDataByteStream,
									 unsigned int imageDataSize,
									 unsigned int *pFontID)
{
	Font *pNewFont = new Font(initInfoByteStream,initInfoStreamSize,
						  imageDataByteStream,imageDataSize,
						  &textureMan,(flags & VBO)!=0);

	if(!fontMan.AddItem(pNewFont,pFontID))
	{
		delete pNewFont;
		return R_FAILED;
	}

	return R_OK;
}

int RendererImp::GetFontLineDistance(unsigned int fontID)
{
	SharedPtr<Font> ptr = fontMan.GetItemPointer(fontID);
	if(ptr == NULL)
		return 0;
	return ptr->GetFontLineDistance();
}


float RendererImp::GetStringLength(const char* String,unsigned int fontID,float spaceBetweenChar)
{
	SharedPtr<Font> ptr = fontMan.GetItemPointer(fontID);
	if(ptr == NULL)
		return 0.0f;
	return ptr->GetStringLength(String,spaceBetweenChar);
}

int RendererImp::DrawString(const char *String,unsigned int fontID, float X, float Y,float space)
{
	if(!(flags & TEXT_MODE))//not in text mode
		return R_FAILED_NOT_IN_TEXT_MODE;

	SharedPtr<Font> pFont = fontMan.GetItemPointer(fontID);
	if(pFont == NULL)
		return R_FAILED_FONT_NOT_EXISTED;
	pFont->DrawString(String,X,Y,space);
	return R_OK;
}