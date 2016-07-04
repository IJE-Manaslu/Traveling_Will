#include "tw_playable_level.h"
#include "tw_level_factory.h"
#include "tw_result.h"

Level * TWLevelFactory::make_level(const string& level_id){
    if(level_id == "menu")
        return new TWMenu("menu", "limbo", "res/menu/menu.mp3", -1);
    if(level_id == "menu")
        return new TWMenu("menu", "cutscene-intro", "res/menu/menu.mp3", -1);
    else if(level_id == "cutscene-intro")
        return new TWCutscene("cutscene-intro", "1", "res/cutscene-intro/cutscene-intro.mp3", 28000);
    else if(level_id == "1")
        return new TWPlayableLevel("1", "result", "res/1/music.wav", 31000);
	else if(level_id == "result")
        return new TWResult("result", "2", "res/1/music.wav", 31000);
    else if(level_id == "2")
        return new TWPlayableLevel("2", "3", "res/2/music.wav", 10000);
    else if(level_id == "3")
        return new TWPlayableLevel("3", "cutscene-end", "res/3/music.wav", 147000);
    else if(level_id == "4")
        return new TWPlayableLevel("4", "cutscene-end", "res/4/music.wav", 147000);
    else if(level_id == "5")
        return new TWPlayableLevel("5", "cutscene-end", "res/5/music.wav", 147000);
    else if(level_id == "6")
        return new TWPlayableLevel("6", "cutscene-end", "res/6/music.wav", 147000);
    else if(level_id == "limbo")
        return new TWLimbo("limbo", "", "res/1/music.wav", -1); //FIXME
    else
        return nullptr;
}

void TWLevelFactory::release(Level *level){
    delete level;
}
