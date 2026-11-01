#include "MicroROSController.h"
#include <math.h>
#include <string.h>

namespace {
const uint32_t RECONNECT_RETRY_MS = 2000;

void finalizeSubscription(rcl_subscription_t *subscription, rcl_node_t *node) {
  const rcl_ret_t result = rcl_subscription_fini(subscription, node);
  (void)result;
}
void finalizePublisher(rcl_publisher_t *publisher, rcl_node_t *node) {
  const rcl_ret_t result = rcl_publisher_fini(publisher, node);
  (void)result;
}
void finalizeNode(rcl_node_t *node) {
  const rcl_ret_t result = rcl_node_fini(node);
  (void)result;
}
void finalizeInitOptions(rcl_init_options_t *options) {
  const rcl_ret_t result = rcl_init_options_fini(options);
  (void)result;
}
}

MicroROSController *MicroROSController::activeInstance_ = 0;

MicroROSController::MicroROSController(
    ServoController &servoController, const MicroROSControllerConfig &config)
    : servoController_(servoController), config_(config), entitiesReady_(false),
      supportReady_(false), nodeReady_(false), subscriptionReady_(false),
      publisherReady_(false), executorReady_(false), lastCommandMs_(0),
      lastStatePublishMs_(0), lastReconnectAttemptMs_(0) {
  allocator_ = rcl_get_default_allocator();
  support_ = rclc_support_t{};
  node_ = rcl_get_zero_initialized_node();
  subscription_ = rcl_get_zero_initialized_subscription();
  statePublisher_ = rcl_get_zero_initialized_publisher();
  executor_ = rclc_executor_get_zero_initialized_executor();
  std_msgs__msg__Float32MultiArray__init(&commandMessage_);
  commandMessage_.data.data = commandData_;
  commandMessage_.data.capacity = servoController_.count();
  commandMessage_.data.size = 0;
  sensor_msgs__msg__JointState__init(&stateMessage_);
  stateMessage_.name.data = stateNameData_;
  stateMessage_.name.capacity = servoController_.count();
  stateMessage_.name.size = servoController_.count();
  stateMessage_.position.data = statePositionData_;
  stateMessage_.position.capacity = servoController_.count();
  stateMessage_.position.size = servoController_.count();
  for (uint8_t i = 0; i < servoController_.count(); ++i) {
    const char *name = config_.jointNames == 0 ? 0 : config_.jointNames[i];
    stateNameData_[i].data = const_cast<char *>(name == 0 ? "" : name);
    stateNameData_[i].size = strlen(name == 0 ? "" : name);
    stateNameData_[i].capacity = stateNameData_[i].size + 1;
    statePositionData_[i] = 0.0;
  }
}

void MicroROSController::commandCallback(const void *msgIn) {
  if (activeInstance_ == 0 || msgIn == 0) return;
  const std_msgs__msg__Float32MultiArray *msg =
      static_cast<const std_msgs__msg__Float32MultiArray *>(msgIn);
  if (msg->data.size != activeInstance_->servoController_.count() ||
      msg->data.data == 0) return;
  for (uint8_t i = 0; i < msg->data.size; ++i) {
    if (!isfinite(msg->data.data[i])) return;
  }
  activeInstance_->servoController_.setTargetsRadians(
      msg->data.data, (uint8_t)msg->data.size);
  activeInstance_->lastCommandMs_ = millis();
}

