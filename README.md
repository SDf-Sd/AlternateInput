# AlternateInput

A GDExtension that allows Godot to read alternative inputs from support gamepads.
The only gamepads supported right now are the Nintendo Switch Pro Controller and the JoyCons (not fully tested)

### Repository structure:
- `hidapi/` Submodule needed for AlternateInput compilation
- `include/` Include files needed for compilation
- `lib/` Libraries needed for compilation. As of now just libhid.
- `examples/` Examples of AlternateInput being used in a Godot 4.x Project.
- `src/` - Source code of this extension.
- `godot-cpp/` - Submodule needed for AlternateInput compilation.

### Compilation

To compile it for Windows, run 'scons platform=windows' and this will compile godot-cpp and then AlternateInput.

It works fine with MSVC, but it should work with MINGW. You just have to run compile it with 'use_mingw=true'
For me, the linking step takes like 30 minutes for godot-cpp and I don't know why.

It should be able to be compiled and run on macos and linux but there are no guarentees.

### Getting Started

As of now, it has only been tested for Windows.

All of the usage should be through a singleton added called 'AlternateInput'.

There is also a class added called AlterinateGamepadInfo that is used for seeing what gamepads are currently connected over USB or Bluetooth.

As of now, only Bluetooth controllers have been tested.

There are two projects using AlternateInput included in the examples, although they are not finished and I haven't tested them recently.

### How It Works

First, all the names of the gamepads currently connected in Godot are used to get the actual hid_device_info using libhid. Then AlternateInput 'steals' or rather sends a subcommand to Nintendo Switch controllers to start sending gyroscope and accelerometer data to the computer. However, this means it is no longer treated like an XInput Controller so we have to parse all the data sent. The parsing is done in `src/switch_gamepad.cpp`.


### Goals

I have abandoned this project for about 1-2 years. I might update it a little bit but there are probably a lot of problems that could be fixed and additions that could be made.
Originally, I wanted this to be an extension that could be compatible with all sorts of controllers from consoles like the Wii, PS3, PS4. I also had wanted to get other data from the Switch Joycons like the IR data.

If you feel like updating or adding new additions, feel free to fork the project and get it working.

### Compatibility with Godot Versions

This current branch is built for Godot 4.3.

### Dependenices

- HIDAPI
- godot-cpp 4.3

To run the examples (for windows atleast), you need hidapi.dll included with the gdextension libraries.

### Resources

[text](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering)
[text](https://github.com/Davidobot/BetterJoy)

It would not have been possible for this to be made without the above repos.

## This project structure is based on [text](https://github.com/paddy-exe/GDExtensionSummator)