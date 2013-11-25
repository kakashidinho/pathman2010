// Renderer.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "RendererImp.h"

PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLMAPBUFFERPROC glMapBuffer;
PFNGLUNMAPBUFFERPROC glUnmapBuffer;

PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

/*-------------------create object and return interface pointer------------------------------------*/

Renderer *CreateRenderer(unsigned int sWidth, unsigned int sHeight)
{
	RendererImp* renderer=new RendererImp(sWidth,sHeight);
	return renderer;
}
/*-------------------------------------------------------*/
RendererImp::RendererImp(unsigned int sWidth, unsigned int sHeight)
{
	this->flags=0;
	this->sWidth=sWidth;
	this->sHeight=sHeight;
	
	this->currentVBuffer = INVALID_ID;
	this->currentIBuffer = INVALID_ID;

	this->Init();//init default openGL state
}

RendererImp::~RendererImp()
{
}

int RendererImp::Release()
{
	delete this;
	return R_OK;
}

int RendererImp::LoadTextureFromFile(const char* imgFile,unsigned int* pTextureID)//load texture from file,only support tga image
{
	return textureMan.LoadTextureFromFile(imgFile,pTextureID);
};

int RendererImp::LoadTextureFromMemory(const unsigned char* byteStream,unsigned int streamSize,unsigned int* pTextureID)//load texture from byte stream,only support tga image
{
	return textureMan.LoadTextureFromMemory(byteStream,streamSize,pTextureID);
};

int RendererImp::ReleaseAllTexture()//release all textures
{
	return textureMan.ReleaseAllTexture();
};

int RendererImp::ReleaseTexture(unsigned int id)//release texture
{
	return textureMan.ReleaseTexture(id);
};

int RendererImp::GetTextureImageRect(unsigned int textureID,Rect &rect)
{
	SharedPtr<Texture> pTex = textureMan.GetTexture(textureID);
	if(pTex == NULL)
		return TEXTURE_NOT_AVAIL;
	rect.left = 0;
	rect.right = pTex->width;
	rect.top = 0;
	rect.bottom = pTex->height;

	return R_OK;
}

int RendererImp::SetTexture(unsigned int textureID)//set current active texture for rendering
{
	return textureMan.SetTexture(textureID);
}

void RendererImp::Init2DMatrices()
{
	//setup 2D matrices
	Matrix4OrthoProjRH((float)sWidth,(float)sHeight,0.0f,1.0f,&this->ortho2DMatrix);
	Matrix4View(&XaxisPos,&YaxisPos,&ZaxisPos,&Vector4(sWidth/2.0f,sHeight/2.0f,0.0f,1.0f),&this->view2DMatrix);
}

