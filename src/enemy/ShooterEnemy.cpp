#include "ShooterEnemy.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/node2d.hpp>

using namespace godot;

void ShooterEnemy::_bind_methods() {
    ClassDB::bind_method(D_METHOD("fire_at_player"), &ShooterEnemy::fire_at_player);
}

ShooterEnemy::ShooterEnemy()  {}
ShooterEnemy::~ShooterEnemy() {}

void ShooterEnemy::_ready() {
    EnemyBase::_ready();
    projectile_scene = ResourceLoader::get_singleton()
        ->load("res://scenes/Projectile.tscn");
}

void ShooterEnemy::_physics_process(double delta) {
    /* simple hover or stand still: no x-motion */
    EnemyBase::_physics_process(delta);

    cooldown -= delta;
    if (cooldown <= 0) {
        fire_at_player();
        cooldown = fire_rate;
    }
}

void ShooterEnemy::fire_at_player() {
    if (!projectile_scene.is_valid()) return;

    Node2D *proj = Object::cast_to<Node2D>(projectile_scene->instantiate());
    if (!proj) return;

    get_parent()->add_child(proj);
    proj->set_global_position(get_global_position());

    /* very simple aiming toward player (assumes GameManager autoload) */
    if (Engine::get_singleton()->has_singleton("GameManager")) {
        Node *gm = Object::cast_to<Node>(Engine::get_singleton()->get_singleton("GameManager"));
        if (gm && gm->has_method("get_player_global_position")) {
            Vector2 target = gm->call("get_player_global_position");
            Vector2 dir = (target - get_global_position()).normalized();
            proj->set("velocity", dir * 400);   // projectile script should read this
        }
    }
}
