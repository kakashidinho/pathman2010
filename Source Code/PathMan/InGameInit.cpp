#include "HelperType.h"
#include "GameFlowControl.h"
#include "Game.h"
#include <list>

struct MeshSetupInfo
{
	char meshFileName[4][256];
};

GameInfo::GameInfo()
{
	musicChannel = -1;
	paused = false;
	board = NULL;
	life = 3;
	section = 0;//current section.
	havePopup = false;
	coinCurrentAngle = 0.0f;
	currentXAngle = 0.0f;
	currentYAngle = 0.0f;
	player = NULL;
	invisible = true;
	gameStarted = false;
	win = false;
}
GameInfo::~GameInfo()
{
	std::list<AIController*>::iterator ite;
	for(ite = gameInfo->ghosts.begin();ite != gameInfo->ghosts.end();++ite)
	{
		delete (*ite);
	}
	gameInfo->ghosts.clear();

	if(sound && musicChannel != -1)
		pMixer->Stop(musicChannel);

	pMixer->ReleaseAudio(musicID);
	pMixer->ReleaseAudio(hurtSoundID);
	pMixer->ReleaseAudio(coinSoundID);
	pMixer->ReleaseAudio(loseMusicID);
	pMixer->ReleaseAudio(winMusicID);

	SafeDelete(board);
	SafeDelete(player);
	pMeshMan->RemoveAllMeshes();
	pMeshMan->ReleaseTextures();
	pGUI->RemoveButton(homeButtonID);
	pGUI->RemoveButton(soundButtonID[0]);
	pGUI->RemoveButton(soundButtonID[1]);
	pGUI->RemoveImage(crossImgID[0]);
	pGUI->RemoveImage(crossImgID[1]);

	pRenderer->ReleaseTexture(bgTexID);
	pRenderer->ReleaseTexture(winLoseTexID);
	pRenderer->ReleaseTexture(boardTexID);
	pRenderer->DisableCullBackFace();
	for(int i=0;i<gsetup.numLights;++i)
	{
		pRenderer->DisableLightSource(i);
	}

	pRenderer->DisableLighting();

	pMeshMan->ReleaseTextures();
	if(packMan)
		packMan->ClearBuffer(meshPackageBufferID);
}



int level = 1;//first level
bool firstTimeInit = true;//first time init?
GameInfo *gameInfo = NULL;
TextureImageSrcInfo * pBgInfo = NULL;//for background texture
TextureImageSrcInfo * pBoardInfo = NULL;//for board texture
TextureImageSrcInfo * pWinLoseInfo = NULL;//for win/lose texture
MeshSetupInfo *meshSetup = NULL; 

void PressedHome();//home button clicked event handler

namespace Game
{
	void EnableSound();
	void DisableSound();
	void InitLevel();
	void ChangeSection(int section)
	{
		gameInfo->section = section;
		timer.Start();
	}
	void DeActiveAllButtons()
	{
		pGUI->DeActiveButton(gameInfo->homeButtonID);
		pGUI->DeActiveButton(gameInfo->soundButtonID[0]);
		pGUI->DeActiveButton(gameInfo->soundButtonID[1]);
	}

	void ActiveAllButtons()
	{
		pGUI->ActiveButton(gameInfo->homeButtonID);
		if(sound)
			pGUI->ActiveButton(gameInfo->soundButtonID[0]);
		else
			pGUI->ActiveButton(gameInfo->soundButtonID[1]);
	}

