#include "traveling_will_playable_level.h"

#include <ijengine/canvas.h>
#include <ijengine/engine.h>
#include <ijengine/keyboard_event.h>

#include <unistd.h>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace ijengine;

static const int GAME_EVENT_JUMP =              1 << 4;
static const int GAME_EVENT_SLIDE_PRESSED =     1 << 5;
static const int GAME_EVENT_SLIDE_RELEASED =    1 << 6;
static const int GAME_EVENT_MENU_SELECT =       1 << 7;
static const int GAME_MOUSE_CLICK =             1 << 8;
static const int GAME_MOUSE_MOVEMENT =          1 << 9;
static const int GAME_MOUSE_MOTION =            1 << 10;
static const int GAME_EVENT_PUNCH =             1 << 11;
static const int NUMBER_OF_SCREENS =            12;
static const int WILL_HEIGHT =                  45;
static const int WILL_WIDTH =                   57;
static const int COLLECTABLE_DIMENSION =        34;
static const int COLLECTABLE_SIZE =             WILL_HEIGHT + COLLECTABLE_DIMENSION;
static const int ENEMY_DIMENSION =              45;
static const int ENEMY_SIZE =                   WILL_HEIGHT + ENEMY_DIMENSION;
static const int BACK_BUTTON =                  0;

TravelingWillPlayableLevel::TravelingWillPlayableLevel(int r, int g, int b, const string &current_level, const string& next_level, const string audio_path, 
int audio_duration) : 
	level_started(false), level_finished(false), m_will_enemy_type(-1), collectable_it(-10000000), enemy_it(-10000000), m_r(r), m_g(g), m_b(b),
	m_audio_duration(audio_duration), m_audio_start(0), m_audio_counter(0), m_start(-1), change(0), n_collectables(0), n_enemies(0), m_y_speed(0), 
	sprite_counter(0), m_sprite_speed(1/170.0), m_camera_x(0), m_reverse_camera_x(1), m_reverse_camera_y(480), m_will_collectable(-100), m_will_enemy(-100), 
	m_x_speed(5/19.0), m_camera_y(0), m_will_x(50), m_boss_x(690), m_boss_y(190), m_number(resources::get_texture("numbers.png")){

	m_current_level = current_level;
	m_audio = audio_path;
	m_next = next_level;
	m_done = false;
	m_state = RUNNING;

	m_progress_bar[0] = resources::get_texture("whole-progress-bar.png");
	m_progress_bar[1] = resources::get_texture("progress-bar.png"); 
	m_progress_bar[2] = resources::get_texture("begin-progress-bar.png");
	m_will_progress_bar = resources::get_texture("tiny-will-progress-bar.png");

	//Read level design from txt
	fstream level_design("res/" + m_current_level + "/level_design.txt");
	if(not level_design.is_open()){
		printf("Level design txt not available\n");
		exit(0);
	}
	level_design >> n_screens;

	platform_height = new double[n_screens + 1];
	collectable_height = new double[n_screens + 1];
	enemy_height = new double[n_screens + 1];

	collectable = new int[n_screens + 1];
	enemy = new int[n_screens + 1];
	enemy_type = new int[n_screens + 1];
	level_it = new int[n_screens + 1];

	for(int i = n_screens - 1; i >= 0; i--){
		level_design >> platform_height[i];
		level_design >> enemy[i];

		if(enemy[i]){
			level_design >> enemy_type[i];
			level_design >> enemy_height[i];
		} 

		level_design >> collectable[i];

		if(collectable[i]){
			level_design >> collectable_height[i];
		}
	}
	level_design.close();
	

    //Sets initial will height based on level design
	m_will_y = m_will_floor = 480 - platform_height[n_screens - 1] - WILL_HEIGHT;


	//Get platforms textures
	for(int i = 1; i < 9; i++){
	    m_level[i] = resources::get_texture(m_current_level + "/" + to_string(i*50) + ".png");
    }

	m_boss = resources::get_texture(m_current_level + "/perdeu.png");

	if(current_level == "1"){
		m_background[0] = resources::get_texture(m_current_level + "/background_floresta_0.png");
		m_background[1] = resources::get_texture(m_current_level + "/background_floresta_1.png");
		m_background[2] = resources::get_texture(m_current_level + "/background_floresta_2.png");

		m_will[RUNNING] = resources::get_texture(m_current_level + "/will-running.png");
		m_will[JUMPING] = resources::get_texture(m_current_level + "/will-jumping.png");
		m_will[SLIDING] = resources::get_texture(m_current_level + "/will-sliding.png");
		m_will[FALLING] = resources::get_texture(m_current_level + "/will-falling.png");
		m_will[GAME_OVER] = resources::get_texture(m_current_level + "/will-gameover.png");
		m_will[PUNCHING]= resources::get_texture(m_current_level + "/will-punching.png");

		m_enemy[0] = resources::get_texture(m_current_level + "/enemy1.png");
		m_enemy[1] = resources::get_texture(m_current_level + "/enemy2.png");

		m_collectable = resources::get_texture(m_current_level + "/collectable.png");
		m_collectable_icon = resources::get_texture(m_current_level + "/collectable_icon.png");
	}

	event::register_listener(this);
}

