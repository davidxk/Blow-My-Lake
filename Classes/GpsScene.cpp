#include "GlobalPath.h"
#include "GpsScene.h"
#include "AppMacros.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#endif
USING_NS_CC;

CCScene* Gps::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    Gps *layer = Gps::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool Gps::init()
{
    if ( !CCLayer::init() )
    {
        return false;
    }
	//csv文件读取数据
	CsvUtil::sharedCsvUtil()->loadFile(MAP_CSV_PATH);
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
	//longitude latitude
	longitude = 0;
	latitude = 0;
	pLabel = CCLabelTTF::create("", "Arial", 60);
    pLabel->setPosition(ccp(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - pLabel->getContentSize().height-150));
	pLabel2 = CCLabelTTF::create("", "Arial", 30);
	pLabel2->setPosition(ccp(origin.x + visibleSize.width/5,
		origin.y + visibleSize.height - pLabel2->getContentSize().height-50));
	
	
	this->addChild(pLabel,10);
	this->addChild(pLabel2,10);

	this->schedule(schedule_selector(Gps::updateView),3.0f);

	//加载地图
	wholeMap = CCSprite::create(WHOLE_MAP_MAP_PATH);
	wholeMap->setAnchorPoint(CCPointZero);
	wholeMap->setPosition(CCPointZero);
	this->addChild(wholeMap);
	//加载位置指针
    mapPoint = CCSprite::create(MAP_POINT_MAP_PATH);
	mapPoint->setPosition(ccp(200,150));
	this->addChild(mapPoint,10);
	touchPoint = CCSprite::create(TOUCH_POINT_MAP_PATH);
	touchPoint->setPosition(ccp(200,150));
	this->addChild(touchPoint,10);
	
	pLabel3 = CCLabelTTF::create("", "Arial", 30);
	pLabel3->setPosition(ccp(touchPoint->getPosition().x,touchPoint->getPosition().y+80));
	this->addChild(pLabel3,10);

	CCSprite* m1 = CCSprite::create("mapinfo/menu3.png");
	CCSprite* m2 = CCSprite::create("mapinfo/menu3_select.png");
	CCSprite* m3 = CCSprite::create("mapinfo/menu3.png");
	CCMenuItemSprite* menu3= CCMenuItemSprite::create(m1,m2,m3,this,menu_selector(Gps::menu3CallBack));
	CCMenu* returnMenu = CCMenu::create(menu3,NULL);
	returnMenu->alignItemsHorizontally();
	returnMenu->setPosition(ccp(610,420));
	this->addChild(returnMenu);

	//触摸事件
	this->setTouchEnabled(true);

    return true;
}


void Gps::updateView(float dt)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	int num;
	JniMethodInfo minfo;
	bool isHave = JniHelper::getStaticMethodInfo(minfo,"com/bluemylake/game/hello_cocos","getInstance","()Lcom/bluemylake/game/hello_cocos;");
	jobject jobj;//存对象
	if (isHave) {
		//这里的调用getInstance，返回Test类的对象。
		jobj = minfo.env->CallStaticObjectMethod(minfo.classID, minfo.methodID);

		isHave = JniHelper::getMethodInfo(minfo,"com/bluemylake/game/hello_cocos","getLocationJ","()D");
		if (isHave) {
			//调用openWebview, 参数1：Test对象   参数2：方法ID
			longitude = minfo.env->CallDoubleMethod(jobj, minfo.methodID);
		}
		isHave = JniHelper::getMethodInfo(minfo,"com/bluemylake/game/hello_cocos","getLocationW","()D");
		if (isHave) {
			//调用openWebview, 参数1：Test对象   参数2：方法ID
			latitude = minfo.env->CallDoubleMethod(jobj, minfo.methodID);
		}	
	}
	//CCString* str = CCString::createWithFormat("%f",(float)longitude);
	//pLabel->setString(str->getCString());

	/*CCString* str2 = CCString::createWithFormat("%f",(float)latitude);
	pLabel2->setString(str2->getCString());*/
	num = Distance();
	showPoint(num);
#else
	//CCString* str = CCString::createWithFormat("%f",(float)longitude);
	pLabel2->setString("无GPS信号");
	pLabel2->setColor(ccc3(0,0,200));
	mapPoint->setPosition(ccp(0,0));
#endif
	
}