	void InitGame()
	{
		if(firstTimeInit)
		{
			AppFW->AddExitTask(OnExit5);
			firstTimeInit = false;
		}
		gameInfo = new GameInfo();
		pBgInfo = new TextureImageSrcInfo();
		pBoardInfo = new TextureImageSrcInfo();
		pWinLoseInfo = new TextureImageSrcInfo();
		meshSetup = new MeshSetupInfo;

		int iconIndex[4];
		Rect rect[2];
		unsigned char *ByteStream;
		float animDur,poseDur;
		unsigned int animFrames,poseFrames,numPoses;
		unsigned int bufferID,streamSize;
		unsigned int varGroup;


		packMan->UnPack("../Resources/InGame.qpackage",&bufferID);//unpack package containing resources for menu
		
		LoadingProgress::SetProgressVal(10);
		Sleep(10);
		
		ByteStream=packMan->GetSubByteStream(bufferID,"init.txt",&streamSize);//get byte stream from package
		
		/*------parse setting info-----------*/
		/*---create variable group-----------*/
		pVar->CreateVariableGroup(&varGroup);
		SharedPtr<VariableGroup> pVarGroup = pVar->GetVariableGroup(varGroup); 
		pVarGroup->ParseVariablesFromMemory(ByteStream,streamSize);
		
		LoadingProgress::SetProgressVal(20);
		Sleep(10);
		//get line's location for drawing fps indicating text
		float f[2];
		memcpy(f,pVarGroup->GetFloat2("locFps"),2*sizeof(float));
		gameInfo->fpsTextX = f[0];
		gameInfo->fpsTextY = f[1];

		//get rotation info
		gameInfo->anglePerFrame = (*pVarGroup->GetFloat("anglePerSecond")) * _PI / (180.0f * AppFW->QueryFramesByTime(1.0f));
		gameInfo->maxXAngle = (*pVarGroup->GetFloat("maxXAngle")) * _PI /180.0f;
		gameInfo->minXAngle = (*pVarGroup->GetFloat("minXAngle")) * _PI /180.0f;
		gameInfo->maxYAngle = (*pVarGroup->GetFloat("maxYAngle")) * _PI /180.0f;
		gameInfo->minYAngle = (*pVarGroup->GetFloat("minYAngle")) * _PI /180.0f;
		//get text info

		strcpy(gameInfo->lifeTextPrefix,pVarGroup->GetString("lifeTextPrefix"));
		strcpy(gameInfo->coinTextPrefix,pVarGroup->GetString("coinTextPrefix"));
		
		sprintf(gameInfo->levelText,"%s %d",pVarGroup->GetString("levelTextPrefix"),level);

		memcpy(f,pVarGroup->GetFloat2("lifeTextLoc"),2*sizeof(float));
		gameInfo->lifeTextX = f[0];
		gameInfo->lifeTextY = f[1];

		memcpy(f,pVarGroup->GetFloat2("coinTextLoc"),2*sizeof(float));
		gameInfo->coinTextX = f[0];
		gameInfo->coinTextY = f[1];

		memcpy(f,pVarGroup->GetFloat2("levelTextLoc"),2*sizeof(float));
		gameInfo->levelTextX = f[0];
		gameInfo->levelTextY = f[1];

		gameInfo->lifeTextFontID = font[*pVarGroup->GetUint("lifeTextFont")];
		gameInfo->coinTextFontID = font[*pVarGroup->GetUint("coinTextFont")];
		gameInfo->levelTextFontID = font[*pVarGroup->GetUint("levelTextFont")];

		//get background texture info
		strcpy(pBgInfo->imageName,pVarGroup->GetString("bgImage"));
		pBgInfo->bufferID = bufferID;

		LoadingProgress::SetProgressVal(30);
		Sleep(10);

		//button animation info
		animDur = *pVarGroup->GetFloat("banimDuration");
		poseDur = *pVarGroup->GetFloat("bposeDuration");
		
		animFrames = AppFW->QueryFramesByTime(animDur);
		poseFrames = AppFW->QueryFramesByTime(poseDur);

		//home button icon
		iconIndex[0] = *pVarGroup->GetInt("icon0");
		iconIndex[1] = *pVarGroup->GetInt("icon1");

		//button rectangle on screen 
		memcpy(&rect[0],pVarGroup->GetInt4("rect0"),4*sizeof(int));

		pGUI->AddButton(PlayButtonClickSound,PressedHome,animFrames,poseFrames,
						icons[iconIndex[0]].texID,icons[iconIndex[0]].rect,
						icons[iconIndex[1]].texID,icons[iconIndex[1]].rect,
						rect[0],&gameInfo->homeButtonID);

		//sound button info
		iconIndex[0] = *pVarGroup->GetInt("icon2");
		iconIndex[1] = *pVarGroup->GetInt("icon3");
		iconIndex[2] = *pVarGroup->GetInt("icon4");//cross icon
		iconIndex[3] = *pVarGroup->GetInt("icon5");//cross icon

		memcpy(&rect[0],pVarGroup->GetInt4("rect1"),4*sizeof(int));
		memcpy(&rect[1],pVarGroup->GetInt4("rect2"),4*sizeof(int));
		//disable sound button
		pGUI->AddButton(PlayButtonClickSound,DisableSound,animFrames,poseFrames,
			icons[iconIndex[0]].texID,icons[iconIndex[0]].rect,
			icons[iconIndex[1]].texID,icons[iconIndex[1]].rect,
			rect[0],&gameInfo->soundButtonID[0]);

		//enable sound button
		pGUI->AddButton(PlayButtonClickSound,EnableSound,animFrames,poseFrames,
			icons[iconIndex[1]].texID,icons[iconIndex[1]].rect,
			icons[iconIndex[0]].texID,icons[iconIndex[0]].rect,
			rect[0],&gameInfo->soundButtonID[1]);

		//cross image
		pGUI->AddImage(icons[iconIndex[2]].texID,&icons[iconIndex[2]].rect,&rect[1],&gameInfo->crossImgID[0]);
		pGUI->AddImage(icons[iconIndex[3]].texID,&icons[iconIndex[3]].rect,&rect[1],&gameInfo->crossImgID[1]);

		LoadingProgress::SetProgressVal(40);
		
		//get invisible animation info
		poseDur = *pVarGroup->GetFloat("vposeDuration");

		poseFrames = AppFW->QueryFramesByTime(poseDur);

		AnimController<bool> inviAnim(2);

		PoseInfo<bool>* pose = inviAnim.LoopPose();
		pose->endFrame = poseFrames;
		pose->info = true;

		pose = inviAnim.LoopPose();
		pose->endFrame = 2 * poseFrames;
		pose->info = false;

		gameInfo->inviAnimCtrl = inviAnim;

		gameInfo->maxInviFrames = AppFW->QueryFramesByTime(3.0f);//invisible in max 3 seconds
		
		//get win/lose text animation info
		strcpy(pWinLoseInfo->imageName,pVarGroup->GetString("wlImage"));

		animDur = *pVarGroup->GetFloat("wlanimDuration");
		gameInfo->endSceneFrames = AppFW->QueryFramesByTime(animDur);
		//get rectangle area of text in image
		memcpy(&gameInfo->wlImgRect[0],pVarGroup->GetInt4("winRect0"),4*sizeof(int));
		memcpy(&gameInfo->wlImgRect[1],pVarGroup->GetInt4("winRect1"),4*sizeof(int));
		memcpy(&gameInfo->wlImgRect[2],pVarGroup->GetInt4("loseRect0"),4*sizeof(int));
		memcpy(&gameInfo->wlImgRect[3],pVarGroup->GetInt4("loseRect1"),4*sizeof(int));

		gameInfo->wlH = *pVarGroup->GetInt("wlH");

		memcpy(gameInfo->wX,pVarGroup->GetInt4("wX"),4*sizeof(int));
		memcpy(gameInfo->lX,pVarGroup->GetInt4("lX"),4*sizeof(int));
		
		numPoses = *pVarGroup->GetUint("wlanimPoses"); 
		poseDur = *pVarGroup->GetFloat("wlposeDuration");
		poseFrames = AppFW->QueryFramesByTime(poseDur);
		if(poseFrames == 0)
			poseFrames = 1;
		AnimController<int> wlAnimCtrl(numPoses);
		for(unsigned int i=0;i < numPoses ;++i)
		{
			char vname[5] = "wlY";
			vname[3] = '0' + i;
			vname[4] = '\0';
			PoseInfo<int> *poseInfo = wlAnimCtrl.LoopPose();
			poseInfo->endFrame = (i+1) * poseFrames;
			poseInfo->info = *pVarGroup->GetInt(vname);
		}

		gameInfo->winLoseAnim = wlAnimCtrl;

		//get map rectangle
		memcpy(&gameInfo->mapRect,pVarGroup->GetInt4("mapRect"),4*sizeof(int));
		
		//get sound info
		ByteStream = packMan->GetSubByteStream(audioBufferID,pVarGroup->GetString("hurt"),&streamSize);
		pMixer->LoadAudioFromMemory(ByteStream,streamSize,gameInfo->hurtSoundID);

		ByteStream = packMan->GetSubByteStream(audioBufferID,pVarGroup->GetString("coin"),&streamSize);
		pMixer->LoadAudioFromMemory(ByteStream,streamSize,gameInfo->coinSoundID);

		ByteStream = packMan->GetSubByteStream(audioBufferID,pVarGroup->GetString("lose"),&streamSize);
		pMixer->LoadAudioFromMemory(ByteStream,streamSize,gameInfo->loseMusicID);

		ByteStream = packMan->GetSubByteStream(audioBufferID,pVarGroup->GetString("win"),&streamSize);
		pMixer->LoadAudioFromMemory(ByteStream,streamSize,gameInfo->winMusicID);

		/*----------------------------*/

		pVar->RemoveVariableGroup(varGroup);

		//unpack mesh package

		packMan->UnPack("../Resources/Mesh.qpackage",&gameInfo->meshPackageBufferID);
		
		/*--------read level info------------*/
		InitLevel();
		
	}
	void BeginGame()
	{
		//load background texture 
		unsigned char *ByteStream;
		unsigned int streamSize;
		ByteStream = packMan->GetSubByteStream(pBgInfo->bufferID,pBgInfo->imageName,&streamSize);
		if(pRenderer->LoadTextureFromMemory(ByteStream,streamSize,&gameInfo->bgTexID)!=R_OK)
			exit(-1);
		//load win/lose texture
		ByteStream = packMan->GetSubByteStream(pBgInfo->bufferID,pWinLoseInfo->imageName,&streamSize);
		if(pRenderer->LoadTextureFromMemory(ByteStream,streamSize,&gameInfo->winLoseTexID)!=R_OK)
			exit(-1);

		packMan->ClearBuffer(pBgInfo->bufferID);//release buffer contains byte stream of background image & win/lose image
		SafeDelete(pBgInfo);//no longer need it
		
		//load floor texture 
		ByteStream = packMan->GetSubByteStream(pBoardInfo->bufferID,pBoardInfo->imageName,&streamSize);
		if(pRenderer->LoadTextureFromMemory(ByteStream,streamSize,&gameInfo->boardTexID)!=R_OK)
			exit(-1);
		
		gameInfo->board->InitGraphicsResource(gameInfo->boardTexID);//init graphics resources for rendering board

		packMan->ClearBuffer(pBoardInfo->bufferID);//release buffer contains byte stream of board'floor image
		SafeDelete(pBoardInfo);//no longer need it

		pRenderer->Disable2DMode();

		/*-------createMesh------------------*/
		pMeshMan->CreateMeshFromPackage(packMan,gameInfo->meshPackageBufferID,
										meshSetup->meshFileName[0],&gameInfo->meshIDs[0]);//player
		pMeshMan->CreateMeshFromPackage(packMan,gameInfo->meshPackageBufferID,
										meshSetup->meshFileName[1],&gameInfo->meshIDs[1]);//ghost
		pMeshMan->CreateMeshFromPackage(packMan,gameInfo->meshPackageBufferID,
										meshSetup->meshFileName[2],&gameInfo->meshIDs[2]);//coin
		pMeshMan->CreateMeshFromPackage(packMan,gameInfo->meshPackageBufferID,
										meshSetup->meshFileName[3],&gameInfo->meshIDs[3]);//box
		
		SafeDelete(meshSetup);
		
		gameInfo->player->ComputeObjectAABB(*pMeshMan->GetMeshAABB(gameInfo->meshIDs[0]));

		/*-------setup camera & light--------*/
		
		
		for(int i=0;i<gameInfo->gsetup.numLights;++i)
		{
			pRenderer->EnableLighting();
			pRenderer->EnableLightSource(i);
			pRenderer->SetupDirectionLight(i,gameInfo->gsetup.dLights[i]);
		}
		
		SafeDeleteArray(gameInfo->gsetup.dLights);
		/*----------------------------------*/
		pRenderer->EnableCullBackFace();

		AppFW->SetUpdateFunc(GameUpdate);
		AppFW->SetRenderFunc(GameRender);
		
		ActiveAllButtons();

		pGUI->SetBackGround(gameInfo->bgTexID,NULL,NULL);//set current background
			
		gameInfo->gameStarted = true;

		gameInfo->inviAnimCtrl.StartAnimation(gameInfo->maxInviFrames);//loop invisible animation in 3 seconds
		
		if(sound)
		{
			gameInfo->musicChannel = pMixer->Play(gameInfo->musicID,0);
			pGUI->ActiveButton(gameInfo->soundButtonID[0]);
			pGUI->DeActiveButton(gameInfo->soundButtonID[1]);
		}
		else
		{
			pGUI->DeActiveButton(gameInfo->soundButtonID[0]);
			pGUI->ActiveButton(gameInfo->soundButtonID[1]);
		}

		timer.Start();
	}
};