TravelingWillPlayableLevel::~TravelingWillPlayableLevel(){
    event::unregister_listener(this);
}

bool TravelingWillPlayableLevel::done() const{
    return m_done;
}

string TravelingWillPlayableLevel::next() const{
    return m_next;
}

string TravelingWillPlayableLevel::audio() const{
    return m_audio;
}

bool TravelingWillPlayableLevel::on_event(const GameEvent &event){
	if(m_state != GAME_OVER){
		if(event.id() == GAME_EVENT_PUNCH && m_state != SLIDING && event.timestamp() - m_punch_counter > 230){
            m_is_punching = true;
            m_punch_counter = event.timestamp();
            printf("PUNCHING\n\n\n");
            return true;
        }

        if(event.id() == GAME_EVENT_JUMP && m_state == RUNNING){
            m_y_speed = -0.5;
            m_state = JUMPING;
            return true;
        }   

        if(event.id() == GAME_EVENT_SLIDE_PRESSED && m_state != JUMPING && m_state != FALLING){
            m_state = SLIDING;
            return true;
        }

        if(event.id() == GAME_EVENT_SLIDE_RELEASED && m_state == SLIDING){
            m_state = RUNNING;
            return true;
        }
	}

	return false;
}

void TravelingWillPlayableLevel::update_self(unsigned now, unsigned){
	if(m_start == -1){
        m_start = now;
        m_audio_start = m_start;
    }

	//Update counters based on time
	sprite_counter += (now - m_start) * m_sprite_speed;
	m_camera_x += (now - m_start) * m_x_speed;
	m_reverse_camera_x += (now - m_start) * m_x_speed;
	m_will_y += (now - m_start) * m_y_speed;

	if(m_state != GAME_OVER && not level_finished){
	    m_audio_counter = now - m_audio_start;
	}

	//Checking if music has ended
    if(m_audio_duration != -1 && m_audio_counter >= m_audio_duration){
        m_state = RUNNING;
        m_done = true;
    }

	//Reset value of reverse camera for each part of the level
    if(m_reverse_camera_x > 142 && m_current_level == "1"){
        m_reverse_camera_x -= 142;
        change++;
    }

	//Reset background camera
    if(m_camera_x > 1704){
        m_camera_x -= 1704;
    }

    //Reset sprite counter
    if(sprite_counter > 5.9){
        sprite_counter -= 5.9;
    }

    if(m_is_punching && (now - m_punch_counter) > 80){
		m_punch_counter = now;
        m_is_punching = false;
    }

	//Test Will colision
    if(m_will_y > m_will_floor + 20){
        m_state = GAME_OVER;
    }

    //Start jump if Will is at the end of a cliff
    if(m_will_y < m_will_floor && m_state != JUMPING && m_state != FALLING && m_state != GAME_OVER){
        m_y_speed = 1/300.0 * 0.5;
        m_state = FALLING;
    }

    //Calculate jump speed and stop jump if hits the ground
    if(m_state == JUMPING || m_state == FALLING){
        m_y_speed += (now - m_start)/300.0 * 0.5;
        if(m_y_speed >= 0.001) m_state = FALLING;

        if(m_will_y + (now - m_start) * m_y_speed > m_will_floor){
            m_will_y = m_will_floor;
            m_y_speed = 0;
            m_state = RUNNING;
        }
    }

	int aux = n_screens - 7, height, it;

    int aux_it = 0;
    for(int i =(int)m_reverse_camera_x; i <= 994; i += 142){
        it = aux - change;
        level_it[aux_it++] = max(it, 0);

        if(it >= 0){
            height = platform_height[it];
            if(852 - i >= m_will_x && 852 - i <= m_will_x + WILL_WIDTH){
                m_will_floor = min(480.0 - height - WILL_HEIGHT, m_will_floor);
            }

            if(852 - i + 142 >= m_will_x && 852 - i <= m_will_x + WILL_WIDTH){

                if(852 - i + 56 + COLLECTABLE_DIMENSION >= m_will_x && 852 - i + 56 <= m_will_x + WILL_WIDTH){
                    if(collectable[it]){
                        collectable_it = it;
                        m_will_collectable = 480.0 - collectable_height[it] - WILL_HEIGHT;
                    }else{
                        m_will_collectable = -1000000000;
                    }
                }else{
                    m_will_collectable = -1000000000;
                }

                if(852 - i + 48 + ENEMY_DIMENSION >= m_will_x && 852 - i + 48 <= m_will_x + WILL_WIDTH){
                    if(enemy[it]){
                        enemy_it = it;
                        m_will_enemy = 480.0 - enemy_height[it] - WILL_HEIGHT;
                        m_will_enemy_type = enemy_type[it];
                    }else{
                        m_will_enemy = -1000000000;
                        m_will_enemy_type = -1;
                    }
                }else{
                    m_will_enemy = -1000000000;
                    m_will_enemy_type = -1;
                }
            }

            if(852 - i >= m_will_x && 852 - i <= m_will_x + 30){
                m_will_floor = 480.0 - height - WILL_HEIGHT;
            }
        }
        aux++;
    }

	if(m_will_y >= m_will_collectable && m_will_y + 15*(m_state == SLIDING ? 1 : 0) <= m_will_collectable + COLLECTABLE_SIZE){
        ++n_collectables;
        
        collectable[collectable_it] = 0;
        m_will_collectable = -10000000;
    }

    if(m_will_y >= m_will_enemy && m_will_y + 15*(m_state == SLIDING ? 1 : 0) <= m_will_enemy + ENEMY_SIZE){
        if(m_will_enemy_type == 0 || not m_is_punching){
            m_state = GAME_OVER;
        }else{
            enemy[enemy_it] = 0;
            m_will_enemy = -10000000;
            m_will_enemy_type = -1;
        }
    }

    if(m_state == GAME_OVER){
        m_y_speed = 0;
        m_x_speed = 0;
        m_next = m_current_level;
        m_done = true;
    }

    m_start = now;
}

