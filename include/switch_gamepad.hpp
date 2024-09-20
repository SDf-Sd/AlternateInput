#pragma once

#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/classes/input_event_joypad_button.hpp>
#include <godot_cpp/classes/input_event_joypad_motion.hpp>

#include <chrono>

#include "alternate_gamepad.hpp"

enum JoyConButton
{
	DPAD_DOWN = 0,
	DPAD_RIGHT,
	DPAD_UP,
	DPAD_LEFT,
	SL,
	SR,
	MINUS,
	HOME,
	PLUS,
	CAPTURE,
	STICK,
	SHOULDER_1,
	SHOULDER_2,

	// pro controller
	B,
	A,
	Y,
	X,
	STICK2,
	SHOULDER2_1,
	SHOULDER2_2,
	JOYCONBUTTON_MAX
};

using namespace std::chrono;
using namespace godot;

// remove this
/**
 * @author icy_float
 * @attention This gamepad only supports pro controllers for now (only third-party ones have been tested) 
 * TODO: 
 *  * Add support for pairs of joycons
 *  * Add rumble send/receive
 *  * Get access to IR
 * 
 * @date 5/29/2023
 */

class SwitchGamepad : public AlternateGamepad
{
private:
    bool current_buttons_pressed[JOYCONBUTTON_MAX];
    bool buttons_just_pressed[JOYCONBUTTON_MAX];
    bool buttons_just_released[JOYCONBUTTON_MAX];

    Vector2 stick;

    uint8_t stick_raw[3];
    uint16_t stick_cal[6];
    uint16_t deadzone;
    uint16_t stick_precal[2];

    Vector2 stick2;

    uint8_t stick2_raw[3];
    uint16_t stick2_cal[6];
    uint16_t deadzone2;
    uint16_t stick2_precal[2];

    int16_t pro_hor_offset[3] { -710, 0, 0 };
    int16_t left_hor_offset[3] { 0, 0, 0 };
    int16_t right_hor_offset[3] { 0, 0, 0 };

    int16_t acc_r[3];
    int16_t acc_neutral[3];
    int16_t acc_sensiti[3];
    Vector3 acc_g;
    Vector3 acc_p;

    int16_t gyr_r[3];
    int16_t gyr_neutral[3];
    int16_t gyr_sensiti[3];
    Vector3 gyr_g; // degs / second (ZYX)
    Vector3 gyr_p; // rads / second

    uint16_t acc_sen[3] { 16384, 16384, 16384 };
    uint16_t gyr_sen[3] { 18642, 18642, 18642 };

    Vector3 rotation;

    steady_clock::time_point last_rot_update = steady_clock::now();

protected:
    int8_t global_count = 0;

    void PollThreadFunc() override;
    void StartPollThread() override;
    void StopPollThread() override;
public:
    SwitchGamepad(hid_device_info* _dev_info, int _device_id);
    ~SwitchGamepad();

	static const uint16_t product_l = 0x2006;
	static const uint16_t product_r = 0x2007;
	static const uint16_t product_pro = 0x2009;

    const float stick_scaling_factor = 1.0;
    const float stick2_scaling_factor = 1.0;

    bool is_pro = true; // for now always true
    bool is_left = true; // for now always true
    bool is_thirdparty = false; // for now always true

    static JoyButton to_joybutton(JoyConButton from);
    void parse_button_event(JoyButton btn, bool pressed);
    void parse_axis_event(JoyAxis axis, float value);

    void reset_orientation() override;
    Vector3 get_gyroscope() override;
    Vector3 get_accelerometer() override;
    Vector3 get_rotation() override;

    void extract_button_and_stick_info(const uint8_t* buf, size_t len);
    void extract_imu_values(const uint8_t* response, size_t len, int n = 0);
    Vector2 calculate_stick_data(uint16_t vals[2], uint16_t cal[6], uint16_t deadzone, float scale);
    void update_rotation();
    
    void set_led_by_player(int num);
    void read_spi(uint8_t* response, uint8_t addr1, uint8_t addr2, size_t len);
    void dump_calibration_data();

    AlternateGamepadType get_type() { return AlternateGamepadType::SWITCH_PRO_CONTROLLER; };
    size_t write_subcommand(uint8_t* response, uint8_t subcommand, uint8_t* data, size_t data_len) override;
};