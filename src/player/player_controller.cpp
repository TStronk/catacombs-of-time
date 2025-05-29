#include "player_controller.h"
#include "sprite_sheet_loader.h"

#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/classes/placeholder_texture2d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/physics_server2d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters2d.hpp>
#include <godot_cpp/classes/physics_direct_space_state2d.hpp>
#include <godot_cpp/classes/world2d.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/classes/camera2d.hpp>

using namespace godot;

void PlayerController::_bind_methods() {
    // Methods
    ClassDB::bind_method(D_METHOD("reset"), &PlayerController::reset);
    ClassDB::bind_method(D_METHOD("reset_to", "pos"), &PlayerController::reset_to);
    ClassDB::bind_method(D_METHOD("damage", "amount"), &PlayerController::damage);
    ClassDB::bind_method(D_METHOD("heal", "amount"), &PlayerController::heal);
    ClassDB::bind_method(D_METHOD("add_score", "points"), &PlayerController::add_score);
    ClassDB::bind_method(D_METHOD("get_score"), &PlayerController::get_score);
    ClassDB::bind_method(D_METHOD("is_alive"), &PlayerController::is_alive);
    ClassDB::bind_method(D_METHOD("is_dashing"), &PlayerController::is_dashing);
    ClassDB::bind_method(D_METHOD("is_attacking"), &PlayerController::is_attacking);

    // Properties
    ClassDB::bind_method(D_METHOD("set_move_speed", "speed"), &PlayerController::set_move_speed);
    ClassDB::bind_method(D_METHOD("get_move_speed"), &PlayerController::get_move_speed);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "move_speed", PROPERTY_HINT_RANGE, "0,1000,10"), "set_move_speed", "get_move_speed");

    ClassDB::bind_method(D_METHOD("set_jump_force", "force"), &PlayerController::set_jump_force);
    ClassDB::bind_method(D_METHOD("get_jump_force"), &PlayerController::get_jump_force);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "jump_force", PROPERTY_HINT_RANGE, "0,2000,10"), "set_jump_force", "get_jump_force");

    ClassDB::bind_method(D_METHOD("set_gravity", "gravity"), &PlayerController::set_gravity);
    ClassDB::bind_method(D_METHOD("get_gravity"), &PlayerController::get_gravity);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity", PROPERTY_HINT_RANGE, "0,3000,10"), "set_gravity", "get_gravity");

    ClassDB::bind_method(D_METHOD("set_max_health", "health"), &PlayerController::set_max_health);
    ClassDB::bind_method(D_METHOD("get_max_health"), &PlayerController::get_max_health);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_health", PROPERTY_HINT_RANGE, "0,1000,1"), "set_max_health", "get_max_health");

    ClassDB::bind_method(D_METHOD("set_current_health", "health"), &PlayerController::set_current_health);
    ClassDB::bind_method(D_METHOD("get_current_health"), &PlayerController::get_current_health);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "current_health", PROPERTY_HINT_RANGE, "0,1000,1"), "set_current_health", "get_current_health");

    ClassDB::bind_method(D_METHOD("set_debug_draw", "enabled"), &PlayerController::set_debug_draw);
    ClassDB::bind_method(D_METHOD("get_debug_draw"), &PlayerController::get_debug_draw);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_draw"), "set_debug_draw", "get_debug_draw");

    ClassDB::bind_method(D_METHOD("set_sprite_sheet_path", "path"), &PlayerController::set_sprite_sheet_path);
    ClassDB::bind_method(D_METHOD("get_sprite_sheet_path"), &PlayerController::get_sprite_sheet_path);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "sprite_sheet_path", PROPERTY_HINT_FILE, "*.png"), "set_sprite_sheet_path", "get_sprite_sheet_path");
}

PlayerController::PlayerController() {
    UtilityFunctions::print("PlayerController created");
}

PlayerController::~PlayerController() {}

void PlayerController::_ready() {
    spawn_point = get_global_position();
    ensure_input_actions();
    setup_placeholder_visuals();
    setup_animations();
    create_debug_label();
    input_initialized = true;
    current_health = max_health;
    
    // Enable processing
    set_physics_process(true);
    set_process(true);
}

