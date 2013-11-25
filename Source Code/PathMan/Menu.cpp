#include "GameFlowControl.h"
#include "HelperType.h"

struct commonButtonInfo
{
	unsigned int fontID;
	unsigned int animFrames,poseFrames;
	unsigned int iconIndex[2];
};

struct MenuInfo
{
	unsigned int buttonID[5];//common buttons (Start Help About Exit OK)
	unsigned int buttonTextID[5];//texts on buttons
	unsigned int helpTextID;
	unsigned int aboutTextID;
	float fadeLvl;//help & about page fading level
	unsigned int soundButtonID[2];//enable / disable sound button id
	unsigned int crossImgID[2];
	unsigned int bgTexID;//background texture id
	int currentPage;//main = 0 , help = 1 , about = 2
	unsigned int musicID;//music audio ID in mixer
	int musicChannel;
	bool isFadingOut;
	bool havePopup;//is there a popup on screen,if so, we need to disable all buttons
	MenuInfo()
	{
		musicChannel = -1;
		isFadingOut = false;
		havePopup = false;
		currentPage = 0;
	}
	~MenuInfo()
	{
		if(sound && musicChannel != -1)
			pMixer->Stop(musicChannel);
		pMixer->ReleaseAudio(musicID);

		pGUI->RemoveButton(buttonID[0]);
		pGUI->RemoveButton(buttonID[1]);
		pGUI->RemoveButton(buttonID[2]);
		pGUI->RemoveButton(buttonID[3]);
		pGUI->RemoveButton(buttonID[4]);
		pGUI->RemoveButton(soundButtonID[0]);
		pGUI->RemoveButton(soundButtonID[1]);
		pGUI->RemoveImage(crossImgID[0]);
		pGUI->RemoveImage(crossImgID[1]);

		pGUI->RemoveText(buttonTextID[0]);
		pGUI->RemoveText(buttonTextID[1]);
		pGUI->RemoveText(buttonTextID[2]);
		pGUI->RemoveText(buttonTextID[3]);
		pGUI->RemoveText(buttonTextID[4]);
		pGUI->RemoveText(helpTextID);
		pGUI->RemoveText(aboutTextID);

		pRenderer->ReleaseTexture(bgTexID);
	}
};



bool firstTime = true;//first time init?
MenuInfo *menuInfo = NULL;
TextureImageSrcInfo * bgInfo = NULL;

void OnExit4();
void PressExit();//Exit button clicking event handler
void PressStart();//start button clicking event handler
void Press_OK();
void PressHelp();
void PressAbout();


namespace Menu
{
	void EnableSound();
	void DisableSound();
	void DrawMainPage();
	void DrawHelpPage();
	void DrawAboutPage();
	void DeActiveAllButtons()
	{
		pGUI->DeActiveButton(menuInfo->buttonID[0]);
		pGUI->DeActiveButton(menuInfo->buttonID[1]);
		pGUI->DeActiveButton(menuInfo->buttonID[2]);
		pGUI->DeActiveButton(menuInfo->buttonID[3]);
		pGUI->DeActiveButton(menuInfo->buttonID[4]);
		pGUI->DeActiveButton(menuInfo->soundButtonID[0]);
		pGUI->DeActiveButton(menuInfo->soundButtonID[1]);
	}

	void ActiveAllButtons()
	{
		switch(menuInfo->currentPage)
		{
		case 0://main menuInfo->currentPage
			pGUI->ActiveButton(menuInfo->buttonID[0]);
			pGUI->ActiveButton(menuInfo->buttonID[1]);
			pGUI->ActiveButton(menuInfo->buttonID[2]);
			pGUI->ActiveButton(menuInfo->buttonID[3]);
			if(sound)
				pGUI->ActiveButton(menuInfo->soundButtonID[0]);
			else
				pGUI->ActiveButton(menuInfo->soundButtonID[1]);
			break;
		case 1:case 2://help & about menuInfo->currentPage
			pGUI->ActiveButton(menuInfo->buttonID[4]);
			break;
		}
	}
	void InitButton(int index,SharedPtr<VariableGroup> &pVarGroup,const commonButtonInfo & commonInfo ,CallbackFunc clickHandler,unsigned int &buttonID,unsigned int &textID );

