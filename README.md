# Teensy micro-ROS Servo Controller

An Arduino library for controlling calibrated hobby servos from ROS 2 through
micro-ROS on Teensy boards.

The library provides:

- Per-servo pin, angle, pulse-width, direction, speed, and startup settings
- Smooth movement toward commanded targets
- ROS 2 command and joint-state topics
- Fixed-capacity storage for up to 32 servos
- Automatic micro-ROS entity recreation after a connection loss

This project is a small foundation, and there is plenty of room to improve it.
If it is useful to you, contributions, feedback, and ideas from the robotics
community are very welcome. Together, we can make it more capable and useful
for a wider range of robots and applications.

> **Important:** Published joint positions are estimated from commanded servo
> motion. This library does not provide encoder feedback.

## Requirements

- Arduino IDE with Teensy board support installed
- A Teensy 3.x or 4.x board supported by the selected
  [`micro_ros_arduino` Humble build](https://github.com/micro-ROS/micro_ros_arduino/tree/humble)
- [`PWMServo`](https://github.com/PaulStoffregen/PWMServo)
- `micro_ros_arduino`
- A ROS 2 installation with `micro_ros_agent`

Follow the [`micro_ros_arduino` Humble
instructions](https://github.com/micro-ROS/micro_ros_arduino/tree/humble) for
the micro-ROS library and any required Teensyduino setup.

## Installation

1. Install `PWMServo` and `micro_ros_arduino` using the Arduino IDE or the
   installation instructions above.
2. Download or clone this repository into your Arduino `libraries` directory.
3. Restart the Arduino IDE.
4. Open
   [`examples/TeensyServoExample/TeensyServoExample.ino`](examples/TeensyServoExample/TeensyServoExample.ino).
5. Select your Teensy board and port.
6. Configure the servos as described below, then compile and upload.

## Configure the servos

Servo definitions are in
[`examples/TeensyServoExample/ServoConfig.h`](examples/TeensyServoExample/ServoConfig.h).
Create one `ServoDefinition` for each servo:

```cpp
static const ServoDefinition SERVO_DEFINITIONS[] = {
  // name, pin, min angle, max angle, min pulse, max pulse,
  // reverse, speed (deg/s), startup angle
  {"base",     2, -90.0f, 90.0f, 1000, 2000, false, 30.0f, 0.0f},
  {"shoulder", 3, -45.0f, 45.0f, 1100, 1900, true,  20.0f, 0.0f},
};

static const uint8_t SERVO_COUNT =
    sizeof(SERVO_DEFINITIONS) / sizeof(SERVO_DEFINITIONS[0]);

static const char *const JOINT_NAMES[SERVO_COUNT] = {
  "base_joint",
  "shoulder_joint"
};
```

Configure each field as follows:

| Field | Description | Unit |
| --- | --- | --- |
| `name` | Internal servo name | — |
| `pin` | Teensy PWM-capable output pin | — |
| `minAngleDeg`, `maxAngleDeg` | Allowed logical angle range | degrees |
| `minPwmUs`, `maxPwmUs` | Pulse-width range for the servo | microseconds |
| `reverse` | Reverses movement within the configured range | `true`/`false` |
| `speedDegPerSec` | Maximum movement speed | degrees/second |
| `startupAngleDeg` | Initial commanded angle | degrees |

The order of `SERVO_DEFINITIONS` must match the order of `JOINT_NAMES`.
The controller accepts no more than 32 servos. Verify the servo datasheet,
wiring, power supply, pulse limits, direction, and mechanical range before
connecting a mechanism.

## Configure micro-ROS

The example creates the controllers with this configuration:

```cpp
ServoController servoController(SERVO_DEFINITIONS, SERVO_COUNT);

const MicroROSControllerConfig rosConfig = {
  "teensy_servo_controller",  // node name
  "/teensy/joint_commands",   // command topic
  "/teensy/joint_states",     // state topic
  JOINT_NAMES,
  42,                         // ROS domain ID
  1000,                       // command timeout in milliseconds
  50                          // state publish period in milliseconds
};

MicroROSController microRosController(servoController, rosConfig);
```

`commandTimeoutMs` holds the current servo targets when no command has been
received for the configured duration. Set it to `0` to disable this behavior.
Set `statePublishPeriodMs` to `0` to disable state publishing.

The required lifecycle calls are:

```cpp
void setup() {
  servoController.begin();
  microRosController.begin();
}

void loop() {
  microRosController.spinOnce();
  servoController.update();
  delay(2);
}
```

The default `begin()` call configures the serial micro-ROS transport. To use
another transport supported by your micro-ROS build, configure that transport
first and call `microRosController.begin(false)`.

## ROS interface

| Direction | Topic | Message | Data |
| --- | --- | --- | --- |
| ROS 2 -> Teensy | `/teensy/joint_commands` | `std_msgs/msg/Float32MultiArray` | Target angles in radians |
| Teensy -> ROS 2 | `/teensy/joint_states` | `sensor_msgs/msg/JointState` | Joint names and estimated positions in radians |

Command arrays must contain exactly one finite value for every configured
servo, in servo-definition order. Values outside a servo's configured angle
range are clamped.

For example, with four configured servos:

```bash
ros2 topic pub --once /teensy/joint_commands \
  std_msgs/msg/Float32MultiArray \
  "{data: [0.0, 0.5, -0.5, 1.0]}"
```

Inspect the published state with:

```bash
ros2 topic echo /teensy/joint_states
```

If you change the topic names in `MicroROSControllerConfig`, use those names
in your ROS 2 commands.

## Start the micro-ROS Agent

After uploading the sketch, connect the Teensy and start the agent on the ROS 2
machine:

```bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyACM0 -v6
```

Replace `/dev/ttyACM0` with the Teensy's serial device. The node uses ROS domain
ID `42` in the example, so the ROS 2 environment and agent-side application
must use the same domain.
