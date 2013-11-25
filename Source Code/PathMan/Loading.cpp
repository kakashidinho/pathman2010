#include "GameFlowControl.h"
#include <math.h>

struct ProgressPoint 
{
	bool passed;
	AnimController<Rect> animCtrl;
};

struct LoadingInfo
{
	LoadingInfo()
	{
		tAnim = NULL;
		iAnim = NULL;
		points = NULL;
	}
	~LoadingInfo()
	{
		SafeDelete(iAnim);
		SafeDelete(tAnim);
		SafeDeleteArray(points);
	}
	AnimController<bool> *tAnim ;//pointer to animation controller for controlling animation of loading text animation
	AnimController<Rect> *iAnim ;//pointer to animation controller associated with "pacman" icon
	ProgressPoint *points;//list of animation controllers associated with points in loading progress bar
	unsigned int icon1Width,icon1Height;// icon1's width & height in window space - "pacman" icon
	unsigned int icon2Width,icon2Height;// icon2's width & height in window space - point icon
	unsigned int loadIconID;//loading icon texture id
	unsigned int xLogoGUIitemID;//id of GUI image item that uses xtreme logo texture for displaying
	unsigned int textGUIitemID;//id of GUI text item used for display loading text
	unsigned int loadBgID;//loading background texture id
	unsigned int numPoints;//number of points in loading progress bar
};

//global variables
LoadingInfo * loadingInfo = NULL;
unsigned int progress;//0 - > 100%
bool LoadingProgress::waitingForCont;
bool LoadingProgress::userCont = false;



/*---function prototype------*/
void ProgressThreadFunc(void *arg);
void OnExit3();

namespace LoadingProgress
{
	CallbackFunc progressTask;
	CallbackFunc doneTask;

