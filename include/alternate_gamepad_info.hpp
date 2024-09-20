#pragma once

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "hidapi.h"

using namespace godot;

class AlternateGamepadInfo : public RefCounted
{
    GDCLASS(AlternateGamepadInfo, RefCounted);

protected:
    static void _bind_methods();
public:
    AlternateGamepadInfo();
    ~AlternateGamepadInfo();

    void from(hid_device_info* dev, int id);

    // all of these should not be modifiable in gdscript.

    String device_name;
    String manufacturer_name;
    uint16_t usage_page = 0;
    uint16_t usage = 0;
    uint16_t vendor_id = 0;
    uint16_t product_id = 0;
    int device_id;
    bool opened = false;

    void set_device_name(String value) { /* device_name = value; */ }
    String get_device_name() { return device_name; }

    void set_manufacturer_name(String value) { /* manufacturer_name = value; */ }
    String get_manufacturer_name() { return manufacturer_name; }

    void set_usage_page(int value) { /* usage_page = value; */ }
    int get_usage_page() { return usage_page; }

    void set_usage(int value) { /* usage = value; */ }
    int get_usage() { return usage; }

    void set_vendor_id(int value) { /* vendor_id = value; */ }
    int get_vendor_id() { return vendor_id; }

    void set_product_id(int value) { /* product_id = value; */ }
    int get_product_id() { return product_id; }

    void set_opened(bool value) { /* opened = value; */ }
    bool get_opened() { return opened; }

    void set_device_id(int value) { }
    int get_device_id() { return device_id; }

};