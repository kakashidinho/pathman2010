#if defined(_DEBUG)||defined (DEBUG)

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#endif


#include <windows.h>
#include "GameFlowControl.h"
#include "Game.h"

AppFrameWork* AppFW=NULL;
Renderer * pRenderer=NULL;
Mixer *pMixer = NULL;
SharedPtr<GUImanager> pGUI;
SharedPtr<VariableManager> pVar ;
SharedPtr<MeshManager> pMeshMan;
StopWatch timer;
PackageManager *packMan=NULL;//for handling package file

void OnExit1()
{
	SafeDeleteArray(font);
	SafeDeleteArray(icons);
	SafeDelete(packMan);
}




#ifdef _WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#elif defined (_CONSOLE)
int main()
#endif
{
#if defined (_DEBUG) || defined(DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	packMan=CreatePackageManager();
	AppFW = AppFrameWork::GetInstancePointer();//get app framework object's pointer
	if(!AppFW)
		return -1;

	pRenderer=AppFW->GetRenderer();//get renderer pointer
	if(!pRenderer)
		return -2;

	pGUI = AppFW->GetGUImanager();//get GUI manager
	if(pGUI == NULL)
		return -3;

	pVar = AppFW->GetVariableManager();//get variable manager
	if(pVar == NULL)
		return -4;

	pMeshMan = AppFW->GetMeshManager();//get mesh manager
	if(pMeshMan == NULL)
		return -5;

	pMixer = AppFW->GetMixer();//get mixer
	if(pMixer == NULL)
		return -6;
	

	AppFW->SetRenderFunc(Intro::IntroRendering);
	AppFW->SetKeyPressFunc(Game::KeyPressed);
	
	AppFW->AddExitTask(OnExit1);

	Intro::InitIntro();
	AppFW->Start();
	return 0;
}
