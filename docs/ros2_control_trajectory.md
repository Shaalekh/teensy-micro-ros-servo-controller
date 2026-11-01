# Sending a timed trajectory to the InMoov

This guide shows how to send a joint-space trajectory through the workspace's existing `ros2_control` setup. The active controller is `inmoov_trajectory_controller`, a position `joint_trajectory_controller` controlling the 20 joints connected to the Teensy.

## Prerequisites

- The workspace has been built and sourced in each terminal:

  ```bash
  source /opt/ros/humble/setup.bash
  source /home/ros2_inmoov_ws/inmoov_ws/install/setup.bash
  ```

- The Teensy firmware has been uploaded and configured for the installed servos.
- A micro-ROS agent is connected to the Teensy. For example, replace `/dev/ttyACM0` with the actual serial device:

  ```bash
  docker run -it --rm -v /dev:/dev --privileged --net=host \
    microros/micro-ros-agent:humble serial --dev /dev/ttyACM0 -v6
  ```

- The servo power, wiring, mechanical travel, and emergency stop have been checked. The checked-in firmware servo values are starter values and need calibration for the actual build.

## Start the control stack

In another sourced terminal:

```bash
ros2 launch inmoov_control control.launch.py
```

Check that the controllers are active and that the trajectory action is available:

```bash
ros2 control list_controllers
ros2 control list_hardware_interfaces
ros2 action list | grep follow_joint_trajectory
```

The expected action is:

```text
/inmoov_trajectory_controller/follow_joint_trajectory
```

The launch starts the controller manager, the joint state broadcaster, the trajectory controller, the Teensy command bridge, and RViz. The robot description routes the `ros2_control` position commands through `topic_based_ros2_control` to the bridge, which translates named joints into the Teensy's fixed channel order.

## Check current positions

Before constructing the goal, inspect the Teensy's reported state:

```bash
ros2 topic echo --once /inmoov/joint_states
```

Use the `name` and `position` arrays together: the value at each position index belongs to the joint name at the same index. These values are in radians. Use the reported current values for joints you want to hold, then change only the target joint or joints.

**These are not encoder measurements.** The Teensy has no encoders; it reports positions tracked from commanded servo motion. Check the actual robot before moving it.

## Send a timed trajectory

The controller is configured with `allow_partial_joints_goal: false`, so every goal must include all 20 controlled joints. The following example sends one waypoint: it asks the left wrist to move to `0.05` radians, reaching that point 3 seconds after the trajectory starts.

The other values below are the project's nominal startup positions (`0.785398` radians for the first 10 finger joints, zero for the remaining joints). **Do not use those values unchanged if they do not match the current robot state.** Replace them with the values reported by `/inmoov/joint_states` for a hold-position move, and choose a small, mechanically safe target for the joint being tested.

```bash
ros2 action send_goal --feedback \
  /inmoov_trajectory_controller/follow_joint_trajectory \
  control_msgs/action/FollowJointTrajectory \
  "{trajectory: {
    joint_names: [
      l_thumb1_joint, l_index1_joint, l_middle1_joint, l_ring1_joint, l_pinky1_joint,
      r_thumb1_joint, r_index1_joint, r_middle1_joint, r_ring1_joint, r_pinky1_joint,
      l_wrist_roll_joint, r_wrist_roll_joint,
      l_elbow_flex_joint, l_shoulder_lift_joint, l_shoulder_out_joint, l_upper_arm_roll_joint,
      r_elbow_flex_joint, r_shoulder_lift_joint, r_shoulder_out_joint, r_upper_arm_roll_joint
    ],
    points: [{
      positions: [
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0,
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0
      ],
      time_from_start: {sec: 3, nanosec: 0}
    }]
  }}"
```

The `joint_names` and `positions` arrays must have the same length and order. `time_from_start` is the time at which that waypoint should be reached, measured from the beginning of the trajectory. Add more entries under `points` to specify more waypoints; each entry needs a full 20-joint position array for this controller configuration.

The command prints feedback and the final action result. A successful result means the controller accepted and completed the trajectory; it does not verify that the joints physically reached their targets. You can inspect the reported state again with:

```bash
ros2 topic echo --once /inmoov/joint_states
```

## What this controller can move

The current controller has 20 Teensy joints: five proximal finger joints per hand, two wrists, and four arm joints per side. The other joints in the 53-joint model use mock hardware for visualization and are not moved by this Teensy trajectory controller. Finger segments beyond the first joint are also not independently actuated in this mapping.

The hardware uses position commands only. Servo travel limits and direction are configured in the Teensy firmware and the URDF; calibrate both together before hardware motion. The Teensy also clamps commanded targets, but those software clamps do not replace mechanical checks.

## Relevant files

- `src/inmoov_control/config/controllers.yaml` — controller type, controlled-joint list, and partial-goal setting.
- `src/inmoov_control/launch/control.launch.py` — control stack launch description.
- `src/inmoov_control/inmoov_control/teensy_command_bridge.py` — name-to-firmware-order mapping.
- `src/inmoov_description/robots/inmoov.urdf.xacro` — `ros2_control` hardware and joint interfaces.
- `src/InMoovTeensy-ArduinoIDE/InMoovTeensy/ServoDefinitions.h` — firmware servo order, limits, and nominal startup angles.
