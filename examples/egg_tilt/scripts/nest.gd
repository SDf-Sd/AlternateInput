extends Area2D

signal egg_entered

func _ready():
	connect("body_entered", _on_body_entered)


func _on_body_entered(body):
	if body.is_in_group("egg"):
		egg_entered.emit()
