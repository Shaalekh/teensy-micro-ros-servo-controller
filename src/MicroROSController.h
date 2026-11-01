#ifndef MICROROS_SERVO_MICROROS_CONTROLLER_H
#define MICROROS_SERVO_MICROROS_CONTROLLER_H

#include <Arduino.h>
#include <micro_ros_arduino.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/float32_multi_array.h>
#include <sensor_msgs/msg/joint_state.h>
#include "ServoController.h"

struct MicroROSControllerConfig {
  const char *nodeName;
  const char *commandTopic;
  const char *stateTopic;
  const char *const *jointNames;
  size_t domainId;
  uint32_t commandTimeoutMs;
  uint32_t statePublishPeriodMs;
};

class MicroROSController {
 public:
  MicroROSController(ServoController &servoController,
                     const MicroROSControllerConfig &config);
  // Set configureSerialTransport=false after configuring another transport.
  bool begin(bool configureSerialTransport = true);
  void spinOnce();

 private:
  static void commandCallback(const void *msgIn);
  bool createEntities();
  void destroyEntities();

  ServoController &servoController_;
  MicroROSControllerConfig config_;
  rcl_allocator_t allocator_;
  rclc_support_t support_;
  rcl_node_t node_;
  rcl_subscription_t subscription_;
  rcl_publisher_t statePublisher_;
  rclc_executor_t executor_;
  std_msgs__msg__Float32MultiArray commandMessage_;
  sensor_msgs__msg__JointState stateMessage_;
  float commandData_[SERVO_CONTROLLER_MAX_SERVOS];
  double statePositionData_[SERVO_CONTROLLER_MAX_SERVOS];
  rosidl_runtime_c__String stateNameData_[SERVO_CONTROLLER_MAX_SERVOS];
  bool entitiesReady_;
  bool supportReady_;
  bool nodeReady_;
  bool subscriptionReady_;
  bool publisherReady_;
  bool executorReady_;
  uint32_t lastCommandMs_;
  uint32_t lastStatePublishMs_;
  uint32_t lastReconnectAttemptMs_;

  static MicroROSController *activeInstance_;
};

#endif
