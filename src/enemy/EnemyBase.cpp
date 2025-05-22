#include "EnemyBase.h"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

/* ------------------ binding ------------------ */
void EnemyBase::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_move_speed","s"), &EnemyBase::set_move_speed);
    ClassDB::bind_method(D_METHOD("get_move_speed"),      &EnemyBase::get_move_speed);
    ClassDB::add_property("EnemyBase",
        PropertyInfo(Variant::FLOAT,"move_speed"), "set_move_speed","get_move_speed");

    ClassDB::bind_method(D_METHOD("set_gravity","g"), &EnemyBase::set_gravity);
    ClassDB::bind_method(D_METHOD("get_gravity"),     &EnemyBase::get_gravity);
    ClassDB::add_property("EnemyBase",
        PropertyInfo(Variant::FLOAT,"gravity"), "set_gravity","get_gravity");

    ClassDB::bind_method(D_METHOD("reset"), &EnemyBase::reset);
}
/* ------------------ lifecycle ---------------- */
EnemyBase::EnemyBase()  { }
EnemyBase::~EnemyBase() { }

void EnemyBase::_ready() {
    spawn_point = get_global_position();
}

void EnemyBase::_physics_process(double delta) {
    apply_gravity(delta);

    /* horizontal patrol handled by subclasses */
    move_and_slide();
}

/* ------------------ gravity ------------------ */
void EnemyBase::apply_gravity(double delta) {
    Vector2 v = get_velocity();

    if (!is_on_floor()) v.y += gravity * delta;
    else                v.y  = 0;

    set_velocity(v);
}

/* ------------------ reset -------------------- */
void EnemyBase::reset() { reset_to(spawn_point); }

void EnemyBase::reset_to(const Vector2 &p) {
    set_global_position(p);
    set_velocity(Vector2());
}

/* ------------------ setters ------------------ */
void EnemyBase::set_move_speed(float s){ move_speed = s; }
float EnemyBase::get_move_speed() const{ return move_speed; }

void EnemyBase::set_gravity(float g)    { gravity = g; }
float EnemyBase::get_gravity() const    { return gravity; }
