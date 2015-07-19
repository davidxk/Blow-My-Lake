#include "Hero.h"

bool Hero::init() 
{
    setID(0); picNo=0; isWalking=false;  
    map=(Map*)rGlobal->map;
    dir=sGlobal->mapState->faceDir;

    WalkingMan::initAnimation();
    initSprite();
    initAction(dir);
    sprite->setAnchorPoint(CCPointZero+ccp(0.5,-0.625));//x+1�������ܣ�y+1��������
    return true;
}

void Hero::doEvent(CCPoint heroTilePos)
{
    //@EventManager takes over from here
    CCPoint facingTile = getHeroTilePos()+
	ccp((move/(map->getTileSize().height)).x,
		-(move/(map->getTileSize().height)).y);
    ControllerListener* lst=eManager->happen(facingTile,STAND_TRIG);

    //��ȡ����event��tile id
    int tileGid = map->layerNamed(EVENT_MLYR)->tileGIDAt(heroTilePos); 
    if (!tileGid)return;

    CCDictionary *properties = map->propertiesForGID(tileGid);
    if (!properties)return;

    const CCString *eventScene = properties->valueForKey("stand");
    //·���¼�
    if(!eventScene->compare("place name"))
    {
	int curPlaceID=properties->valueForKey("id")->intValue();
	rGlobal->rwindow->load(map,properties);
	rGlobal->rwindow->respond(curPlaceID);
    }
    //�����л��¼�inte
    else if(!eventScene->compare("change scene"))
    {
	int id=properties->valueForKey("id")->intValue();
	switch(id)
	{
	    case 0:case 1:case 2:case 3:case 4:
	    case 5:case 6:case 7:case 8:case 9:
		{
		    if(id==sGlobal->mapState->storyCnt)
		    {
			touchEnded=dir;walkEnd();//endWalking
			this->focus=false;
			CCEGLView::sharedOpenGLView()->setDesignResolutionSize(JX_RESOLUWID, JX_RESOLUHEI, kResolutionExactFit);
			CCScene* story=StoryWorld::scene();
			CCDirector::sharedDirector()->pushScene(story);
			this->gotFocusT();
			sGlobal->mapState->storyCnt++;
		    }
		}
		break;

	    case MAP11:
		touchEnded=dir;walkEnd();
		map->removeAllChildrenWithCleanup(true);
		map=Map::create(MAP12_PATH);//@
		map->crossMap(heroTilePos,MAP11);
		this->getParent()->addChild(map);
		break;

	    case MAP12:
		touchEnded=dir;walkEnd();
		map->removeAllChildrenWithCleanup(true);
		map=Map::create(MAP11_PATH);//@
		map->crossMap(heroTilePos,MAP12);
		this->getParent()->addChild(map);
		break;
	    default:break;
	}
    }
    else if(!eventScene->compare("combat"))
    {
	float i = CCRANDOM_0_1();
	if (i<0.3)
	{
	    touchEnded=dir;walkEnd();//endWalking
	    this->focus=false;
	    CCEGLView::sharedOpenGLView()->setDesignResolutionSize(JX_RESOLUWID, JX_RESOLUHEI, kResolutionExactFit);
	    CCScene* combat=Combat::scene();
	    CCDirector::sharedDirector()->pushScene(combat);//@
	}
	this->focus=true;
    ;}
    else if(!eventScene->compare("portal"))
    {
	touchEnded=dir;walkEnd();//endWalking
	this->focus=false;
	int x=properties->valueForKey("idx")->intValue();//@
	int y=properties->valueForKey("idy")->intValue();
	CCPoint dest=ccp(x,y);
	CCAnimate* spin=CCAnimate::create(this->createAnimationForDirection(5));//����
	CCRepeat* spinM=CCRepeat::create(spin,6);
	CCCallFunc* regotF=CCCallFunc::create(this,callfunc_selector(Hero::gotFocusT));
	CCSequence* goSpin=CCSequence::create(spinM,regotF,NULL);
	CCDelayTime* delay=CCDelayTime::create(1);
	CCPlace* mov=CCPlace::create(map->humanPosForTileMove(dest));
	CCSequence* fly=CCSequence::create(delay,mov,NULL);
	sprite->runAction(goSpin);
	map->runAction(fly);
    }     
}








void Hero::respond(int dir) 
{
    if(isWalking||!focus){return;}
    touchEnded=-1;
    initAction(dir);
    letsGo(0);
}

