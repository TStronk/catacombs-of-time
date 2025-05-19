#  res://scripts/game_manager.gd
#
#  Autoload this script via:
#     Project → Project Settings → Autoload → [Add…]  Path: res://scripts/game_manager.gd  Node Name: GameManager
#
#  It keeps references to the player and every enemy that a level registers.
#  Calling GameManager.reset_level() (or press Esc by default) will:
#       • teleport the player back to its original spawn point
#       • zero-out the player’s velocity (handled inside its C++   reset() )
#       • do the same for every enemy that was registered
#

extends Node

# -----------------------------------------------------------------------
#  Run-time references
# -----------------------------------------------------------------------
var current_level : Node        = null
var player        : Node        = null          # PlayerController (C++)
var enemies       : Array[Node] = []            # BasicEnemy   (C++)

# -----------------------------------------------------------------------
#  Optional counters / scoreboard (leave if you use them)
# -----------------------------------------------------------------------
var enemy_count : int = 0
var score       : int = 0

# -----------------------------------------------------------------------
#  Debug helpers
# -----------------------------------------------------------------------
var kill_plane_y : float      = -1000           # auto-respawn if player falls below
var debug_key    : StringName = "ui_cancel"     # Esc by default

func _ready() -> void:
	print("GameManager ready — waiting for level to register nodes")

# -----------------------------------------------------------------------
#  Registration helpers  (call these once in each level’s _ready)
# -----------------------------------------------------------------------
func register_level(p_level : Node) -> void:
	current_level = p_level
	enemies.clear()
	enemy_count = 0
	print("Level registered:", p_level.name)

func register_player(p_player : Node) -> void:
	player = p_player
	print("Player registered")

func register_enemy(p_enemy : Node) -> void:
	enemies.append(p_enemy)
	enemy_count += 1
	print("Enemy registered — total:", enemy_count)

# -----------------------------------------------------------------------
#  Reset / respawn API  (these call the C++ reset() you added)
# -----------------------------------------------------------------------
func respawn_player() -> void:
	if player and player.is_inside_tree():
		player.reset()          # C++  →  snap to stored spawn, velocity = 0
		print("Player respawned")

func respawn_enemies() -> void:
	for e in enemies:
		if e and e.is_inside_tree():
			e.reset()           # C++  →  snap + velocity = 0
	print("Enemies respawned")

func reset_level() -> void:
	respawn_player()
	respawn_enemies()
	# add extra score/timer resets here if you need
	print("Level reset complete")

# -----------------------------------------------------------------------
#  Convenience: press Esc to reset; auto-respawn if player falls
# -----------------------------------------------------------------------
func _input(event) -> void:
	if event.is_action_pressed(debug_key):
		reset_level()

func _physics_process(_d : float) -> void:
	if player and player.global_position.y < kill_plane_y:
		respawn_player()
