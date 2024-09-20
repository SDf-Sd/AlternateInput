#pragma once

#include <godot_cpp/variant/vector3.hpp>

#include "hidapi.h"
#include <thread>

#include "flag.hpp"

using namespace godot;

enum AlternateGamepadType
{
    INVALID_CONTROLLER = -1,
    SWITCH_PRO_CONTROLLER = 0
};

// abstract class
class AlternateGamepad
{
protected:
    int device_id;
    hid_device_info* dev_info;
    hid_device* dev;

    std::thread poll_thread;
    Flag poll_thread_running;

    virtual void PollThreadFunc() = 0;
    virtual void StartPollThread() = 0; // prolly doesnt need to be virtual
    virtual void StopPollThread() = 0;  // prolly doesnt need to be virtual

public:
    AlternateGamepad(hid_device_info* _dev_info, int _device_id) : dev_info(_dev_info), device_id(_device_id) {
        dev = hid_open(dev_info->vendor_id, dev_info->product_id, NULL);
    }
    virtual ~AlternateGamepad() {
        hid_close(dev);
    }

    hid_device_info* get_device_info() const { return dev_info; }
    virtual AlternateGamepadType get_type() = 0;

    virtual void reset_orientation() = 0;

    virtual Vector3 get_gyroscope() = 0;
    virtual Vector3 get_accelerometer() = 0;
    virtual Vector3 get_rotation() = 0;

    virtual size_t write_subcommand(uint8_t* response, uint8_t subcommand, uint8_t* data, size_t data_len) = 0;
    // method to bring parse inputs in godot
    // what features does it have (gyroscope, aceelerometer, touchpad, etc.)
};