#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/variant/utility_functions.hpp>


using namespace godot;

class PlayerController : public CharacterBody2D {
    GDCLASS(PlayerController, CharacterBody2D);

public:
    PlayerController();
    ~PlayerController();

    // Godot callbacks
    virtual void _ready() override;
    virtual void _physics_process(double delta) override;

    // Exposed to Godot
    void reset();
    void reset_to(const Vector2 &p);
    void damage(int amount);

    void set_move_speed(float s);
    float get_move_speed() const;

    void set_jump_force(float j);
    float get_jump_force() const;

    void set_gravity(float g);
    float get_gravity() const;

protected:
    static void _bind_methods();

private:
    // --- Input sanity check ---
    bool input_initialized = false;
    void ensure_input_actions();
    bool is_action_safe(const String &action) const;

    // --- Movement & state ---
    Vector2 spawn_point;
    float move_speed = 200.0f;
    float jump_force = 400.0f;
    float gravity    = 1200.0f;
    float dash_speed = 600.0f;
    float dash_time  = 0.2f;
    float dash_timer = 0.0f;
    float crouch_scale = 0.5f;
    bool facing_right = true;

    enum ActionState {
        STATE_IDLE,
        STATE_RUN,
        STATE_JUMP,
        STATE_CROUCH,
        STATE_DASH,
        STATE_ATTACK1,
        STATE_ATTACK2
    };
    ActionState state = STATE_IDLE;

    // Core logic
    void handle_input(double delta);
    void apply_gravity(double delta);
    void start_dash();
    void set_state(ActionState new_state);
};

#endif // PLAYER_CONTROLLER_H
