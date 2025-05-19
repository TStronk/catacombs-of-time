extends Node2D


func _ready():
	GameManager.register_level(self)
	GameManager.register_player($PlayerController)  
