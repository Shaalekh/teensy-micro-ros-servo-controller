<div align="center">

# Teensy micro-ROS Servo Controller

**A reusable Arduino library for commanding calibrated hobby servos from ROS 2 through micro-ROS.**

[![Platform: Teensy](https://img.shields.io/badge/platform-Teensy%203.x%20%7C%204.x-314cb6)](https://www.pjrc.com/teensy/)
[![ROS 2: Humble](https://img.shields.io/badge/ROS%202-Humble-22314e)](https://docs.ros.org/en/humble/)
[![Arduino library](https://img.shields.io/badge/Arduino-library-00979D)](https://docs.arduino.cc/)

</div>

Use the same controller code with your own servo pins, calibration, joint names, and ROS topics. The included example provides a starting point for a custom Teensy-based servo project.

> **Compatibility:** targets the ROS 2 Humble branch of [`micro_ros_arduino`](https://github.com/micro-ROS/micro_ros_arduino/tree/humble). Teensy support status varies by board; see [Supported boards](#supported-boards).

## At a glance

```mermaid
flowchart LR
    R[ROS 2 application] <--> A[micro-ROS Agent]
    A <--> M[Teensy micro-ROS client]
    M <--> C[Servo controller]
    C --> S[PWM servos]
```

- **Configurable:** servo count, pins, travel and pulse limits, direction, speed, startup angle, ROS node and topics, and joint names.
- **Embedded-friendly:** fixed-capacity storage for up to 32 servos; no heap allocation in the controller.
- **Motion handling:** clamps target angles and steps toward targets at the configured speed.
- **Connection behavior:** retries micro-ROS entity creation after a disconnect and can hold the last estimated position after a command timeout.
- **Example included:** configurable Teensy servo setup.

## Supported boards

This library targets Teensy 3.x and 4.x boards supported by the selected Humble micro-ROS Arduino build. The [Humble branch README](https://github.com/micro-ROS/micro_ros_arduino/blob/humble/README.md#supported-boards) lists the following:

| Teensy board | `micro_ros_arduino` Humble status |
| --- | --- |
| 3.1 / 3.2 | Supported |
| 3.5 | Not tested |
| 3.6 | Supported |
| 4.0 | Not tested |
| 4.1 | Supported |

Those statuses describe micro-ROS's board support, not a guarantee that every servo library, pin assignment, or project configuration works unchanged. The included example is configured for Teensy 4.1. Select PWM-capable pins and confirm the chosen `PWMServo` version and board toolchain for your target.

## Install

### 1. Install the dependencies

This library declares `PWMServo` and `micro_ros_arduino` in `library.properties`. Arduino tooling that supports dependency resolution may offer to install declared dependencies. The correct Humble micro-ROS build and Teensyduino setup can require extra steps, so follow the [Humble `micro_ros_arduino` instructions](https://github.com/micro-ROS/micro_ros_arduino/tree/humble), including its Teensyduino patch instructions when required by your release.

### 2. Install this library

In Arduino IDE, choose **Sketch → Include Library → Add .ZIP Library…** and select a ZIP of this repository. You can also place or clone the repository into your Arduino `libraries` directory.

### 3. Open the example

Open [`examples/TeensyServoExample/TeensyServoExample.ino`](examples/TeensyServoExample/TeensyServoExample.ino), select your Teensy board and port, then build and upload. Start a micro-ROS Agent on the host using the transport configured in the sketch. For USB serial, for example:

```bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyACM0 -v6
```

Replace `/dev/ttyACM0` with the Teensy's serial device. The ROS 2 installation on the host must include the matching Humble micro-ROS Agent.

## Configure your project

Copy the example and replace its [`ServoConfig.h`](examples/TeensyServoExample/ServoConfig.h) with your servo definitions. Keep the servo definitions and ROS joint names in the same order.

```cpp
static const ServoDefinition MY_SERVOS[] = {
  // name, pin, angle min/max, pulse min/max, reverse, speed deg/s, startup angle
  {"base", 2, -90.0f, 90.0f, 1000, 2000, false, 45.0f, 0.0f},
  {"shoulder", 3, -45.0f, 45.0f, 1100, 1900, true, 30.0f, 0.0f},
};
```

Create the controllers with that configuration and a matching array of ROS joint names:

```cpp
static const char *const JOINT_NAMES[] = {"base_joint", "shoulder_joint"};
static const uint8_t SERVO_COUNT = sizeof(MY_SERVOS) / sizeof(MY_SERVOS[0]);

ServoController servoController(MY_SERVOS, SERVO_COUNT);
const MicroROSControllerConfig rosConfig = {
  "my_robot_teensy",       // node name
  "/my_robot/joint_commands",
  "/my_robot/joint_states",
  JOINT_NAMES,
  42,                       // ROS domain ID
  1000,                     // command timeout in ms; 0 disables timeout hold
  50                        // state publish period in ms; 0 disables publishing
};
MicroROSController microRosController(servoController, rosConfig);
```

In `setup()`, call `servoController.begin()` and `microRosController.begin()`. In `loop()`, call `microRosController.spinOnce()` and `servoController.update()` regularly. See the complete [Teensy servo example](examples/TeensyServoExample/TeensyServoExample.ino).

By default, `begin()` configures the micro-ROS Arduino serial transport. To use another transport available in your selected micro-ROS build, configure that transport first and call `microRosController.begin(false)`.

## ROS interface

| Direction | Topic | Message | Meaning |
| --- | --- | --- | --- |
| ROS 2 → Teensy | `/teensy/joint_commands` | `std_msgs/msg/Float32MultiArray` | Target angles in radians, one per servo in configured order |
| Teensy → ROS 2 | `/teensy/joint_states` | `sensor_msgs/msg/JointState` | Joint names and estimated angles in radians, in configured order |

The topic names above are example defaults; configure your own names in `MicroROSControllerConfig`. A command array must contain exactly one finite value per servo. Targets are clamped to each servo's configured angle limits.

**State positions are estimates based on commanded movement, not encoder measurements.** This library drives hobby-servo PWM outputs and does not measure physical joint position.

## Calibration and safe startup

The example values are starting points, not verified calibration for your robot. Before connecting mechanisms, verify servo power, wiring, pin assignments, pulse endpoints, direction, mechanical travel, speed, and startup pose. Start with a small range and test each servo individually. Software limits do not replace mechanical checks or an emergency stop.

## More information

- [Installation and configuration guide](docs/GettingStarted.md)
- [Trajectory control guide](docs/ros2_control_trajectory.md)
- [`micro_ros_arduino` Humble branch](https://github.com/micro-ROS/micro_ros_arduino/tree/humble)
- [micro-ROS Agent](https://github.com/micro-ROS/micro-ROS-Agent)
