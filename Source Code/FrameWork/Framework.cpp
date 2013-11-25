#include "stdafx.h"
#include "../Framework.h"

AppFrameWork AppFrameWork::instance;
bool AppFrameWork::ready;

/*----------------default callback functions-----------------*/
void GameLoop()
{
	AppFrameWork& fw=AppFrameWork::instance;
	
	if(fw.exit)
		exit(fw.exitCode);

	fw.timer.Start();//start timer
	LARGE_INTEGER start= fw.timer.GetStart();//get start check point
	/*-----update step---------------*/

	fw.pGui->UpdateButtonStatus();
	fw.updateFunc();

	/*-----rendering step-----------*/

	fw.pRenderer->BeginRender();//clear buffers

	fw.displayFunc();//main rendering step

	fw.pRenderer->EndRender();//flush buffers
	glutSwapBuffers();//swap frame buffers

	fw.timer.Stop();//stop timer

	fw.elapsedTime=fw.timer.GetElapsedTime();//get elapsed time
	//calculate sleep time
	fw.sleepTime = (fw.period - fw.elapsedTime) - fw.overSleepTime;

	if(fw.sleepTime > 0)//elapsed time shorter than desired period
	{
		fw.timer.Start();
		//so we sleep a bit
		Sleep(fw.sleepTime * 1000);
		fw.timer.Stop();
		fw.overSleepTime = fw.timer.GetElapsedTime() - fw.sleepTime;//get  oversleep time
	}
	else//we took longer than expected
	{
		fw.excessTime -= fw.sleepTime;//exceeded time
		fw.overSleepTime=0.0;
	}
	while (fw.excessTime > fw.period)
	{
		fw.excessTime -= fw.period;
		fw.updateFunc();//only update,not render
	}
	
	fw.timer.Stop();
	
	if(fw.calFps)//we will calculate frames per second
	{
		fw.frames++;//increase number of frames rendered
		fw.elapsedTime=fw.timer.GetElapsedTime(start,fw.timer.GetStop());//get total time spending in this function
		fw.totalTime += fw.elapsedTime;//total time since last reset
		if(fw.totalTime > fw.fpsUpdatePeriod)
		{
			//calculate average fps & reset frames and total time values
			fw.averageFps=fw.frames/fw.totalTime;
			fw.totalTime=0.0;
			fw.frames=0;
		}
		if(fw.averageFps == 0.0f)//first time calculation
			fw.averageFps = 1/fw.elapsedTime;
	}
}
void GameRender()
{
	AppFrameWork& fw=AppFrameWork::instance;
	fw.pRenderer->BeginRender();//clear buffers
	fw.displayFunc();//main rendering step
	fw.pRenderer->EndRender();//flush buffers
	glutSwapBuffers();//swap frame buffers
}

void Keyboard(unsigned char key,int x,int y)
{
	AppFrameWork& fw=AppFrameWork::instance;
	fw.keyPressFunc(key);
}
void Mouse(int button, int state,int x, int y)
{
	AppFrameWork& fw=AppFrameWork::instance;
	y = fw.pRenderer->GetHeight() - y;
	if(state == GLUT_DOWN)
	{
		if(button == GLUT_LEFT_BUTTON)
		{
			fw.pGui->IsAnyButtonClicked(x,y);
			fw.mouseState.flags |= LEFT_MOUSE_CLICK;
		}
		if(button == GLUT_RIGHT_BUTTON)
		{
			fw.mouseState.flags |= RIGHT_MOUSE_CLICK;
		}
	}
	else if (state == GLUT_UP)
	{
		if(button == GLUT_LEFT_BUTTON)
		{
			fw.mouseState.flags &= (~LEFT_MOUSE_CLICK);
		}
		if(button == GLUT_RIGHT_BUTTON)
		{
			fw.mouseState.flags &= (~RIGHT_MOUSE_CLICK);
		}
	}
}
void MouseMotion(int x,int y)
{
	AppFrameWork& fw=AppFrameWork::instance;
	fw.mouseState.dX = x - fw.mouseState.X;
	fw.mouseState.dY = y - fw.mouseState.Y;
	fw.mouseState.X = x;
	fw.mouseState.Y = y;

}
void DummyFunc(void)//do nothing
{
}

