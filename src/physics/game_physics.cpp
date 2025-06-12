#include "game_physics.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/physics_server2d.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GamePhysics::_bind_methods() {
    // Gravity
    ClassDB::bind_method(D_METHOD("set_gravity", "gravity"), &GamePhysics::set_gravity);
    ClassDB::bind_method(D_METHOD("get_gravity"), &GamePhysics::get_gravity);
    ClassDB::add_property("GamePhysics", PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");
    
    // Determinism flag
    ClassDB::bind_method(D_METHOD("set_is_deterministic", "deterministic"), &GamePhysics::set_is_deterministic);
    ClassDB::bind_method(D_METHOD("get_is_deterministic"), &GamePhysics::get_is_deterministic);
    ClassDB::add_property("GamePhysics", PropertyInfo(Variant::BOOL, "is_deterministic"), "set_is_deterministic", "get_is_deterministic");
    
    // Fixed timestep
    ClassDB::bind_method(D_METHOD("set_fixed_timestep", "timestep"), &GamePhysics::set_fixed_timestep);
    ClassDB::bind_method(D_METHOD("get_fixed_timestep"), &GamePhysics::get_fixed_timestep);
    ClassDB::add_property("GamePhysics", PropertyInfo(Variant::FLOAT, "fixed_timestep"), "set_fixed_timestep", "get_fixed_timestep");
}

// Constructor
GamePhysics::GamePhysics() {
    UtilityFunctions::print("GamePhysics created.");
}

// Destructor
GamePhysics::~GamePhysics() {
    UtilityFunctions::print("GamePhysics destroyed.");
}

void GamePhysics::_ready() {
    physics_server = PhysicsServer2D::get_singleton();
    
    // Apply our gravity setting
    if (physics_server) {
        UtilityFunctions::print("Physics server ready.");
        /*
        // Get the current project gravity
        Vector2 project_gravity = physics_server->area_get_param(RID(), PhysicsServer2D::AREA_PARAM_GRAVITY_VECTOR);
        float project_gravity_magnitude = physics_server->area_get_param(RID(), PhysicsServer2D::AREA_PARAM_GRAVITY);
        
        UtilityFunctions::print("Project gravity: ", project_gravity * project_gravity_magnitude);
        
        // Note: In a real implementation, you would want to modify the Physics2DServerExtension
        // to properly override gravity. This is just a demonstration.*/
    }
    
    UtilityFunctions::print("GamePhysics ready with gravity: ", gravity);
}

void GamePhysics::_process(double delta) {
    if (!is_deterministic) return;
    
    // Simiulate a physics loop
    physics_time_accumulator += delta;

    while (physics_time_accumulator >= fixed_timestep){
        _step_physics(fixed_timestep);
        physics_time_accumulator -= fixed_timestep;
    }
}

void GamePhysics::_step_physics(double step) {
    //UtilityFunctions::print("Physics step run with timestep: ", step); (causes nonstop print)


    // Update game objects here.
}

// Getters and setters
void GamePhysics::set_gravity(float p_gravity) {
    gravity = p_gravity;
    
    // In a real implementation, you would update the PhysicsServer here
    if (physics_server) {
        // This is a simplified example - full implementation would be more complex
        UtilityFunctions::print("Setting gravity to: ", gravity);
    }
}

float GamePhysics::get_gravity() const {
    return gravity;
}

void GamePhysics::set_is_deterministic(bool p_deterministic) {
    is_deterministic = p_deterministic;
}

bool GamePhysics::get_is_deterministic() const {
    return is_deterministic;
}

void GamePhysics::set_fixed_timestep(float p_timestep) {
    fixed_timestep = p_timestep;
}

float GamePhysics::get_fixed_timestep() const {
    return fixed_timestep;
}