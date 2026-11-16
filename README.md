# Teensy micro-ROS Servo Controller

Arduino library for controlling calibrated hobby servos from ROS 2 through
micro-ROS on Teensy boards.

## Requirements

- Teensy 3.x or 4.x supported by the selected
  [`micro_ros_arduino` Humble build](https://github.com/micro-ROS/micro_ros_arduino/tree/humble)
- [`PWMServo`](https://github.com/PaulStoffregen/PWMServo)
- `micro_ros_arduino`

Install the dependencies and this repository in the Arduino `libraries`
directory, then open
[`examples/TeensyServoExample/TeensyServoExample.ino`](examples/TeensyServoExample/TeensyServoExample.ino).
Select the Teensy board, configure the servos in
[`ServoConfig.h`](examples/TeensyServoExample/ServoConfig.h), and upload.

Start the micro-ROS Agent on the host:

```bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyACM0 -v6
```

Replace `/dev/ttyACM0` with the Teensy's serial device. For the required
Teensyduino and micro-ROS setup, follow the
[`micro_ros_arduino` Humble instructions](https://github.com/micro-ROS/micro_ros_arduino/tree/humble).

## ROS interface

The example uses:

| Direction | Topic | Message |
| --- | --- | --- |
| ROS 2 -> Teensy | `/teensy/joint_commands` | `std_msgs/msg/Float32MultiArray` |
| Teensy -> ROS 2 | `/teensy/joint_states` | `sensor_msgs/msg/JointState` |

Commands and state positions are in radians and follow the configured servo
order. State positions are estimates from commanded motion; this library does
not read encoder feedback.
