#include "GameFlowControl.h"
#include "Game.h"
namespace Game{
	void DrawMap();
	void UpdatePlayer();
	void DrawWinLoseAnim();
};
void Game::KeyPressed(unsigned char key)
{
	if(PopUp::isPopping)//popup is showing,any key will not be accepted
		return;
	switch(key)
	{
	case 27://escape
		PopUp::BeginPopUp(PopUp::P_EXIT);
		break;
	case 'w':case 'W':
		if(gameInfo!=NULL && gameInfo->gameStarted)
			gameInfo->player->MoveUp();
		break;
	case 's':case 'S':
		if(gameInfo!=NULL && gameInfo->gameStarted)
			gameInfo->player->MoveDown();
		break;
	case 'a':case 'A':
		if(gameInfo!=NULL && gameInfo->gameStarted)
			gameInfo->player->MoveLeft();
		break;
	case 'd':case 'D':
		if(gameInfo!=NULL && gameInfo->gameStarted)
			gameInfo->player->MoveRight();
		break;
	case 32://space
		if(gameInfo!=NULL && gameInfo->gameStarted)
			gameInfo->paused = !gameInfo->paused;
		break;
	case '\t':
		if(gameInfo!=NULL && gameInfo->gameStarted)
		{
			drawFps = !drawFps;
			if(drawFps)
				AppFW->EnableCalFps();
			else
				AppFW->DisableCalFps();
		}
		break;
	}
	if(LoadingProgress::waitingForCont)
	{
		LoadingProgress::userCont = true;
		LoadingProgress::waitingForCont = false;
		
		timer.Start();//reset timer for beginning fading out loading screen
	}
}

