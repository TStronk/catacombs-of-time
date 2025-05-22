#ifndef GAME_PHYSICS_H
#define GAME_PHYSICS_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/physics_server2d.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class GamePhysics : public Node {
    GDCLASS(GamePhysics, Node);

private:
    float gravity = 30.0f;
    bool is_deterministic = false;
    float fixed_timestep = 1.0f/60.0f;
    PhysicsServer2D* physics_server;

protected:
    static void _bind_methods();

public:
    GamePhysics();
    ~GamePhysics();

    virtual void _ready() override;
    virtual void _process(double delta) override;
    
    // Getters and setters
    void set_gravity(float p_gravity);
    float get_gravity() const;
    
    void set_is_deterministic(bool p_deterministic);
    bool get_is_deterministic() const;
    
    void set_fixed_timestep(float p_timestep);
    float get_fixed_timestep() const;
};

#endif // GAME_PHYSICS_H