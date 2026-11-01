# Getting started

This repository provides an Arduino library for servo control and micro-ROS
command/state handling on Teensy boards. It targets the ROS 2 Humble build of
[`micro_ros_arduino`](https://github.com/micro-ROS/micro_ros_arduino/tree/humble).

## Supported boards

The library is intended for Teensy 3.x and Teensy 4.x boards that are supported
by the selected `micro_ros_arduino` build. The Humble branch README lists these
statuses:

| Board | Humble branch status |
| --- | --- |
| Teensy 3.1 / 3.2 | Supported |
| Teensy 3.5 | Not tested |
| Teensy 3.6 | Supported |
| Teensy 4.0 | Not tested |
| Teensy 4.1 | Supported |

“Supported” here refers to the board status in that micro-ROS branch. The
chosen PWM servo library, pin availability, memory use, and board toolchain
must also be suitable for the particular project. The included InMoov example
uses a Teensy 4.1 pin configuration.

## Install dependencies

The library depends on `PWMServo` and `micro_ros_arduino`. `library.properties`
declares both dependencies for Arduino tooling that supports dependency
resolution. Install a Humble-compatible `micro_ros_arduino` release separately
if it is not available through your IDE's library manager. Use the Humble
branch's Teensyduino setup instructions and follow its platform patch
instructions when required by the selected precompiled release. Arduino IDE
does not necessarily automate all of that board-specific setup.

Then install this library from a release ZIP or clone/copy it into the Arduino
libraries directory. Open `examples/InMoovTeensy/InMoovTeensy.ino` in Arduino
IDE, select the target Teensy board, and build/upload.

## Configure another robot

Copy the example and replace `InMoovServoConfig.h` with your own definitions.
Each `ServoDefinition` provides a name, PWM pin, logical angle limits, pulse
width limits, direction, maximum movement speed, and startup angle. Supply one
ROS joint name per servo in the same order. The library supports up to
`SERVO_CONTROLLER_MAX_SERVOS` servos and uses fixed storage rather than heap
allocation.

The `MicroROSControllerConfig` sets the node name, command topic, state topic,
joint names, ROS domain ID, command timeout, and state publish period. Commands
use `std_msgs/msg/Float32MultiArray` in radians, with one value per servo in
configuration order. State is published as `sensor_msgs/msg/JointState`, also
in radians and the same order. A zero timeout disables timeout holding; a zero
publish period disables state publishing.

By default, `begin()` configures the micro-ROS Arduino serial transport. For a
different transport supported by the selected micro-ROS build, configure it
with that library's transport API first, then call `begin(false)`.

## Safety and calibration

The example's calibration values are starting points only. Check servo power,
mechanical range, wiring, pulse endpoints, direction, and startup position
before attaching a mechanism. A servo's PWM command does not provide measured
position feedback; the published joint positions are the controller's
estimated commanded positions.