void PlayerController::_physics_process(double delta) {
    if (!input_initialized) {
        apply_gravity(delta);
        move_and_slide();
        return;
    }

    update_timers(delta);
    handle_input(delta);
    handle_movement(delta);
    handle_combat(delta);
    handle_state_transitions();
    apply_gravity(delta);
    apply_movement(delta);
    move_and_slide();
    update_animation();
    
 // Update debug label - COMMENT THIS OUT FOR NOW
/*
if (debug_label) {
    debug_label->set_text(String("State: ") + state_to_string(state) + 
                         String("\nVel: ") + String::num(get_velocity().x, 0) + ", " + String::num(get_velocity().y, 0) +
                         String("\nHP: ") + String::num(current_health) + "/" + String::num(max_health));
}
*/
}

void PlayerController::_draw() {
    if (!debug_draw || !Engine::get_singleton()->is_editor_hint()) return;
    
    // Draw state indicator
    draw_circle(Vector2(0, -40), 5, debug_color);
    
    // Draw facing direction
    Vector2 dir_start(0, -20);
    Vector2 dir_end = dir_start + Vector2(facing_right ? 20 : -20, 0);
    draw_line(dir_start, dir_end, Color(0, 1, 0), 2);
}

void PlayerController::ensure_input_actions() {
    auto *imap = InputMap::get_singleton();

    auto add_key_to_action = [&](const String &name, Key keycode) {
        if (!imap->has_action(name)) {
            imap->add_action(name);
        }
        
        Ref<InputEventKey> ev;
        ev.instantiate();
        ev->set_keycode(keycode);
        imap->action_add_event(name, ev);
    };

    auto add_mouse_to_action = [&](const String &name, MouseButton button) {
        if (!imap->has_action(name)) {
            imap->add_action(name);
        }
        
        Ref<InputEventMouseButton> ev;
        ev.instantiate();
        ev->set_button_index(button);
        imap->action_add_event(name, ev);
    };

    // Movement
    add_key_to_action("move_left", Key::KEY_A);
    add_key_to_action("move_right", Key::KEY_D);
    add_key_to_action("move_down", Key::KEY_S);
    
    // Jump - both W and Space
    add_key_to_action("jump", Key::KEY_W);
    add_key_to_action("jump", Key::KEY_SPACE);
    
    // Advanced movement
    add_key_to_action("dash", Key::KEY_SHIFT);
    add_key_to_action("slide", Key::KEY_C);
    
    // Combat
    add_mouse_to_action("attack", MouseButton::MOUSE_BUTTON_LEFT);
    add_key_to_action("attack", Key::KEY_J);  // Keyboard alternative
    
    UtilityFunctions::print("Input actions initialized");
}

void PlayerController::setup_placeholder_visuals() {
    // Create collision shape
    collision_shape = memnew(CollisionShape2D);
    collision_shape->set_name("CollisionShape2D");
    
    Ref<RectangleShape2D> rect;
    rect.instantiate();
    rect->set_size(Vector2(20, 30));  // Smaller collision box to match sprite
    collision_shape->set_shape(rect);
    add_child(collision_shape);
    
    // Create placeholder sprite
    animated_sprite = memnew(AnimatedSprite2D);
    animated_sprite->set_name("AnimatedSprite2D");
    add_child(animated_sprite);
    
    // ADD CAMERA HERE
    Camera2D* camera = memnew(Camera2D);
    camera->set_name("Camera2D");
    camera->set_enabled(true);
    camera->set_position_smoothing_enabled(true);
    camera->set_position_smoothing_speed(5.0);
    add_child(camera);
}

void PlayerController::clone_animation(Ref<SpriteFrames> sprite_frames, 
                                      const String& source_name, 
                                      const String& dest_name, 
                                      float new_fps) {
    if (!sprite_frames->has_animation(source_name)) {
        return;
    }
    
    sprite_frames->add_animation(dest_name);
    sprite_frames->set_animation_loop(dest_name, sprite_frames->get_animation_loop(source_name));
    sprite_frames->set_animation_speed(dest_name, new_fps);
    
    int frame_count = sprite_frames->get_frame_count(source_name);
    for (int i = 0; i < frame_count; i++) {
        Ref<Texture2D> frame = sprite_frames->get_frame_texture(source_name, i);
        sprite_frames->add_frame(dest_name, frame);
    }
}