void RendererImp::Init()
{
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	this->Init2DMatrices();//init matrices
	
	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	
	//enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//enable texture mapping
	glEnable(GL_TEXTURE_2D);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	//gouraud shading model
	glShadeModel(GL_SMOOTH);
	//default alpha blend function
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	/*----------------extension string------------*/
	char * extensions = (char*)glGetString(GL_EXTENSIONS);
	/*---check if support non power of 2 texture----*/
	if(strstr(extensions,"GL_ARB_texture_non_power_of_two"))
		textureMan.npot=true;
	/*---check if support swap interval control----*/
	if(strstr(extensions,"WGL_EXT_swap_control"))
	{
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		if(wglSwapIntervalEXT)//not null
		{
			flags |= SWAP_CONTROL;
			wglSwapIntervalEXT(0);//not use vsync
		}
	}
	/*---check if support vertex buffer object----*/
	char * version = (char*)glGetString(GL_VERSION);
	float GLversion=1.0f;
	sscanf(version,"%f",&GLversion);
	if(GLversion >= 1.5f)
		goto supportVBO;//VBO is core in 1.5 and later

	
	if(strstr(extensions,"GL_ARB_vertex_buffer_object"))
	{
supportVBO:
		glGenBuffers=(PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
		if(!glGenBuffers)
		{
			glGenBuffers=(PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffersARB");
			if(!glGenBuffers)
				goto next;
		}

		glBindBuffer=(PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
		if(!glBindBuffer)
		{
			glBindBuffer=(PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBufferARB");
			if(!glBindBuffer)
				goto next;
		}

		glBufferData=(PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
		if(!glBufferData)
		{
			glBufferData=(PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferDataARB");
			if(!glBufferData)
				goto next;
		}

		glBufferSubData=(PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
		if(!glBufferSubData)
		{
			glBufferSubData=(PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubDataARB");
			if(!glBufferSubData)
				goto next;
		}

		glDeleteBuffers=(PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
		if(!glDeleteBuffers)
		{
			glDeleteBuffers=(PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffersARB");
			if(!glDeleteBuffers)
				goto next;
		}

		glMapBuffer=(PFNGLMAPBUFFERPROC)wglGetProcAddress("glMapBuffer");
		if(!glMapBuffer)
		{
			glMapBuffer=(PFNGLMAPBUFFERPROC)wglGetProcAddress("glMapBufferARB");
			if(!glMapBuffer)
				goto next;
		}

		glUnmapBuffer=(PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");
		if(!glUnmapBuffer)
		{
			glUnmapBuffer=(PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBufferARB");
			if(!glUnmapBuffer)
				goto next;
		}

		flags |= VBO;
	}
next:

	return;
}

void RendererImp::BeginRender()
{
	if(flags & RENDERING)//not call EndRender() before this call
		return;
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	flags |= RENDERING;//turn "rendering" flag on
}

void RendererImp::EndRender()
{
	if(!(flags & RENDERING))//not call BeginRender() before call this function
		return;

	glFlush();

	flags &=(~RENDERING);//remove "rendering" flag
}
void RendererImp::SetViewport(Rect *rect)
{
	if(rect == NULL)
		glViewport(0,0,sWidth,sHeight);
	else
		glViewport(rect->left,rect->bottom,rect->right-rect->left,rect->top-rect->bottom);
}
void RendererImp::SetViewport1(int left,int right,int bottom,int top)
{
	glViewport(left,bottom,right-left,top-bottom);
}
void RendererImp::SetViewport2(int x,int y,int width,int height)
{
	glViewport(x,y,width,height);
}

void RendererImp::EnableLighting()
{
	if(!(flags & LIGHTING))
	{
		glEnable(GL_LIGHTING);
		flags |= LIGHTING;
	}
}
void RendererImp::DisableLighting()
{
	if((flags & LIGHTING))
	{
		glDisable(GL_LIGHTING);
		flags &= (~LIGHTING);
	}
}

void RendererImp::EnableLightSource(unsigned int sourceIndex)
{
	glEnable(GL_LIGHT0 + sourceIndex);
}
void RendererImp::DisableLightSource(unsigned int sourceIndex)
{
	glDisable(GL_LIGHT0 + sourceIndex);
}

void RendererImp::SetupDirectionLight(unsigned int lightIndex,DirectionLight& dlight)
{
	float lightDirection[] = {-dlight.direction[0],-dlight.direction[1],-dlight.direction[2],0};
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glLightfv(GL_LIGHT0 + lightIndex,GL_POSITION,lightDirection);
	glLightfv(GL_LIGHT0 + lightIndex,GL_AMBIENT,dlight.ambient.c);
	glLightfv(GL_LIGHT0 + lightIndex,GL_DIFFUSE,dlight.diffuse.c);
	glLightfv(GL_LIGHT0 + lightIndex,GL_SPECULAR,dlight.specular.c);
	
	glPopMatrix();
}

void RendererImp::EnableAlphaBlend()
{
	if(!(flags & ALPHA_BLEND))
	{
		glEnable(GL_BLEND);
		flags |= ALPHA_BLEND;
	}
}
void RendererImp::DisableAlphaBlend()
{
	if((flags & ALPHA_BLEND))
	{
		glDisable(GL_BLEND);
		flags &= (~ALPHA_BLEND);
	}
}


void RendererImp::EnableDepthTest()
{
	if(!(flags & DEPTH_TEST))
	{
		glEnable(GL_DEPTH_TEST);
		flags |= DEPTH_TEST;
	}
}
void RendererImp::DisableDepthTest()
{
	if((flags & DEPTH_TEST))
	{
		glDisable(GL_DEPTH_TEST);
		flags &= (~DEPTH_TEST);
	}
}


void RendererImp::EnableCullBackFace()
{
	if(!(flags & CULL))
	{
		glEnable(GL_CULL_FACE);
		flags |= CULL;
	}
}
void RendererImp::DisableCullBackFace()
{
	if((flags & CULL))
	{
		glDisable(GL_CULL_FACE);
		flags &= (~CULL);
	}
}


bool RendererImp::SetViewMatrix(const float *matrix)
{
	if(flags & _2D_MODE || flags & TEXT_MODE)
		return false;
	memcpy(viewMatrix,matrix,sizeof(Matrix4x4));
	CalModelViewMatrix();
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelViewMatrix);
	return true;
}
bool RendererImp::SetWorldMatrix(const float *matrix)
{
	if(flags & _2D_MODE || flags & TEXT_MODE)
		return false;
	if(matrix == NULL)
		worldMatrix.Identity();
	else
		memcpy(worldMatrix,matrix,sizeof(Matrix4x4));
	CalModelViewMatrix();
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelViewMatrix);
	return true;
}
bool RendererImp::SetProjectionMatrix(const float *matrix)
{
	if(flags & _2D_MODE || flags & TEXT_MODE)
		return false;
	memcpy(projMatrix,matrix,sizeof(Matrix4x4));
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projMatrix);
	return true;
}

void RendererImp::CalModelViewMatrix()
{
	modelViewMatrix = worldMatrix * viewMatrix;
}

void RendererImp::SetMaterial(Material &mat)
{
	glMaterialfv(GL_FRONT,GL_AMBIENT,mat.ambient.c);
	glMaterialfv(GL_FRONT,GL_DIFFUSE,mat.diffuse.c);
	glMaterialfv(GL_FRONT,GL_SPECULAR,mat.specular.c);
	glMaterialf(GL_FRONT,GL_SHININESS,mat.shininess);
}

int RendererImp::Enable2DMode()
{
	if(flags & TEXT_MODE)//renderer in text mode , can't enter 2D mode
		return R_FAILED_CANT_SWITCH_TEXT_TO_2D_MODE;
	if(flags & _2D_MODE)//already in 2D mode
		return R_OK;

	//switch to 2D mode
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(this->ortho2DMatrix);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(this->view2DMatrix);
	
	if(!(flags & ALPHA_BLEND))
		glEnable(GL_BLEND);

	flags |= _2D_MODE;

	return R_OK;
}

int RendererImp::Disable2DMode()
{
	if((flags & _2D_MODE) == 0 || (flags & TEXT_MODE)!=0 )
		return R_FAILED;

	//switch back to normal mode
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	flags &=(~_2D_MODE);
	
	if(!(flags & ALPHA_BLEND))
		glDisable(GL_BLEND);

	textureMan.ReActiveTexture();

	return R_OK;
}

int RendererImp::EnableTextMode()
{
	if(flags & TEXT_MODE)//already in text mode
		return R_OK;
	if(flags & CULL)
		glDisable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	if(flags & LIGHTING)
		glDisable(GL_LIGHTING);//don't need light shading

	if(!(flags & _2D_MODE))//not in 2D mode , if already in 2D mode, matrices are already switched to 2D matrices so we don't need to switch them again
	{
		//switch to 2D matrices
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixf(this->ortho2DMatrix);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadMatrixf(this->view2DMatrix);

		//enable alpha blend
		if(!(flags & ALPHA_BLEND))
			glEnable(GL_BLEND);
	}
	
	flags |= TEXT_MODE;

	return R_OK;
}

int RendererImp::DisableTextMode()
{
	if((flags & TEXT_MODE) == 0 )//not in text mode
		return R_FAILED;
	
	if(flags & CULL)
		glEnable(GL_CULL_FACE);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	//switch back to normal 3D matrices if not in 2D mode,if already in 2D mode ,matrices will be switched by calling Disable2DMode()
	if(!(flags & _2D_MODE))
	{
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		//disable alpha blend
		if(!(flags & ALPHA_BLEND))
			glDisable(GL_BLEND);

		textureMan.ReActiveTexture();
	}
	if(flags & LIGHTING)
		glEnable(GL_LIGHTING);
	
	this->ReActiveVertexBuffer();

	flags &=(~TEXT_MODE);

	return R_OK;
}

RenderMode RendererImp::GetCurrentMode()
{
	if(flags & TEXT_MODE)
		return R_TEXT;
	if (flags & _2D_MODE)
		return R_2D;
	return R_3D;
}



int RendererImp::BlitTextureToScreen(unsigned int textureID,float depth,const Rect *pTexRect,const Rect *pDestRect)
{
	if((flags & _2D_MODE)==0 || (flags & TEXT_MODE)!=0)
		return R_FAILED_NOT_IN_2D_MODE;
#if defined(_DEBUG) || defined(DEBUG)
	glGetError();//reset error state
#endif
	
	if(depth < 0.0f)
		depth = 0.0f;
	if(depth > 1.0f)
		depth = 1.0f;

	SharedPtr<Texture> pTex=textureMan.GetTexture(textureID);
	if(pTex==NULL)
		return TEXTURE_NOT_AVAIL;//invalid texture ID
	if(flags & LIGHTING)
		glDisable(GL_LIGHTING);//don't need light shading

	//render quad with texture
	glBindTexture(GL_TEXTURE_2D,pTex->texture);

	//texRect is in image space 
	Rect texRect={0,pTex->width,pTex->height,0};
	//destRect is in window space
	Rect destRect={0,sWidth,0,sHeight};

	if(pTexRect!=NULL)
	{
		texRect=*pTexRect;

	}
	if(pDestRect!=NULL)
	{
		destRect=*pDestRect;
	}
	/*image space origin is opposite to texture space origin ,so need to take care*/
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f((float)texRect.left/pTex->width , 1.0f - (float)texRect.bottom/pTex->height ); 
	glVertex3f((float)destRect.left , (float)destRect.bottom, -depth);
	glTexCoord2f((float)texRect.right/pTex->width  , 1.0f - (float)texRect.bottom/pTex->height ); 
	glVertex3f((float)destRect.right , (float)destRect.bottom, -depth);
	glTexCoord2f((float)texRect.left/pTex->width  , 1.0f - (float)texRect.top/pTex->height ); 
	glVertex3f((float)destRect.left , (float)destRect.top, -depth);
	glTexCoord2f((float)texRect.right/pTex->width , 1.0f - (float)texRect.top/pTex->height ); 
	glVertex3f((float)destRect.right , (float)destRect.top, -depth);
	glEnd();
	

	if(flags & LIGHTING)
		glEnable(GL_LIGHTING);

#if defined(_DEBUG) || defined(DEBUG)
	if(glGetError()!=0)//get error report
		return R_FAILED;
#endif
	return R_OK;
}

int RendererImp::FadingScreen(float fadeLevel)
{
	if(fadeLevel == 0.0f)
		return R_FAILED_NOT_IN_2D_MODE;
	if(!(flags & RENDERING))
		return R_FAILED_NOT_BEGIN_RENDER;
	bool _2DMode=true;
	if(!(flags & _2D_MODE))
	{
		_2DMode = false;
		if(this->Enable2DMode()==R_FAILED_CANT_SWITCH_TEXT_TO_2D_MODE)
			return R_FAILED_CANT_SWITCH_TEXT_TO_2D_MODE;//in text mode,can't do this operation
	}
	if(flags & LIGHTING)
		glDisable(GL_LIGHTING);//don't need light shading
	glDisable(GL_TEXTURE_2D);
	
	glColor4f(0.0f,0.0f,0.0f,fadeLevel);

	glBegin(GL_TRIANGLE_STRIP);
	glVertex3i(0,0,0);
	glVertex3i(sWidth,0,0);
	glVertex3i(0,sHeight,0);
	glVertex3i(sWidth,sHeight,0);
	glEnd();

	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glEnable(GL_TEXTURE_2D);
	if(flags & LIGHTING)
		glEnable(GL_LIGHTING);

	if(!_2DMode)
		this->Disable2DMode();
	return R_OK;
}