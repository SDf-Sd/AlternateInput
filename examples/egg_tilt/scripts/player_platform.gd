extends CharacterBody2D


func _physics_process(delta):
	rotation = lerp_angle(rotation, deg_to_rad(-round(rad_to_deg(AlternateInput.get_any_rotation().x))), 0.5) 
