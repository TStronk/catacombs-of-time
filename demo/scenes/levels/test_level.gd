extends Node2D



func _ready():
	await get_tree().process_frame
	await get_tree().process_frame
	# Wait for GameManager
	if not GameManager.input_map_ready.is_connected(_on_input_ready):
		GameManager.input_map_ready.connect(_on_input_ready)
		
	GameManager.register_level(self)
	GameManager.register_player($Player)  
	
	# Rest of your TestLevel code here...

func _on_input_ready():
	print("Input map is ready, TestLevel can proceed")
	# Move your PlayerController creation here
