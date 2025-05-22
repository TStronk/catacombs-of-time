#ifndef BASIC_ENEMY_H
#define BASIC_ENEMY_H

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector2.hpp>

using namespace godot;

class BasicEnemy : public CharacterBody2D {
    GDCLASS(BasicEnemy, CharacterBody2D);

private:
    /* movement parameters */
    float move_speed = 30.0f;
    float gravity    = 30.0f;
    float direction  = 1.0f;     // 1 = right, −1 = left

    bool  should_turn_at_wall  = true;
    bool  should_turn_at_ledge = true;

    /* stored spawn point for quick resets */
    Vector2 spawn_point;

protected:
    static void _bind_methods();

public:
    BasicEnemy();
    ~BasicEnemy();

    void _ready() override;
    void _physics_process(double delta) override;

    /* gameplay helpers */
    void apply_gravity(double delta);
    void detect_turn_conditions();
    void update_animation();

    /* reset helpers */
    void reset();                       // back to original spawn_point
    void reset_to(const Vector2 &pos);  // teleport anywhere

    /* setters / getters exposed to Godot */
    void  set_move_speed(float p_speed);
    float get_move_speed() const;

    void  set_gravity(float p_gravity);
    float get_gravity() const;

    void  set_should_turn_at_wall(bool p_turn);
    bool  get_should_turn_at_wall() const;

    void  set_should_turn_at_ledge(bool p_turn);
    bool  get_should_turn_at_ledge() const;
};

#endif // BASIC_ENEMY_H
