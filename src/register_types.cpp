#include "register_types.h"

#include "game_physics.h"
#include "gdexample.h"
#include "player/player_controller.h"
#include "enemy/basic_enemy.h"
#include "physics/game_physics.h"
#include "Enemy/EnemyBase.h"
#include "Enemy/PatrolEnemy.h"
#include "Enemy/ShooterEnemy.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    ClassDB::register_class<GDExample>();
    ClassDB::register_class<PlayerController>();
    ClassDB::register_class<BasicEnemy>();
    ClassDB::register_class<GamePhysics>();
    ClassDB::register_class<EnemyBase>();
    ClassDB::register_class<PatrolEnemy>();
    ClassDB::register_class<ShooterEnemy>();
}

void uninitialize_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}

extern "C" {
    // Initialization.
    GDExtensionBool GDE_EXPORT gdexample_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
        
        init_obj.register_initializer(initialize_module);
        init_obj.register_terminator(uninitialize_module);
        
        return init_obj.init();
    }
}