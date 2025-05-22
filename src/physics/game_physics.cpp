#include "game_physics.h"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void GamePhysics::_bind_methods() {
    // Properties
    ClassDB::bind_method(D_METHOD("set_gravity", "gravity"), &GamePhysics::set_gravity);
    ClassDB::bind_method(D_METHOD("get_gravity"), &GamePhysics::get_gravity);
    ClassDB::add_property("GamePhysics", PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");
    
    ClassDB::bind_method(D_METHOD("set_is_deterministic", "deterministic"), &GamePhysics::set_is_deterministic);
    ClassDB::bind_method(D_METHOD("get_is_deterministic"), &GamePhysics::get_is_deterministic);
    ClassDB::add_property("GamePhysics", PropertyInfo(Variant::BOOL, "is_deterministic"), "set_is_deterministic", "get_is_deterministic");
    
    ClassDB::bind_method(D_METHOD("set_fixed_timestep", "timestep"), &GamePhysics::set_fixed_timestep);
    ClassDB::bind_method(D_METHOD("get_fixed_timestep"), &GamePhysics::get_fixed_timestep);
    ClassDB::add_property("GamePhysics", PropertyInfo(Variant::FLOAT, "fixed_timestep"), "set_fixed_timestep", "get_fixed_timestep");
}

GamePhysics::GamePhysics() {
    // Constructor
    UtilityFunctions::print("GamePhysics created");
}

GamePhysics::~GamePhysics() {
    // Destructor
    UtilityFunctions::print("GamePhysics destroyed");
}

void GamePhysics::_ready() {
    physics_server = PhysicsServer2D::get_singleton();
    
    // Apply our gravity setting
    if (physics_server) {
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
    // If using deterministic physics, we would handle custom physics updates here
    if (is_deterministic) {
        // This would require a more complex implementation
        // Usually involves manual stepping of physics and fixed timestep
    }
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