	void LoadingScreen();//display loading screen
	void IncreaseProgressVal(unsigned int step)
	{
		if(PopUp::isPopping)//popup will freeze all progress
			return;
		AppFW->LockMutex(mutexID);
		progress += step;
		if(progress > 100)
			progress = 100;
		AppFW->UnlockMutex(mutexID);
	}
	void SetProgressVal(unsigned int percentage)
	{
		if(PopUp::isPopping)//popup will freeze all progress
			return;
		AppFW->LockMutex(mutexID);
		progress = percentage;
		if(progress > 100)
			progress = 100;
		AppFW->UnlockMutex(mutexID);
	}
	void InitLoadingStuff()
	{
		loadingInfo = new LoadingInfo();
		
		char line[100],imgName[256];
		unsigned int streamSize;
		unsigned char* ByteStream;
		unsigned int bufferID;
		unsigned int xLogoID;//xtreme logo texture ID

		packMan->UnPack("../Resources/Loading.qpackage",&bufferID);//unpack package containing resources for loading screen rendering

		ByteStream = packMan->GetSubByteStream(bufferID,"init.txt",&streamSize);
		
		//create variable group
		unsigned int varGroup;
		pVar->CreateVariableGroup(&varGroup);
		SharedPtr<VariableGroup> pVarGroup = pVar->GetVariableGroup(varGroup); 
		pVarGroup->ParseVariablesFromMemory(ByteStream,streamSize);
		
		/*---------parse string stream---------*/
		
		strcpy(imgName,pVarGroup->GetString("iconPacMan"));
		
		//load icon
		ByteStream = packMan->GetSubByteStream(bufferID,imgName,&streamSize);

		if(pRenderer->LoadTextureFromMemory(ByteStream,streamSize,&loadingInfo->loadIconID)!=R_OK)
			exit(-1);
		//get icon1 size
		int * pSize = pVarGroup->GetInt2("pacManSize");
		loadingInfo->icon1Width=pSize[0];
		loadingInfo->icon1Height=pSize[1];

		//get icon1 animation info
		unsigned int numPose=0;
		unsigned int poseFrames;
		float poseDuration = 0.5f;
		
		numPose = *pVarGroup->GetUint("numPoses1");
		poseDuration = *pVarGroup->GetFloat("poseDur1");

		poseFrames = AppFW->QueryFramesByTime(poseDuration);//number of frames displaying 1 pose in animation
		loadingInfo->iAnim = new AnimController<Rect>(numPose);
		
		for(unsigned int i=0;i<numPose ;++i)
		{
			PoseInfo<Rect>* pInfo = loadingInfo->iAnim->LoopPose();
			pInfo->endFrame = (i+1)*poseFrames;
			Rect* pRect = &pInfo->info;
			char name[10] = "ImgRect1";
			name[8] = i+'0';
			memcpy(pRect,pVarGroup->GetInt4(name),4*sizeof(int));

		}
		loadingInfo->iAnim->ResetCurrentPose();

		//get icon2 size
		pSize = pVarGroup->GetInt2("pointSize");
		loadingInfo->icon2Width=pSize[0];
		loadingInfo->icon2Height=pSize[1];

		//get animation info
		numPose = *pVarGroup->GetUint("numPoses2");
		poseDuration = *pVarGroup->GetFloat("poseDur2");

		AnimController<Rect> animControl(numPose);

		for(unsigned int i=0;i<numPose ;++i)
		{
			PoseInfo<Rect>* pInfo = animControl.LoopPose();
			pInfo->endFrame = (i+1)*poseFrames;
			Rect* pRect = &pInfo->info;

			char name[10] = "ImgRect2";
			name[8] = i+'0';
			name[9] = '\0';
			memcpy(pRect,pVarGroup->GetInt4(name),4*sizeof(int));
		}
		
		loadingInfo->numPoints = *pVarGroup->GetUint("numPoints");
		
		loadingInfo->points = new ProgressPoint[loadingInfo->numPoints];//create list of Animation controllers associated with points in loading screen's progress bar

		for(unsigned int i=0;i< loadingInfo->numPoints;++i)
		{
			loadingInfo->points[i].passed = false;
			loadingInfo->points[i].animCtrl = animControl;//copy animation controller,since this list of animation controllers share the same list of poses
		}

		//getting "xtreme" logo info
		strcpy(imgName,pVarGroup->GetString("logoImg"));
		ByteStream = packMan->GetSubByteStream(bufferID,imgName,&streamSize);

		if(pRenderer->LoadTextureFromMemory(ByteStream,streamSize,&xLogoID)!=R_OK)
			exit(-1);

		Rect rect1,rect2;
		memcpy(&rect1,pVarGroup->GetInt4("logoImgRect"),4*sizeof(int));
		memcpy(&rect2,pVarGroup->GetInt4("logoWinRect"),4*sizeof(int));
		
		pGUI->AddImage(xLogoID,&rect1,&rect2,&loadingInfo->xLogoGUIitemID);

		//getting background info
		strcpy(imgName,pVarGroup->GetString("bgImg"));
		
		//load background
		ByteStream = packMan->GetSubByteStream(bufferID,imgName,&streamSize);

		if(pRenderer->LoadTextureFromMemory(ByteStream,streamSize,&loadingInfo->loadBgID)!=R_OK)
			exit(-1);
		
		/*----getting "done loading" indicating text infos-------*/
		float textX,textY;//starting location of the line where done loading indicating text is drawn
		unsigned int fontIndex;//index of font objects used in rendering done loading indicating text
		unsigned int nlines;//number of lines

		//get done loading indicating text's animation pose duration
		poseDuration = *pVarGroup->GetFloat("textPoseDuration");

		poseFrames = AppFW->QueryFramesByTime(poseDuration);//number of frames displaying 1 pose in animation
		loadingInfo->tAnim = new AnimController<bool>(2);
		
		//1st pose
		PoseInfo<bool> * pose = loadingInfo->tAnim->LoopPose();
		pose->endFrame = poseFrames;
		pose->info = true;
		//2nd pose
		pose = loadingInfo->tAnim->LoopPose();
		pose->endFrame = 2*poseFrames;
		pose->info = false;

		//get line of text in screen
		textY = *pVarGroup->GetFloat("textLine");

		//get the text that will be displayed as done loading indicator 
		std::string text="";
		ByteStream = packMan->GetSubByteStream(textBufferID,"doneLoading.txt",&streamSize);

		pVarGroup->ParseVariablesFromMemory(ByteStream,streamSize);
		
		nlines = *pVarGroup->GetUint("numLines");//get number of lines

		fontIndex = *pVarGroup->GetUint("fontIndex");//get font index of font object used for this text

		float length=0.0f;
		for(unsigned int i = 0;i<nlines;++i)//loop throught all lines
		{
			char name[10]="textLine";
			name[8] = i + '0';
			name[9] = '\0';
			strcpy(line,pVarGroup->GetString(name));
			float len = pRenderer->GetStringLength(line,font[fontIndex]);
			if(length < len)
				length = len;//get max length of all lines

			text+=line;
			text+='\n';
			pVarGroup->DeleteVariable(name);
		}
		
		textX = (pRenderer->GetWidth() -  length)/2;

		pGUI->AddText(text.c_str(),textX,textY,font[fontIndex],&loadingInfo->textGUIitemID);//add text to GUI manager

		/*--------------------done getting infos-----------------------------*/
		pVar->RemoveVariableGroup(varGroup);

		packMan->ClearBuffer(bufferID);//no longer use this buffer

		AppFW->AddExitTask(OnExit3);

	}
	