	void UpdateMenu()
	{
		if(PopUp::isPopping)//popup will freeze time
		{
			if(!menuInfo->havePopup)//this flag not set yet
			{
				DeActiveAllButtons();

				menuInfo->havePopup = true ; //set this flag to true
			}
			return;
		}
		if(menuInfo->havePopup)//there was a popup showing up before but not now , need to reactive buttons
		{
			ActiveAllButtons();

			menuInfo->havePopup = false ; //set this flag to false
		}

		timer.Stop();
	}
	void InitMenu()
	{
		if(firstTime)//first time init
		{
			firstTime = false;
			AppFW->AddExitTask(OnExit4);
		}
		
		menuInfo = new MenuInfo();
		bgInfo = new TextureImageSrcInfo();
		
		int bIcons[4];
		Rect rect[2];
		int fontIndex;
		unsigned char *ByteStream;
		commonButtonInfo commonInfo ;//common info of a group of buttons
		float animDur,poseDur;
		unsigned int bufferID,streamSize;
		unsigned int varGroup;
		std::string text = "";
		char line[256];
		unsigned int numLines;
		float helpTextX,helpTextY,aboutTextX,aboutTextY;

		packMan->UnPack("../Resources/Menu.qpackage",&bufferID);//unpack package containing resources for menu
		
		LoadingProgress::SetProgressVal(10);
		Sleep(10);
		
		ByteStream=packMan->GetSubByteStream(bufferID,"menu.txt",&streamSize);//get byte stream from package
		
		/*------parse setting info-----------*/
		/*---create variable group-----------*/
		pVar->CreateVariableGroup(&varGroup);
		SharedPtr<VariableGroup> pVarGroup = pVar->GetVariableGroup(varGroup); 
		pVarGroup->ParseVariablesFromMemory(ByteStream,streamSize);
		
		LoadingProgress::SetProgressVal(20);
		Sleep(10);

		//get background texture info
		strcpy(bgInfo->imageName , pVarGroup->GetString("bgImage"));
		bgInfo->bufferID = bufferID;

		LoadingProgress::SetProgressVal(30);
		Sleep(10);
		
		//get sound info
		ByteStream = packMan->GetSubByteStream(audioBufferID,pVarGroup->GetString("music"),&streamSize);

		pMixer->LoadAudioFromMemory(ByteStream,streamSize,menuInfo->musicID);

		//getting common info for a group of common buttons (Start , help , about, exit)
		//button animation info
		animDur = *pVarGroup->GetFloat("animDuration");
		poseDur = *pVarGroup->GetFloat("poseDuration");
		
		commonInfo.animFrames = AppFW->QueryFramesByTime(animDur);
		commonInfo.poseFrames = AppFW->QueryFramesByTime(poseDur);

		//button's text font
		fontIndex = *pVarGroup->GetInt("font");
		commonInfo.fontID = font[fontIndex];

		//button icon
		commonInfo.iconIndex[0] = *pVarGroup->GetInt("icon0");
		commonInfo.iconIndex[1] = *pVarGroup->GetInt("icon1");
		LoadingProgress::SetProgressVal(40);
		Sleep(10);
		
		//init each button
		InitButton(0,pVarGroup,commonInfo,PressStart,menuInfo->buttonID[0],menuInfo->buttonTextID[0]);//start button

		LoadingProgress::SetProgressVal(50);
		Sleep(10);

		InitButton(1,pVarGroup,commonInfo,PressHelp ,menuInfo->buttonID[1],menuInfo->buttonTextID[1]);//help button

		LoadingProgress::SetProgressVal(60);
		Sleep(10);

		InitButton(2,pVarGroup,commonInfo,PressAbout ,menuInfo->buttonID[2],menuInfo->buttonTextID[2]);//about button

		LoadingProgress::SetProgressVal(70);
		Sleep(10);

		InitButton(3,pVarGroup,commonInfo,PressExit ,menuInfo->buttonID[3],menuInfo->buttonTextID[3]);//exit button
		
		InitButton(4,pVarGroup,commonInfo,Press_OK ,menuInfo->buttonID[4],menuInfo->buttonTextID[4]);//ok button
		
		LoadingProgress::SetProgressVal(80);
		Sleep(10);

		//sound button
		bIcons[0] = *pVarGroup->GetInt("icon2");
		bIcons[1] = *pVarGroup->GetInt("icon3");
		bIcons[2] = *pVarGroup->GetInt("icon4");//cross icon
		bIcons[3] = *pVarGroup->GetInt("icon5");//cross icon

		memcpy(&rect[0],pVarGroup->GetInt4("rect5"),4*sizeof(int));
		memcpy(&rect[1],pVarGroup->GetInt4("rect6"),4*sizeof(int));
		//disable sound button
		pGUI->AddButton(PlayButtonClickSound,DisableSound,commonInfo.animFrames,commonInfo.poseFrames,
			icons[bIcons[0]].texID,icons[bIcons[0]].rect,
			icons[bIcons[1]].texID,icons[bIcons[1]].rect,
			rect[0],&menuInfo->soundButtonID[0]);

		//enable sound button
		pGUI->AddButton(PlayButtonClickSound,EnableSound,commonInfo.animFrames,commonInfo.poseFrames,
			icons[bIcons[1]].texID,icons[bIcons[1]].rect,
			icons[bIcons[0]].texID,icons[bIcons[0]].rect,
			rect[0],&menuInfo->soundButtonID[1]);

		//cross image
		pGUI->AddImage(icons[bIcons[2]].texID,&icons[bIcons[2]].rect,&rect[1],&menuInfo->crossImgID[0]);
		pGUI->AddImage(icons[bIcons[3]].texID,&icons[bIcons[3]].rect,&rect[1],&menuInfo->crossImgID[1]);
		/*---------help & about page info------------*/

		menuInfo->fadeLvl = *pVarGroup->GetFloat("ha_transparency");
		helpTextX = *pVarGroup->GetFloat("helpTextX");
		helpTextY = *pVarGroup->GetFloat("helpTextY");
		aboutTextX = *pVarGroup->GetFloat("aboutTextX");
		aboutTextY = *pVarGroup->GetFloat("aboutTextY");

		pVar->RemoveVariableGroup(varGroup);
		//help text
		ByteStream=packMan->GetSubByteStream(textBufferID,"help.txt",&streamSize);
		pVar->CreateVariableGroup(&varGroup);
		pVarGroup = pVar->GetVariableGroup(varGroup); 
		pVarGroup->ParseVariablesFromMemory(ByteStream,streamSize);
		
		fontIndex = (int)*pVarGroup->GetUint("fontIndex");
		
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
		pGUI->AddText(text.c_str(),helpTextX,helpTextY,font[fontIndex],&menuInfo->helpTextID);
		pVar->RemoveVariableGroup(varGroup);
		//about text
		text = "";
		ByteStream=packMan->GetSubByteStream(textBufferID,"about.txt",&streamSize);
		pVar->CreateVariableGroup(&varGroup);
		pVarGroup = pVar->GetVariableGroup(varGroup); 
		pVarGroup->ParseVariablesFromMemory(ByteStream,streamSize);
		
		fontIndex = (int)*pVarGroup->GetUint("fontIndex");
		
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
		pGUI->AddText(text.c_str(),aboutTextX,aboutTextY,font[fontIndex],&menuInfo->aboutTextID);
		pVar->RemoveVariableGroup(varGroup);

		LoadingProgress::SetProgressVal(90);
		Sleep(10);

		LoadingProgress::SetProgressVal(100);
		

	}
	void DisplayMenu()
	{
		double time=timer.GetElapsedTime();//get total time since timer was started

		//float totalTimeDisplay = SplashTime + 2*fadingTime ;//display duration + fading in duration + fading out duration

		float fadeLevel=0.0f;
		if(time < fadingTime)//fading
		{
			if(menuInfo->isFadingOut)//fading out
				fadeLevel=(float)time/fadingTime;
			else//fading in
				fadeLevel=1.0f-(float)time/fadingTime;
		}

		else if(menuInfo->isFadingOut)//finished fading out,change to game session
		{
			OnExit4();

			LoadingProgress::BeginLoading( Game::InitGame,Game::BeginGame);
			return;
		}

		pGUI->DrawBackground();
		
		switch (menuInfo->currentPage)
		{
		case 0:
			DrawMainPage();
			break;
		case 1:
			DrawHelpPage();
			break;
		case 2:
			DrawAboutPage();
			break;
		}
		
		pRenderer->FadingScreen(fadeLevel);

		if(PopUp::isPopping)
		{
			PopUp::PopUpRendering();
		}
	}
	void BeginDisplayMenu()
	{
		//load background texture 
		unsigned char * ByteStream;
		unsigned int streamSize;
		
		ByteStream = packMan->GetSubByteStream(bgInfo->bufferID,bgInfo->imageName,&streamSize);
		
		if(pRenderer->LoadTextureFromMemory(ByteStream,streamSize,&menuInfo->bgTexID)!=R_OK)
			exit(-1);
		packMan->ClearBuffer(bgInfo->bufferID);//release buffer contains byte stream of background image
		SafeDelete(bgInfo)//no longer need it

		AppFW->SetUpdateFunc(UpdateMenu);
		AppFW->SetRenderFunc(DisplayMenu);

		pRenderer->Enable2DMode();
		
		ActiveAllButtons();

		pGUI->SetBackGround(menuInfo->bgTexID,NULL,NULL);//set current background
		
		if(sound)
		{
			menuInfo->musicChannel = pMixer->Play(menuInfo->musicID,0);
			pGUI->ActiveButton(menuInfo->soundButtonID[0]);
			pGUI->DeActiveButton(menuInfo->soundButtonID[1]);
		}
		else
		{
			pGUI->DeActiveButton(menuInfo->soundButtonID[0]);
			pGUI->ActiveButton(menuInfo->soundButtonID[1]);
		}
		timer.Start();
	}
};

