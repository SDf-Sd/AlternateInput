extends PopupMenu


func _ready():
	connect("id_pressed", on_id_pressed)


func on_id_pressed(id: int):
	AlternateInput.steal_gamepad(id, AlternateInput.SWITCH_PRO_CONTROLLER) # only type rn


func _on_connect_controllers_pressed():
	for i in item_count:
		remove_item(i)
	
	var ag = AlternateInput.get_available_gamepads()
	for key in ag:
		add_check_item(ag[key], key)
	
	popup_centered()
