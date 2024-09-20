extends RigidBody2D

signal egg_broken
signal egg_jumped
signal egg_touched_ground

var broken := false

var last_linear_velocity := Vector2()
var on_ground := false

func  _ready():
	$Sprite2D.play("default")


func _physics_process(delta):
	last_linear_velocity = linear_velocity
	
	var direction := Input.get_axis("move_left", "move_right")#Input.get_joy_axis(0, JOY_AXIS_LEFT_Y)
		
	print(direction)
	
	apply_force(Vector2(direction * 100, 0), Vector2(0, -5))
	
	if Input.is_action_just_pressed("jump") and on_ground:
		apply_central_impulse(Vector2.UP * 200)
		egg_jumped.emit()
	


func _integrate_forces(state):
	var last_ground_state = on_ground
	for i in state.get_contact_count():
		var layer = PhysicsServer2D.body_get_collision_layer(state.get_contact_collider(i))
		
		var last_on_ground = on_ground
		var normal: Vector2 = state.get_contact_local_normal(i)
		on_ground = normal.dot(Vector2.UP) > 0.80 or last_on_ground
		
		if not last_ground_state and on_ground:
			var tween = create_tween()
			tween.tween_property($Sprite2D, "scale", Vector2(1.1, 1.1), 0.125)
			tween.tween_property($Sprite2D, "scale", Vector2.ONE, 0.125)
			
			egg_touched_ground.emit()
#			tween.tween_property($Sprite2D, "scale", Vector2.ONE, 0.125)
		
		if layer == 2:
			return
		
		if layer == 1 and last_linear_velocity.length() > 200 and not broken:
			break_egg(normal)
	if state.get_contact_count() == 0:
		if on_ground:
			var tween = create_tween()
			tween.tween_property($Sprite2D, "scale", Vector2(0.9, 0.9), 0.125)
#			tween.tween_property($Sprite2D, "scale", Vector2(1.0, 1.0), 0.125)
		
		on_ground = false

func break_egg(normal: Vector2):
	broken = true
	rotation = normal.angle() + (PI / 2)
	set_deferred("freeze", true)
	$Sprite2D.play("broken")
	egg_broken.emit()
