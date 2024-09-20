
#include "switch_gamepad.hpp"

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/string.hpp>

#define REPORT_LEN 49

SwitchGamepad::SwitchGamepad(hid_device_info* _dev_info, int _device_id) : AlternateGamepad(_dev_info, _device_id)
{
    // rotation.z = Math_PI;

    is_thirdparty = true;

    switch (_dev_info->product_id)
    {
        case product_l:
            UtilityFunctions::print("left");
            is_left = true;
            is_pro = false;
            break;
        case product_r:
            UtilityFunctions::print("right");
            is_left = false;
            is_pro = false;
            break;
        case product_pro:
            UtilityFunctions::print("pro");
            is_left = true;
            is_pro = true;
            break;
        default:
            UtilityFunctions::print("default");
            is_pro = true;
            is_left = true;
            is_thirdparty = true;
            break;
    }

    {
        uint8_t response[REPORT_LEN];
        uint8_t data[] {0x30};
        size_t ret = write_subcommand(response, 0x03, data, 1); // set input to 0x30
    }
    {
        uint8_t response[REPORT_LEN];
        uint8_t data[] {0x01};
        size_t ret = write_subcommand(response, 0x40, data, 1); // enable gyro
    }

    dump_calibration_data();

    set_led_by_player(device_id);

    StartPollThread();
}


SwitchGamepad::~SwitchGamepad()
{
    StopPollThread();

    {
        uint8_t response[REPORT_LEN];
        uint8_t data[1] {0x3F};
        size_t ret = write_subcommand(response, 0x03, data, 1); // set input to 0x3F (standard)
    }
    {
        uint8_t response[REPORT_LEN];
        uint8_t data[1] {0x00};
        write_subcommand(response, 0x40, data, 1); // disable gyro
    }
}


constexpr uint8_t header[8] {0x0, 0x1, 0x40, 0x40, 0x0, 0x1, 0x40, 0x40};

size_t SwitchGamepad::write_subcommand(uint8_t* response, uint8_t subcommand, uint8_t* data, size_t data_len)
{
    uint8_t* buf = new uint8_t[data_len+11];
    memcpy(buf+2, header, 8);
    memcpy(buf+11, data, data_len);
    buf[10] = subcommand;
    buf[0] = 0x1;
    buf[1] = global_count;
    if (global_count == 0xf) global_count = 0;
    else global_count++;

    hid_write(dev, buf, 11+data_len);

    delete[] buf;

    int res;
	int tries = 0;
	do 
	{
		res = hid_read_timeout(dev, response, REPORT_LEN, 100);
		if (res < 1);
		tries++;
	}
	while(tries < 10 && response[0] != 0x21 && response[14] != subcommand);
	
	return res;
}

void SwitchGamepad::read_spi(uint8_t* response, uint8_t addr1, uint8_t addr2, size_t len)
{
    uint8_t buf[5] { addr2, addr1, 0x00, 0x00, (uint8_t)len };
    uint8_t* sc_r_buf = new uint8_t[len + 20];

    for (size_t i = 0; i < 100; i++)
    {
        write_subcommand(sc_r_buf, 0x10, buf, 5);
        if (sc_r_buf[15] == addr2 && sc_r_buf[16] == addr1)
        {
            break;
        }
    }

    memcpy(response, sc_r_buf+20, len);
    
    delete[] sc_r_buf;
}