bool MicroROSController::createEntities() {
  rcl_init_options_t initOptions = rcl_get_zero_initialized_init_options();
  if (rcl_init_options_init(&initOptions, allocator_) != RCL_RET_OK) return false;
  if (rcl_init_options_set_domain_id(&initOptions, config_.domainId) != RCL_RET_OK) {
    finalizeInitOptions(&initOptions);
    return false;
  }
  const rcl_ret_t supportResult =
      rclc_support_init_with_options(&support_, 0, 0, &initOptions, &allocator_);
  finalizeInitOptions(&initOptions);
  if (supportResult != RCL_RET_OK) return false;
  supportReady_ = true;
  if (rclc_node_init_default(&node_, config_.nodeName, "", &support_) != RCL_RET_OK)
    return false;
  nodeReady_ = true;
  if (rclc_publisher_init_default(
          &statePublisher_, &node_,
          ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
          config_.stateTopic) != RCL_RET_OK) return false;
  publisherReady_ = true;
  if (rclc_subscription_init_default(
          &subscription_, &node_,
          ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32MultiArray),
          config_.commandTopic) != RCL_RET_OK) return false;
  subscriptionReady_ = true;
  if (rclc_executor_init(&executor_, &support_.context, 1, &allocator_) != RCL_RET_OK)
    return false;
  executorReady_ = true;
  if (rclc_executor_add_subscription(
          &executor_, &subscription_, &commandMessage_,
          &MicroROSController::commandCallback, ON_NEW_DATA) != RCL_RET_OK) return false;
  return true;
}

void MicroROSController::destroyEntities() {
  if (executorReady_) {
    const rcl_ret_t result = rclc_executor_fini(&executor_);
    (void)result;
    executorReady_ = false;
  }
  if (subscriptionReady_) {
    finalizeSubscription(&subscription_, &node_);
    subscriptionReady_ = false;
  }
  if (publisherReady_) {
    finalizePublisher(&statePublisher_, &node_);
    publisherReady_ = false;
  }
  if (nodeReady_) {
    finalizeNode(&node_);
    nodeReady_ = false;
  }
  if (supportReady_) {
    const rcl_ret_t result = rclc_support_fini(&support_);
    (void)result;
    supportReady_ = false;
  }
}

bool MicroROSController::begin(bool configureSerialTransport) {
  if (servoController_.count() == 0 || config_.nodeName == 0 ||
      config_.commandTopic == 0 || config_.stateTopic == 0) return false;
  if (configureSerialTransport) set_microros_transports();
  activeInstance_ = this;
  entitiesReady_ = createEntities();
  if (!entitiesReady_) destroyEntities();
  lastCommandMs_ = millis();
  lastStatePublishMs_ = millis();
  lastReconnectAttemptMs_ = millis();
  return entitiesReady_;
}

void MicroROSController::spinOnce() {
  if (!entitiesReady_) {
    const uint32_t now = millis();
    if ((uint32_t)(now - lastReconnectAttemptMs_) >= RECONNECT_RETRY_MS) {
      lastReconnectAttemptMs_ = now;
      entitiesReady_ = createEntities();
      if (!entitiesReady_) destroyEntities();
    }
    return;
  }
  const rcl_ret_t rc = rclc_executor_spin_some(&executor_, RCL_MS_TO_NS(5));
  if (rc != RCL_RET_OK && rc != RCL_RET_TIMEOUT) {
    destroyEntities();
    entitiesReady_ = false;
    return;
  }
  const uint32_t now = millis();
  if (config_.commandTimeoutMs > 0 &&
      (uint32_t)(now - lastCommandMs_) > config_.commandTimeoutMs) {
    float hold[SERVO_CONTROLLER_MAX_SERVOS];
    for (uint8_t i = 0; i < servoController_.count(); ++i)
      hold[i] = servoController_.currentAngleRadians(i);
    servoController_.setTargetsRadians(hold, servoController_.count());
  }
  if (config_.statePublishPeriodMs > 0 &&
      (uint32_t)(now - lastStatePublishMs_) >= config_.statePublishPeriodMs) {
    lastStatePublishMs_ = now;
    for (uint8_t i = 0; i < servoController_.count(); ++i)
      statePositionData_[i] = servoController_.currentAngleRadians(i);
    const rcl_ret_t publishResult = rcl_publish(&statePublisher_, &stateMessage_, 0);
    (void)publishResult;
  }
}
