#pragma once


#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "hidapi.h"
#include <memory>
#include <map>

#include "alternate_gamepad_info.hpp"

#include "alternate_gamepad.hpp"

using namespace godot;

class AlternateInput : public Object
{
	GDCLASS(AlternateInput, Object);

	static AlternateInput *singleton;

protected:
	static void _bind_methods();

private:
	std::map<int, std::shared_ptr<AlternateGamepad>> borrowed_gamepads;
	std::map<int, hid_device_info*> available_gamepads;
	std::map<int, Ref<AlternateGamepadInfo>> all_gamepads;



public:
	static AlternateInput *get_singleton();

	AlternateInput();
	~AlternateInput();

	Vector3 get_gyroscope(int32_t device);     // should return in rads / second
	Vector3 get_accelerometer(int32_t device); // should return in meters / second^2
	Vector3 get_rotation(int32_t device); // should return in rads 

	Vector3 get_any_gyroscope();
	Vector3 get_any_accelerometer();
	Vector3 get_any_rotation();

	void gd_steal_gamepad(int32_t device, int32_t type);
	void reset_orientation(int32_t device);

	void steal_gamepad(int32_t device, AlternateGamepadType type);
	void return_gamepad(int32_t device); // automatically done on clean up
	void enumerate_gamepads(uint16_t vid = 0, uint16_t pid = 0);

	TypedArray<AlternateGamepadInfo> gd_get_gamepads_info();

	void on_joy_connection_changed(int device, bool connected);
};