void SwitchGamepad::dump_calibration_data()
{
    // if thirdparty fetch values from def settings
    if (is_thirdparty)
    {
        // from BetterJoy Default Settings
        // add accel + gyro
        acc_sensiti[0] = 16384; acc_sensiti[1] = 16384; acc_sensiti[2] = 16384;
        gyr_sensiti[0] = 18642; gyr_sensiti[1] = 18642; gyr_sensiti[2] = 18642;
        stick_cal[0] = 0x780; stick_cal[1] = 0x780; stick_cal[2] = 0x780; 
        stick_cal[3] = 0x830; stick_cal[4] = 0x780; stick_cal[5] = 0x780;
        deadzone = 200;
        stick2_cal[0] = 0x780; stick2_cal[1] = 0x780; stick2_cal[2] = 0x780; 
        stick2_cal[3] = 0x830; stick2_cal[4] = 0x780; stick2_cal[5] = 0x780;
        deadzone2 = 200;

        return;
    }

    hid_set_nonblocking(dev, 0);
    uint8_t spi_response[16]; // max size read is 16 bytes
    read_spi(spi_response, 0x80, (is_left ? 0x12 : 0x1d), 9);
    bool found = false;
    for (int i = 0; i < 9; i++)
    {
        if (spi_response[i] != 0x0ff)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        read_spi(spi_response, 0x60, (is_left ? 0x3d : 0x46), 9);
    }

    stick_cal[is_left ? 0 : 2] = (uint16_t)((spi_response[1] << 8) & 0xF00 | spi_response[0]); // X Axis Max above center
    stick_cal[is_left ? 1 : 3] = (uint16_t)((spi_response[2] << 4) | (spi_response[1] >> 4));  // Y Axis Max above center
    stick_cal[is_left ? 2 : 4] = (uint16_t)((spi_response[4] << 8) & 0xF00 | spi_response[3]); // X Axis Center
    stick_cal[is_left ? 3 : 5] = (uint16_t)((spi_response[5] << 4) | (spi_response[4] >> 4));  // Y Axis Center
    stick_cal[is_left ? 4 : 0] = (uint16_t)((spi_response[7] << 8) & 0xF00 | spi_response[6]); // X Axis Min below center
    stick_cal[is_left ? 5 : 1] = (uint16_t)((spi_response[8] << 4) | (spi_response[7] >> 4));  // Y Axis Min below center

    if (is_pro)
    {
        read_spi(spi_response, 0x80, (!is_left ? 0x12 : 0x1d), 9);
        bool found = false;
        for (int i = 0; i < 9; i++)
        {
            if (spi_response[i] != 0x0ff)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            read_spi(spi_response, 0x60, (!is_left ? 0x3d : 0x46), 9);
        }
        stick2_cal[!is_left ? 0 : 2] = (uint16_t)((spi_response[1] << 8) & 0xF00 | spi_response[0]); // X Axis Max above center
        stick2_cal[!is_left ? 1 : 3] = (uint16_t)((spi_response[2] << 4) | (spi_response[1] >> 4));  // Y Axis Max above center
        stick2_cal[!is_left ? 2 : 4] = (uint16_t)((spi_response[4] << 8) & 0xF00 | spi_response[3]); // X Axis Center
        stick2_cal[!is_left ? 3 : 5] = (uint16_t)((spi_response[5] << 4) | (spi_response[4] >> 4));  // Y Axis Center
        stick2_cal[!is_left ? 4 : 0] = (uint16_t)((spi_response[7] << 8) & 0xF00 | spi_response[6]); // X Axis Min below center
        stick2_cal[!is_left ? 5 : 1] = (uint16_t)((spi_response[8] << 4) | (spi_response[7] >> 4));  // Y Axis Min below center
    
        read_spi(spi_response, 0x60, (!is_left ? 0x86 :0x98), 16);
        deadzone2 = (uint16_t)((spi_response[4] << 8) & 0xF00 | spi_response[3]);
    }

    read_spi(spi_response, 0x60, (is_left ? 0x86 :0x98), 16);
    deadzone = (uint16_t)((spi_response[4] << 8) & 0xF00 | spi_response[3]);

    // read accel + gyro calibration data too

    hid_set_nonblocking(dev, 1);
}


void SwitchGamepad::PollThreadFunc()
{
    while (poll_thread_running.IsSet())
    {
        uint8_t response[REPORT_LEN];
        uint8_t ret = hid_read_timeout(dev, response, REPORT_LEN, 5);
        if (ret > 0)
        {
            extract_button_and_stick_info(response, ret);
            extract_imu_values(response, ret);
            update_rotation();
        }
    }
}


void SwitchGamepad::StartPollThread()
{
	if (!poll_thread_running.TestAndSet())
	{
		return;
	}

	poll_thread = std::thread(&SwitchGamepad::PollThreadFunc, this);
}


void SwitchGamepad::StopPollThread()
{
	if (!poll_thread_running.TestAndClear())
	{
		return;
	}

	poll_thread.join();
}