void PlayerController::setup_animations() {
    Ref<SpriteFrames> sprite_frames;
    sprite_frames.instantiate();
    
    // Remove default animation if it exists
    if (sprite_frames->has_animation("default")) {
        sprite_frames->remove_animation("default");
    }
    
    // DEBUG: Check actual sprite dimensions
    String base_path = "res://Extraordinary Pixelvania - Free Asset Pack/Sprites/Characters/Heros/Main Hero/";
    String test_file = "Idle-Sheet.png";
    Ref<Texture2D> test_texture = ResourceLoader::get_singleton()->load(base_path + test_file);
    if (test_texture.is_valid()) {
        Vector2i texture_size = test_texture->get_size();
        UtilityFunctions::print("=== SPRITE SIZE DEBUG ===");
        UtilityFunctions::print("Idle-Sheet.png total size: ", texture_size);
        UtilityFunctions::print("With 20 frames, each frame should be: ", texture_size.x / 20, " pixels wide");
        UtilityFunctions::print("========================");
    }
    
    // Rest of your setup_animations code...
    
    // Structure to hold animation data
    struct AnimationData {
        String animation_name;
        String file_name;
        int frame_count;
        float fps;
        bool loop;
        Vector2i frame_size;
    };
    
   std::vector<AnimationData> animations = {
    // Basic movement - CORRECTED SIZE: 33x32
    {"idle", "Idle-Sheet.png", 17, 10.0f, true, Vector2i(39, 32)},
    {"run", "Run-Sheet.png", 8, 12.0f, true, Vector2i(39, 32)},
    {"jump", "Jump-Sheet.png", 4, 10.0f, false, Vector2i(39, 32)},
    {"fall", "Falling-Sheet.png", 4, 8.0f, true, Vector2i(39, 32)},
    
    // Combat - CORRECTED SIZE
    {"hit", "Hit-Sheet.png", 4, 10.0f, false, Vector2i(39, 32)},
    {"die", "Die-Sheet.png", 13, 8.0f, false, Vector2i(39, 32)},
    
    // Aiming animations - CORRECTED SIZE
    {"aim_side", "Aim_Side-Sheet.png", 3, 8.0f, false, Vector2i(39, 32)},
    {"aim_up", "Aim_Up-Sheet.png", 5, 8.0f, false, Vector2i(39, 32)},
    
    // Shooting animations - CORRECTED SIZE
    {"shot_side", "Shot_Side-Sheet.png", 3, 12.0f, false, Vector2i(39, 32)},
    {"shot_up", "Shot_Up-Sheet.png", 4, 12.0f, false, Vector2i(39, 32)},
    
    // Running and shooting - CORRECTED SIZE
    {"run_shot_side", "Run_Shot_Side-Sheet.png", 9, 12.0f, true, Vector2i(39, 32)},
    {"run_shot_up", "Run_Shoting_Up-Sheet.png", 9, 12.0f, true, Vector2i(39, 32)},
    
    // Jumping aim animations - CORRECTED SIZE
    {"jump_aim_side", "Jump_Aim_Side-Sheet.png", 1, 8.0f, false, Vector2i(39, 32)},
    {"jump_aim_up", "Jump_Aim_Up-Sheet.png", 4, 8.0f, false, Vector2i(39, 32)},
    
    // Jumping shot animations - CORRECTED SIZE
    {"jump_shot_side", "Jump_Shot_Side-Sheet.png", 3, 12.0f, false, Vector2i(39, 32)},
    {"jump_shot_up", "Jump_Shot_Up-Sheet.png", 7, 12.0f, false, Vector2i(39, 32)},
};
    
  // Replace the entire "Load each animation" loop with this:
for (const auto& anim : animations) {
    String full_path = base_path + anim.file_name;
    
    if (ResourceLoader::get_singleton()->exists(full_path)) {
        Ref<Texture2D> texture = ResourceLoader::get_singleton()->load(full_path);
        
        if (texture.is_valid()) {
            // Add animation to sprite frames
            sprite_frames->add_animation(anim.animation_name);
            sprite_frames->set_animation_loop(anim.animation_name, anim.loop);
            sprite_frames->set_animation_speed(anim.animation_name, anim.fps);
            
            // Calculate frame dimensions FOR THIS SPECIFIC ANIMATION
            Vector2i texture_size = texture->get_size();
            float exact_frame_width = float(texture_size.x) / float(anim.frame_count);
            
            UtilityFunctions::print("Animation: ", anim.animation_name, 
                                   " - Texture width: ", texture_size.x,
                                   " / ", anim.frame_count, " frames = ", 
                                   exact_frame_width, " pixels per frame");
            
               // In the frame-adding loop:
for (int i = 0; i < anim.frame_count; i++) {
    Ref<AtlasTexture> frame;
    frame.instantiate();
    frame->set_atlas(texture);
    
    int x_position;
    
    // If it's a clean 39px animation, use simple math
    if (exact_frame_width == 39.0f) {
        x_position = i * 39 + 3;  // 3px offset to center the 32px sprite
    } else {
        // For fractional widths, use centered extraction
        float frame_center = (i + 0.5f) * exact_frame_width;
        x_position = round(frame_center - 16);
    }
    
    // Ensure we don't go out of bounds
    x_position = Math::clamp(x_position, 0, texture_size.x - 32);
    
    frame->set_region(Rect2i(x_position, 0, 32, 32));
    sprite_frames->add_frame(anim.animation_name, frame);
}
                        
            UtilityFunctions::print("Loaded animation: ", anim.animation_name, " with ", anim.frame_count, " frames");
        }
    }
}
    
    // Map animations to states that don't have exact matches
    if (sprite_frames->has_animation("shot_side")) {
        clone_animation(sprite_frames, "shot_side", "attack1", 12.0f);
    }
    
    if (sprite_frames->has_animation("shot_up")) {
        clone_animation(sprite_frames, "shot_up", "attack2", 12.0f);
    }
    
    if (sprite_frames->has_animation("jump_shot_side")) {
        clone_animation(sprite_frames, "jump_shot_side", "attack3", 12.0f);
    }
    
    if (sprite_frames->has_animation("hit")) {
        clone_animation(sprite_frames, "hit", "hurt", 8.0f);
    }
    
    if (sprite_frames->has_animation("run")) {
        clone_animation(sprite_frames, "run", "dash", 18.0f);
        clone_animation(sprite_frames, "run", "slide", 15.0f);
    }
    
    if (sprite_frames->has_animation("idle")) {
        clone_animation(sprite_frames, "idle", "land", 8.0f);
        clone_animation(sprite_frames, "idle", "crouch", 4.0f);
    }
    
    if (sprite_frames->has_animation("fall")) {
        clone_animation(sprite_frames, "fall", "wall_slide", 4.0f);
    }
    
    // Set the sprite frames
    animated_sprite->set_sprite_frames(sprite_frames);
    animations_loaded = true;
    
    // IMPORTANT: Make sure sprite is visible
    animated_sprite->set_visible(true);
    // At the end of setup_animations()
    animated_sprite->set_centered(true);
    animated_sprite->set_offset(Vector2(0, 0));
    
    
    // Start with idle
    animated_sprite->play("idle");
    
    UtilityFunctions::print("==========================================");
    UtilityFunctions::print("All animations loaded successfully!");
    UtilityFunctions::print("Sprite visible: ", animated_sprite->is_visible());
    UtilityFunctions::print("==========================================");
}

