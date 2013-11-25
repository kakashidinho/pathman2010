#include "stdafx.h"
#include "../Renderer.h"
#include "../GUI.h"
Button::Button(void (*clickedHandler1)(void),void (*clickedHandler2)(void),
			   unsigned int animFrames,unsigned int framePerPose,
				unsigned int textureID1,Rect& rect1,
				unsigned int textureID2,Rect& rect2,
				Rect& buttonRect,Renderer *pRenderer):clickAnimCtrl(2)
{
	isActive = false;
	this->pRenderer=pRenderer;
	this->animFrames = animFrames;
	this->ClickedHandler1 = clickedHandler1;
	this->ClickedHandler2 = clickedHandler2;
	this->rect = buttonRect;
	//pose 1
	PoseInfo<ButtonAnimPose> *pPose = clickAnimCtrl.LoopPose();
	ButtonAnimPose *pInfo = &pPose->info;
	pInfo->rect = rect1;
	pInfo->textureID = textureID1;

	pPose->endFrame = framePerPose;

	//pose 2
	pPose = clickAnimCtrl.LoopPose();
	pInfo = &pPose->info;
	pInfo->rect = rect2;
	pInfo->textureID = textureID2;
	pPose->endFrame = 2*framePerPose;

	clickAnimCtrl.ResetCurrentPose();

}

Button::~Button()
{
}
bool Button::IsClicked(int X,int Y)
{
	if(!isActive || clickAnimCtrl.IsAnimating())
		return false;
	
	if(X<rect.left || X>rect.right || Y<rect.bottom || Y>rect.top)
		return false;

	clickAnimCtrl.StartAnimation(animFrames);
	if(ClickedHandler1)
		ClickedHandler1();//call clicked event handler
	return true;
}

void Button::UpdateStatus()
{
	if(clickAnimCtrl.IsAnimating())
	{
		clickAnimCtrl.LoopPose();
		if(!clickAnimCtrl.IsAnimating() && ClickedHandler2)//done animating
			ClickedHandler2();//call  callback function
	}
}
void Button::Draw()
{
	ButtonAnimPose *pInfo = &clickAnimCtrl.CurrentPoseInfo()->info;
	pRenderer->BlitTextureToScreen(pInfo->textureID,
								   0.0f,
								   &pInfo->rect,
								   &rect);
}

/*---------------------GUI manager----------------*/
/*---------button management-----------*/

int GUImanager::AddButton(void (*clickedHandler1)(void),void (*clickedHandler2)(void),
		   unsigned int animFrames,unsigned int framePerPose,
		   unsigned int textureID1,Rect& rect1,
		   unsigned int textureID2,Rect& rect2,
		   Rect& buttonRect,
		   unsigned int* pButtonID)
{
	Button *pNewButton = new Button(clickedHandler1,clickedHandler2,animFrames,framePerPose,textureID1,rect1,textureID2,rect2,buttonRect,pRenderer);
	if(!buttonMan.AddItem(pNewButton,pButtonID))
	{
		delete pNewButton;
		return 0;
	}
	return 1;
}

void GUImanager::DrawButtons()
{
	RenderMode mode = pRenderer->GetCurrentMode();
	bool in2DMode = (mode == R_2D);
	if(!in2DMode )//if not current in 2D mode
	{
		pRenderer->Enable2DMode();
	}

	
	ItemManager<Button>::Iterator ite;
	buttonMan.GetIterator(ite);
	while(!ite.isAtEnd())
	{
		ite->Draw();
		++ite;
	}

	if(!in2DMode )
	{
		pRenderer->Disable2DMode();
	}

}

void GUImanager::DrawButton(unsigned int id)
{
	RenderMode mode = pRenderer->GetCurrentMode();
	bool in2DMode = (mode == R_2D);
	if(!in2DMode )//if not current in 2D mode
	{
		pRenderer->Enable2DMode();
	}

	SharedPtr<Button> pButton = buttonMan.GetItemPointer(id);
	if(pButton!=NULL)
	{
		pButton->Draw();
	}

	if(!in2DMode )
	{
		pRenderer->Disable2DMode();
	}
}

bool GUImanager::IsButtonClicked(unsigned int buttonID,int x,int y)
{
	SharedPtr<Button> pButton = buttonMan.GetItemPointer(buttonID);
	if(pButton!=NULL)
		return pButton->IsClicked(x,y);
	return false;
}

bool GUImanager::IsAnyButtonClicked(int x,int y)
{
	ItemManager<Button>::Iterator ite;
	buttonMan.GetIterator(ite);
	while(!ite.isAtEnd())
	{
		if(ite->IsClicked(x,y))
			return true;
		++ite;
	}

	return false;
}

void GUImanager::UpdateButtonStatus()
{
	ItemManager<Button>::Iterator ite;
	buttonMan.GetIterator(ite);
	while(!ite.isAtEnd())
	{
		ite->UpdateStatus();
		++ite;
	}
}
void GUImanager::ActiveButton(unsigned int buttonID)
{
	SharedPtr<Button> pButton = buttonMan.GetItemPointer(buttonID);
	if(pButton!=NULL)
	{
		pButton->Active();
	}
}
void GUImanager::DeActiveButton(unsigned int buttonID)
{
	SharedPtr<Button> pButton = buttonMan.GetItemPointer(buttonID);
	if(pButton!=NULL)
	{
		pButton->DeActive();
	}
}
/*---------------Text Management-------------*/
int GUImanager::AddText(const char * text,float x,float y,unsigned int fontID,unsigned int *pTextID)
{
	DisplayText* pNewText = new DisplayText();
	pNewText->String = new char[strlen(text)+1];
	strcpy(pNewText->String,text);
	pNewText->FontID = fontID;
	pNewText->X=x;
	pNewText->Y=y;
	if(!textMan.AddItem(pNewText,pTextID))
	{
		delete pNewText;
		return 0;
	}
	return 1;
}