void SwitchGamepad::extract_button_and_stick_info(const uint8_t* buf, size_t len)
{
	if (len > REPORT_LEN)
	{
		UtilityFunctions::push_warning("Input report buffer length is not the standard one for 0x30.");
	}
	if (buf[0] != 0x30)
	{
		UtilityFunctions::push_warning("Cannot read input report of type: ", String::num_uint64(buf[0], 16, true));
	}

    stick_raw[0] = buf[6 + (is_left ? 0 : 3)];
    stick_raw[1] = buf[7 + (is_left ? 0 : 3)];
    stick_raw[2] = buf[8 + (is_left ? 0 : 3)];

    if (is_pro)
    {
        stick2_raw[0] = buf[6 + (!is_left ? 0 : 3)];
        stick2_raw[1] = buf[7 + (!is_left ? 0 : 3)];
        stick2_raw[2] = buf[8 + (!is_left ? 0 : 3)];
    }

    Vector2 last_stick = stick;

    stick_precal[0] = (uint16_t) (stick_raw[0] | ((stick_raw[1] & 0xf) << 8));
    stick_precal[1] = (uint16_t) ((stick_raw[1] >> 4) | (stick_raw[2] << 4));
    stick = calculate_stick_data(stick_precal, stick_cal, deadzone, is_left ? stick_scaling_factor : stick2_scaling_factor);

    if (is_pro)
    {
        Vector2 last_stick2 = stick2;

        stick2_precal[0] = (uint16_t) (stick2_raw[0] | ((stick2_raw[1] & 0xf) << 8));
        stick2_precal[1] = (uint16_t) ((stick2_raw[1] >> 4) | (stick2_raw[2] << 4));
        stick2 = calculate_stick_data(stick2_precal, stick2_cal, deadzone2, stick2_scaling_factor);

        if (!Math::is_equal_approx(stick2.x, last_stick2.x)) // stick changed x
        {
            parse_axis_event(JOY_AXIS_RIGHT_X, stick2.x);
        }
        if (!Math::is_equal_approx(stick2.y, last_stick2.y)) // stick changed y
        {
            // have to reverse the y axis
            parse_axis_event(JOY_AXIS_RIGHT_Y, -stick2.y);
        }
    }

    if (!Math::is_equal_approx(stick.x, last_stick.x)) // stick changed x
    {
        parse_axis_event(JOY_AXIS_LEFT_X, stick.x);
    }
    if (!Math::is_equal_approx(stick.y, last_stick.y)) // stick changed y
    {
        // have to reverse the y axis
        parse_axis_event(JOY_AXIS_LEFT_Y, -stick.y);
    }

    // add support for two joycons connected together

    bool last_buttons_pressed[JOYCONBUTTON_MAX];
    memcpy(last_buttons_pressed, current_buttons_pressed, sizeof(current_buttons_pressed));

	current_buttons_pressed[DPAD_DOWN]  = (buf[3 + (is_left ? 2 : 0)] & (is_left ? 0x01 : 0x04)) != 0;
	current_buttons_pressed[DPAD_RIGHT] = (buf[3 + (is_left ? 2 : 0)] & (is_left ? 0x04 : 0x08)) != 0;
	current_buttons_pressed[DPAD_UP]    = (buf[3 + (is_left ? 2 : 0)] & (is_left ? 0x02 : 0x02)) != 0;
	current_buttons_pressed[DPAD_LEFT]  = (buf[3 + (is_left ? 2 : 0)] & (is_left ? 0x08 : 0x01)) != 0;
	current_buttons_pressed[HOME]       = (buf[4] & 0x10) != 0;
	current_buttons_pressed[CAPTURE]    = (buf[4] & 0x20) != 0;
	current_buttons_pressed[MINUS]      = (buf[4] & 0x01) != 0;
	current_buttons_pressed[PLUS]       = (buf[4] & 0x02) != 0;
	current_buttons_pressed[STICK]      = (buf[4] & (is_left ? 0x08 : 0x04)) != 0;
	current_buttons_pressed[SHOULDER_1] = (buf[3 + (is_left ? 2 : 0)] & 0x40) != 0;
	current_buttons_pressed[SHOULDER_2] = (buf[3 + (is_left ? 2 : 0)] & 0x80) != 0;
	current_buttons_pressed[SR]         = (buf[3 + (is_left ? 2 : 0)] & 0x10) != 0;
	current_buttons_pressed[SL]         = (buf[3 + (is_left ? 2 : 0)] & 0x20) != 0;

    if (is_pro)
	{
		current_buttons_pressed[B] = (buf[3 + (!is_left ? 2 : 0)] & (!is_left ? 0x01 : 0x04)) != 0;
		current_buttons_pressed[A] = (buf[3 + (!is_left ? 2 : 0)] & (!is_left ? 0x04 : 0x08)) != 0;
		current_buttons_pressed[X] = (buf[3 + (!is_left ? 2 : 0)] & (!is_left ? 0x02 : 0x02)) != 0;
		current_buttons_pressed[Y] = (buf[3 + (!is_left ? 2 : 0)] & (!is_left ? 0x08 : 0x01)) != 0;

		current_buttons_pressed[STICK2] = (buf[4] & (!is_left ? 0x08 : 0x04)) != 0;
		current_buttons_pressed[SHOULDER2_1] = (buf[3 + (!is_left ? 2 : 0)] & 0x40) != 0;
		current_buttons_pressed[SHOULDER2_2] = (buf[3 + (!is_left ? 2 : 0)] & 0x80) != 0;
	}

    for (size_t i = 0; i < JOYCONBUTTON_MAX; i++)
    {
        buttons_just_released[i] = (last_buttons_pressed[i] & !current_buttons_pressed[i]);
        buttons_just_pressed[i] = (!last_buttons_pressed[i] & current_buttons_pressed[i]);

        if (buttons_just_released[i])
        {
			switch ((JoyConButton)i)
			{
			case SHOULDER_2:
                parse_axis_event(JOY_AXIS_TRIGGER_LEFT, 0.0);
				break;
			case SHOULDER2_2:
                parse_axis_event(JOY_AXIS_TRIGGER_RIGHT, 0.0);
				break;
			default:
                parse_button_event(to_joybutton(((JoyConButton)i)), false);
            }
        }
        else if (buttons_just_pressed[i])
        {
			switch ((JoyConButton)i)
			{
			case SHOULDER_2:
                parse_axis_event(JOY_AXIS_TRIGGER_LEFT, 1.0);
				break;
			case SHOULDER2_2:
                parse_axis_event(JOY_AXIS_TRIGGER_RIGHT, 1.0);
				break;
			default:
                parse_button_event(to_joybutton(((JoyConButton)i)), true);
            }
        }
    }
}