	void BeginLoading(CallbackFunc progressTask,CallbackFunc doneTask)
	{
		LoadingProgress::progressTask = progressTask;
		LoadingProgress::doneTask = doneTask;

		userCont = false;
		pGUI->SetBackGround(loadingInfo->loadBgID,NULL,NULL);//set current background

		pRenderer->Enable2DMode();

		AppFW->StartThread(ProgressThreadFunc);

		loadingInfo->iAnim->StartAnimation();

		loadingInfo->tAnim->ResetCurrentPose();
		loadingInfo->tAnim->StartAnimation();
		
		//reset progress percentage value
		AppFW->LockMutex(mutexID);
		progress = 0;
		AppFW->UnlockMutex(mutexID);

		LoadingProgress::waitingForCont = false;
		
		AppFW->SetUpdateFunc(Updating);
		AppFW->SetRenderFunc(LoadingScreen);

		timer.Start();//reset starting time of timer
	}
};

void ProgressThreadFunc(void *arg)
{
	if(LoadingProgress::progressTask)
		LoadingProgress::progressTask();

	progress = 100;//finish task
}


void OnExit3()
{
	SafeDelete(loadingInfo);
}

void LoadingProgress::Updating()
{
	if(!PopUp::isPopping)//popup will freeze time
	{
		timer.Stop();
		loadingInfo->iAnim->LoopPose();
		if(progress == 100)//progress is finished.
			loadingInfo->tAnim->LoopPose();
		
	}
	if(waitingForCont)
	{
		const MouseState & mouseState = AppFW->GetMouseState();
		if(mouseState.flags & RIGHT_MOUSE_CLICK || mouseState.flags & LEFT_MOUSE_CLICK)
		{
			userCont = true;
			waitingForCont = false;
			
			timer.Start();//reset timer for beginning fading out loading screen
		}
	}
}

void LoadingProgress::LoadingScreen()
{
	
	double time=timer.GetElapsedTime();//get total time since timer was started

	//float totalTimeDisplay = SplashTime + 2*fadingTime ;//display duration + fading in duration + fading out duration

	float visible=1.0f;
	if(time < fadingTime)//fading in
		visible=(float)time/fadingTime;//visible -> opaque
	
	if(LoadingProgress::userCont)
	{
		visible = 1.0f - visible;//user continued ,so fading out
		if(time > fadingTime)
		{
			if(LoadingProgress::doneTask)
				LoadingProgress::doneTask();
			return;
		}
	}

	//draw background
	pGUI->DrawBackground();
	//draw xtreme logo
	pGUI->DrawImage(loadingInfo->xLogoGUIitemID);
	
	/*---------draw progress bar---------------*/
	int dw = pRenderer->GetWidth()/(loadingInfo->numPoints + 1);
	int progressStep = 100/(loadingInfo->numPoints - 1);
	unsigned int ip;//for calculating position of "pacman" icon in loading progress bar
	AppFW->LockMutex(mutexID);//variable <progress> is shared in another thread
	ip = progress / progressStep ;
	AppFW->UnlockMutex(mutexID);

	Rect* pRect;//pointer to rectangle area in image space that used for mapping to screen

	Rect dRect;//rectangle area in screen that icon will be drawed to
	dRect.bottom = pRenderer->GetHeight()/2 - loadingInfo->icon2Height/2;
	dRect.top = dRect.bottom + loadingInfo->icon2Height;

	//draw points in loading progress bar
	for(unsigned int i=0;i< loadingInfo->numPoints; ++i)
	{
		if(i == ip)//"pacman" is in this location
			continue;

		if(loadingInfo->points[i].passed == false && i < ip )
		{
			loadingInfo->points[i].passed = true;
			pRect = &loadingInfo->points[i].animCtrl.LoopPose()->info;
		}
		else
			pRect = &loadingInfo->points[i].animCtrl.CurrentPoseInfo()->info;
		dRect.left = (i+1) * dw - loadingInfo->icon2Width/2;
		dRect.right = dRect.left + loadingInfo->icon2Width;
		pRenderer->BlitTextureToScreen(loadingInfo->loadIconID,0.0f,pRect,&dRect);
	}

	//draw "pacman" icon
	dRect.left = (ip+1)*dw - loadingInfo->icon1Width/2;
	dRect.right = dRect.left + loadingInfo->icon1Width;
	dRect.bottom = pRenderer->GetHeight()/2 - loadingInfo->icon1Height/2;
	dRect.top = dRect.bottom + loadingInfo->icon1Height;

	pRect = &loadingInfo->iAnim->CurrentPoseInfo()->info;

	pRenderer->BlitTextureToScreen(loadingInfo->loadIconID,0.0f,pRect,&dRect);
	pRenderer->FadingScreen(1.0f - visible);

	/*--------draw done loading indicating text if progress is 100%-----*/
	if(!LoadingProgress::userCont && progress == 100)
	{
		if(!LoadingProgress::waitingForCont)
		{
			LoadingProgress::waitingForCont = true;
		}
		bool draw = loadingInfo->tAnim->CurrentPoseInfo()->info;
		if(draw)
			pGUI->Draw_Text(loadingInfo->textGUIitemID);
	}
	if(PopUp::isPopping)
	{
		PopUp::PopUpRendering();
	}
}