void GUImanager::DrawTexts()
{
	RenderMode mode = pRenderer->GetCurrentMode();
	bool inTextMode = (mode == R_TEXT);
	if(!inTextMode )
	{
		pRenderer->EnableTextMode();
	}

	
	ItemManager<DisplayText>::Iterator ite;
	textMan.GetIterator(ite);
	while(!ite.isAtEnd())
	{
		pRenderer->DrawString(ite->String,
							  ite->FontID,
							  ite->X,
							  ite->Y);
		++ite;
	}

	if(!inTextMode )
	{
		pRenderer->DisableTextMode();
	}
}

void GUImanager::Draw_Text(unsigned int id)
{
	RenderMode mode = pRenderer->GetCurrentMode();
	bool inTextMode = (mode == R_TEXT);
	if(!inTextMode )
	{
		pRenderer->EnableTextMode();
	}

	SharedPtr<DisplayText> pText = textMan.GetItemPointer(id);
	if(pText == NULL)
		return;
	pRenderer->DrawString(pText->String,
						  pText->FontID,
						  pText->X,
						  pText->Y);

	if(!inTextMode )
	{
		pRenderer->DisableTextMode();
	}
}

/*---------------background & image Management-------------*/
int GUImanager::SetBackGround(unsigned int textureID,Rect *pImgRect,Rect *pWinRect)
{
	backGround.textureID = textureID;
	if(pImgRect)
		backGround.imageRect = *pImgRect;
	else 
	{
		if(pRenderer->GetTextureImageRect(textureID,backGround.imageRect)!=R_OK)
			return 0;
	}
	if(pWinRect)
		backGround.windowRect = *pWinRect;
	else
	{
		backGround.windowRect.left = 0;
		backGround.windowRect.bottom =0;
		backGround.windowRect.right = pRenderer->GetWidth();
		backGround.windowRect.top = pRenderer->GetHeight();
	}
	return 1;
}

int GUImanager::AddImage(unsigned int textureID,Rect *pImgRect,Rect *pWinRect,unsigned int *pImageID)
{
	Image *pImage = new Image();

	pImage->textureID = textureID;
	if(pImgRect)
		pImage->imageRect = *pImgRect;
	else 
	{
		if(pRenderer->GetTextureImageRect(textureID,pImage->imageRect)!=R_OK)
			return 0;
	}
	if(pWinRect)
		pImage->windowRect = *pWinRect;
	else
	{
		pImage->windowRect.left = 0;
		pImage->windowRect.bottom =0;
		pImage->windowRect.right = pRenderer->GetWidth();
		pImage->windowRect.top = pRenderer->GetHeight();
	}

	if(!this->AddItem(pImage,pImageID))
	{
		delete pImage;
		return 0;
	}

	return 1;
}


int GUImanager::RemoveButton(unsigned int buttonID)
{
	return buttonMan.ReleaseSlot(buttonID);
}

int GUImanager::RemoveText(unsigned int textID)
{
	return textMan.ReleaseSlot(textID);
}

void GUImanager::DrawBackground()
{
	RenderMode mode = pRenderer->GetCurrentMode();
	bool in2DMode = (mode == R_2D);
	if(!in2DMode )//if not current in 2D mode
	{
		pRenderer->Enable2DMode();
	}

	pRenderer->BlitTextureToScreen(backGround.textureID,
								   1.0f,
								   &backGround.imageRect,
								   &backGround.windowRect);

	if(!in2DMode )
	{
		pRenderer->Disable2DMode();
	}
}

void GUImanager::DrawImage(unsigned int imageID)
{
	SharedPtr<Image> pImage = this->GetItemPointer(imageID);
	if(pImage == NULL)
		return;

	RenderMode mode = pRenderer->GetCurrentMode();
	bool in2DMode = (mode == R_2D);
	if(!in2DMode )//if not current in 2D mode
	{
		pRenderer->Enable2DMode();
	}
	
	pRenderer->BlitTextureToScreen(pImage->textureID,
								   0.0f,
								   &pImage->imageRect,
								   &pImage->windowRect);

	if(!in2DMode )
	{
		pRenderer->Disable2DMode();
	}
}
void GUImanager::DrawImages()
{
	RenderMode mode = pRenderer->GetCurrentMode();
	bool in2DMode = (mode == R_2D);
	if(!in2DMode )//if not current in 2D mode
	{
		pRenderer->Enable2DMode();
	}
	
	if(slots)
	{
		for(unsigned int i=0;i<allocSlots;++i)
		{
			SharedPtr<Image> pImage = this->GetItemPointer(i);
			if(pImage != NULL){
				pRenderer->BlitTextureToScreen(pImage->textureID,
									   0.0f,
									   &pImage->imageRect,
									   &pImage->windowRect);
			}
		}
	}
	if(!in2DMode )
	{
		pRenderer->Disable2DMode();
	}
}
int GUImanager::RemoveImage(unsigned int imageID)
{
	return this->ReleaseSlot(imageID);
}
void GUImanager::RemoveAllItems()
{
	buttonMan.ReleaseAllSlot();
	textMan.ReleaseAllSlot();
	this->ReleaseAllSlot();
}