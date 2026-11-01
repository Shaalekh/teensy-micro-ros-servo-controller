#ifndef INMOOV_SERVO_CONFIG_H
#define INMOOV_SERVO_CONFIG_H

#include <ServoDefinitions.h>

// Starter calibrations: verify wiring, mechanical travel, pulse endpoints,
// direction, and startup pose for every installed servo before use.
static const ServoDefinition INMOOV_SERVOS[] = {
  {"left_thumb", 0, -90.0f, 0.0f, 1500, 2500, false, 30.0f, 0.0f},
  {"left_index", 1, 0.0f, 90.0f, 1500, 2500, true, 30.0f, 0.0f},
  {"left_middle", 2, 0.0f, 90.0f, 1650, 2500, true, 30.0f, 0.0f},
  {"left_ring", 3, 0.0f, 90.0f, 1500, 2500, true, 30.0f, 0.0f},
  {"left_pinky", 4, 0.0f, 90.0f, 1700, 2500, true, 30.0f, 0.0f},
  {"right_thumb", 23, 0.0f, 90.0f, 1500, 2500, true, 30.0f, 0.0f},
  {"right_index", 22, 0.0f, 90.0f, 1650, 2500, true, 30.0f, 0.0f},
  {"right_middle", 19, 0.0f, 90.0f, 1450, 2500, true, 30.0f, 0.0f},
  {"right_ring", 18, -90.0f, 0.0f, 1500, 2500, false, 30.0f, 0.0f},
  {"right_pinky", 15, -90.0f, 0.0f, 1500, 2500, false, 30.0f, 0.0f},
  {"left_wrist", 5, -45.0f, 45.0f, 1600, 2500, false, 30.0f, 0.0f},
  {"right_wrist", 14, -45.0f, 45.0f, 800, 1800, false, 30.0f, 0.0f},
  {"left_bicep_flex", 24, 0.0f, 90.0f, 1500, 2500, false, 30.0f, 45.0f},
  {"left_shoulder_lift", 25, 0.0f, 90.0f, 1500, 2500, false, 30.0f, 45.0f},
  {"left_shoulder_side", 28, 0.0f, 90.0f, 1500, 2500, false, 30.0f, 45.0f},
  {"left_bicep_rotation", 29, 0.0f, 90.0f, 1500, 2500, false, 30.0f, 45.0f},
  {"right_bicep_flex", 13, 0.0f, 90.0f, 1500, 2500, false, 30.0f, 45.0f},
  {"right_shoulder_lift", 37, 0.0f, 90.0f, 1500, 2500, false, 30.0f, 45.0f},
  {"right_shoulder_side", 36, 0.0f, 90.0f, 1500, 2500, false, 30.0f, 45.0f},
  {"right_bicep_rotation", 33, 0.0f, 90.0f, 1500, 2500, false, 30.0f, 45.0f},
};

static const uint8_t INMOOV_SERVO_COUNT =
    sizeof(INMOOV_SERVOS) / sizeof(INMOOV_SERVOS[0]);

static const char *const INMOOV_JOINT_NAMES[INMOOV_SERVO_COUNT] = {
  "l_thumb1_joint", "l_index1_joint", "l_middle1_joint", "l_ring1_joint",
  "l_pinky1_joint", "r_thumb1_joint", "r_index1_joint", "r_middle1_joint",
  "r_ring1_joint", "r_pinky1_joint", "l_wrist_roll_joint", "r_wrist_roll_joint",
  "l_elbow_flex_joint", "l_shoulder_lift_joint", "l_shoulder_out_joint",
  "l_upper_arm_roll_joint", "r_elbow_flex_joint", "r_shoulder_lift_joint",
  "r_shoulder_out_joint", "r_upper_arm_roll_joint"
};

#endif
