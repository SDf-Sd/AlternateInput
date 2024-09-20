extends StaticBody3D

signal collectible_collected

@export var collectible_scene : PackedScene

var scale_xz := 100.0

func _ready():
	rotation = AlternateInput.get_rotation(0)
	spawn_collectible()


func _process(delta):
	print(AlternateInput.get_gyroscope(0))
	print(AlternateInput.get_accelerometer(0))
	
	rotation.x = lerp_angle(rotation.x, AlternateInput.get_rotation(0).x, 0.1)
	rotation.z = lerp_angle(rotation.z, AlternateInput.get_rotation(0).z, 0.1)
	
	if scale_xz > 10.0:
		scale_xz = lerp(scale_xz, 10.0, 0.001)
	
	$CollisionShape3D3.scale.x = scale_xz / 100.0
	$CollisionShape3D3.scale.z = scale_xz / 100.0
	$MeshInstance3D.scale.x = scale_xz / 100.0
	$MeshInstance3D.scale.z = scale_xz / 100.0


func spawn_collectible():
	var collectible = collectible_scene.instantiate()
	var polar_deg = randf_range(0, TAU)
	var polar_dist = randf_range(0, 20 * (scale_xz / 100.0) - 5)
	
	collectible.position = Vector3(cos(polar_deg) * polar_dist, 6, sin(polar_deg) * polar_dist)
	collectible.body_entered.connect(on_collectible_body_entered.bind(collectible))
	
	call_deferred("add_child", collectible)


func on_collectible_body_entered(body, collectible):
	# assuming body is player
	collectible_collected.emit()
	spawn_collectible()
	
	scale_xz += 10.0
	scale_xz = clamp(scale_xz, 0.0, 100.0)
	collectible.queue_free()