void Menu::InitButton(int index,SharedPtr<VariableGroup> &pVarGroup,const commonButtonInfo & commonInfo  ,CallbackFunc clickHandler,unsigned int &buttonID,unsigned int &textID)
{
	char text[100];
	float textLength,textX,textY;
	Rect rect;
	char varName[12];

	//get button text
	sprintf(varName,"text%d",index);
	strcpy(text,pVarGroup->GetString(varName));

	//get textHeight
	sprintf(varName,"textHeight%d",index);
	textY = *pVarGroup->GetFloat(varName);

	//get button rectangle on screen space
	sprintf(varName,"rect%d",index);
	memcpy(&rect,pVarGroup->GetInt4(varName),4*sizeof(int));

	textLength = pRenderer->GetStringLength(text,commonInfo.fontID);//get text length on screen
	textX = (rect.right + rect.left - textLength)/2.0f;

	pGUI->AddButton(PlayButtonClickSound,clickHandler,commonInfo.animFrames,commonInfo.poseFrames,
					icons[commonInfo.iconIndex[0]].texID,
					icons[commonInfo.iconIndex[0]].rect,
					icons[commonInfo.iconIndex[1]].texID,
					icons[commonInfo.iconIndex[1]].rect,rect,
					&buttonID);
	pGUI->AddText(text,textX,textY,commonInfo.fontID,&textID);
}

