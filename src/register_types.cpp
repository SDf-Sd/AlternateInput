#include "register_types.h"
#include "alternate_input.hpp"
#include "alternate_gamepad_info.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

static AlternateInput *_alternate_input;

void initialize_alternateinput_types(ModuleInitializationLevel p_level)
{
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<AlternateGamepadInfo>();
	ClassDB::register_class<AlternateInput>();

	_alternate_input = memnew(AlternateInput);
	Engine::get_singleton()->register_singleton("AlternateInput", AlternateInput::get_singleton());
}

void uninitialize_alternateinput_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	Engine::get_singleton()->unregister_singleton("AlternateInput");
	memdelete(_alternate_input);
}

extern "C"
{

	// Initialization.

	GDExtensionBool GDE_EXPORT alternateinput_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
		GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

		init_obj.register_initializer(initialize_alternateinput_types);
		init_obj.register_terminator(uninitialize_alternateinput_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}
