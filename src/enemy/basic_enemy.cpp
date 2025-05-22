#include "basic_enemy.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/ray_cast2d.hpp>

using namespace godot;

/* ---------------------------------------------------------
 *  Class registration
 * ---------------------------------------------------------*/
void BasicEnemy::_bind_methods() {
    /* methods */
    ClassDB::bind_method(D_METHOD("apply_gravity", "delta"), &BasicEnemy::apply_gravity);
    ClassDB::bind_method(D_METHOD("detect_turn_conditions"), &BasicEnemy::detect_turn_conditions);
    ClassDB::bind_method(D_METHOD("update_animation"),       &BasicEnemy::update_animation);

    /* reset helpers */
    ClassDB::bind_method(D_METHOD("reset"),                  &BasicEnemy::reset);
    ClassDB::bind_method(D_METHOD("reset_to", "pos"),        &BasicEnemy::reset_to);

    /* properties */
    ClassDB::bind_method(D_METHOD("set_move_speed", "speed"), &BasicEnemy::set_move_speed);
    ClassDB::bind_method(D_METHOD("get_move_speed"),          &BasicEnemy::get_move_speed);
    ClassDB::add_property("BasicEnemy",
                          PropertyInfo(Variant::FLOAT, "move_speed"), "set_move_speed", "get_move_speed");

    ClassDB::bind_method(D_METHOD("set_gravity", "gravity"), &BasicEnemy::set_gravity);
    ClassDB::bind_method(D_METHOD("get_gravity"),            &BasicEnemy::get_gravity);
    ClassDB::add_property("BasicEnemy",
                          PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");

    ClassDB::bind_method(D_METHOD("set_should_turn_at_wall", "turn"),  &BasicEnemy::set_should_turn_at_wall);
    ClassDB::bind_method(D_METHOD("get_should_turn_at_wall"),          &BasicEnemy::get_should_turn_at_wall);
    ClassDB::add_property("BasicEnemy",
                          PropertyInfo(Variant::BOOL, "should_turn_at_wall"), "set_should_turn_at_wall",
                          "get_should_turn_at_wall");

    ClassDB::bind_method(D_METHOD("set_should_turn_at_ledge", "turn"), &BasicEnemy::set_should_turn_at_ledge);
    ClassDB::bind_method(D_METHOD("get_should_turn_at_ledge"),         &BasicEnemy::get_should_turn_at_ledge);
    ClassDB::add_property("BasicEnemy",
                          PropertyInfo(Variant::BOOL, "should_turn_at_ledge"), "set_should_turn_at_ledge",
                          "get_should_turn_at_ledge");
}

/* ---------------------------------------------------------
 *  Lifecycle
 * ---------------------------------------------------------*/
BasicEnemy::BasicEnemy()  { UtilityFunctions::print("BasicEnemy created");  }
BasicEnemy::~BasicEnemy() { UtilityFunctions::print("BasicEnemy destroyed"); }

void BasicEnemy::_ready() {
    UtilityFunctions::print("BasicEnemy ready");
    spawn_point = get_global_position();

    if (should_turn_at_ledge && !has_node("FloorDetector")) {
        UtilityFunctions::print("WARNING: BasicEnemy should_turn_at_ledge is true but no FloorDetector found");
    }
}

void BasicEnemy::_physics_process(double delta) {
    apply_gravity(delta);
    detect_turn_conditions();
    update_animation();

    /* horizontal patrol */
    Vector2 v = get_velocity();
    v.x       = direction * move_speed;
    set_velocity(v);

    move_and_slide();
}

/* ---------------------------------------------------------
 *  Gameplay helpers
 * ---------------------------------------------------------*/
void BasicEnemy::apply_gravity(double delta) {
    Vector2 v = get_velocity();

    if (!is_on_floor()) v.y += gravity * delta;
    else                v.y  = 0;         // clamp when grounded

    set_velocity(v);
}

void BasicEnemy::detect_turn_conditions() {
    if (should_turn_at_wall && is_on_wall()) direction *= -1;

    if (should_turn_at_ledge && is_on_floor()) {
        if (RayCast2D *ray = Object::cast_to<RayCast2D>(get_node_or_null("FloorDetector"))) {
            if (!ray->is_colliding()) direction *= -1;
        }
    }
}

void BasicEnemy::update_animation() {
    if (Node *sprite = get_node_or_null("Sprite2D")) {
        Object::cast_to<Node2D>(sprite)->set_scale(Vector2(direction > 0 ? 1 : -1, 1));
    }
}

/* ---------------------------------------------------------
 *  Reset helpers
 * ---------------------------------------------------------*/
void BasicEnemy::reset() {
    reset_to(spawn_point);
}

void BasicEnemy::reset_to(const Vector2 &pos) {
    set_global_position(pos);
    set_velocity(Vector2());     // zero both axes
    UtilityFunctions::print("BasicEnemy reset to ", pos);
}

/* ---------------------------------------------------------
 *  Getters / setters
 * ---------------------------------------------------------*/
void BasicEnemy::set_move_speed(float p_speed)  { move_speed = p_speed; }
float BasicEnemy::get_move_speed() const        { return move_speed;    }

void BasicEnemy::set_gravity(float p_gravity)   { gravity = p_gravity;  }
float BasicEnemy::get_gravity() const           { return gravity;       }

void BasicEnemy::set_should_turn_at_wall(bool p)  { should_turn_at_wall  = p; }
bool BasicEnemy::get_should_turn_at_wall() const  { return should_turn_at_wall; }

void BasicEnemy::set_should_turn_at_ledge(bool p) { should_turn_at_ledge = p; }
bool BasicEnemy::get_should_turn_at_ledge() const { return should_turn_at_ledge; }
