#include "alternate_input.hpp"


#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/string.hpp>

#include "switch_gamepad.hpp"

AlternateInput *AlternateInput::singleton = nullptr;

void AlternateInput::_bind_methods()
{
	// ADD_SIGNAL(MethodInfo("gamepad_shaked", PropertyInfo(Variant::INT, "device_id"), PropertyInfo(Variant::FLOAT, "magnitude")));
	
	ClassDB::bind_integer_constant("AlternateInput", "AlternateGamepadType", "INVALID_CONTROLLER", AlternateGamepadType::INVALID_CONTROLLER);
	ClassDB::bind_integer_constant("AlternateInput", "AlternateGamepadType", "SWITCH_PRO_CONTROLLER", AlternateGamepadType::SWITCH_PRO_CONTROLLER);

	ClassDB::bind_method(D_METHOD("on_joy_connection_changed"), &AlternateInput::on_joy_connection_changed); // so godot recognizes it

	ClassDB::bind_method(D_METHOD("steal_gamepad", "device", "type"), &AlternateInput::gd_steal_gamepad);	
	ClassDB::bind_method(D_METHOD("return_gamepad"), &AlternateInput::return_gamepad);
	ClassDB::bind_method(D_METHOD("get_gyroscope", "device"), &AlternateInput::get_gyroscope);
	ClassDB::bind_method(D_METHOD("get_any_gyroscope"), &AlternateInput::get_any_gyroscope);
	ClassDB::bind_method(D_METHOD("get_accelerometer", "device"), &AlternateInput::get_accelerometer);
	ClassDB::bind_method(D_METHOD("get_any_accelerometer"), &AlternateInput::get_any_accelerometer);
	ClassDB::bind_method(D_METHOD("get_rotation", "device"), &AlternateInput::get_rotation);
	ClassDB::bind_method(D_METHOD("get_any_rotation"), &AlternateInput::get_any_rotation);
	ClassDB::bind_method(D_METHOD("reset_orientation", "device"), &AlternateInput::reset_orientation);
	ClassDB::bind_method(D_METHOD("get_gamepads_info"), &AlternateInput::gd_get_gamepads_info);
}

AlternateInput *AlternateInput::get_singleton()
{
	return singleton;
}

AlternateInput::AlternateInput()
{
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;

	if (Engine::get_singleton()->is_editor_hint())
	{
		UtilityFunctions::print("Hello from AlternateInput in the Editor!");
	}
	else
	{
		UtilityFunctions::print("Hello from AlternateInput in the Game!");

		Input::get_singleton()->connect("joy_connection_changed", Callable(this, "on_joy_connection_changed"), 0);

		hid_init();
		enumerate_gamepads();

		// TODO: we want to complete the interfacing with the switch controller hid api
		
		// TODO: maybe also add dualshock 4 support? or any controller (i mean its called alternateinput)
		// so like wiimotes and other things.

		// There is a web version using JavascriptBridge, but Itch.io does not support WebHID.
	}
}

AlternateInput::~AlternateInput()
{
	if (Engine::get_singleton()->is_editor_hint())
	{

	}
	else
	{
		hid_exit();
	}

	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}


void AlternateInput::steal_gamepad(int32_t device, AlternateGamepadType type)
{
	std::map<int, hid_device_info*>::iterator it = available_gamepads.find(device);


	if (it != available_gamepads.end())
	{
		hid_device_info* info = it->second;
		std::shared_ptr<AlternateGamepad> gamepad; 

		switch (type)
		{
		case AlternateGamepadType::SWITCH_PRO_CONTROLLER:
			UtilityFunctions::print("here");
			gamepad = std::make_shared<SwitchGamepad>(info, device);
			// gamepad = std::shared_ptr<AlternateGamepad>(new SwitchGamepad(info, device)); // this sometimes causes a crash...
			break;
		default:
			UtilityFunctions::push_warning("Cannot steal invalid AlternateGamepad type.");
			return;
			break;
		}

		borrowed_gamepads.insert(std::pair<int32_t, std::shared_ptr<AlternateGamepad>>(
			device, std::move(gamepad)
		));


		UtilityFunctions::print("size: ", all_gamepads.begin()->first, " at: ", device);
		all_gamepads.at(device)->opened = true;
		available_gamepads.erase(it);
		UtilityFunctions::print("size: ", all_gamepads.size());
	}
	else
	{
		UtilityFunctions::push_warning("Device #", device, " is not available to be stolen.");
	}
}


void AlternateInput::return_gamepad(int32_t device)
{
	std::map<int, std::shared_ptr<AlternateGamepad>>::iterator it = borrowed_gamepads.find(device);

	if (it != borrowed_gamepads.end())
	{
		available_gamepads.insert(std::pair<int, hid_device_info*>(
			device, it->second->get_device_info()
		));

		all_gamepads.at(device)->opened = false;
		borrowed_gamepads.erase(it);
	}
	else
	{
		UtilityFunctions::push_warning("Device #", device, " is not available to be returned.");
	}
}


