extends CharacterBody2D

@onready var anim = $Turn/AnimatedSprite2D
@onready var yellow_projectile = preload("res://scenes/common_projectile.tscn")

# Main "constant" vars
var SPEED = 200
var JUMP_VELOCITY_STEP = 60
var GRAVITY = 1200

# Shot cooldown
var can_shoot = true

# Jump vars
var jump_power_initial = -300
var jump_power = 0
var jump_time_max = 0.3
var jump_timer = 0.0
var is_jumping = false

# Shoot vars
var is_shooting = false

func _physics_process(delta):
	var is_running = Input.get_axis("ui_left","ui_right")

	velocity.x = SPEED * is_running

	# Give player turning
	if is_running < 0:
		$Turn.scale.x = -1
		
		
		
	elif is_running > 0:
		$Turn.scale.x = 1


	if is_on_floor():

		jump_timer = 0.0
		is_jumping = false

		# Running
		if is_running and not is_shooting:
			anim.play("run")

		# Idling
		elif not is_running and not is_shooting:
			anim.play("idle")

		# Jumping (logic intended to create dynamic height input)
		if Input.is_action_just_pressed("jump") && not is_jumping:
			jump_timer = 0.0
			is_jumping = true
			apply_jump_force(jump_power_initial)
			jump_power = jump_power_initial
		
		if Input.is_action_just_pressed("jump") && is_jumping && jump_timer < jump_time_max:
		
			# Increase jump power as time held increases
			jump_power -= JUMP_VELOCITY_STEP
			apply_jump_force(jump_power)
			
	# Jump physics 
	if not is_on_floor():
		jump_timer += delta
		velocity.y += GRAVITY * delta
		# "Jumping"
		if velocity.y > 0 and not is_shooting:
			anim.play("jump")



		# Falling
		elif velocity.y <= 0 and not is_shooting:
			anim.play("fall")
	

	# Still horizontal shot
	if Input.is_action_just_pressed("shoot") and is_on_floor:
		
		can_shoot = false

		var common_projectile = yellow_projectile.instantiate()
		common_projectile.global_position = $Turn/ProjectileOrigin.global_position
		common_projectile.vel = $Turn.scale.x

		

		get_parent().add_child(common_projectile)

		is_shooting = true
		anim.play("shoot still")
		await anim.animation_finished
		await get_tree().create_timer(0.21).timeout
		is_shooting = false

		can_shoot = true



	move_and_slide()

func apply_jump_force(power):
	velocity.y = power


func _input(event: InputEvent) -> void:
	if event.is_action_released("jump") && is_jumping:
		jump_timer = jump_time_max
