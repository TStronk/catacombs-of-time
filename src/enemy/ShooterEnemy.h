#ifndef SHOOTER_ENEMY_H
#define SHOOTER_ENEMY_H
#include "EnemyBase.h"
#include <godot_cpp/classes/packed_scene.hpp>     // ← add
#include <godot_cpp/variant/typed_array.hpp>

class ShooterEnemy : public EnemyBase {
    GDCLASS(ShooterEnemy, EnemyBase);

    Ref<PackedScene> projectile_scene;
    float fire_rate = 1.5f;
    float cooldown  = 0.0f;

protected:
    static void _bind_methods();

public:
    ShooterEnemy();
    ~ShooterEnemy();

    void _ready() override;
    void _physics_process(double delta) override;

    void fire_at_player();
};
#endif // SHOOTER_ENEMY_H
