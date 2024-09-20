extends Label

@export var base_string := "Score: %d"

var value := 0 :
	set(new_value):
		value = new_value
		text = base_string % value
	get:
		return value

func _ready():
	text = base_string % value


func increment(by := 1):
	value += by
