#ifndef PATROL_ENEMY_H
#define PATROL_ENEMY_H
#include "EnemyBase.h"

class PatrolEnemy : public EnemyBase {
    GDCLASS(PatrolEnemy, EnemyBase);

    bool turn_at_wall  = true;
    bool turn_at_ledge = true;

protected:
    static void _bind_methods();

public:
    PatrolEnemy();
    ~PatrolEnemy();

    void _ready() override;
    void _physics_process(double delta) override;

    /* helpers */
    void detect_turn_conditions();
    void update_animation();
};
#endif // PATROL_ENEMY_H