void TravelingWillPlayableLevel::draw_self(Canvas *canvas, unsigned now, unsigned){
	canvas->clear();
	canvas->draw(m_background[0].get(), Rectangle(0, 0, 852, 480), 0, 0);

	canvas->draw(m_background[1].get(), Rectangle(m_camera_x/2, m_camera_y, 852, 480), 0, 0);
	canvas->draw(m_background[2].get(), Rectangle(m_camera_x, m_camera_y, 852, 480), 0, 0);

	//Draws each of the seven parts of the screen
	int aux = 0, it, height;
	for(int i = (int)m_reverse_camera_x; i <= 994; i += 142){
		it = level_it[aux++];
		height = platform_height[it];
		canvas->draw(m_level[height/50].get(), Rectangle(0, 0, 142, height), 852 - i, 480 - height);

		if(enemy[it]){
			canvas->draw(m_enemy[enemy_type[it]].get(), Rectangle(ENEMY_DIMENSION * (int) sprite_counter, 0, 45, 45), 852 - i + 48, 480 - 
					enemy_height[it]);
		}

		if(collectable[it]){
			canvas->draw(m_collectable.get(), Rectangle(COLLECTABLE_DIMENSION * (int) sprite_counter, 0, 30, 30), 852 - i + 56, 480 - 
					collectable_height[it]);
		}
	}

	canvas->draw(m_will[m_is_punching ? PUNCHING : m_state].get(), Rectangle(WILL_WIDTH* (int) sprite_counter, 0, WILL_WIDTH, WILL_HEIGHT 
				- 15*(m_state == SLIDING ? 1 : 0)), m_will_x, m_will_y + 15*(m_state == SLIDING ? 1 : 0));

	double bar_width = 20 + (7.64 * 100 * m_audio_counter) / m_audio_duration;

	canvas->draw(m_progress_bar[0].get(), Rectangle(0, 0, 800, 19), 26, 18);
	canvas->draw(m_progress_bar[1].get(), Rectangle(0, 0, bar_width, 15), 30, 20);
	canvas->draw(m_progress_bar[2].get(), Rectangle(0, 0, 2, 15), 28, 20);
	canvas->draw(m_will_progress_bar.get(), Rectangle(0, 0, 20, 17), bar_width + 20, 20 - 1);

	canvas->draw(m_collectable_icon.get(), 705, 425);

	aux = n_collectables;
	int x_digit = 805;
	do{
		canvas->draw(m_number.get(), Rectangle(23 * (aux % 10), 0, 23, 36), x_digit, 435);
		aux /= 10;
		x_digit -= 25;
	}while(aux);

	if(m_state == GAME_OVER){
		canvas->draw(m_boss.get(), 100, 100);
	}
}