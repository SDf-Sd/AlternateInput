extends Window


func _ready():
	about_to_popup.connect(_on_about_to_popup)


func _on_about_to_popup():
	get_tree().set_deferred("paused", true)


func _exit_tree():
	get_tree().set_deferred("paused", false)