void DummyKeyFunc(unsigned char key)
{
}

/*---------------class methods------------------*/

AppFrameWork::AppFrameWork():pGui(),pVarMan()
{
	/*-----default values-----------------*/
	exit = false;

	unsigned int width=800;//window's width
	unsigned int height=600;//window's height
	unsigned int fps=60;//frames per second 
	ready=false;
	mutexes=NULL;
	numMutex=0;
	this->elapsedTime=this->overSleepTime=this->sleepTime=this->excessTime=this->totalTime=0.0;
	fpsUpdatePeriod=0.13;
	frames=0;
	calFps=false;
	memset(&mouseState,0,sizeof(MouseState));
	/*-------------------------------------*/
	//read config.ini
	FILE* f;
	f=fopen("../Config/config.ini","r");
	if(f)
	{
		fscanf(f,"width=%u\n",&width);
		fscanf(f,"height=%u\n",&height);
		fscanf(f,"fps=%u\n",&fps);

		unsigned int ms;
		fscanf(f,"fpsUpdatePeriod=%u\n",&ms);//miliseconds
		fpsUpdatePeriod = ms/1000.0f;
		fclose(f);
	}
	//calculate period of an game loop's iteration
	this->period=1.0/fps;

	pRendererDll=LoadLibrary(L"Renderer.dll");//load dll
	if(!pRendererDll)
		return;
	pCreateRendererProc createRenderer=(pCreateRendererProc)GetProcAddress(pRendererDll,"CreateRenderer");
	if(!createRenderer)
	{
		FreeLibrary(pRendererDll);
		pRendererDll=NULL;
		return;
	}

	pMixerDll=LoadLibrary(L"Mixer.dll");//load dll
	if(!pMixerDll)
		return;
	pCreateDefaultMixerProc createMixer=(pCreateDefaultMixerProc)GetProcAddress(pMixerDll,"CreateDefaultMixer");
	if(!createMixer)
	{
		FreeLibrary(pMixerDll);
		pMixerDll=NULL;
		return;
	}
	//create mixer

	pMixer=createMixer();
	if(pMixer==NULL)
		return;

	InitWindow(width,height);

	//create renderer

	pRenderer=createRenderer(width,height);
	if(pRenderer==NULL)
		return;
	//init default matrices for renderer
	Matrix4x4 matrix;
	Matrix4PerspectiveProjRH(_PIOVER4,(float)width/height,1.0f,1000.0f,&matrix);

	pRenderer->SetProjectionMatrix(matrix);

	Matrix4LookAtRH(&Vector4(0,0,0,1),&Vector4(0,0,-1,1),&Vector4(0,1,0,0),&matrix);

	pRenderer->SetViewMatrix(matrix);

	//create GUI manager
	pGui = SharedPtr<GUImanager>(new GUImanager(pRenderer),false);
	//create variable manager
	pVarMan = SharedPtr<VariableManager>(new VariableManager(),false);

	//create mesh manager
	pMeshMan = SharedPtr<MeshManager>(new MeshManager(pRenderer),false);

	//default callback
	updateFunc=&DummyFunc;
	displayFunc=&DummyFunc;
	keyPressFunc=&DummyKeyFunc;

	ready=true;//now ready to use
}

AppFrameWork::~AppFrameWork()
{
	/*----------exit tasks----------*/
	std::list<CallbackFunc>::iterator ite;
	for(ite = exitTasks.begin() ; ite != exitTasks.end(); ++ite)
	{
		(*ite)();
	}
	exitTasks.clear();
	/*------------------------------*/
	ready=false;

	if(mutexes)
	{
		for (unsigned int i=0;i<numMutex;++i)
		{
			CloseHandle(mutexes[i]);//destroy mutex
		}
		free(mutexes);
	}
	SafeRelease(pRenderer);
	if(pRendererDll)
	{
		FreeLibrary(pRendererDll);
		pRendererDll=NULL;
	}

	SafeRelease(pMixer);
	if(pMixerDll)
	{
		FreeLibrary(pMixerDll);
		pMixerDll=NULL;
	}
	

}