void Hero::endRespond()
{
    touchEnded=dir;
}

void Hero::stepDown()
{
    if (checkEvent(getHeroTilePos())==kEvent)
	doEvent(getHeroTilePos());
    walkEnd();
}

void Hero::stepUp()
{
    CCPoint heroCoord=getHeroTilePos();
    CCPoint moveCoord=move/map->getTileSize().height;
    if(checkCollision(ccp(heroCoord.x+moveCoord.x,
		    heroCoord.y-moveCoord.y))==kWall)
    {
	touchEnded=dir;walkEnd();
    }
}

void Hero::walkEnd()
{
    if(touchEnded>-1)
    {
	map->stopAction(moveMap);
	sprite->stopAction(moveLegs);
	moveMap=NULL,moveLegs=NULL;
	setFaceDirection(dir);
	isWalking=false;

	sGlobal->mapState->positionX=getHeroTilePos().x; 
	sGlobal->mapState->positionY=getHeroTilePos().y;
	sGlobal->mapState->faceDir=dir;
    }
}

void Hero::letsGo(float dt)
{
    isWalking = true;
    map->runAction(moveMap);
    sprite->runAction(moveLegs);
}

void Hero::initAction(int dir)
{
    CCPoint moveByPosition;
    switch(dir)
    {
	case Down:moveByPosition=ccp(0,-32);break;
	case Left:moveByPosition=ccp(-32,0);break;
	case Right:moveByPosition=ccp(32,0);break;
	case Up:moveByPosition=ccp(0,32);break;
	default:break;
    }
    CCPoint revMoveByPosition=-moveByPosition;
    this->move=moveByPosition,this->dir=dir;

    //���ߺ�λ����������
    const float duration=STD_WALK_DURATION/sGlobal->superPower->speed;
    //CCJumpBy* jump=CCJumpBy::create(duration,ccp(0,0),16,1);
    CCMoveBy* revShift=CCMoveBy::create(duration,revMoveByPosition);
    CCAnimate* anim=CCAnimate::create(walkAnimations[dir]);//����
    CCDelayTime* delay=CCDelayTime::create(1.5f/32.0f*duration*1.8);
    CCCallFunc* stepU=CCCallFunc::create(this,callfunc_selector(Hero::stepUp));
    CCCallFunc* stepD=CCCallFunc::create(this,callfunc_selector(Hero::stepDown));
    //CCSpawn* animJ=CCSpawn::create(anim,jump,NULL);
    CCSequence* revAct=CCSequence::create(stepU,delay,revShift,stepD,NULL);
    moveLegs=CCRepeatForever::create(anim);
    moveMap=CCRepeatForever::create(revAct);
}








//��ͼ�¼����
CollisionType Hero::checkEvent(CCPoint tileCoord)
{
    int tileGid = map->layerNamed(EVENT_MLYR)->tileGIDAt(tileCoord);
    if (tileGid)return kEvent;  
    if (eManager->eStand->tileAt(tileCoord)>NO_EVENT_FLAG)return kEvent;  
    return kNone;
}

//��ײ���
CollisionType Hero::checkCollision(CCPoint tileCoord)
{
    //������ͼ�߽緵��kWall
    if (tileCoord.x<0||tileCoord.x>map->getMapSize().width-1||
	    tileCoord.y>map->getMapSize().height-2||tileCoord.y<0)
	return kWall;

    int wTileGid= map->layerNamed(WALL_MLYR)->tileGIDAt(tileCoord);
    int bTileGid= map->layerNamed(BUILDING_MLYR)->tileGIDAt(tileCoord);
    int b2TileGid= map->layerNamed(BUILDING2_MLYR)->tileGIDAt(tileCoord);
    int wtTileGid= map->layerNamed(WATER_MLYR)->tileGIDAt(tileCoord);
    int eTileGid= map->layerNamed(EVENT_MLYR)->tileGIDAt(tileCoord);bool batrg=false;
    if(eTileGid)batrg=map->propertiesForGID(eTileGid)->valueForKey("atrg")->compare("")!=0;
    if(wTileGid||bTileGid||b2TileGid||batrg||wtTileGid)return kWall;
    if(eManager->eAtrgr->tileAt(tileCoord)>NO_EVENT_FLAG)return kWall;
    return kNone;
}

CCPoint Hero::getHeroTilePos()
{
    return map->tileCoordFromPosition(getPosition()-map->getPosition());
}

void Hero::gotFocusT()
{
    focus=true;
}