void Game::GameUpdate()
{
	if(PopUp::isPopping || gameInfo->paused)//popup will freeze time
	{
		if(!gameInfo->havePopup)//this flag not set yet
		{
			DeActiveAllButtons();

			gameInfo->havePopup = true ; //set this flag to true
		}
		return;
	}
	if(gameInfo->havePopup)//there was a popup showed up before but not now , need to reactive buttons
	{
		ActiveAllButtons();

		gameInfo->havePopup = false ; //set this flag to false
	}
	if(gameInfo->invisible)
	{
		gameInfo->inviAnimCtrl.LoopPose();
		if(!gameInfo->inviAnimCtrl.IsAnimating())
		{
			gameInfo->invisible = false;
		}
	}
	timer.Stop();
	if(gameInfo->section)//going to change to other section
		return;
	/*------updating-------------*/
	const MouseState & mouseState = AppFW->GetMouseState();
	if(mouseState.flags & RIGHT_MOUSE_CLICK)//reset rotaion angle
	{
		gameInfo->currentXAngle = 0.0f;
		gameInfo->currentYAngle = 0.0f;
	}
	else if(mouseState.flags & LEFT_MOUSE_CLICK)
	{
		if (mouseState.dX < 0)
		{
			if (gameInfo->currentYAngle > gameInfo->minYAngle)
				gameInfo->currentYAngle -= gameInfo->anglePerFrame;
		}
		else if (mouseState.dX > 0)
		{
			if (gameInfo->currentYAngle < gameInfo->maxYAngle)
				gameInfo->currentYAngle += gameInfo->anglePerFrame;
		}
		if (mouseState.dY < 0)
		{
			if (gameInfo->currentXAngle > gameInfo->minXAngle)
				gameInfo->currentXAngle -= gameInfo->anglePerFrame;
		}
		else if (mouseState.dY > 0)
		{
			if (gameInfo->currentXAngle < gameInfo->maxXAngle)
				gameInfo->currentXAngle += gameInfo->anglePerFrame;
		}

	}
	Matrix4x4 rotX;
	Matrix4RotateX(gameInfo->currentXAngle,&rotX);
	Matrix4RotateY(gameInfo->currentYAngle,&gameInfo->rotationMatrix);
	gameInfo->rotationMatrix *= rotX;

	/*------player has lost/won----------------*/
	if(gameInfo->winLoseAnim.IsAnimating())
	{
		gameInfo->winLoseAnim.LoopPose();
		if(!gameInfo->winLoseAnim.IsAnimating())
		{
			if(gameInfo->win)//won
			{
				level = gameInfo->nextLevel;//to next level
				Game::ChangeSection(2);
			}
			else//lost
			{
				PopUp::BeginPopUp(PopUp::P_RETRY);//popup retry question dialog
			}
		}
		return;
	}
	/*-----------------------------------------*/
	gameInfo->coinCurrentAngle += gameInfo->coinRotAngle;

	Matrix4RotateY(gameInfo->coinCurrentAngle,&gameInfo->coinRotMatrix);

	UpdatePlayer();
	
	//get location in board of player
	int row,col;
	char info = gameInfo->board->GetTileInfo(gameInfo->player->GetPosition(),row,col);
	if(info == '3')//player hit coin
	{
		if(sound)
			pMixer->Play(gameInfo->coinSoundID,1);
		gameInfo->board->SetInfo(row,col,'1');
		gameInfo->coins.erase(row * gameInfo->board->GetTilesPerRow() + col);
		if (gameInfo->coins.size() == 0)//collected all coins
		{
			if(sound)
			{
				if(gameInfo->musicChannel!=-1)
				{
					pMixer->Stop(gameInfo->musicChannel);
					gameInfo->musicChannel = -1;
				}
				pMixer->Play(gameInfo->winMusicID,1);
			}
			gameInfo->win = true;
			gameInfo->winLoseAnim.StartAnimation(gameInfo->endSceneFrames);
			return;
		}
	}
	
	std::list<AIController*>::iterator ite;
	for(ite = gameInfo->ghosts.begin();ite != gameInfo->ghosts.end();++ite)
	{
		(*ite)->Control(gameInfo->board,gameInfo->player,gameInfo->invisible);
		if(!gameInfo->invisible)
		{
			if(Collision(*(*ite)->GetControlledObject(),*gameInfo->player,*gameInfo->board))//ghost caught player
			{
				gameInfo->life--;
				if(gameInfo->life == 0)//lose
				{
					if(sound)
					{
						if(gameInfo->musicChannel!=-1)
						{
							pMixer->Stop(gameInfo->musicChannel);
							gameInfo->musicChannel = -1;
						}
						pMixer->Play(gameInfo->loseMusicID,1);
					}
					gameInfo->winLoseAnim.StartAnimation(gameInfo->endSceneFrames);
					sprintf(gameInfo->lifeText,"%s 0",gameInfo->lifeTextPrefix);
					return;
				}
				if(sound)
					pMixer->Play(gameInfo->hurtSoundID,1);
				gameInfo->invisible = true;
				gameInfo->inviAnimCtrl.StartAnimation(gameInfo->maxInviFrames);
			}
		}
	}

	sprintf(gameInfo->lifeText,"%s %d",gameInfo->lifeTextPrefix,gameInfo->life);
	sprintf(gameInfo->coinText,"%s %u",gameInfo->coinTextPrefix,gameInfo->coins.size());
	/*--------------------------*/
}
void Game::GameRender()
{
	double time=timer.GetElapsedTime();//get total time since timer was started

	float fadeLevel=0.0f;
	if(time < fadingTime)//fading
	{
		if(gameInfo->section)//chaging to other section,so fade out
			fadeLevel=(float)time/fadingTime;
		else//fading in
			fadeLevel=1.0f-(float)time/fadingTime;
	}
	else if(gameInfo->section)//finished fading out,change to another section
	{
		switch(gameInfo->section)
		{
		case 1://to menu
			OnExit5();
			level = 1;//next time we start game again,we will be at first level
			LoadingProgress::BeginLoading( Menu::InitMenu,Menu::BeginDisplayMenu);
			break;
		case 2://to this section again (retry)
			OnExit5();
			LoadingProgress::BeginLoading( Game::InitGame,Game::BeginGame);
		}
		return;
	}
	/*--------background-------------*/
	pGUI->DrawBackground();
	
	/*----------main rendering-------*/
	pRenderer->SetViewMatrix(gameInfo->ViewMatrix);
	pRenderer->SetProjectionMatrix(gameInfo->ProjMatrix);

	pRenderer->SetWorldMatrix(gameInfo->rotationMatrix);
	//draw board
	gameInfo->board->Draw();
	
	
	//draw boxes
	std::list<StaticObject>::iterator ite;
	for(ite = gameInfo->boxes.begin();ite != gameInfo->boxes.end();++ite)
	{
		pRenderer->SetWorldMatrix(ite->translation * gameInfo->rotationMatrix);
		pMeshMan->DrawMesh(gameInfo->meshIDs[3]);
	}
	//draw coins
	std::map<int,StaticObject>::iterator mite;
	for(mite = gameInfo->coins.begin();mite != gameInfo->coins.end();++mite)
	{
		pRenderer->SetWorldMatrix(gameInfo->coinRotMatrix * mite->second.translation * gameInfo->rotationMatrix);
		pMeshMan->DrawMesh(gameInfo->meshIDs[2]);
	}
	//draw ghosts
	std::list<AIController*>::iterator ite2;
	for(ite2 = gameInfo->ghosts.begin();ite2 != gameInfo->ghosts.end();++ite2)
	{
		pRenderer->SetWorldMatrix((*ite2)->GetControlledObject()->GetTransformMatrix() * gameInfo->rotationMatrix);
		pMeshMan->DrawMesh(gameInfo->meshIDs[1]);
	}
	//draw player
	bool draw = true;
	if(gameInfo->invisible)
	{
		draw = gameInfo->inviAnimCtrl.CurrentPoseInfo()->info;
	}
	if(draw)
	{
		pRenderer->SetWorldMatrix(gameInfo->player->GetTransformMatrix() * gameInfo->rotationMatrix);
		pMeshMan->DrawMesh(gameInfo->meshIDs[0]);
	}

	//draw map
	DrawMap();
	/*-----------GUI-----------------*/
	pRenderer->Enable2DMode();
	pGUI->DrawButton(gameInfo->homeButtonID);
	if(sound)
	{
		pGUI->DrawButton(gameInfo->soundButtonID[0]);
		pGUI->DrawImage(gameInfo->crossImgID[0]);
	}
	else
	{
		pGUI->DrawButton(gameInfo->soundButtonID[1]);
		pGUI->DrawImage(gameInfo->crossImgID[1]);
	}

	pRenderer->EnableTextMode();
	
	//render text
	pRenderer->DrawString(gameInfo->lifeText,gameInfo->lifeTextFontID,
						  gameInfo->lifeTextX,gameInfo->lifeTextY);
	pRenderer->DrawString(gameInfo->coinText,gameInfo->coinTextFontID,
						  gameInfo->coinTextX,gameInfo->coinTextY);
	pRenderer->DrawString(gameInfo->levelText,gameInfo->levelTextFontID,
						  gameInfo->levelTextX,gameInfo->levelTextY);

	if(drawFps)
		AppFW->DrawFps(gameInfo->fpsTextX,gameInfo->fpsTextY,font[0]);

	pRenderer->DisableTextMode();

	/*------player has lost/won----------------*/
	if(gameInfo->winLoseAnim.IsAnimating())
		DrawWinLoseAnim();
	/*-----------------------------------------*/

	pRenderer->FadingScreen(fadeLevel);
	
	if(PopUp::isPopping)
	{
		PopUp::PopUpRendering();
	}

	pRenderer->Disable2DMode();
}

