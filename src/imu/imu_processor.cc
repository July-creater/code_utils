#include "imu/imu_processor.h"

namespace IMU {

ImuProcessor::ImuProcessor(const Utils::ImuConfig& config) {
  SetConfig(config);
  Initialize(config);
}

void ImuProcessor::SetConfig(const Utils::ImuConfig& config) {
  // 配置 Logger
  logger_.EnableConsoleLog(config.logger_config_.enable_console_log_);  // 启用控制台日志
  logger_.SetConsoleLevel("ALL");  // 设置控制台日志级别
  logger_.Log(INFO) << "Initializing IMU sensor.";
}

void ImuProcessor::Initialize(const Utils::ImuConfig& config) {
  pre_integrator_ = std::make_shared<IMUPreIntegration>(config.pre_integration_config_);
}

// TODO
bool ImuProcessor::ProcessImu() { return true; }

bool ImuProcessor::ReadData(const std::string& file_path, std::vector<IMUData>* const data_vec) {
  if (file_path.empty()) {
    logger_.Log(ERROR) << "File path is empty.";
    return false;
  }
  data_vec->clear();
  std::ifstream file(file_path);
  std::string line;
  // time gx gy gz ax ay az
  if (file.is_open()) {
    logger_.Log(INFO) << "Successfully opened file at: " << file_path;
    uint64_t count = 0;
    while (getline(file, line)) {
      count++;
      std::istringstream iss(line);
      double timestamp, ax, ay, az, gx, gy, gz;
      if (iss >> timestamp >> gx >> gy >> gz >> ax >> ay >> az) {
        Eigen::Vector3d acc(ax, ay, az);
        Eigen::Vector3d gyr(gx, gy, gz);
        data_vec->emplace_back(IMUData(timestamp / 1.0e9, acc, gyr));
      }
      if (count % 10000 == 0) {
        logger_.Log(INFO) << "Processed " + std::to_string(count) + " data entries...";
      }
    }
    file.close();
    logger_.Log(INFO) << "Finished reading total of " << data_vec->size()
                      << " data entries from IMU sensor.";
  } else {
    logger_.Log(ERROR) << "Unable to open file at " << file_path;
  }
  logger_.Log(INFO) << "Completed data reading from IMU sensor.";
  return true;
}
}  // namespace IMU