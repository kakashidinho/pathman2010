#include "GameFlowControl.h"

struct IntroInfo
{
	unsigned int GameLoftLogoTexID;//id of logo texture
	unsigned int SplashTexID;//id of splash screen texture
	float IntroTime;//time period for displaying gameloft logo
	float SplashTime;//time period for displaying splash screen
};

IntroInfo *introInfo = NULL;
unsigned int *font;
Icon* icons;
float fadingTime;
unsigned int mutexID;
unsigned int textBufferID;
unsigned int audioBufferID;
bool sound;
bool drawFps = false;
unsigned int buttonClickID;//button clicked sound

void OnExit2()
{
	SafeDelete(introInfo);
}

void PlayButtonClickSound()
{
	if(sound)
		pMixer->Play(buttonClickID,1);
}

namespace Intro
{
	
	void InitIcon()
	{
		char iconImage[256];
		unsigned int streamSize;
		unsigned char* ByteStream;
		unsigned int bufferID;
		unsigned int texID;
		int numIcons;

		packMan->UnPack("../Resources/Icon.qpackage",&bufferID);//unpack package containing resources for icons
		
		ByteStream=packMan->GetSubByteStream(bufferID,"icon.txt",&streamSize);//get byte stream from package
		
		/*------parse setting info-----------*/
		/*---create variable group-----------*/
		unsigned int varGroup;
		pVar->CreateVariableGroup(&varGroup);
		SharedPtr<VariableGroup> pVarGroup = pVar->GetVariableGroup(varGroup); 
		pVarGroup->ParseVariablesFromMemory(ByteStream,streamSize);

		numIcons = *pVarGroup->GetInt("numIcons");
		icons = new Icon[numIcons];

		strcpy(iconImage,pVarGroup->GetString("image"));
		//load texture
		ByteStream=packMan->GetSubByteStream(bufferID,iconImage,&streamSize);//get byte stream from package
		if(pRenderer->LoadTextureFromMemory(ByteStream,streamSize,&texID)!=R_OK)
			exit(-1);

		for(int i=0;i<numIcons;++i)
		{
			char name[2];
			name[0] = i+'0';
			name[1] = '\0';
			icons[i].texID = texID;
			memcpy(&icons[i].rect,pVarGroup->GetInt4(name),4*sizeof(int));
		}

		pVar->RemoveVariableGroup(varGroup);
		packMan->ClearBuffer(bufferID);
	}

	void InitFont()
	{
		char line[100],infoFileName[256],imageName[256];
		int notUseInt;
		unsigned int bufferID;
		unsigned int streamSize1,streamSize2;
		unsigned char* ByteStream1,*ByteStream2;
		/*-----------------init font ----------------------------------*/
		packMan->UnPack("../Resources/Font.qpackage",&bufferID);//unpack package containing resources for font
		ByteStream1=packMan->GetSubByteStream(bufferID,"init.txt",&streamSize1);//get byte stream from package
		

		
		CStringStream stream((char*) ByteStream1,streamSize1);
		//get number of fonts
		int numFonts = 0;
		stream.GetLine(line,100);
		sscanf(line,"numFonts %d",&numFonts);
		font = new unsigned int[numFonts];
		stream.GetLine(line,100);//nothing to do with this line
		
		for(int i =0 ; i < numFonts ; ++i)
		{
			stream.GetLine(line,100);
			sscanf(line,"%d	%s	%s",&notUseInt,infoFileName,imageName);
		
			ByteStream1=packMan->GetSubByteStream(bufferID,infoFileName,&streamSize1);//get byte stream from package
			ByteStream2=packMan->GetSubByteStream(bufferID,imageName,&streamSize2);//get byte stream from package
			if(pRenderer->CreateFontFromMemory(ByteStream1,streamSize1,
												ByteStream2,streamSize2,
												&font[i])!=R_OK)
				exit(-1);
		}

		packMan->ClearBuffer(bufferID);
	}


