#include "PatrolEnemy.h"
#include <godot_cpp/classes/ray_cast2d.hpp>

using namespace godot;

void PatrolEnemy::_bind_methods() {
    /* you can expose turn_at_wall/ledge as bool props later */
}

PatrolEnemy::PatrolEnemy()  { }
PatrolEnemy::~PatrolEnemy() { }

void PatrolEnemy::_ready()  { EnemyBase::_ready(); }

void PatrolEnemy::_physics_process(double delta) {
    /* horizontal motion */
    Vector2 v = get_velocity();
    v.x       = direction * move_speed;
    set_velocity(v);

    detect_turn_conditions();
    update_animation();

    EnemyBase::_physics_process(delta);
}

void PatrolEnemy::detect_turn_conditions() {
    if (turn_at_wall && is_on_wall()) direction *= -1;

    if (turn_at_ledge && is_on_floor()) {
        if (auto ray = Object::cast_to<RayCast2D>(get_node_or_null("FloorDetector"))) {
            if (!ray->is_colliding()) direction *= -1;
        }
    }
}

void PatrolEnemy::update_animation() {
    if (auto sprite = get_node_or_null("Sprite2D")) {
        Object::cast_to<Node2D>(sprite)->set_scale(Vector2(direction > 0 ? 1 : -1, 1));
    }
}