void Game::UpdatePlayer()
{
	const Vector3 &playerPos = gameInfo->player->GetPosition();
	const AABB &untransbox = gameInfo->player->GetUntranslateBoundingBox();
	bool playerMoving = gameInfo->player->IsMoving();
	float playerVelocity = gameInfo->player->GetVelocity();

	int row,col;//location of player in board

	switch(gameInfo->player->GetDirection())
	{
	case DOWN:
		if(playerMoving)
		{
			gameInfo->board->GetTileInfo(playerPos,row,col);//get location in board
			
			if(gameInfo->board->GetTileInfo(row + 1,col) == '5')//we have box in next tile
			{
				AABB box;
				box.vMax = (untransbox.vMax + playerPos) + Vector4(0,0,playerVelocity);
				box.vMin = (untransbox.vMin + playerPos) + Vector4(0,0,playerVelocity);
				if(gameInfo->board->IsBoxStandingAtTile(box,row + 1,col) )
				{
					gameInfo->player->StopMoving();
					break;
				}
			}
		}
		break;
	case UP:
		if(playerMoving)
		{
			gameInfo->board->GetTileInfo(playerPos,row,col);//get location in board
			
			if(gameInfo->board->GetTileInfo(row - 1,col) == '5')//we have box in next tile
			{
				AABB box;
				box.vMax = (untransbox.vMax + playerPos) - Vector4(0,0,playerVelocity);
				box.vMin = (untransbox.vMin + playerPos) - Vector4(0,0,playerVelocity);
				if(gameInfo->board->IsBoxStandingAtTile(box,row - 1,col) )
				{
					gameInfo->player->StopMoving();
					break;
				}
			}
		}
		break;
	case RIGHT:
		if(playerMoving)
		{
			gameInfo->board->GetTileInfo(playerPos,row,col);//get location in board
			
			if(gameInfo->board->GetTileInfo(row,col+1) == '5')//we have box in next tile
			{
				AABB box;
				box.vMax = (untransbox.vMax + playerPos) + Vector4(playerVelocity,0,0);
				box.vMin = (untransbox.vMin + playerPos) + Vector4(playerVelocity,0,0);
				if(gameInfo->board->IsBoxStandingAtTile(box,row,col + 1) )
				{
					gameInfo->player->StopMoving();
					break;
				}
			}
		}
		break;
	case LEFT:
		if(playerMoving)
		{
			gameInfo->board->GetTileInfo(playerPos,row,col);
			
			if(gameInfo->board->GetTileInfo(row,col - 1) == '5')//we have box in next tile
			{
				AABB box;
				box.vMax = (untransbox.vMax + playerPos) - Vector4(playerVelocity,0,0);
				box.vMin = (untransbox.vMin + playerPos) - Vector4(playerVelocity,0,0);
				if(gameInfo->board->IsBoxStandingAtTile(box,row,col - 1) )
				{
					gameInfo->player->StopMoving();
					break;
				}
			}
		}
		break;
	}

	gameInfo->player->Update();
}

