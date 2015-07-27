﻿#include "CombatScene.h"

USING_NS_CC;

CCScene* Combat::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    // 'layer' is an autorelease object
    Combat *layer = Combat::create();
    // add layer as a child to scene
    scene->addChild(layer);
    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool Combat::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }
    
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
	//角色创建
	cplayer = CPlayer::create();
	cplayer->setPlayer();
	this->addChild(cplayer);
	monster = Monster::create();
	monster->setMonster();
	this->addChild(monster);
	//血条创建
	playerblood = Blood::create();
	playerblood->setBloodSlider();
	playerblood->setSliderPosition(ccp(150,visibleSize.height-50));
	playerblood->setTotalBlood(cplayer->healthPoint);
	playerblood->setCurrentBlood(cplayer->currentHp);
	
	monsterblood = Blood::create();
	monsterblood->setBloodSlider();
	monsterblood->setSliderPosition(ccp(visibleSize.width-150,visibleSize.height-50));
	monsterblood->setTotalBlood(monster->healthPoint);
	monsterblood->setCurrentBlood(monster->currentHp);

	this->addChild(playerblood);
	this->addChild(monsterblood);

	//按钮创建
	playerbutton = AbilityButton::create();
	monsterbutton = AbilityButton::create();
	playerbutton->CreateButton();
	playerbutton->setButtonPosition(ccp(50,50));
	playerbutton->updateDamage(cplayer->level);
	monsterbutton->CreateButton();
	monsterbutton->setButtonPosition(ccp(420,50));
	monsterbutton->LockButtonTouch();
	monsterbutton->updateDamage(monster->level);
	this->addChild(playerbutton);
	this->addChild(monsterbutton);


	this->scheduleUpdate();

    return true;
}



void Combat::update(float delta)
{
	if (playerbutton->isTouch==true)
	{
		//锁定playerbutton触摸
		playerbutton->LockButtonTouch();
		//怪物选择button
		monsterButtonTag = monster->chooseButton();
		//判断属性胜负
		int winnerNum = checkButtonTag(playerbutton->tag,monsterButtonTag);
		CCLOG("win:%d",winnerNum);
		//伤害计算
		int damage = damageCompute(winnerNum);
		CCLOG("damage:%d",damage);
		//播放战斗动画
		//更新血条
		updateBlood(winnerNum,damage);

		playerbutton->isTouch=false;
		//判断是否结束
		checkGameOver();

		
	}
	
}

//判断双方选择属性的胜负
int Combat::checkButtonTag(int playerTag,int monsterTag)
{
	//平局
	if (playerTag==monsterTag)
	{
		if(playerbutton->getDamageByTag(playerTag)>=monsterbutton->getDamageByTag(monsterTag))
			return playerWin;
		else
			return monsterWin;
	}
	else if ((playerTag==0&&monsterTag==3)||(playerTag==1&&monsterTag==0)||(playerTag==2&&monsterTag==1))
	{
		return playerWin;
	}
	else
	{
		return monsterWin;
	}
}

//伤害计算
int Combat::damageCompute(int winnerNum)
{
	int damage,pdamage,mdamage;
	//平局情况伤害为双方差值
	if (playerbutton->tag==monsterButtonTag)
	{
		pdamage = playerbutton->getDamageByTag(playerbutton->tag);
		mdamage = monsterbutton->getDamageByTag(monsterButtonTag);
		damage = abs(pdamage-mdamage);
	}
	//非平局情况伤害值
	else
	{
		if (winnerNum==playerWin)
		{
			damage = playerbutton->getDamageByTag(playerbutton->tag);
		}
		else if (winnerNum==monsterWin)
		{
			damage = monsterbutton->getDamageByTag(monsterButtonTag);
		}
	}
	return damage;
}

//刷新血条
void Combat::updateBlood(int winnerNum,int damage)
{
	if (winnerNum==playerWin)
	{
		monster->currentHp-=damage; 
		if(monster->currentHp==0)
			monster->currentHp=0;
	}
	else
	{
		cplayer->currentHp-=damage;
		if(cplayer->currentHp==0)
			cplayer->currentHp=0;
	}

	playerblood->setCurrentBlood(cplayer->currentHp);
	monsterblood->setCurrentBlood(monster->currentHp);
}

void Combat::checkGameOver()
{
	if (cplayer->currentHp<=0)
	{
		CCLOG("monster win!");
	}
	else if (monster->currentHp<=0)
	{
		CCLOG("player win!");
	}
	else
	{
		//刷新一次button
		playerbutton->updateDamage(cplayer->level);
		monsterbutton->updateDamage(monster->level);
		//恢复按钮可触摸状态
		playerbutton->UnlockButtonTouch();
	}
		
}