void AlternateInput::on_joy_connection_changed(int device, bool connected)
{
	if (connected)
	{
		String name = Input::get_singleton()->get_joy_name(device);

		hid_device_info *devs, *cur_dev;
		devs = hid_enumerate(0, 0);
		cur_dev = devs;
		while (cur_dev)
		{
			if (name == String(cur_dev->product_string))
			{
				available_gamepads.insert(std::pair<int, hid_device_info*>(device, cur_dev));
				Ref<AlternateGamepadInfo> new_gamepad;
				new_gamepad.instantiate();
				new_gamepad->from(cur_dev, device);
				all_gamepads.insert(std::pair<int, Ref<AlternateGamepadInfo>>(device, new_gamepad));
				break;
			}
			cur_dev = cur_dev->next;
		}
	}
	else
	{
		std::map<int, hid_device_info*>::iterator ag_it = available_gamepads.find(device);

		if (ag_it != available_gamepads.end())
		{
			available_gamepads.erase(ag_it);
		}

		std::map<int, std::shared_ptr<AlternateGamepad>>::iterator bg_it = borrowed_gamepads.find(device);

		if (bg_it != borrowed_gamepads.end())
		{
			borrowed_gamepads.erase(bg_it);
		}

		std::map<int, Ref<AlternateGamepadInfo>>::iterator all_it = all_gamepads.find(device);

		if (all_it != all_gamepads.end())
		{
			all_gamepads.erase(all_it);
		}
	}

}


void AlternateInput::enumerate_gamepads(uint16_t vid, uint16_t pid)
{
	TypedArray<int> connected_joypads = Input::get_singleton()->get_connected_joypads();
	TypedArray<String> connected_joypad_names;
	for (size_t i = 0; i < connected_joypads.size(); i++)
	{
		connected_joypad_names.append(Input::get_singleton()->get_joy_name(connected_joypads[i]));
	}

	hid_device_info *all_devs, *cur_dev;
	all_devs = hid_enumerate(vid, pid);
	cur_dev = all_devs;
	while (cur_dev)
	{
		int index = connected_joypad_names.find(String(cur_dev->product_string));
		if (index >= 0)
		{
			auto all_it = all_gamepads.find(index);
			auto bg_it = borrowed_gamepads.find(index);
			auto ag_it = available_gamepads.find(index);
			if (bg_it == borrowed_gamepads.end() || ag_it == available_gamepads.end())
			{
				available_gamepads.insert(std::pair<int, hid_device_info*>(index, cur_dev));
			}
			if (all_it == all_gamepads.end())
			{
				Ref<AlternateGamepadInfo> new_gamepad;
				new_gamepad.instantiate();
				new_gamepad->from(cur_dev, index);
				new_gamepad->opened = (bg_it != borrowed_gamepads.end()); // most likely worthless...
				all_gamepads.insert(std::pair<int, Ref<AlternateGamepadInfo>>(index, new_gamepad));
			}
		}
		else if (cur_dev->product_id == SwitchGamepad::product_l || cur_dev->product_id == SwitchGamepad::product_r
				|| cur_dev->product_id == SwitchGamepad::product_pro)
		{
			available_gamepads.insert(std::pair<int, hid_device_info*>(0, cur_dev));
			Ref<AlternateGamepadInfo> new_gamepad;
			new_gamepad.instantiate();
			new_gamepad->from(cur_dev, 0);
			all_gamepads.insert(std::pair<int, Ref<AlternateGamepadInfo>>(0, new_gamepad));
		}
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(all_devs);
}


Vector3 AlternateInput::get_gyroscope(int32_t device)
{
	auto it = borrowed_gamepads.find(device);
	if (it != borrowed_gamepads.end())
	{
		// maybe check if device is capable of returning gyroscope/accel values
		return it->second->get_gyroscope();
	}
	else
	{
		UtilityFunctions::push_warning("Cannot get gyroscope of unborrowed device.");
		return Vector3();
	}
}


Vector3 AlternateInput::get_accelerometer(int32_t device)
{
	auto it = borrowed_gamepads.find(device);
	if (it != borrowed_gamepads.end())
	{
		// maybe check if device is capable of returning gyroscope/accel values
		return it->second->get_accelerometer();
	}
	else
	{
		UtilityFunctions::push_warning("Cannot get accelerometer of unborrowed device.");
		return Vector3();
	}
}


Vector3 AlternateInput::get_rotation(int32_t device)
{
	auto it = borrowed_gamepads.find(device);
	if (it != borrowed_gamepads.end())
	{
		// maybe check if device is capable of returning gyroscope/accel values
		return it->second->get_rotation();
	}
	else
	{
		UtilityFunctions::push_warning("Cannot get rotation of unborrowed device.");
		return Vector3();
	}
}


Vector3 AlternateInput::get_any_accelerometer()
{
	for (auto pair : borrowed_gamepads)
	{
		return pair.second->get_accelerometer();
	}
	UtilityFunctions::push_warning("No available gamepads to get accel data from.");
	return Vector3();
}


Vector3 AlternateInput::get_any_gyroscope()
{
	for (auto pair : borrowed_gamepads)
	{
		return pair.second->get_gyroscope();
	}
	UtilityFunctions::push_warning("No available gamepads to get gyroscope data from.");
	return Vector3();
}


Vector3 AlternateInput::get_any_rotation()
{
	for (auto pair : borrowed_gamepads)
	{
		return pair.second->get_rotation();
	}
	UtilityFunctions::push_warning("No available gamepads to get rotation data from.");
	return Vector3();
}


void AlternateInput::reset_orientation(int32_t device)
{
	auto it = borrowed_gamepads.find(device);
	if (it != borrowed_gamepads.end())
	{
		// maybe check if device is capable of returning gyroscope/accel values
		it->second->reset_orientation();
	}
	else
	{
		UtilityFunctions::push_warning("Cannot reset orientation of unborrowed device.");
	}
}


void AlternateInput::gd_steal_gamepad(int32_t device, int32_t type)
{
	steal_gamepad(device, (AlternateGamepadType)type);
}

TypedArray<AlternateGamepadInfo> AlternateInput::gd_get_gamepads_info()
{
	TypedArray<AlternateGamepadInfo> arr;
	for (auto pair : all_gamepads)
	{
		arr.push_back(pair.second);
	}
	return arr;
}