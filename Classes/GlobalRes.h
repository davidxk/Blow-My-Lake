#ifndef _GLOBAL_RES_H_
#define _GLOBAL_RES_H_
#include "Singleton.hpp"
#include "HumanEntity.h"
#include "DiaWindow.h"
#include "Backpack.h"

class GlobalRes
{
public:
    CCTMXTiledMap* map;
    HumanEntity* hero;
    Window* rwindow;
    DiaWindow* diawindow;
    Backpack* backpack;
};
//note that when an event wish to walk the hero, do as follow:
//WalkingMan* walkHero=(WalkingMan*)rGloble->hero;
//walkHero->walk(vect);
typedef Singleton<GlobalRes> sGlobalRes;
#define rGlobal sGlobalRes::instance()
#endif