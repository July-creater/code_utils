#pragma once

#include <algorithm>
#include <cstdint>
#include <execution>
#include <filesystem>
#include <fstream>
#include <functional>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

#include "Eigen/Core"
#include "pcl/point_cloud.h"
#include "pcl/point_types.h"

namespace Utils {

inline Eigen::Array3i FastFloor(const Eigen::Array3d& pt) {
  const Eigen::Array3i ncoord = pt.cast<int>();
  return ncoord - (pt < ncoord.cast<double>()).cast<int>();
};

// vg-icp
struct VgIcpHash {
 public:
  inline int64_t operator()(const Eigen::Vector3i& x) const {
    const int64_t p1 = 73856093;
    const int64_t p2 = 19349669;  // 19349663 was not a prime number
    const int64_t p3 = 83492791;
    return static_cast<int64_t>((x[0] * p1) ^ (x[1] * p2) ^ (x[2] * p3));
  }

  static int64_t Hash(const Eigen::Vector3i& x) { return VgIcpHash()(x); }
  static bool Equal(const Eigen::Vector3i& x1, const Eigen::Vector3i& x2) { return x1 == x2; }
};

// zelos
struct ZelosHash {
  inline int64_t operator()(const Eigen::Vector3i& key) const {
    return ((key.x() * 73856093) ^ (key.y() * 471943) ^ (key.z() * 83492791)) & ((1 << 20) - 1);
  }
};

// kiss-icp
struct KissIcpHash {
  inline int64_t operator()(const Eigen::Vector3i& voxel) const {
    const uint32_t* vec = reinterpret_cast<const uint32_t*>(voxel.data());
    return ((1 << 20) - 1) & (vec[0] * 73856093 ^ vec[1] * 19349669 ^ vec[2] * 83492791);
  }
};

inline double ComputeDistance(const Eigen::Vector3d& p1, const Eigen::Vector3d& p2) {
  return (p1 - p2).norm();
}

// TODO，后期将对这个函数封装到一个类中用作pcl的接口, 此外可以指定并行线程的数量，以及使用omp等效率
inline std::vector<Eigen::Vector3d> PclToEigen3d(
    const pcl::PointCloud<pcl::PointXYZ>::Ptr& input_cloud) {
  if (!input_cloud || input_cloud->empty()) {
    throw std::invalid_argument("Input cloud is empty!");
  }
  std::vector<Eigen::Vector3d> out_points(input_cloud->points.size());  // 预分配内存
  std::transform(std::execution::par, input_cloud->points.begin(), input_cloud->points.end(),
                 out_points.begin(), [](const pcl::PointXYZ& point) -> Eigen::Vector3d {
                   return point.getArray3fMap().cast<double>();
                 });

  return out_points;
}

static void GenerateRandomCoefficientsAndData(
    const std::function<double(const Eigen::VectorXd&, double)>& func, const int32_t param_count,
    const int32_t data_size, const std::pair<double, double>& param_range,
    const std::pair<double, double>& noise_range, const std::string log_path,
    Eigen::VectorXd* const parameters, std::vector<std::pair<double, double>>* const data) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> gen_param(param_range.first, param_range.second);
  std::uniform_real_distribution<> gen_noise(noise_range.first, noise_range.second);

  // Generate random parameters
  parameters->resize(param_count);
  for (int32_t i = 0; i < param_count; ++i) {
    (*parameters)(i) = gen_param(gen);
  }

  if (std::filesystem::exists(log_path)) {
    std::filesystem::remove(log_path);
  }

  std::fstream log;
  if (!log_path.empty()) {
    log.open(log_path, std::ios::app);
  }

  // Generate data with noise
  data->clear();
  data->resize(data_size);
  for (int32_t i = 0; i < data_size; ++i) {
    double x = static_cast<double>(i) / data_size;
    double y = func(*parameters, x) + gen_noise(gen);

    data->at(i) = {x, y};
    if (!log_path.empty()) {
      log << x << " " << y << "\n";
    }
  }

  LOG(INFO) << "Generated function with parameters: ";

  for (int32_t i = 0; i < param_count; ++i) {
    LOG(INFO) << (*parameters)(i);
  }
}

}  // namespace Utils