void PlayerController::create_debug_label() {
    debug_label = memnew(Label);
    debug_label->set_name("DebugLabel");
    debug_label->set_position(Vector2(-30, -40));  // Much closer to player
    debug_label->add_theme_font_size_override("font_size", 8);  // Much smaller
    debug_label->set_scale(Vector2(0.5, 0.5));  // Scale it down even more
    add_child(debug_label);
}

bool PlayerController::is_action_safe(const String &action) const {
    return input_initialized && InputMap::get_singleton()->has_action(action);
}

void PlayerController::handle_input(double delta) {
    Input *input = Input::get_singleton();
    
    // Get movement input
    input_buffer_x = 0.0f;
    if (is_action_safe("move_right") && input->is_action_pressed("move_right")) {
        input_buffer_x += 1.0f;
    }
    if (is_action_safe("move_left") && input->is_action_pressed("move_left")) {
        input_buffer_x -= 1.0f;
    }
    
    // Update facing direction
    if (input_buffer_x != 0 && state != STATE_WALL_SLIDE) {
        update_facing_direction(input_buffer_x);
    }
    
    // Jump input
    if (is_action_safe("jump") && input->is_action_just_pressed("jump")) {
        jump_buffer_timer = jump_buffer_time;
        if (is_wall_sliding) {
            wall_jump_buffer_timer = wall_jump_buffer_time;
        }
    }
    
    // Dash input
    if (is_action_safe("dash") && input->is_action_just_pressed("dash") && can_dash()) {
        start_dash();
    }
    
    // Slide input
    if (is_action_safe("move_down") && input->is_action_pressed("move_down") && is_on_floor()) {
        if (Math::abs(get_velocity().x) > 50) {
            start_slide();
        } else {
            set_state(STATE_CROUCH);
        }
    }
    
    // Attack input
    if (is_action_safe("attack") && input->is_action_just_pressed("attack") && can_attack()) {
        if (combo_timer > 0 && current_attack < 3) {
            continue_combo();
        } else {
            start_attack(1);
        }
    }
}

