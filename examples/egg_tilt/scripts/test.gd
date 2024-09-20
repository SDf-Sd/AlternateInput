extends Node2D

func _physics_process(delta):
	
	var accel_data = AlternateInput.get_any_accelerometer()
	var magnitude = sqrt(accel_data.x*accel_data.x+accel_data.y*accel_data.y+accel_data.z*accel_data.z)
	
	print("accel: ", accel_data)
	print("mag: ", magnitude)
	
	if magnitude > 24:
		print("shSadjadhsaidh[aosd]")
	