Vector2 SwitchGamepad::calculate_stick_data(uint16_t vals[2], uint16_t cal[6], uint16_t deadzone, float scale)
{
    Vector2 s;
    float dx = vals[0] - cal[2], dy = vals[1] - cal[3];

    if (Math::abs(dx * dx + dy * dy) < deadzone * deadzone)
    {
        return s;
    }
    s.x = dx / (dx > 0 ? cal[0] : cal[4]);
    s.y = dy / (dy > 0 ? cal[1] : cal[5]);

    // as it stands scale should always be 1.0f
    // if (scale != 1.0f)
    // {
    //     s.x *= scale;
    //     s.y *= scale;

    //     s.x = Math::max(Math::min(s.x, 1.0f), -1.0f);
    //     s.y = Math::max(Math::min(s.y, 1.0f), -1.0f);
    // }

    return s;
}


void SwitchGamepad::extract_imu_values(const uint8_t* buf, size_t len, int n)
{
    gyr_r[0] = (int16_t) (buf[19 + n * 12] | ((buf[20 + n * 12] << 8) & 0xff00));
    gyr_r[1] = (int16_t) (buf[21 + n * 12] | ((buf[22 + n * 12] << 8) & 0xff00));
    gyr_r[2] = (int16_t) (buf[23 + n * 12] | ((buf[24 + n * 12] << 8) & 0xff00));
    acc_r[0] = (int16_t) (buf[13 + n * 12] | ((buf[14 + n * 12] << 8) & 0xff00));
    acc_r[1] = (int16_t) (buf[15 + n * 12] | ((buf[16 + n * 12] << 8) & 0xff00));
    acc_r[2] = (int16_t) (buf[17 + n * 12] | ((buf[18 + n * 12] << 8) & 0xff00));

    // allow calibration?

    int16_t offset[3];

    if (is_pro)
        memcpy(offset, pro_hor_offset, sizeof(pro_hor_offset));
    else if (is_left)
        memcpy(offset, left_hor_offset, sizeof(pro_hor_offset));
    else
        memcpy(offset, right_hor_offset, sizeof(pro_hor_offset));
    
    for (size_t i = 0; i < 3; i++)
    {
        switch (i)
        {
        case 0:
            acc_g.x = (acc_r[i] - offset[i]) * (1.0f / (acc_sensiti[i] - acc_neutral[i])) * 4.0f;
            gyr_g.x = (gyr_r[i] - gyr_neutral[i]) * (816.0f / (gyr_sensiti[i] - gyr_neutral[i]));
            break;
        case 1:
            acc_g.y = (!is_left ? -1 : 1) * (acc_r[i] - offset[i]) * (1.0f / (acc_sensiti[i] - acc_neutral[i])) * 4.0f;
            gyr_g.y = -(is_left ? -1 : 1) * (gyr_r[i] - gyr_neutral[i]) * (816.0f / (gyr_sensiti[i] - gyr_neutral[i]));
            break;
        case 2:
            acc_g.z = (!is_left ? -1 : 1) * (acc_r[i] - offset[i]) * (1.0f / (acc_sensiti[i] - acc_neutral[i])) * 4.0f;
            gyr_g.z = -(is_left ? -1 : 1) * (gyr_r[i] - gyr_neutral[i]) * (816.0f / (gyr_sensiti[i] - gyr_neutral[i]));
            break;
        }
    }

    // add input from other joycon (when in joycon pair)

    acc_p = Vector3(acc_g.y, -acc_g.z, acc_g.x) * 9.80665; // wrong -> right order + g -> m/s^2
    gyr_p = Vector3(gyr_g.y, gyr_g.z, gyr_g.x) * (Math_TAU / 360.0); // degs/s -> rads/s

    // acc_g *= 9.80665; // g -> m/s^2
    // gyr_g *= Math_TAU / 360.0; // degs/s -> rads/s
}