	void Init()
	{
		char logoName[256],splashName[256];
		unsigned int streamSize;
		unsigned char* ByteStream;
		unsigned int bufferID;
		/*------------init logo & splash screen texture ---------------*/
		packMan->UnPack("../Resources/Intro.qpackage",&bufferID);//unpack package containing resources for screen rendering
		
		ByteStream=packMan->GetSubByteStream(bufferID,"init.txt",&streamSize);//get byte stream from package
		
		/*------parse setting info-----------*/
		/*---create variable group-----------*/
		unsigned int varGroup;
		pVar->CreateVariableGroup(&varGroup);
		SharedPtr<VariableGroup> pVarGroup = pVar->GetVariableGroup(varGroup); 
		pVarGroup->ParseVariablesFromMemory(ByteStream,streamSize);
		
		strcpy(logoName,pVarGroup->GetString("logoImage"));
		
		Rect rectLogoT;//rectangle area of logo texture that will be used to map to window screen
		Rect rectLogoW;//rectangle area of window screen that logo will be mapped to

		memcpy(&rectLogoT,pVarGroup->GetInt4("logoImageRect"),4*sizeof(int));
		memcpy(&rectLogoW,pVarGroup->GetInt4("logoWinRect"),4*sizeof(int));
		
		memcpy(&introInfo->IntroTime,pVarGroup->GetFloat("logoDisplayDuration"),sizeof(float));
		
		strcpy(splashName,pVarGroup->GetString("splashScreenImage"));
		
		memcpy(&introInfo->SplashTime,pVarGroup->GetFloat("splashDuration"),sizeof(float));

		memcpy(&fadingTime,pVarGroup->GetFloat("fading"),sizeof(float));
		
		//init click sound audio
		ByteStream = packMan->GetSubByteStream(audioBufferID,pVarGroup->GetString("clickSound"),&streamSize);

		pMixer->LoadAudioFromMemory(ByteStream,streamSize,buttonClickID);
		
		/*----done parsing--------------------*/

		pVar->RemoveVariableGroup(varGroup);
		
		ByteStream=packMan->GetSubByteStream(bufferID,logoName,&streamSize);//get byte stream from package
		if(pRenderer->LoadTextureFromMemory(ByteStream,streamSize,&introInfo->GameLoftLogoTexID)!=R_OK)//load texture from image
			exit(-1);

		ByteStream=packMan->GetSubByteStream(bufferID,splashName,&streamSize);//get byte stream from package
		if(pRenderer->LoadTextureFromMemory(ByteStream,streamSize,&introInfo->SplashTexID)!=R_OK)//load texture from image
			exit(-1);
		
		packMan->ClearBuffer(bufferID);

		pGUI->SetBackGround(introInfo->GameLoftLogoTexID,&rectLogoT,&rectLogoW);
	}

	void InitIntro()
	{
		//create mutex
		AppFW->CreateNewMutex(&mutexID);
		
		//create loading info object
		introInfo = new IntroInfo();

		//Load package file that contains info about texts in game
		packMan->UnPack("../Resources/Text.qpackage",&textBufferID);
		//load package file that contains info about audio in game
		packMan->UnPack("../Resources/Audio.qpackage",&audioBufferID);
		InitIcon();
		InitFont();
		PopUp::InitPopUp();
		Init();


		pRenderer->Enable2DMode();//enable 2D mode

		AppFW->AddExitTask(OnExit2);

		timer.Start();
	}

	void IntroRendering(void)//display gameloft logo
	{
		if(!PopUp::isPopping)//popup will freeze time
			timer.Stop();
		double time=timer.GetElapsedTime();//get total time since timer was started
		
		float totalTimeDisplay = introInfo->IntroTime + 2*fadingTime ;//display duration + fading in duration + fading out duration

		if(time > totalTimeDisplay)//it's time for change to display splash screen
		{
			pRenderer->ReleaseTexture(introInfo->GameLoftLogoTexID);
			pGUI->SetBackGround(introInfo->SplashTexID,NULL,NULL);
			AppFW->SetRenderFunc(SplashScreen);

			timer.Start();//reset starting time of timer
		}
		else
		{
			float visible=1.0f;
			if(time < fadingTime)//fading in
				visible=(float)time/fadingTime;//visible -> opaque
			else if (time > introInfo->IntroTime + fadingTime)//fading out
				visible=(totalTimeDisplay - (float)time)/ (fadingTime);//opaque -> visible 
			
			pGUI->DrawBackground();
			pRenderer->FadingScreen(1.0f - visible);
			if(PopUp::isPopping)
			{
				PopUp::PopUpRendering();
			}
		}
	}

	void SplashScreen(void)//splash screen
	{
		if(!PopUp::isPopping)//popup will freeze time
			timer.Stop();
		double time=timer.GetElapsedTime();//get total time since timer was started

		float totalTimeDisplay = introInfo->SplashTime + 2*fadingTime ;//display duration + fading in duration + fading out duration

		if(time > totalTimeDisplay)//it's time for change to display loading screen
		{
			pRenderer->ReleaseTexture(introInfo->SplashTexID);

			if(PopUp::isPopping)//another popup already shows up
			{
				return;//wait for next loop
			}

			
			LoadingProgress::InitLoadingStuff();//init loading stuffs

			SafeDelete(introInfo);//done using loading info,we will no longer return to this intro session
			
			PopUp::BeginPopUp(PopUp::P_SOUND);
			AppFW->SetRenderFunc(PopUp::PopUpRendering);
		}
		else
		{
			float visible=1.0f;
			if(time < fadingTime)//fading in
				visible=(float)time/fadingTime;//visible -> opaque
			else if (time > introInfo->SplashTime + fadingTime)//fading out
				visible=(totalTimeDisplay - (float)time)/ (fadingTime);//opaque -> visible 
			
			pGUI->DrawBackground();
			pRenderer->FadingScreen(1.0f - visible);
			if(PopUp::isPopping)
			{
				PopUp::PopUpRendering();
			}
		}
	}
};