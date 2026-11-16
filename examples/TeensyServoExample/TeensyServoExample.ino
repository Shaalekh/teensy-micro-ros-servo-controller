#include <ServoController.h>
#include <MicroROSController.h>
#include "ServoConfig.h"

ServoController servoController(SERVO_DEFINITIONS, SERVO_COUNT);

const MicroROSControllerConfig rosConfig = {
  "teensy_servo_controller",
  "/teensy/joint_commands",
  "/teensy/joint_states",
  JOINT_NAMES,
  42,    // ROS_DOMAIN_ID
  1000,  // command timeout in ms; zero disables timeout hold
  50     // joint-state publish period in ms; zero disables publishing
};

MicroROSController microRosController(servoController, rosConfig);

void setup() {
  servoController.begin();
  microRosController.begin();
}

void loop() {
  microRosController.spinOnce();
  servoController.update();
  delay(2);
}