void SwitchGamepad::set_led_by_player(int num)
{
    if (num > 3)
    {
        num = 3;
    }
    {
        uint8_t data[] {0x1 << num }; 
        uint8_t response[REPORT_LEN];
        write_subcommand(response, 0x30, data, 1);
    }

}

Vector3 SwitchGamepad::get_gyroscope()
{
    return gyr_p;
}

Vector3 SwitchGamepad::get_accelerometer()
{
    return acc_p;
}

Vector3 SwitchGamepad::get_rotation() 
{
    return rotation;
}

// rotation.z is roll
// rotation.x is pitch
// rotation.y is yaw

void SwitchGamepad::update_rotation()
{
    steady_clock::time_point now = steady_clock::now();
    auto delta_t = duration_cast<duration<float>>(now - last_rot_update).count(); // delta between controller updates (idk if its better than 1/60)

    using namespace Math;

    
    rotation.x = wrapf(rotation.x + gyr_p.x * delta_t, 0, Math_TAU) * 0.90 + wrapf(atan2f(acc_p.z, acc_p.y), 0, Math_TAU) * 0.10;
    rotation.z = wrapf(rotation.z + gyr_p.z * delta_t, 0, Math_TAU) * 0.90 + wrapf(atan2f(acc_p.x, acc_p.y), 0, Math_TAU) * 0.10;
    // rotation.z = (rotation.z + gyr_p.z * delta_t) * 0.90 + (atan2f(acc_p.x, acc_p.y)) * 0.10;
    rotation.y = wrapf(rotation.y - gyr_p.y * delta_t, 0, Math_TAU);

    last_rot_update = now;
}


void SwitchGamepad::reset_orientation()
{
    rotation = Vector3();
}



JoyButton SwitchGamepad::to_joybutton(JoyConButton from)
{
	switch (from)
	{
		case DPAD_DOWN:
			return JOY_BUTTON_DPAD_DOWN;
			break;
		case DPAD_LEFT:
			return JOY_BUTTON_DPAD_LEFT;
			break;
		case DPAD_RIGHT:
			return JOY_BUTTON_DPAD_RIGHT;
			break;
		case DPAD_UP:
			return JOY_BUTTON_DPAD_UP;
			break;
		case SL:
			return JOY_BUTTON_LEFT_SHOULDER;
			break;
		case SR:
			return JOY_BUTTON_RIGHT_SHOULDER;
			break;
		case MINUS:
			return JOY_BUTTON_BACK;
			break;
		case HOME:
			return JOY_BUTTON_GUIDE;
			break;
		case PLUS:
			return JOY_BUTTON_START;
			break;
		case CAPTURE:
			return JOY_BUTTON_MISC1;
			break;
		case STICK:
			return JOY_BUTTON_LEFT_STICK;
			break;
		case SHOULDER_1:
			return JOY_BUTTON_LEFT_SHOULDER;
			break;
		case A:
			return JOY_BUTTON_B;
			break;
		case B:
			return JOY_BUTTON_A;
			break;
		case Y:
			return JOY_BUTTON_X;
			break;
		case X:
			return JOY_BUTTON_Y;
			break;
		case STICK2:
			return JOY_BUTTON_RIGHT_STICK;
			break;
		case SHOULDER2_1:
			return JOY_BUTTON_RIGHT_SHOULDER;
			break;
		default:
			return JOY_BUTTON_INVALID;
			break;
    }
}

void SwitchGamepad::parse_axis_event(JoyAxis axis, float value)
{
    InputEventJoypadMotion* event = memnew(InputEventJoypadMotion);
    event->set_axis(axis);
    event->set_axis_value(value);
    event->set_device(device_id);
    Input::get_singleton()->parse_input_event(event);
}

void SwitchGamepad::parse_button_event(JoyButton btn, bool pressed)
{
    InputEventJoypadButton* event = memnew(InputEventJoypadButton);
    event->set_button_index(btn);
    event->set_pressed(pressed);
    event->set_pressure(pressed ? 1.0 : 0.0);

    event->set_device(device_id);
    Input::get_singleton()->parse_input_event(event);
}