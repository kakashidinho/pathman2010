#include "GameFlowControl.h"

unsigned int OnExitTextID;//id of GUI text item that will used for displaying question text for asking if user want to quit or not
unsigned int OnBackMenuTextID;//id of GUI text item that will used for displaying question text for asking if user to go back to main menu or not
unsigned int EnableSoundTextID;//id of GUI text item that will used for displaying question text for asking if user want to enable sound or not
PopUp::PopUpType currentType = PopUp::P_EXIT;
unsigned int buttonID[2],buttonTextID[2],imageID,dialogTextID[4];//GUI item ids
float fadeLevel;

void PressOK()
{
	pGUI->DeActiveButton(buttonID[0]);
	pGUI->DeActiveButton(buttonID[1]);
	PopUp::isPopping = false;

	switch(currentType)
	{
	case PopUp::P_EXIT:
		AppFW->ExitApp(0);
		break;
	case PopUp::P_SOUND:	
		sound = true;
		LoadingProgress::BeginLoading( Menu::InitMenu,Menu::BeginDisplayMenu);
		break;
	case PopUp::P_MENU://this is when gamer want to go back to main manu from game section
		Game::ChangeSection(1);
		break;
	case PopUp::P_RETRY:
		Game::ChangeSection(2);
		break;
	}
}
void PressCancel()
{
	pGUI->DeActiveButton(buttonID[0]);
	pGUI->DeActiveButton(buttonID[1]);
	PopUp::isPopping = false;

	switch(currentType)
	{
	case PopUp::P_SOUND:	
		sound = false;
		LoadingProgress::BeginLoading( Menu::InitMenu,Menu::BeginDisplayMenu);
		break;
	case PopUp::P_MENU://this is when gamer want to go back to main manu from game section
		break;
	case PopUp::P_RETRY:
		Game::ChangeSection(1);
		break;
	}

}

namespace PopUp
{
	bool isPopping = false;
	void InitQuestionText(const char *nameOfFile,Rect &dialogBoxRect,unsigned int &GUItextID)
	{
		float textX,textY;
		unsigned int fontIndex,lineDistance,numLines;
		unsigned char *ByteStream;
		unsigned int streamSize;
		unsigned int varGroup;
		std::string text="";
		char line[256];
		
		/*----------get questioning text -----------------*/
		ByteStream=packMan->GetSubByteStream(textBufferID,nameOfFile,&streamSize);
		pVar->CreateVariableGroup(&varGroup);
		SharedPtr<VariableGroup> pVarGroup = pVar->GetVariableGroup(varGroup); 
		pVarGroup->ParseVariablesFromMemory(ByteStream,streamSize);

		fontIndex = *pVarGroup->GetUint("fontIndex");
		lineDistance = pRenderer->GetFontLineDistance(font[fontIndex]);
		
		numLines = *pVarGroup->GetUint("numLines");
		
		for(unsigned int i =0 ;i< numLines;++i)
		{
			char name[10]="textLine";
			name[8] = i+'0';
			name[9] ='\0';
			strcpy(line,pVarGroup->GetString(name));
			text+=line;
			text+="\n";
		}
		
		textY = dialogBoxRect.top - lineDistance;
		textX = dialogBoxRect.left;
		
		pGUI->AddText(text.c_str(),textX,textY,font[fontIndex],&GUItextID);

		pVar->RemoveVariableGroup(varGroup);

	}
	
