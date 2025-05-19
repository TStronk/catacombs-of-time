extends CharacterBody2D

var SPEED = 300
var vel : float 

func _physics_process(delta):

	move_local_x(vel * SPEED * delta)