void PlayerController::handle_movement(double delta) {
    Vector2 vel = get_velocity();
    
    // Different movement handling based on state
    switch (state) {
        case STATE_DASH:
            vel.x = facing_right ? dash_speed : -dash_speed;
            break;
            
        case STATE_SLIDE:
            vel.x = facing_right ? slide_speed : -slide_speed;
            break;
            
        case STATE_WALL_SLIDE:
            // Apply wall slide physics
            if (vel.y > wall_slide_speed) {
                vel.y = wall_slide_speed;
            }
            break;
            
        case STATE_HURT:
            // Knocked back, limited control
            vel.x *= 0.95f;
            break;
            
        default:
    // Normal movement
    if (state != STATE_CROUCH) {
        float target_vel = input_buffer_x * move_speed;
        if (is_on_floor()) {
            // Instant response on ground
            vel.x = target_vel;
        } else {
            // Some air control
            vel.x = Math::lerp(vel.x, target_vel, 0.2f);
        }
    }
    break;
    }
    
    // Handle jumps
    if (jump_buffer_timer > 0.0f) {
        if (can_jump()) {
            if (is_on_floor() || coyote_timer > 0.0f) {
                start_jump();
            } else if (can_double_jump) {
                start_double_jump();
            }
        }
    }
    
    // Handle wall jumps
    if (wall_jump_buffer_timer > 0.0f && can_wall_jump()) {
        start_wall_jump();
    }
    
    // Check for wall sliding
    if (!is_on_floor() && vel.y > 0) {
        check_wall_slide();
    } else {
        is_wall_sliding = false;
    }
    
    // Apply friction when no input
    if (is_on_floor() && input_buffer_x == 0 && state != STATE_DASH && state != STATE_SLIDE) {
        vel.x = Math::lerp(vel.x, 0.0f, friction);
    }
    
    // Clamp fall speed
    if (vel.y > max_fall_speed) {
        vel.y = max_fall_speed;
    }
    
    set_velocity(vel);
}

void PlayerController::handle_combat(double delta) {
    // Handle attack timing
    if (is_attacking()) {
        if (attack_timer <= 0.0f) {
            reset_combo();
            set_state(STATE_IDLE);
        }
    }
    
    // Handle hurt state
    if (state == STATE_HURT) {
        if (hurt_timer <= 0.0f) {
            set_state(STATE_IDLE);
        }
    }
    
    // Handle invincibility flashing
    if (invincibility_timer > 0.0f) {
        float flash_rate = 10.0f;
        bool visible = fmod(invincibility_timer * flash_rate, 1.0f) > 0.5f;
        flash_sprite(visible);
    } else {
        flash_sprite(true);
    }
}

void PlayerController::handle_state_transitions() {
    // Automatic state transitions based on physics
    if (state == STATE_IDLE || state == STATE_RUN || state == STATE_JUMP || state == STATE_FALL) {
        if (is_on_floor()) {
            if (was_on_floor == false) {
                // Just landed
                set_state(STATE_LAND);
                land_recovery_timer = land_recovery_time;
            } else if (Math::abs(get_velocity().x) > 10) {
                if (state != STATE_RUN) set_state(STATE_RUN);
            } else {
                if (state != STATE_IDLE) set_state(STATE_IDLE);
            }
        } else {
            if (get_velocity().y < 0) {
                if (state != STATE_JUMP) set_state(STATE_JUMP);
            } else {
                if (state != STATE_FALL && !is_wall_sliding) set_state(STATE_FALL);
            }
        }
    }
    
    // Update floor state
    was_on_floor = is_on_floor();
    
    // Update coyote time
    if (was_on_floor && !is_on_floor() && state != STATE_JUMP) {
        coyote_timer = coyote_time;
    }
    
    // Reset double jump on floor
    if (is_on_floor()) {
        can_double_jump = true;
    }
}