	void InitAllQuestionTexts(Rect &dialogBoxRect)
	{
		InitQuestionText("exitPop.txt",dialogBoxRect,dialogTextID[0]);
		InitQuestionText("menuPop.txt",dialogBoxRect,dialogTextID[1]);
		InitQuestionText("soundPop.txt",dialogBoxRect,dialogTextID[2]);
		InitQuestionText("retryPop.txt",dialogBoxRect,dialogTextID[3]);
	}
	void InitPopUp()
	{
		int iconIndex[2];//icon indexes
		char text[100];
		Rect rect;
		float textLength,textX,textY;
		int fontIndex;
		unsigned char *ByteStream;
		unsigned int animFrames,poseFrames;
		float animDur,poseDur;
		unsigned int bufferID,streamSize;
		unsigned int varGroup;

		packMan->UnPack("../Resources/Popup.qpackage",&bufferID);//unpack package containing resources for popup
		
		ByteStream=packMan->GetSubByteStream(bufferID,"popup.txt",&streamSize);//get byte stream from package
		
		/*------parse setting info-----------*/
		/*---create variable group-----------*/
		pVar->CreateVariableGroup(&varGroup);
		SharedPtr<VariableGroup> pVarGroup = pVar->GetVariableGroup(varGroup); 
		pVarGroup->ParseVariablesFromMemory(ByteStream,streamSize);
		
		fadeLevel = *pVarGroup->GetFloat("transparency");

		//button animation info
		animDur = *pVarGroup->GetFloat("animDuration");
		poseDur = *pVarGroup->GetFloat("poseDuration");
		
		animFrames = AppFW->QueryFramesByTime(animDur);
		poseFrames = AppFW->QueryFramesByTime(poseDur);

		//button's text font
		fontIndex = *pVarGroup->GetInt("font");

		//button icon
		iconIndex[0] = *pVarGroup->GetInt("icon1");
		iconIndex[1] = *pVarGroup->GetInt("icon2");
		
		//button OK
		strcpy(text,pVarGroup->GetString("text1"));

		memcpy(&rect,pVarGroup->GetInt4("rect1"),4*sizeof(int));

		textLength=pRenderer->GetStringLength(text,font[fontIndex]);

		textY = *pVarGroup->GetFloat("textHeight1");
		textX = (rect.right + rect.left - textLength)/2.0f;

		pGUI->AddButton(PlayButtonClickSound,PressOK,animFrames,poseFrames,icons[iconIndex[0]].texID,icons[iconIndex[0]].rect,
						icons[iconIndex[1]].texID,icons[iconIndex[1]].rect,rect,&buttonID[0]);
		pGUI->AddText(text,textX,textY,font[fontIndex],&buttonTextID[0]);
		
		//button Cancel
		strcpy(text,pVarGroup->GetString("text2"));

		memcpy(&rect,pVarGroup->GetInt4("rect2"),4*sizeof(int));

		textLength=pRenderer->GetStringLength(text,font[fontIndex]);

		textY = *pVarGroup->GetFloat("textHeight2");
		textX = (rect.right + rect.left - textLength)/2.0f;

		pGUI->AddButton(PlayButtonClickSound,PressCancel,animFrames,poseFrames,icons[iconIndex[0]].texID,icons[iconIndex[0]].rect,
						icons[iconIndex[1]].texID,icons[iconIndex[1]].rect,rect,&buttonID[1]);
		pGUI->AddText(text,textX,textY,font[fontIndex],&buttonTextID[1]);
		
		//image of dialog box
		iconIndex[0] = *pVarGroup->GetInt("icon3");
		memcpy(&rect,pVarGroup->GetInt4("rect3"),4*sizeof(int));
		pGUI->AddImage(icons[iconIndex[0]].texID,&icons[iconIndex[0]].rect,&rect,&imageID);
		
		memcpy(&rect,pVarGroup->GetInt4("textArea"),4*sizeof(int));//get rectangle area where text can be displayed to

		pVar->RemoveVariableGroup(varGroup);
		packMan->ClearBuffer(bufferID);

		InitAllQuestionTexts(rect);
	}

	void BeginPopUp(PopUpType type)
	{
		pGUI->ActiveButton(buttonID[0]);
		pGUI->ActiveButton(buttonID[1]);
		isPopping = true;
		currentType = type;
	}

	void PopUpRendering()
	{
		pRenderer->FadingScreen(fadeLevel);
		pGUI->DrawButton(buttonID[0]);
		pGUI->DrawButton(buttonID[1]);

		pGUI->DrawImage(imageID);

		pRenderer->EnableTextMode();
		pGUI->Draw_Text(buttonTextID[0]);
		pGUI->Draw_Text(buttonTextID[1]);

		switch(currentType)
		{
		case P_EXIT:
			pGUI->Draw_Text(dialogTextID[0]);
			break;
		case P_MENU:
			pGUI->Draw_Text(dialogTextID[1]);
			break;
		case P_SOUND:
			pGUI->Draw_Text(dialogTextID[2]);
			break;
		case P_RETRY:
			pGUI->Draw_Text(dialogTextID[3]);
			break;
		}
		pRenderer->DisableTextMode();
	}
};