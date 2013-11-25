#ifndef _2D_ANIM_CONTROLLER_
#define _2D_ANIM_CONTROLLER_

#include "SharedPointer.h"


/*---------template class pose Info - contains infomation about 1 pose in animation---------------------------------*/
template<class T>
struct PoseInfo
{
	PoseInfo()
	{
		endFrame = 1;
	};
	T info;//user defined info type class
	unsigned int endFrame;//frame number in animation loop that this pose will stop displaying.
};
/*-----------------------------
2D sprite animation controller
----------------------------*/


template<class T>
class AnimController
{
private:
	SharedPtr<PoseInfo<T>> poseInfos;//list of infos about each pose
	unsigned int numPose;//number of poses
	unsigned int currentPose;//current pose
	unsigned int frameCount;//number of frames counted since animation starts
	unsigned int maxFrames;//max number of frames counted since animation starts,if frames counted reaches this value,animation will stop.If this value is 0,animation will loop forever
	bool animationStart;//is animation started?
public:
	AnimController();
	AnimController(unsigned int numPose);
	AnimController(const AnimController& src);//copy contructor
	~AnimController();
	
	AnimController<T> & operator=(const AnimController& src);//copy operation
	
	/*
	loop through pose list.
	Returned value is pointer to current pose's info
	If animation not started,call this function will cause current pose to advance to next one.
	If animation started,call this function will increase counted frames ,
	*/
	PoseInfo<T>* LoopPose();
	PoseInfo<T>* CurrentPoseInfo();//get info of current pose
	void ResetCurrentPose();
	void StartAnimation(unsigned int maxFrames = 0);//start animation loop in <maxFrames> frames.if <maxFrames>=0 animation will loop forever
	void StopAnimation();
	bool IsAnimating(){return animationStart;};
};
template<class T>
AnimController<T>::AnimController() 
{
	animationStart=false;
	frameCount = currentPose = 0;
	this->numPose = 0;
	maxFrames = 0;
}

template<class T>
AnimController<T>::AnimController(unsigned int numPose) : poseInfos(new PoseInfo<T>[numPose],true)
{
	animationStart=false;
	frameCount = currentPose = 0;
	this->numPose = numPose;
	maxFrames = 0;
}

template<class T>
AnimController<T>::AnimController(const AnimController<T>& src):poseInfos(src.poseInfos)
{
	animationStart=false;
	frameCount = currentPose = 0;
	this->numPose = src.numPose;
	maxFrames = 0;
}

template<class T>
AnimController<T>::~AnimController()
{
}

template<class T>
AnimController<T>& AnimController<T>::operator =(const AnimController<T>& src)
{
	if(this != &src)
	{
		poseInfos = src.poseInfos;
		animationStart=false;
		frameCount =currentPose = 0;
		this->numPose = src.numPose;
	}
	return *this;
}

template<class T>
void AnimController<T>::StartAnimation(unsigned int maxFrames)
{
	animationStart=true;
	frameCount = 0;
	this->maxFrames = maxFrames;
}

template<class T>
void AnimController<T>::StopAnimation()
{
	animationStart=false;
	ResetCurrentPose();
}

template<class T>
PoseInfo<T>* AnimController<T>::LoopPose()
{
	if(numPose == 0)
		return NULL;
	PoseInfo<T> * result=&poseInfos[currentPose];
	
	if(!animationStart)
		currentPose++;
	else
	{
		//animating
		frameCount++;
		if((frameCount% (poseInfos[numPose-1].endFrame + 1))== poseInfos[currentPose].endFrame)
			currentPose++;
		if(maxFrames!=0 && frameCount == maxFrames)
			this->StopAnimation();
	}
	if(currentPose >= numPose)
	{
		//reset to starting pose
		currentPose = 0 ;
	}
	return result;//return current pose
}

template <class T>
PoseInfo<T>* AnimController<T>::CurrentPoseInfo()
{
	if(numPose == 0)
		return NULL;
	if(currentPose >= numPose)
	{
		currentPose = 0 ;
	}
	return &poseInfos[currentPose];
}

template <class T>
void AnimController<T>::ResetCurrentPose()
{
	currentPose =0;
	frameCount = 0;
}


#endif