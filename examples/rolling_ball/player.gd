extends RigidBody3D

signal has_fallen

func _ready():
	pass
#	if AlternateInput.get_borrowed_gamepads().is_empty():
#		set_process(false)
#		set_physics_process(false)


func _process(delta):
	pass


func _physics_process(delta):
	if position.y < -20:
		has_fallen.emit()
#	var pitch = AlternateInput.get_rotation(0).x # from 0 to 6.218
#	var roll = AlternateInput.get_rotation(0).z # from 0 to 6.218
	
	
#	apply_central_force(Vector3(roll - 3.14, 0, pitch - 3.14).normalized() * 5)
#
#	if Input.is_action_just_pressed("jump"):
#		print("wow")
#		apply_central_impulse(Vector3(0, 10, 0))
	

