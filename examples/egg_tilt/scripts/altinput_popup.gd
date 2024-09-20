extends PopupPanel


@onready var gamepad_list := $Stuff/GamepadList


func _on_about_to_popup():
	gamepad_list.clear()
	var gamepad_arr : Array[AlternateGamepadInfo] = AlternateInput.get_gamepads_info()
	
	for gamepad in gamepad_arr:
		var index = gamepad_list.add_item("%04X/%04X: %s     Stolen: %s" % [gamepad.vendor_id, gamepad.product_id, gamepad.device_name, gamepad.opened])
		gamepad_list.set_item_metadata(index, gamepad.device_id)
		print(gamepad.device_id)


func _on_steal_pressed():
	if gamepad_list.get_selected_items().size() == 0:
		return
	var index = gamepad_list.get_selected_items()[0]
	var device_id = gamepad_list.get_item_metadata(index)
	AlternateInput.steal_gamepad(device_id, AlternateInput.SWITCH_PRO_CONTROLLER)
	_on_about_to_popup()


func _on_return_pressed():
	if gamepad_list.get_selected_items().size() == 0:
		return
	var index = gamepad_list.get_selected_items()[0]
	var device_id = gamepad_list.get_item_metadata(index)
	AlternateInput.return_gamepad(device_id)
	_on_about_to_popup()