void PlayerController::apply_gravity(double delta) {
    if (state == STATE_DASH) return;  // No gravity during dash
    
    Vector2 vel = get_velocity();
    if (!is_on_floor()) {
        float grav_mult = 1.0f;
        
        // Variable jump height - less gravity when holding jump and moving up
        if (vel.y < 0 && Input::get_singleton()->is_action_pressed("jump")) {
            grav_mult = 0.6f;  // Hold jump to go higher
        } else if (vel.y > 0) {
            grav_mult = 1.3f;  // Fall faster than you rise
        }
        
        vel.y += gravity * grav_mult * delta;
    } else {
        vel.y = 0;  // Reset velocity on floor
    }
    set_velocity(vel);
}

void PlayerController::apply_movement(double delta) {
    // Movement is handled in handle_movement
    // This is here for any post-processing
}

void PlayerController::update_timers(double delta) {
    if (dash_timer > 0.0f) {
        dash_timer -= delta;
        if (dash_timer <= 0.0f && state == STATE_DASH) {
            set_state(STATE_IDLE);
        }
    }
    
    if (dash_cooldown_timer > 0.0f) dash_cooldown_timer -= delta;
    if (coyote_timer > 0.0f) coyote_timer -= delta;
    if (jump_buffer_timer > 0.0f) jump_buffer_timer -= delta;
    if (wall_jump_buffer_timer > 0.0f) wall_jump_buffer_timer -= delta;
    if (attack_timer > 0.0f) attack_timer -= delta;
    if (combo_timer > 0.0f) combo_timer -= delta;
    if (hurt_timer > 0.0f) hurt_timer -= delta;
    if (invincibility_timer > 0.0f) invincibility_timer -= delta;
    if (land_recovery_timer > 0.0f) land_recovery_timer -= delta;
    if (slide_timer > 0.0f) {
        slide_timer -= delta;
        if (slide_timer <= 0.0f && state == STATE_SLIDE) {
            set_state(STATE_IDLE);
        }
    }
}

void PlayerController::start_jump() {
    Vector2 vel = get_velocity();
    vel.y = -jump_force;
    set_velocity(vel);
    jump_buffer_timer = 0.0f;
    coyote_timer = 0.0f;
    set_state(STATE_JUMP);
}

void PlayerController::start_double_jump() {
    Vector2 vel = get_velocity();
    vel.y = -double_jump_force;
    set_velocity(vel);
    jump_buffer_timer = 0.0f;
    can_double_jump = false;
    set_state(STATE_JUMP);
    
    // Could add particle effect here
}

void PlayerController::start_wall_jump() {
    Vector2 vel = get_velocity();
    vel.x = wall_direction * wall_jump_force_x;
    vel.y = -wall_jump_force_y;
    set_velocity(vel);
    
    wall_jump_buffer_timer = 0.0f;
    is_wall_sliding = false;
    facing_right = (wall_direction > 0);
    set_state(STATE_JUMP);
}

void PlayerController::start_dash() {
    set_state(STATE_DASH);
    dash_timer = dash_duration;
    dash_cooldown_timer = dash_cooldown;
    
    // Cancel vertical velocity for horizontal dash
    Vector2 vel = get_velocity();
    vel.y = 0;
    set_velocity(vel);
}

void PlayerController::start_slide() {
    set_state(STATE_SLIDE);
    slide_timer = slide_duration;
}

void PlayerController::check_wall_slide() {
    if (input_buffer_x == 0) {
        is_wall_sliding = false;
        return;
    }
    
    int check_dir = input_buffer_x > 0 ? 1 : -1;
    if (check_wall(check_dir)) {
        is_wall_sliding = true;
        wall_direction = -check_dir;  // Wall is opposite of movement direction
        set_state(STATE_WALL_SLIDE);
    } else {
        is_wall_sliding = false;
    }
}

