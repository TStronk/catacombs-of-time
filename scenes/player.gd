extends CharacterBody2D

const SPEED := 800
const JUMP_FORCE := -1200
const GRAVITY := 5000

func _physics_process(delta):
	var direction := Vector2.ZERO
	
	if Input.is_action_pressed("ui_left"):
		direction.x -= 1
	if Input.is_action_pressed("ui_right"):
		direction.x += 1
		
	velocity.x = direction.x * SPEED
	
	if is_on_floor():
		if Input.is_action_just_pressed("ui_up"):
			velocity.y = JUMP_FORCE
	else:
		velocity.y += GRAVITY * delta
		
	move_and_slide()
		
		
