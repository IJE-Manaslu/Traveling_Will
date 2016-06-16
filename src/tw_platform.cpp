#include "tw_platform.h"

TWPlatform::TWPlatform() : m_width(142) {

}

//platform height
//enemy type
//enemy height
//enemy present
//collectable height
//collectable present
TWPlatform::TWPlatform(string current_level, double ph, double et, double eh, int ep, double ch, int cp){
    m_width = 142;
    m_height = ph;
    m_y = 480 - m_height;

    m_texture = resources::get_texture(current_level + "/platform.png");
    m_floor = resources::get_texture(current_level + "/floor.png");

    if(ep){
        m_enemy = new TWEnemy(et, eh);
        add_child(m_enemy);
    }
	else{
        m_enemy = nullptr;
    }

    if(cp){
        m_collectable = new TWCollectable(current_level, ch);
        add_child(m_collectable);
    }
	else{
        m_collectable = nullptr;
    }

    this->set_priority(4);
}

TWPlatform::~TWPlatform(){
}

double TWPlatform::x(){ return m_x; }
double TWPlatform::y(){ return m_y; }
double TWPlatform::height(){ return m_height; }
double TWPlatform::width(){ return m_width; }
shared_ptr<Texture> TWPlatform::texture(){ return m_texture; }
TWCollectable* TWPlatform::collectable() { return m_collectable; }
TWEnemy* TWPlatform::enemy() { return m_enemy; }

void TWPlatform::set_x(double px){ m_x = px; }
void TWPlatform::set_y(double py){  m_y = py; }
void TWPlatform::set_height(double ph){ m_height = ph; }

void TWPlatform::remove(int option){
    printf("Vamos remover...\n");
    if(option == COLLECTABLE){
        destroy_child(m_collectable);
        m_collectable = nullptr;
    }else{
        destroy_child(m_enemy);
        m_enemy = nullptr;
    }
    printf("Removeu de boa\n");
}

void TWPlatform::register_objects(int current_x){
    if(m_collectable){
        m_collectable->register_self(current_x + 56);
    }
    if(m_enemy){
        m_enemy->register_self(current_x + 48);
    }
}

void TWPlatform::update_self(unsigned, unsigned) {
    if(m_enemy){
         m_enemy->set_x(m_x + 48);
    }

    if(m_collectable){
        m_collectable->set_x(m_x + 56);
    }
}

void TWPlatform::draw_self(Canvas * canvas, unsigned, unsigned) {
    canvas->draw(m_texture.get(), Rectangle(0, 0, m_width, m_height), m_x, m_y);
    canvas->draw(m_floor.get(), Rectangle(0, 0, m_width, 50), m_x, 430);
}

