#include "player_controller.h"

#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/variant/utility_functions.hpp>


using namespace godot;

void PlayerController::_bind_methods() {
    // Reset / damage
    ClassDB::bind_method(D_METHOD("reset"), &PlayerController::reset);
    ClassDB::bind_method(D_METHOD("reset_to", "pos"), &PlayerController::reset_to);
    ClassDB::bind_method(D_METHOD("damage", "amount"), &PlayerController::damage);

    // Properties: move_speed
    ClassDB::bind_method(D_METHOD("set_move_speed", "s"), &PlayerController::set_move_speed);
    ClassDB::bind_method(D_METHOD("get_move_speed"), &PlayerController::get_move_speed);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "move_speed"), "set_move_speed", "get_move_speed");

    // Properties: jump_force
    ClassDB::bind_method(D_METHOD("set_jump_force", "j"), &PlayerController::set_jump_force);
    ClassDB::bind_method(D_METHOD("get_jump_force"), &PlayerController::get_jump_force);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "jump_force"), "set_jump_force", "get_jump_force");

    // Properties: gravity
    ClassDB::bind_method(D_METHOD("set_gravity", "g"), &PlayerController::set_gravity);
    ClassDB::bind_method(D_METHOD("get_gravity"), &PlayerController::get_gravity);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");
}

// --------------------------------------------------
//  Constructor / Destructor
// --------------------------------------------------
PlayerController::PlayerController() {
    UtilityFunctions::print("PlayerController created");
}
PlayerController::~PlayerController() { }

// --------------------------------------------------
//  _ready: record spawn, ensure inputs
// --------------------------------------------------
void PlayerController::_ready() {
    spawn_point = get_global_position();
    ensure_input_actions();
    input_initialized = true;
}

// --------------------------------------------------
//  _physics_process: guard & then handle input + physics
// --------------------------------------------------
void PlayerController::_physics_process(double delta) {
    // Before initialization completes, just apply gravity + slide
    if (!input_initialized) {
        apply_gravity(delta);
        move_and_slide();
        return;
    }

    handle_input(delta);
    apply_gravity(delta);
    move_and_slide();
}

// --------------------------------------------------
//  enforce existence of our 8 actions & bind defaults
// --------------------------------------------------
void PlayerController::ensure_input_actions() {
    auto *imap = InputMap::get_singleton();

    // Helper lambdas
auto add_key = [&](const String &name, Key keycode) {
    if (!imap->has_action(name)) {
        imap->add_action(name);
        Ref<InputEventKey> ev = memnew(InputEventKey);
        ev->set_keycode(keycode);
        imap->action_add_event(name, ev);
        UtilityFunctions::print("Added action:", name);
    }
};

auto add_mouse = [&](const String &name, MouseButton button) {
    if (!imap->has_action(name)) {
        imap->add_action(name);
        Ref<InputEventMouseButton> ev = memnew(InputEventMouseButton);
        ev->set_button_index(button);
        imap->action_add_event(name, ev);
        UtilityFunctions::print("Added action:", name);
    }
};


    // Movement
    add_key("move_left",  Key::KEY_A);
    add_key("move_right", Key::KEY_D);
    add_key("move_up",    Key::KEY_W);
    add_key("move_down",  Key::KEY_S);
    add_key("dash",       Key::KEY_SHIFT);

    // Attacks
    add_mouse("attack_1", MouseButton::MOUSE_BUTTON_LEFT);
    add_mouse("attack_2", MouseButton::MOUSE_BUTTON_RIGHT);
    add_mouse("attack_3", MouseButton::MOUSE_BUTTON_MIDDLE);
}
// --------------------------------------------------
//  safe-check wrapper
// --------------------------------------------------
bool PlayerController::is_action_safe(const String &action) const {
    return input_initialized && InputMap::get_singleton()->has_action(action);
}

// --------------------------------------------------
//  handle_input: all your FSM + movement logic
// --------------------------------------------------
void PlayerController::handle_input(double delta) {
    Input *input = Input::get_singleton();
    Vector2 v    = get_velocity();
    float dir_x  = 0.0f;

    if (is_action_safe("move_right") && input->is_action_pressed("move_right")) {
        dir_x += 1; facing_right = true;
    }
    if (is_action_safe("move_left") && input->is_action_pressed("move_left")) {
        dir_x -= 1; facing_right = false;
    }

    if (dash_timer > 0) {
        dash_timer -= delta;
        v.x = facing_right ? dash_speed : -dash_speed;
        if (dash_timer <= 0) set_state(STATE_IDLE);
    } else {
        v.x = dir_x * move_speed;

        if (is_action_safe("move_up") && input->is_action_just_pressed("move_up") && is_on_floor()) {
            v.y = -jump_force;
            set_state(STATE_JUMP);
        }

        if (is_action_safe("move_down") && input->is_action_pressed("move_down") && is_on_floor()) {
            set_state(STATE_CROUCH);
            set_scale(Vector2(1, crouch_scale));
        } else if (state == STATE_CROUCH) {
            set_state(dir_x == 0 ? STATE_IDLE : STATE_RUN);
            set_scale(Vector2(1, 1));
        }

        if (is_action_safe("dash") && input->is_action_just_pressed("dash") && dir_x != 0 && state != STATE_CROUCH) {
            start_dash();
        }

        if (is_action_safe("attack_1") && input->is_action_just_pressed("attack_1")) set_state(STATE_ATTACK1);
        if (is_action_safe("attack_2") && input->is_action_just_pressed("attack_2")) set_state(STATE_ATTACK2);
        if (is_action_safe("attack_3") && input->is_action_just_pressed("attack_3")) set_state(STATE_ATTACK2);
    }

    // auto-transitions
    if (is_on_floor() && state < STATE_DASH) {
        if (Math::abs(v.x) < 1) set_state(STATE_IDLE);
        else                    set_state(STATE_RUN);
    }

    set_velocity(v);
}

// --------------------------------------------------
//  apply_gravity & move_and_slide
// --------------------------------------------------
void PlayerController::apply_gravity(double delta) {
    Vector2 v = get_velocity();
    if (!is_on_floor()) {
        v.y += gravity * delta;
    } else if (state != STATE_JUMP) {
        v.y = 0;
    }
    set_velocity(v);
}

void PlayerController::start_dash() {
    dash_timer = dash_time;
    set_state(STATE_DASH);
}

// --------------------------------------------------
//  state machine helper
// --------------------------------------------------
void PlayerController::set_state(ActionState new_state) {
    if (state == new_state) return;
    state = new_state;
    // hook up your AnimationTree here if desired
}

// --------------------------------------------------
//  reset & damage
// --------------------------------------------------
void PlayerController::reset() { reset_to(spawn_point); }
void PlayerController::reset_to(const Vector2 &p) {
    set_global_position(p);
    set_velocity(Vector2());
    dash_timer = 0;
    set_state(STATE_IDLE);
}

void PlayerController::damage(int amount) {
    // implement health logic here
}

// --------------------------------------------------
//  setters & getters
// --------------------------------------------------
void PlayerController::set_move_speed(float s) { move_speed = s; }
float PlayerController::get_move_speed() const { return move_speed; }

void PlayerController::set_jump_force(float j) { jump_force = j; }
float PlayerController::get_jump_force() const { return jump_force; }

void PlayerController::set_gravity(float g) { gravity = g; }
float PlayerController::get_gravity() const { return gravity; }
