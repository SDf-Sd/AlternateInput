extends Button

@export var start_focused := false

func _ready():
	pressed.connect(get_tree().reload_current_scene)
	
	if start_focused:
		grab_focus()
