extends Control
# Custom Key-bind tutorial script for Godot 4

var current_button: Button

@onready var vbox       := $VBoxContainer
@onready var info_panel := $PanelContainer
@onready var back_btn   := $Back

func _ready() -> void:
	# Connect each Button inside the HBox rows
	for row in vbox.get_children():
		if row is HBoxContainer:
			for child in row.get_children():
				if child is Button:
					child.text = ""  # clear placeholder
					child.pressed.connect(_on_button_pressed.bind(child))

	# Back hides the panel
	back_btn.pressed.connect(_on_back_pressed)

	_update_labels()
	info_panel.visible = false

func _on_button_pressed(btn: Button) -> void:
	current_button = btn
	info_panel.visible = true

func _input(event: InputEvent) -> void:
	if current_button == null:
		return

	if event is InputEventKey or event is InputEventMouseButton:
		var action := current_button.name

		# remove duplicates
		for a in InputMap.get_actions():
			for ev in InputMap.action_get_events(a):
				if ev.equals(event):
					InputMap.action_erase_event(a, ev)

		# assign new
		InputMap.action_erase_events(action)
		InputMap.action_add_event(action, event)
		ProjectSettings.save()  # persist to project.godot

		current_button = null
		info_panel.visible = false
		_update_labels()

func _update_labels() -> void:
	# refresh each button’s text
	for row in vbox.get_children():
		if row is HBoxContainer:
			for child in row.get_children():
				if child is Button:
					var evs = InputMap.action_get_events(child.name)
					if evs.is_empty():
						child.text = "Unassigned"
					else:
						child.text = evs[0].as_text()

func _on_back_pressed() -> void:
	info_panel.visible = false