void OnExit4()
{
	SafeDelete(menuInfo);
	SafeDelete(bgInfo);
}

void PressExit()//Exit button clicking event handler
{
	PopUp::BeginPopUp(PopUp::P_EXIT);
}

void PressStart()//start button clicking event handler
{
	menuInfo->isFadingOut = true;//fading out menu screen
	timer.Start();
}

void Menu::EnableSound()
{
	sound = true;
	menuInfo->musicChannel = pMixer->Play(menuInfo->musicID,0);
	pGUI->DeActiveButton(menuInfo->soundButtonID[1]);
	pGUI->ActiveButton(menuInfo->soundButtonID[0]);
}
void Menu::DisableSound()
{
	sound = false;
	pMixer->Stop(menuInfo->musicChannel);
	pGUI->ActiveButton(menuInfo->soundButtonID[1]);
	pGUI->DeActiveButton(menuInfo->soundButtonID[0]);
}
void Menu::DrawMainPage()
{
	pGUI->DrawButton(menuInfo->buttonID[0]);
	pGUI->DrawButton(menuInfo->buttonID[1]);
	pGUI->DrawButton(menuInfo->buttonID[2]);
	pGUI->DrawButton(menuInfo->buttonID[3]);

	if(sound)
	{
		pGUI->DrawButton(menuInfo->soundButtonID[0]);
		pGUI->DrawImage(menuInfo->crossImgID[0]);
	}
	else
	{
		pGUI->DrawButton(menuInfo->soundButtonID[1]);
		pGUI->DrawImage(menuInfo->crossImgID[1]);
	}

	pRenderer->EnableTextMode();
	pGUI->Draw_Text(menuInfo->buttonTextID[0]);
	pGUI->Draw_Text(menuInfo->buttonTextID[1]);
	pGUI->Draw_Text(menuInfo->buttonTextID[2]);
	pGUI->Draw_Text(menuInfo->buttonTextID[3]);
	pRenderer->DisableTextMode();
}
void Menu::DrawHelpPage()
{
	pRenderer->FadingScreen(menuInfo->fadeLvl);
	pGUI->DrawButton(menuInfo->buttonID[4]);
	pRenderer->EnableTextMode();
	pGUI->Draw_Text(menuInfo->helpTextID);
	pGUI->Draw_Text(menuInfo->buttonTextID[4]);
	pRenderer->DisableTextMode();
}
void Menu::DrawAboutPage()
{
	pRenderer->FadingScreen(menuInfo->fadeLvl);
	pGUI->DrawButton(menuInfo->buttonID[4]);
	pRenderer->EnableTextMode();
	pGUI->Draw_Text(menuInfo->aboutTextID);
	pGUI->Draw_Text(menuInfo->buttonTextID[4]);
	pRenderer->DisableTextMode();
}
void Press_OK()
{
	menuInfo->currentPage = 0;
	pGUI->ActiveButton(menuInfo->buttonID[0]);
	pGUI->ActiveButton(menuInfo->buttonID[1]);
	pGUI->ActiveButton(menuInfo->buttonID[2]);
	pGUI->ActiveButton(menuInfo->buttonID[3]);
	if(sound)
		pGUI->ActiveButton(menuInfo->soundButtonID[0]);
	else
		pGUI->ActiveButton(menuInfo->soundButtonID[1]);
	pGUI->DeActiveButton(menuInfo->buttonID[4]);
}

void PressHelp()
{
	menuInfo->currentPage = 1;
	pGUI->DeActiveButton(menuInfo->buttonID[0]);
	pGUI->DeActiveButton(menuInfo->buttonID[1]);
	pGUI->DeActiveButton(menuInfo->buttonID[2]);
	pGUI->DeActiveButton(menuInfo->buttonID[3]);
	if(sound)
		pGUI->DeActiveButton(menuInfo->soundButtonID[0]);
	else
		pGUI->DeActiveButton(menuInfo->soundButtonID[1]);
	pGUI->ActiveButton(menuInfo->buttonID[4]);
}

void PressAbout()
{
	menuInfo->currentPage = 2;
	pGUI->DeActiveButton(menuInfo->buttonID[0]);
	pGUI->DeActiveButton(menuInfo->buttonID[1]);
	pGUI->DeActiveButton(menuInfo->buttonID[2]);
	pGUI->DeActiveButton(menuInfo->buttonID[3]);
	if(sound)
		pGUI->DeActiveButton(menuInfo->soundButtonID[0]);
	else
		pGUI->DeActiveButton(menuInfo->soundButtonID[1]);
	pGUI->ActiveButton(menuInfo->buttonID[4]);
}