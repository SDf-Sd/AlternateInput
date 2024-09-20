#include "alternate_gamepad_info.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/string.hpp>

AlternateGamepadInfo::AlternateGamepadInfo()
{

}

AlternateGamepadInfo::~AlternateGamepadInfo()
{

}

void AlternateGamepadInfo::from(hid_device_info* dev, int id)
{
    device_name = String(dev->product_string);
    manufacturer_name = String(dev->manufacturer_string);
    usage_page = dev->usage_page;
    usage = dev->usage;
    vendor_id = dev->vendor_id;
    product_id = dev->product_id;
    device_id = id;
}


void AlternateGamepadInfo::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_device_name"), &AlternateGamepadInfo::get_device_name);
	ClassDB::bind_method(D_METHOD("get_manufacturer_name"), &AlternateGamepadInfo::get_manufacturer_name);
	ClassDB::bind_method(D_METHOD("get_usage_page"), &AlternateGamepadInfo::get_usage_page);
	ClassDB::bind_method(D_METHOD("get_usage"), &AlternateGamepadInfo::get_usage);
	ClassDB::bind_method(D_METHOD("get_vendor_id"), &AlternateGamepadInfo::get_vendor_id);
	ClassDB::bind_method(D_METHOD("get_product_id"), &AlternateGamepadInfo::get_product_id);
	ClassDB::bind_method(D_METHOD("get_device_id"), &AlternateGamepadInfo::get_device_id);
	ClassDB::bind_method(D_METHOD("get_opened"), &AlternateGamepadInfo::get_opened);

    ClassDB::bind_method(D_METHOD("set_device_name", "value"), &AlternateGamepadInfo::set_device_name);
	ClassDB::bind_method(D_METHOD("set_manufacturer_name", "value"), &AlternateGamepadInfo::set_manufacturer_name);
	ClassDB::bind_method(D_METHOD("set_usage_page", "value"), &AlternateGamepadInfo::set_usage_page);
	ClassDB::bind_method(D_METHOD("set_usage", "value"), &AlternateGamepadInfo::set_usage);
	ClassDB::bind_method(D_METHOD("set_vendor_id", "value"), &AlternateGamepadInfo::set_vendor_id);
	ClassDB::bind_method(D_METHOD("set_product_id", "value"), &AlternateGamepadInfo::set_product_id);
    ClassDB::bind_method(D_METHOD("set_device_id", "value"), &AlternateGamepadInfo::set_device_id);
    ClassDB::bind_method(D_METHOD("set_opened", "value"), &AlternateGamepadInfo::set_opened);
    
    ClassDB::add_property("AlternateGamepadInfo", PropertyInfo(
            Variant::STRING, "device_name"), "set_device_name", "get_device_name");
    ClassDB::add_property("AlternateGamepadInfo", PropertyInfo(
            Variant::STRING, "manufacturer_name"), "set_manufacturer_name", "get_manufacturer_name");
    ClassDB::add_property("AlternateGamepadInfo", PropertyInfo(
            Variant::INT, "usage_page"), "set_usage_page", "get_usage_page");
    ClassDB::add_property("AlternateGamepadInfo", PropertyInfo(
            Variant::INT, "usage"), "set_usage", "get_usage");
    ClassDB::add_property("AlternateGamepadInfo", PropertyInfo(
            Variant::INT, "vendor_id"), "set_vendor_id", "get_vendor_id");
    ClassDB::add_property("AlternateGamepadInfo", PropertyInfo(
            Variant::INT, "product_id"), "set_product_id", "get_product_id");
    ClassDB::add_property("AlternateGamepadInfo", PropertyInfo(
            Variant::INT, "device_id"), "set_device_id", "get_device_id");
    ClassDB::add_property("AlternateGamepadInfo", PropertyInfo(
            Variant::BOOL, "opened"), "set_opened", "get_opened");
}