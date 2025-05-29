#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/animated_sprite2d.hpp>
#include <godot_cpp/classes/sprite_frames.hpp>
#include <godot_cpp/classes/collision_shape2d.hpp>
#include <godot_cpp/classes/rectangle_shape2d.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/color.hpp>

namespace godot {

class PlayerController : public CharacterBody2D {
    GDCLASS(PlayerController, CharacterBody2D);

public:
    PlayerController();
    ~PlayerController();

    // Godot callbacks
    virtual void _ready() override;
    virtual void _physics_process(double delta) override;
    virtual void _draw() override;  // For debug visualization

    // Public methods
    void reset();
    void reset_to(const Vector2 &pos);
    void damage(int amount);
    void heal(int amount);
    void add_score(int points);
    
    // Getters for UI
    int get_score() const { return score; }
    bool is_alive() const { return current_health > 0; }
    bool is_dashing() const { return state == STATE_DASH; }
    bool is_attacking() const { return state == STATE_ATTACK1 || state == STATE_ATTACK2 || state == STATE_ATTACK3; }

    // Property setters/getters
    void set_move_speed(float s);
    float get_move_speed() const;

    void set_jump_force(float j);
    float get_jump_force() const;

    void set_gravity(float g);
    float get_gravity() const;

    void set_max_health(int h);
    int get_max_health() const;

    void set_current_health(int h);
    int get_current_health() const;

    void set_debug_draw(bool d);
    bool get_debug_draw() const;

    void set_sprite_sheet_path(const String &path);
    String get_sprite_sheet_path() const;

protected:
    static void _bind_methods();

private:
    // === State Machine ===
    enum ActionState {
        STATE_IDLE,
        STATE_RUN,
        STATE_JUMP,
        STATE_FALL,
        STATE_LAND,
        STATE_WALL_SLIDE,
        STATE_DASH,
        STATE_ATTACK1,
        STATE_ATTACK2,
        STATE_ATTACK3,
        STATE_HURT,
        STATE_DIE,
        STATE_CROUCH,
        STATE_SLIDE
    };
    
    ActionState state = STATE_IDLE;
    ActionState previous_state = STATE_IDLE;
    
    // === Animation System ===
    AnimatedSprite2D *animated_sprite = nullptr;
    CollisionShape2D *collision_shape = nullptr;
    Label *debug_label = nullptr;
    String sprite_sheet_path = "res://assets/sprites/player_spritesheet.png";
    bool animations_loaded = false;
    
    void setup_placeholder_visuals();
    void setup_animations();
    void update_animation();
    String get_animation_name() const;
    void create_debug_label();
    void clone_animation(Ref<SpriteFrames> sprite_frames, const String& source_name, const String& dest_name, float new_fps);
    
    // === Input System ===
    bool input_initialized = false;
    void ensure_input_actions();
    bool is_action_safe(const String &action) const;
    
    // === Movement Parameters ===
    float move_speed = 200.0f;        // Reduced from 300
    float jump_force = 500.0f;        // Reduced from 600 
    float double_jump_force = 450.0f; // Reduced from 500
    float wall_jump_force_x = 300.0f; // Reduced from 400
    float wall_jump_force_y = 400.0f; // Reduced from 500
    float gravity = 1200.0f;          // Reduced from 1800
    float max_fall_speed = 800.0f;    // Reduced from 1000
    float wall_slide_speed = 80.0f;   // Reduced from 100
    float dash_speed = 500.0f;        // Reduced from 800
    float slide_speed = 300.0f;       // Reduced from 400
    float air_control = 0.7f;         // Increased from 0.3 for better control
    float friction = 0.2f;            // Increased from 0.1
    float acceleration = 0.4f;        // Increased from 0.2
    
    // === State Variables ===
    Vector2 spawn_point;
    bool facing_right = true;
    bool was_on_floor = false;
    bool can_double_jump = false;
    bool is_wall_sliding = false;
    int wall_direction = 0;  // -1 left, 1 right
    float input_buffer_x = 0.0f;
    
    // === Timers ===
    float dash_duration = 0.2f;
    float dash_cooldown = 0.5f;
    float dash_timer = 0.0f;
    float dash_cooldown_timer = 0.0f;
    
    float coyote_time = 0.1f;
    float coyote_timer = 0.0f;
    
    float jump_buffer_time = 0.1f;
    float jump_buffer_timer = 0.0f;
    
    float wall_jump_buffer_time = 0.1f;
    float wall_jump_buffer_timer = 0.0f;
    
    float land_recovery_time = 0.1f;
    float land_recovery_timer = 0.0f;
    
    float slide_duration = 0.5f;
    float slide_timer = 0.0f;
    
    // === Combat System ===
    int max_health = 100;
    int current_health = 100;
    int score = 0;
    
    float hurt_duration = 0.5f;
    float hurt_timer = 0.0f;
    float invincibility_duration = 1.0f;
    float invincibility_timer = 0.0f;
    
    float attack_duration = 0.3f;
    float attack_timer = 0.0f;
    float combo_window = 0.5f;
    float combo_timer = 0.0f;
    int current_attack = 0;
    int combo_count = 0;
    
    // === Debug ===
    bool debug_draw = true;
    Color debug_color = Color(1, 0, 0, 0.5);
    
    // === Core Methods ===
    void handle_input(double delta);
    void handle_movement(double delta);
    void handle_combat(double delta);
    void handle_state_transitions();
    void apply_gravity(double delta);
    void apply_movement(double delta);
    void update_timers(double delta);
    
    // === Movement Methods ===
    void start_jump();
    void start_double_jump();
    void start_wall_jump();
    void start_dash();
    void start_slide();
    void check_wall_slide();
    bool check_wall(int direction) const;
    
    // === Combat Methods ===
    void start_attack(int attack_num);
    void continue_combo();
    void reset_combo();
    
    // === State Methods ===
    void set_state(ActionState new_state);
    bool can_dash() const;
    bool can_attack() const;
    bool can_jump() const;
    bool can_wall_jump() const;
    bool can_slide() const;
    
    // === Utility Methods ===
    void update_facing_direction(float input_x);
    void apply_knockback(const Vector2 &direction, float force);
    void flash_sprite(bool on);
    String state_to_string(ActionState s) const;
};

} // namespace godot

#endif // PLAYER_CONTROLLER_H;