void Gps::showPoint(int num)
{
	CCPoint pointPos;
	pointPos = getpointPos(num);
	CCMoveTo* movTo = CCMoveTo::create(0.2,pointPos);
	mapPoint->runAction(movTo);
	showPosName(num);
}

void Gps::showPosName(int num)
{
	const char* name;
	name = CsvUtil::sharedCsvUtil()->get(num,1,MAP_CSV_PATH);
	pLabel2->setString(name);
	pLabel2->setColor(ccc3(0,0,200));
}


CCPoint Gps::getpointPos(int num)
{
	CCPoint pos;
	pos.x = CsvUtil::sharedCsvUtil()->getInt(num,4,MAP_CSV_PATH);
	pos.y = CsvUtil::sharedCsvUtil()->getInt(num,5,MAP_CSV_PATH);
	CCLOG("坐标：%d,%d",pos.x,pos.y);
	return pos;
}


int Gps::Distance()
{//算与经纬度确定的点距离最短的点的序号
	int i;
	int num=0;
	double lon,lat;
	double d;
	double mindis=10.0;
	//int n=70;
	for (i=0;i<PLACE_NUM;i++)
	{
		lat = (double)CsvUtil::sharedCsvUtil()->getFloat(i,2,MAP_CSV_PATH);
		lon = (double)CsvUtil::sharedCsvUtil()->getFloat(i,3,MAP_CSV_PATH);
		d = sqrt((lat-latitude)*(lat-latitude)+(lon-longitude)*(lon-longitude));
		CCLOG("%d,%lf",num,d);

		if (d<mindis)
		{
			mindis = d;
			num = i;
		}
	}
	return num;
}





void Gps::menu3CallBack(CCObject* pSender){
	CCSprite* map=CCSprite::create("mapinfo/map.png");
	float scaleX=map->getContentSize().width/wholeMap->getContentSize().width;
	float scaleY=map->getContentSize().height/wholeMap->getContentSize().height;
	CCActionInterval* seq=CCSequence::create(
			CCScaleTo::create(0.5f,scaleX,scaleY),
			CCCallFunc::create(this,callfunc_selector(Gps::delayedPop)),
			NULL);
	CCSpawn* conc=CCSpawn::create(seq,CCMoveTo::create(0.5f,GAL_MAP_POS),NULL);
	wholeMap->runAction(conc);
}

void Gps::delayedPop()
{
	CsvUtil::sharedCsvUtil()->releaseFile(MAP_CSV_PATH);
	CCDirector::sharedDirector()->popScene();
}


//触摸事件
void Gps::ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent)
{
	int num;
	CCTouch* touch = (CCTouch*)pTouches->anyObject();
	CCPoint touchPos = touch->getLocationInView();
	num=touchDistance(touchPos);
	showTouchPoint(num);
}

//
int Gps::touchDistance(CCPoint touchPos)
{
	int i;
	int num=0;
	double mapx,mapy;
	double d;
	double mindis=1000.0;
	//int n=70;
	CCPoint pos;
	for (i=0;i<PLACE_NUM;i++)
	{
		mapx = (double)CsvUtil::sharedCsvUtil()->getInt(i,4,MAP_CSV_PATH);
		mapy = (double)CsvUtil::sharedCsvUtil()->getInt(i,5,MAP_CSV_PATH);
		d = sqrt((mapx-touchPos.x)*(mapx-touchPos.x)+(mapy-(448-touchPos.y))*(mapy-(448-touchPos.y)));
		//CCLOG("%d,%f",num,d);

		if (d<mindis)
		{
			mindis = d;
			num=i;
		}
	}
	return num;
}

//显示touchpoint
void Gps::showTouchPoint(int num)
{
	CCPoint pointPos;
	pointPos = getpointPos(num);
	CCMoveTo* movTo = CCMoveTo::create(0.2,pointPos);
	touchPoint->runAction(movTo);
	showTouchPosName(num,pointPos);
}

void Gps::showTouchPosName(int num,CCPoint pos)
{
	const char* name;
	name = CsvUtil::sharedCsvUtil()->get(num,1,MAP_CSV_PATH);
	pLabel3->setString(name);
	pLabel3->setColor(ccc3(0,0,200));
	if (pos.y>300)
		pLabel3->setPosition(ccp(pos.x+30,pos.y));
	else
		pLabel3->setPosition(ccp(pos.x,pos.y+80));
}
