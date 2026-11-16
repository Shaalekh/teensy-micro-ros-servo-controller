# Sending a timed trajectory

This guide shows how to send a joint-space trajectory through a ROS 2
`joint_trajectory_controller` connected to the Teensy controller.

## Prerequisites

- The ROS 2 workspace and this library's dependencies are built and sourced.
- The Teensy firmware has been uploaded and configured for the installed servos.
- A micro-ROS agent is connected to the Teensy:

  ```bash
  ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyACM0 -v6
  ```

- Servo power, wiring, mechanical travel, and an emergency stop have been
  checked.

## Start the control stack

Start the `ros2_control` hardware and trajectory controller using the launch
file provided by your application. Check that the controllers are active and
that a trajectory action is available:

```bash
ros2 control list_controllers
ros2 control list_hardware_interfaces
ros2 action list | grep follow_joint_trajectory
```

Use the action name reported by the last command in the trajectory request.

## Check current positions

Before constructing a goal, inspect the state topic configured in the Teensy
firmware:

```bash
ros2 topic echo --once /teensy/joint_states
```

Use the `name` and `position` arrays together. These positions are estimates
tracked from commanded servo motion, not encoder measurements. Check the actual
mechanism before moving it.

## Send a timed trajectory

Replace the action name, joint names, and positions with the configuration used
by your application:

```bash
ros2 action send_goal --feedback \
  /teensy_trajectory_controller/follow_joint_trajectory \
  control_msgs/action/FollowJointTrajectory \
  "{trajectory: {
    joint_names: [servo_0, servo_1],
    points: [{
      positions: [0.0, 0.1],
      time_from_start: {sec: 3, nanosec: 0}
    }]
  }}"
```

The `joint_names` and `positions` arrays must have the same length and order.
Every required joint must be included when the controller does not allow
partial goals. `time_from_start` specifies when the waypoint should be
reached. A successful action result means the controller completed the
requested command; it does not verify physical motion.

Servo travel limits and direction are configured in the Teensy firmware.
Calibrate those values for the mechanism before hardware motion, and do not
rely on software limits as a replacement for mechanical checks.
