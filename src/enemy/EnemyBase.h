#ifndef ENEMY_BASE_H
#define ENEMY_BASE_H

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector2.hpp>

using namespace godot;

class EnemyBase : public CharacterBody2D {
    GDCLASS(EnemyBase, CharacterBody2D);

protected:
    /* tunables */
    float move_speed = 120.0f;      // px/s
    float gravity    = 1200.0f;     // px/s²
    float direction  = 1.0f;        // 1 = right, −1 = left

    Vector2 spawn_point;

    static void _bind_methods();

    /* helpers */
    void apply_gravity(double delta);

public:
    EnemyBase();
    virtual ~EnemyBase();

    /* Godot callbacks */
    void _ready() override;
    void _physics_process(double delta) override;

    /* reset API */
    virtual void reset();
    void reset_to(const Vector2 &pos);

    /* exposed setters / getters */
    void  set_move_speed(float s);
    float get_move_speed() const;

    void  set_gravity(float g);
    float get_gravity() const;
};

#endif // ENEMY_BASE_H
