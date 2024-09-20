extends Button

@export_file var path := ""
@export var packed_scene : PackedScene
@export var start_focused : bool = false

func _ready():
	if not is_instance_valid(packed_scene):
		pressed.connect(get_tree().change_scene_to_file.bind(path))
	else:
		pressed.connect(get_tree().change_scene_to_packed.bind(packed_scene))
	
	if start_focused:
		grab_focus()