bool PlayerController::check_wall(int direction) const {
    // Cast a ray to check for walls
    Ref<World2D> world = get_world_2d();
    if (world.is_null()) {
        return false;
    }
    
    PhysicsDirectSpaceState2D* space_state = PhysicsServer2D::get_singleton()->space_get_direct_state(world->get_space());
    if (!space_state) {
        return false;
    }
    
    Vector2 from = get_global_position();
    Vector2 to = from + Vector2(direction * 20.0f, 0);  // Check 20 pixels to the side
    
    Ref<PhysicsRayQueryParameters2D> ray_params;
    ray_params.instantiate();
    ray_params->set_from(from);
    ray_params->set_to(to);
    
    // Create exclude array
    TypedArray<RID> exclude;
    exclude.push_back(get_rid());
    ray_params->set_exclude(exclude);
    
    ray_params->set_collision_mask(get_collision_mask());
    
    Dictionary result = space_state->intersect_ray(ray_params);
    
    return !result.is_empty();
}

void PlayerController::start_attack(int attack_num) {
    current_attack = attack_num;
    attack_timer = attack_duration;
    combo_timer = combo_window;
    
    switch (attack_num) {
        case 1:
            set_state(STATE_ATTACK1);
            break;
        case 2:
            set_state(STATE_ATTACK2);
            break;
        case 3:
            set_state(STATE_ATTACK3);
            break;
    }
    
    // Stop horizontal movement during attack
    Vector2 vel = get_velocity();
    vel.x *= 0.5f;
    set_velocity(vel);
}

void PlayerController::continue_combo() {
    current_attack++;
    if (current_attack > 3) current_attack = 1;
    
    start_attack(current_attack);
    combo_count++;
}

void PlayerController::reset_combo() {
    current_attack = 0;
    combo_count = 0;
    combo_timer = 0.0f;
}

void PlayerController::set_state(ActionState new_state) {
    if (state == new_state) return;
    
    previous_state = state;
    state = new_state;
    
    // State exit logic
    switch (previous_state) {
        case STATE_CROUCH:
            // Reset collision shape if it was modified
            break;
        case STATE_WALL_SLIDE:
            is_wall_sliding = false;
            break;
        default:
            break;
    }
    
    // State entry logic
    switch (new_state) {
        case STATE_LAND:
            // Could trigger land particles/sound here
            break;
        case STATE_HURT:
            hurt_timer = hurt_duration;
            invincibility_timer = invincibility_duration;
            break;
        case STATE_DIE:
            // Handle death state
            set_physics_process(false);  // Stop physics
            break;
        default:
            break;
    }
    
    // Update animation immediately
    update_animation();
}

bool PlayerController::can_dash() const {
    return dash_cooldown_timer <= 0.0f && 
           state != STATE_DASH && 
           state != STATE_HURT &&
           state != STATE_DIE &&
           (is_on_floor() || can_double_jump);  // Can dash once in air
}

bool PlayerController::can_attack() const {
    return state != STATE_HURT &&
           state != STATE_DIE &&
           state != STATE_DASH &&
           land_recovery_timer <= 0.0f;
}

bool PlayerController::can_jump() const {
    return state != STATE_HURT &&
           state != STATE_DIE &&
           state != STATE_WALL_SLIDE;
}

bool PlayerController::can_wall_jump() const {
    return is_wall_sliding &&
           state == STATE_WALL_SLIDE;
}

bool PlayerController::can_slide() const {
    return is_on_floor() &&
           state != STATE_SLIDE &&
           state != STATE_HURT &&
           state != STATE_DIE;
}

void PlayerController::update_facing_direction(float input_x) {
    if (input_x > 0) {
        facing_right = true;
    } else if (input_x < 0) {
        facing_right = false;
    }
}

void PlayerController::apply_knockback(const Vector2 &direction, float force) {
    Vector2 vel = get_velocity();
    vel += direction.normalized() * force;
    set_velocity(vel);
}

void PlayerController::flash_sprite(bool visible) {
    if (animated_sprite) {
        animated_sprite->set_visible(visible);
    }
}

void PlayerController::update_animation() {
    if (!animated_sprite) return;
    
    // Update sprite direction
    animated_sprite->set_flip_h(!facing_right);
    
    // Get animation name based on state
    String anim_name = get_animation_name();
    
    // Only change animation if different
    if (animated_sprite->get_sprite_frames().is_valid()) {
        if (animated_sprite->get_sprite_frames()->has_animation(anim_name)) {
            if (animated_sprite->get_animation() != anim_name) {
                animated_sprite->play(anim_name);
            }
        }
    }
}