void Game::DrawMap()
{
	pRenderer->DisableLighting();
	pRenderer->SetViewport(&gameInfo->mapRect);
	pRenderer->SetViewMatrix(gameInfo->eagleViewMatrix);
	pRenderer->SetProjectionMatrix(gameInfo->eagleOrthoProj);
	pRenderer->SetWorldMatrix(NULL);
	//draw board

	gameInfo->board->Draw();
	
	
	//draw boxes
	std::list<StaticObject>::iterator ite;
	for(ite = gameInfo->boxes.begin();ite != gameInfo->boxes.end();++ite)
	{
		pRenderer->SetWorldMatrix(ite->translation);
		pMeshMan->DrawMesh(gameInfo->meshIDs[3]);
	}
	/*
	//draw coins
	std::map<int,StaticObject>::iterator mite;
	for(mite = gameInfo->coins.begin();mite != gameInfo->coins.end();++mite)
	{
		pRenderer->SetWorldMatrix(gameInfo->coinRotMatrix * mite->second.translation);
		pMeshMan->DrawMesh(gameInfo->meshIDs[2]);
	}
	*/
	//draw ghosts
	std::list<AIController*>::iterator ite2;
	for(ite2 = gameInfo->ghosts.begin();ite2 != gameInfo->ghosts.end();++ite2)
	{
		pRenderer->SetWorldMatrix((*ite2)->GetControlledObject()->GetTransformMatrix());
		pMeshMan->DrawMesh(gameInfo->meshIDs[1]);
	}
	//draw player
	bool draw = true;
	if(gameInfo->invisible)
	{
		draw = gameInfo->inviAnimCtrl.CurrentPoseInfo()->info;
	}
	if(draw)
	{
		pRenderer->SetWorldMatrix(gameInfo->player->GetTransformMatrix());
		pMeshMan->DrawMesh(gameInfo->meshIDs[0]);
	}
	pRenderer->SetViewport(NULL);
	if(gameInfo->gsetup.numLights > 0)
		pRenderer->EnableLighting();
}

void Game::DrawWinLoseAnim()
{
	Rect rect[2];
	if(gameInfo->win)//draw win text
	{
		rect[0].left = gameInfo->wX[0];
		rect[0].right = gameInfo->wX[1];
		rect[0].bottom = gameInfo->winLoseAnim.CurrentPoseInfo()->info;
		rect[0].top = rect[0].bottom + gameInfo->wlH;
	
		rect[1].left = gameInfo->wX[2];
		rect[1].right = gameInfo->wX[3];
		rect[1].bottom =  pRenderer->GetHeight() - gameInfo->winLoseAnim.CurrentPoseInfo()->info;
		rect[1].top = rect[1].bottom + gameInfo->wlH;

		pRenderer->BlitTextureToScreen(gameInfo->winLoseTexID,0.0f,
									   &gameInfo->wlImgRect[0],&rect[0]);
		pRenderer->BlitTextureToScreen(gameInfo->winLoseTexID,0.0f,
									   &gameInfo->wlImgRect[1],&rect[1]);
	}
	else//draw lose text
	{
		rect[0].left = gameInfo->lX[0];
		rect[0].right = gameInfo->lX[1];
		rect[0].bottom = gameInfo->winLoseAnim.CurrentPoseInfo()->info;
		rect[0].top = rect[0].bottom + gameInfo->wlH;
	
		rect[1].left = gameInfo->lX[2];
		rect[1].right = gameInfo->lX[3];
		rect[1].bottom =  pRenderer->GetHeight() - gameInfo->winLoseAnim.CurrentPoseInfo()->info;
		rect[1].top = rect[1].bottom + gameInfo->wlH;
		pRenderer->BlitTextureToScreen(gameInfo->winLoseTexID,0.0f,
									   &gameInfo->wlImgRect[2],&rect[0]);
		pRenderer->BlitTextureToScreen(gameInfo->winLoseTexID,0.0f,
									   &gameInfo->wlImgRect[3],&rect[1]);
	}
}