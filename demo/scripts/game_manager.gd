# GameManager.gd
#
# Autoload this script via:
#    Project → Project Settings → Autoload → [Add…] Path: res://scripts/game_manager.gd Node Name: GameManager
#
# It keeps references to the player and every enemy that a level registers.
# Calling GameManager.reset_level() (or press Esc by default) will:
#      • teleport the player back to its original spawn point
#      • zero-out the player's velocity (handled inside its C++ reset())
#      • do the same for every enemy that was registered
#
extends Node

# -----------------------------------------------------------------------
# Input Map Configuration
# -----------------------------------------------------------------------
signal input_map_ready

# Define all custom actions with their default inputs
const INPUT_ACTIONS = {
	"move_left": ["key_a", "key_left"],
	"move_right": ["key_d", "key_right"], 
	"move_up": ["key_w", "key_up"],
	"move_down": ["key_s", "key_down"],
	"dash": ["key_shift"],
	"attack_1": ["mouse_left"],
	"attack_2": ["mouse_right"],
	"attack_3": ["mouse_middle"]
}

# -----------------------------------------------------------------------
# Run-time references
# -----------------------------------------------------------------------
var current_level : Node        = null
var player        : Node        = null          # PlayerController (C++)
var enemies       : Array[Node] = []            # BasicEnemy (C++)

# -----------------------------------------------------------------------
# Optional counters / scoreboard
# -----------------------------------------------------------------------
var enemy_count : int = 0
var score       : int = 0

# -----------------------------------------------------------------------
# Debug helpers
# -----------------------------------------------------------------------
var kill_plane_y : float      = -1000.0        # auto-respawn if player falls below
var debug_key    : StringName = "ui_cancel"    # Esc by default

func _ready() -> void:
	setup_input_map()
	input_map_ready.emit()
	print("GameManager ready — waiting for level to register nodes")

# -----------------------------------------------------------------------
# Input Map Setup
# -----------------------------------------------------------------------
func setup_input_map() -> void:
	"""Setup and validate all custom input actions"""
	print("Setting up input map...")
	
	# Force reload project settings first
	InputMap.load_from_project_settings()
	
	# Create missing actions with default bindings
	for action_name in INPUT_ACTIONS:
		if not InputMap.has_action(action_name):
			_create_input_action(action_name, INPUT_ACTIONS[action_name])
	
	print("Input map setup complete")

func _create_input_action(action_name: String, input_list: Array) -> void:
	"""Create a new input action with default bindings"""
	InputMap.add_action(action_name)
	
	for input_def in input_list:
		var event = _create_input_event(input_def)
		if event:
			InputMap.action_add_event(action_name, event)
	
	print("Created input action: ", action_name)

func _create_input_event(input_def: String) -> InputEvent:
	"""Create an InputEvent based on string definition"""
	var event = null
	
	# Handle mouse buttons
	match input_def:
		"mouse_left":
			event = InputEventMouseButton.new()
			event.button_index = MOUSE_BUTTON_LEFT
		"mouse_right":
			event = InputEventMouseButton.new()
			event.button_index = MOUSE_BUTTON_RIGHT
		"mouse_middle":
			event = InputEventMouseButton.new()
			event.button_index = MOUSE_BUTTON_MIDDLE
		_:
			# Handle keyboard keys
			var keycode = _get_keycode_from_string(input_def)
			if keycode != KEY_NONE:
				event = InputEventKey.new()
				event.keycode = keycode
	
	return event

func _get_keycode_from_string(key_str: String) -> Key:
	"""Convert string to Key enum"""
	match key_str:
		"key_a": return KEY_A
		"key_d": return KEY_D
		"key_w": return KEY_W
		"key_s": return KEY_S
		"key_left": return KEY_LEFT
		"key_right": return KEY_RIGHT
		"key_up": return KEY_UP
		"key_down": return KEY_DOWN
		"key_shift": return KEY_SHIFT
		"key_space": return KEY_SPACE
		"key_x": return KEY_X
		"key_c": return KEY_C
		_: return KEY_NONE

func debug_input_map() -> void:
	"""Debug helper to print all input actions and their events"""
	print("=== InputMap Debug ===")
	var actions = InputMap.get_actions()
	print("Total actions: ", actions.size())
	for action in actions:
		print("Action: ", action)
		var events = InputMap.action_get_events(action)
		for event in events:
			print("  Event: ", event)
	print("=== End Debug ===")

# -----------------------------------------------------------------------
# Registration helpers (call these once in each level's _ready)
# -----------------------------------------------------------------------
func register_level(p_level: Node) -> void:
	current_level = p_level
	enemies.clear()
	enemy_count = 0
	print("Level registered: ", p_level.name)

func register_player(p_player: Node) -> void:
	player = p_player
	print("Player registered")

func register_enemy(p_enemy: Node) -> void:
	enemies.append(p_enemy)
	enemy_count += 1
	print("Enemy registered — total: ", enemy_count)

# -----------------------------------------------------------------------
# Reset / respawn API (these call the C++ reset() you added)
# -----------------------------------------------------------------------
func respawn_player() -> void:
	if player and player.is_inside_tree():
		player.reset()          # C++ → snap to stored spawn, velocity = 0
		print("Player respawned")

func respawn_enemies() -> void:
	for e in enemies:
		if e and e.is_inside_tree():
			e.reset()           # C++ → snap + velocity = 0
	print("Enemies respawned")

func reset_level() -> void:
	respawn_player()
	respawn_enemies()
	# Add extra score/timer resets here if you need
	print("Level reset complete")

# -----------------------------------------------------------------------
# Convenience: press Esc to reset; auto-respawn if player falls
# -----------------------------------------------------------------------
func _input(event) -> void:
	if event.is_action_pressed(debug_key):
		reset_level()

func _physics_process(_delta: float) -> void:
	if player and player.global_position.y < kill_plane_y:
		respawn_player()