String PlayerController::get_animation_name() const {
    switch (state) {
        case STATE_IDLE: return "idle";
        case STATE_RUN: return "run";
        case STATE_JUMP: return "jump";
        case STATE_FALL: return "fall";
        case STATE_LAND: return "land";
        case STATE_WALL_SLIDE: return "wall_slide";
        case STATE_DASH: return "dash";
        case STATE_ATTACK1: return "attack1";
        case STATE_ATTACK2: return "attack2";
        case STATE_ATTACK3: return "attack3";
        case STATE_HURT: return "hurt";
        case STATE_DIE: return "die";
        case STATE_CROUCH: return "crouch";
        case STATE_SLIDE: return "slide";
        default: return "idle";
    }
}

String PlayerController::state_to_string(ActionState s) const {
    switch (s) {
        case STATE_IDLE: return "Idle";
        case STATE_RUN: return "Run";
        case STATE_JUMP: return "Jump";
        case STATE_FALL: return "Fall";
        case STATE_LAND: return "Land";
        case STATE_WALL_SLIDE: return "Wall Slide";
        case STATE_DASH: return "Dash";
        case STATE_ATTACK1: return "Attack 1";
        case STATE_ATTACK2: return "Attack 2";
        case STATE_ATTACK3: return "Attack 3";
        case STATE_HURT: return "Hurt";
        case STATE_DIE: return "Die";
        case STATE_CROUCH: return "Crouch";
        case STATE_SLIDE: return "Slide";
        default: return "Unknown";
    }
}

void PlayerController::reset() {
    reset_to(spawn_point);
}

void PlayerController::reset_to(const Vector2 &pos) {
    set_global_position(pos);
    set_velocity(Vector2());
    
    // Reset all timers
    dash_timer = 0.0f;
    dash_cooldown_timer = 0.0f;
    attack_timer = 0.0f;
    combo_timer = 0.0f;
    hurt_timer = 0.0f;
    invincibility_timer = 0.0f;
    coyote_timer = 0.0f;
    jump_buffer_timer = 0.0f;
    wall_jump_buffer_timer = 0.0f;
    land_recovery_timer = 0.0f;
    slide_timer = 0.0f;
    
    // Reset state
    current_health = max_health;
    can_double_jump = true;
    is_wall_sliding = false;
    facing_right = true;
    reset_combo();
    
    set_state(STATE_IDLE);
    set_physics_process(true);
    flash_sprite(true);
}

void PlayerController::damage(int amount) {
    if (state == STATE_HURT || state == STATE_DIE || invincibility_timer > 0.0f) return;
    
    current_health -= amount;
    if (current_health <= 0) {
        current_health = 0;
        set_state(STATE_DIE);
        // You could emit a signal here for game over
    } else {
        set_state(STATE_HURT);
        // Apply knockback
        Vector2 knockback_dir = Vector2(facing_right ? -1 : 1, -0.5f);
        apply_knockback(knockback_dir, 300.0f);
    }
}

void PlayerController::heal(int amount) {
    current_health = Math::min(current_health + amount, max_health);
}

void PlayerController::add_score(int points) {
    score += points;
}

// Property setters/getters
void PlayerController::set_move_speed(float s) { move_speed = s; }
float PlayerController::get_move_speed() const { return move_speed; }

void PlayerController::set_jump_force(float j) { jump_force = j; }
float PlayerController::get_jump_force() const { return jump_force; }

void PlayerController::set_gravity(float g) { gravity = g; }
float PlayerController::get_gravity() const { return gravity; }

void PlayerController::set_max_health(int h) { max_health = h; }
int PlayerController::get_max_health() const { return max_health; }

void PlayerController::set_current_health(int h) { current_health = h; }
int PlayerController::get_current_health() const { return current_health; }

void PlayerController::set_debug_draw(bool d) { 
    debug_draw = d; 
    queue_redraw();
}
bool PlayerController::get_debug_draw() const { return debug_draw; }

void PlayerController::set_sprite_sheet_path(const String &path) { 
    sprite_sheet_path = path;
    if (is_inside_tree()) {
        setup_animations();
    }
}
String PlayerController::get_sprite_sheet_path() const { return sprite_sheet_path; }