void Game::InitLevel()
{
	unsigned char *ByteStream;
	unsigned int bufferID,streamSize;
	unsigned int varGroup;
	
	pVar->CreateVariableGroup(&varGroup);
	SharedPtr<VariableGroup> pVarGroup = pVar->GetVariableGroup(varGroup); 

	LoadingProgress::SetProgressVal(50);
	Sleep(10);

	char boardInfoFile[256];
	char levelFile[29] = "../Resources/Level1.qpackage";
	levelFile[18] = '0' + level;	//get the right level package file	name

	packMan->UnPack(levelFile,&bufferID);
	
	strcpy(levelFile,"Level1.txt");
	levelFile[5] = '0' + level;	//get the right level file name

	ByteStream = packMan->GetSubByteStream(bufferID,levelFile,&streamSize);
	pVarGroup->ParseVariablesFromMemory(ByteStream,streamSize);
	
	gameInfo->nextLevel = *pVarGroup->GetInt("nextLevel");
	
	strcpy(pBoardInfo->imageName,pVarGroup->GetString("boardImg"));
	pBoardInfo->bufferID = bufferID;

	LoadingProgress::SetProgressVal(60);
	Sleep(10);


	/*------------------get sound info-------------------*/
	ByteStream = packMan->GetSubByteStream(audioBufferID,pVarGroup->GetString("music"),&streamSize);

	pMixer->LoadAudioFromMemory(ByteStream,streamSize,gameInfo->musicID);

	/*------------setup board------------------*/

	strcpy(boardInfoFile,pVarGroup->GetString("boardFile"));
	Vector3 boardCorner;//topleft corner position of board
	memcpy(&boardCorner,pVarGroup->GetFloat3("boardCorner"),3*sizeof(float));
	
	int numRowTiles = *pVarGroup->GetInt("rowTiles");
	float boardWidth = *pVarGroup->GetFloat("boardWidth");
	
	ByteStream = packMan->GetSubByteStream(bufferID,boardInfoFile,&streamSize);

	gameInfo->board = new Board(ByteStream,streamSize,boardCorner,numRowTiles,boardWidth);

	/*-------setup light & camera-----------*/
	gameInfo->gsetup.numLights = *pVarGroup->GetInt("numLights");
	gameInfo->gsetup.dLights = new DirectionLight[gameInfo->gsetup.numLights];//number of lights
	
	LoadingProgress::SetProgressVal(70);
	Sleep(10);

	for(int i =0; i< gameInfo->gsetup.numLights; ++i)
	{
		char lightVar[10];
		sprintf(lightVar,"lightA%d",i);
		memcpy(&gameInfo->gsetup.dLights[i].ambient,pVarGroup->GetFloat4(lightVar),4*sizeof(float));
		sprintf(lightVar,"lightD%d",i);
		memcpy(&gameInfo->gsetup.dLights[i].diffuse,pVarGroup->GetFloat4(lightVar),4*sizeof(float));
		sprintf(lightVar,"lightS%d",i);
		memcpy(&gameInfo->gsetup.dLights[i].specular,pVarGroup->GetFloat4(lightVar),4*sizeof(float));
		sprintf(lightVar,"lightDir%d",i);
		memcpy(gameInfo->gsetup.dLights[i].direction,pVarGroup->GetFloat3(lightVar),3*sizeof(float));
	}
	
	memcpy(&gameInfo->gsetup.cameraPos,pVarGroup->GetFloat3("eye"),3*sizeof(float));
	memcpy(&gameInfo->gsetup.cameraLookAt,pVarGroup->GetFloat3("at"),3*sizeof(float));
	gameInfo->gsetup.fov = *pVarGroup->GetFloat("fov");
	
	//main view matrix
	Matrix4LookAtRH(&Vector4(gameInfo->gsetup.cameraPos,1),
					&Vector4(gameInfo->gsetup.cameraLookAt,1),
					&Vector4(0,1,0,0),&gameInfo->ViewMatrix);

	//main projection matrix
	Matrix4PerspectiveProjRH(gameInfo->gsetup.fov * _PI/180.0f,(float)pRenderer->GetWidth()/pRenderer->GetHeight(),
								1.0f,1000.0f,&gameInfo->ProjMatrix);
	
	//eagle view matrix
	Matrix4OrthoProjRH(boardWidth,
					   boardWidth,
					   1.0f,1000.0f,&gameInfo->eagleOrthoProj);
	//eagle projection matrix
	Vector3 boardCenterPoint;
	gameInfo->board->GetCenterPoint(boardCenterPoint);
	Matrix4LookAtRH(&Vector4(boardCenterPoint.x,boardCenterPoint.y+900,boardCenterPoint.z,1),
					&Vector4(boardCenterPoint,1),&Vector4(0,0,-1,0),&gameInfo->eagleViewMatrix);

	LoadingProgress::SetProgressVal(80);
	Sleep(10);

	/*---------init meshes & objects--------------------*/
	
	strcpy(meshSetup->meshFileName[0],pVarGroup->GetString("mc"));
	strcpy(meshSetup->meshFileName[1],pVarGroup->GetString("ghost"));
	strcpy(meshSetup->meshFileName[2],pVarGroup->GetString("coin"));
	strcpy(meshSetup->meshFileName[3],pVarGroup->GetString("box"));

	gameInfo->coinRotAngle = (*pVarGroup->GetFloat("coinRotAngle"))/AppFW->QueryFramesByTime(1.0f);
	gameInfo->coinRotAngle *= _PI/180.0f;
	
	
	unsigned int desiredFps = AppFW->QueryFramesByTime(1.0f);//desired frames per second
	float mcV = *pVarGroup->GetFloat("mcv");//player 's velocity in 1 second
	mcV/=desiredFps;//velocity in 1 frame

	float ghostV = *pVarGroup->GetFloat("ghostv");//ghost's velocity in second
	ghostV/=desiredFps;//velocity in 1 frame

	float ros = *pVarGroup->GetFloat("ros");//ghost 's range of sight for detecting player
	
	///setup coins & boxes list
	for(int i =0;i<numRowTiles;++i)
	{
		for(int j =0;j<numRowTiles;++j)
		{
			switch(gameInfo->board->GetTileInfo(i,j))
			{
			case '3'://coin
				{
					Vector3 position;
					gameInfo->board->GetTileCenterPoint(i,j,position);
					StaticObject object ;
					object.row = i;
					object.col =j;
					Matrix4Translate(position.x,position.y,position.z,&object.translation);
					
					gameInfo->coins[i * numRowTiles + j] = object;
				}
				break;
			case '5'://box
				{
					Vector3 position;
					gameInfo->board->GetTileCenterPoint(i,j,position);
					StaticObject object ;
					object.row = i;
					object.col =j;
					Matrix4Translate(position.x,position.y,position.z,&object.translation);

					gameInfo->boxes.push_back(object);
				}
				break;
			case '4'://ghost
				{
					Vector3 position;
					gameInfo->board->GetTileCenterPoint(i,j,position);
					AIController *pGhostController = new AIController(ghostV,position,ros);

					gameInfo->ghosts.push_back(pGhostController);
				}
				break;
			case '2'://player
				{
					Vector3 position;
					gameInfo->board->GetTileCenterPoint(i,j,position);
					
					gameInfo->player = new MovableObject(mcV,position);
				}
				break;
			}
		}
	}

	LoadingProgress::SetProgressVal(90);
	Sleep(10);
	/*--------------------------------------*/
	
	pVar->RemoveVariableGroup(varGroup);

}


void Game::OnExit5()
{
	SafeDelete(pBgInfo);
	SafeDelete(pBoardInfo);
	SafeDelete(gameInfo);
	SafeDelete(meshSetup);
	SafeDelete(pWinLoseInfo);
}

void PressedHome()
{
	PopUp::BeginPopUp(PopUp::P_MENU);
}

void Game::EnableSound()
{
	sound = true;
	gameInfo->musicChannel = pMixer->Play(gameInfo->musicID,0);
	pGUI->DeActiveButton(gameInfo->soundButtonID[1]);
	pGUI->ActiveButton(gameInfo->soundButtonID[0]);
}
void Game::DisableSound()
{
	sound = false;
	pMixer->Stop(gameInfo->musicChannel);
	pGUI->ActiveButton(gameInfo->soundButtonID[1]);
	pGUI->DeActiveButton(gameInfo->soundButtonID[0]);
}