void AppFrameWork::InitWindow(unsigned int width,unsigned int height)
{
	char dummy[8];
	int argc=1;
	char *argv[1];
	argv[0]=dummy;
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH  |GLUT_RGBA);//set the display mode
    glutInitWindowSize(width, height); //set window size

	RECT desktop;
	SystemParametersInfo(SPI_GETWORKAREA,NULL,&desktop,NULL);
	if((int)height<desktop.bottom-desktop.top&&(int)width<desktop.right-desktop.left)//if window size smaller than desktop's screen size
		glutInitWindowPosition((desktop.right-width)/2,(desktop.bottom-height)/2); // set window position to the center of the desktop's screen
	else glutInitWindowPosition(0,0);
    glutCreateWindow("Path Man");	
	//register callback
	glutIdleFunc(GameLoop);
	glutDisplayFunc(GameRender);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(MouseMotion);
}

AppFrameWork *AppFrameWork::GetInstancePointer()
{
	if(!ready)//not ready yet
		return NULL;
	return &AppFrameWork::instance;
}

void AppFrameWork::StartThread(void (*func)(void *), void *argList)
{
	_beginthread(func,0,argList);//just a wrapper of win32 thread function
}

void AppFrameWork::Start()
{
	glutMainLoop();
}

void AppFrameWork::SetUpdateFunc(void (*func)(void))
{
	if(func==NULL)
		this->updateFunc=DummyFunc;
	else
		this->updateFunc=func;
}

void AppFrameWork::SetRenderFunc(void (*func)(void))
{
	if(func==NULL)
		this->displayFunc=DummyFunc;
	else
		this->displayFunc=func;
}

void AppFrameWork::SetKeyPressFunc(void (*func)(unsigned char))
{
	if(func==NULL)
		this->keyPressFunc=DummyKeyFunc;
	else
		this->keyPressFunc=func;
}



void AppFrameWork::AddExitTask(void (*func)(void))
{
	if(func == NULL)
		return;
	exitTasks.push_back(func);
}

bool AppFrameWork::CreateNewMutex(unsigned int *mutexID)
{
	if(numMutex%5 == 0)
	{
		unsigned int newMaxSize=numMutex+5;
		if(newMaxSize > MAX_ID)
			return false;
		HANDLE* newPtr=(HANDLE*)realloc(mutexes,newMaxSize*sizeof(HANDLE));//increase space by 5
		if(!newPtr)
			return false;
		mutexes=newPtr;
	}
	mutexes[numMutex]=CreateMutex(0,0,0);
	if(mutexID)
		*mutexID=numMutex;
	numMutex++;//increse number of mutexes by 1
	return true;
}

void AppFrameWork::LockMutex(unsigned int mutexID)
{
	if(mutexID >= numMutex)
		return;//invalid id
	WaitForSingleObject(mutexes[mutexID],INFINITE);
}
void AppFrameWork::UnlockMutex(unsigned int mutexID)
{
	if(mutexID >= numMutex)
		return;//invalid id
	ReleaseMutex(mutexes[mutexID]);
}


void AppFrameWork::EnableCalFps()//begin calculating fps
{
	calFps=true;
}
void AppFrameWork::DisableCalFps()//end calculating fps
{
	calFps= false;
	//reset values
	averageFps = 0;
	totalTime =0;
}
void AppFrameWork::DrawFps(float x,float y,unsigned int fontID)
{
	char str[50];
	RenderMode mode = pRenderer->GetCurrentMode();
	bool inTextMode = (mode == R_TEXT);
	if(!inTextMode )
	{
		pRenderer->EnableTextMode();
	}
	sprintf(str,"Fps : %.3f",averageFps);
	pRenderer->DrawString(str,fontID,x,y);//draw string

	if(!inTextMode )
	{
		pRenderer->DisableTextMode();
	}
}

unsigned int AppFrameWork::QueryFramesByTime(float time)
{
	return (int)(time / period);
}


void AppFrameWork::ExitApp(int exitCode)
{
	exit = true;
	this->exitCode = exitCode;
}