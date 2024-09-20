extends Control


func _ready():
	if AlternateInput.get_borrowed_gamepads().is_empty():
		set_process(false)


func _process(delta):
	pass
#	position.y = remap(AlternateInput.get_rotation(0).x, deg_to_rad(0 + ), TAU - 2, 0, 648)
#	position.x = remap(AlternateInput.get_rotation(0).y, TAU - 1.57, 1.57, 0, 1152)
#	print(position.x)
#	position = position.clamp(Vector2(0, 0), Vector2(1132, 628))
#	print(rad_to_deg(AlternateInput.get_rotation(0).y))

