#include "driver_nodelet.hpp"

#include <pluginlib/class_list_macros.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

PLUGINLIB_EXPORT_CLASS(cepton_ros::DriverNodelet, nodelet::Nodelet);

namespace cepton_ros {

DriverNodelet::~DriverNodelet() { cepton_sdk_deinitialize(); }

const std::map<std::string, cepton_sdk::FrameMode> frame_mode_lut = {
    {"COVER", CEPTON_SDK_FRAME_COVER},
    {"CYCLE", CEPTON_SDK_FRAME_CYCLE},
    {"STREAMING", CEPTON_SDK_FRAME_TIMED},
};

void DriverNodelet::onInit() {
  node_handle = getNodeHandle();
  private_node_handle = getPrivateNodeHandle();

  // Get parameters
  std::string multi_ip;
  private_node_handle.param("multi_ip", multi_ip, std::string("")); 

  std::string local_ip;
  private_node_handle.param("local_ip", local_ip, std::string("")); 

  int port_number;
  private_node_handle.param("port_number", port_number, 0); 

  private_node_handle.param("parent_frame_id", parent_frame_id, std::string("cepton"));

  bool capture_loop = true;
  private_node_handle.param("capture_loop", capture_loop, capture_loop);

  std::string capture_path = "";
  private_node_handle.param("capture_path", capture_path, capture_path);

  int control_flags = 0;
  private_node_handle.param("control_flags", control_flags, control_flags);

  std::string frame_mode_str = "CYCLE";
  private_node_handle.param("frame_mode", frame_mode_str, frame_mode_str);
  const cepton_sdk::FrameMode frame_mode = frame_mode_lut.at(frame_mode_str);

  std::string transforms_path = "";
  private_node_handle.param("transforms_path", transforms_path, transforms_path);
  if (!transforms_path.empty()) {
    parse_transforms_file(transforms_path);
  } else {
    set_up_default_transform();
  }
  
  ///////////////////join mcast group/////////////
  struct sockaddr_in localif_cepton;
  struct ip_mreq mreq_cepton;
  int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  localif_cepton.sin_family = AF_INET;
  localif_cepton.sin_port   = htons(port_number);
  localif_cepton.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(s, (sockaddr *)&localif_cepton, sizeof(localif_cepton));
  mreq_cepton.imr_interface.s_addr = inet_addr(local_ip.c_str());
  mreq_cepton.imr_multiaddr.s_addr = inet_addr(multi_ip.c_str());
  int reuse = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
  perror("setsockopt(SO_REUSEADDR) failed");
  setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq_cepton, sizeof(mreq_cepton));
  //////////////////////////////////////////////////////////////////////////////////////////////

  // Initialize sdk
  cepton_sdk::SensorError error;
  NODELET_INFO("[%s] cepton_sdk %s", getName().c_str(),
               cepton_sdk::get_version_string().c_str());

  error = error_callback.listen([this](cepton_sdk::SensorHandle handle,
                                       const cepton_sdk::SensorError &error) {
    NODELET_WARN("[%s] %s", getName().c_str(), error.what());
  });
  FATAL_ERROR(error);

  auto options = cepton_sdk::create_options();
  options.control_flags = control_flags;
  if (!capture_path.empty())
    options.control_flags |= CEPTON_SDK_CONTROL_DISABLE_NETWORK;
  options.frame.mode = frame_mode;
  if (frame_mode == CEPTON_SDK_FRAME_TIMED) options.frame.length = 0.01f;
  error = cepton_sdk::initialize(
      CEPTON_SDK_VERSION, options,
      &cepton_sdk::api::SensorErrorCallback::global_on_callback,
      &error_callback);
  FATAL_ERROR(error);

  // Start capture
  if (!capture_path.empty()) {
    error = cepton_sdk::api::open_replay(capture_path);
    FATAL_ERROR(error);
    error = cepton_sdk::capture_replay::set_enable_loop(capture_loop);
    FATAL_ERROR(error);
    error = cepton_sdk::capture_replay::resume();
    FATAL_ERROR(error);
  }

  // Listen
  error = image_frame_callback.initialize();
  FATAL_ERROR(error);
  error = image_frame_callback.listen(this, &DriverNodelet::on_image_points);
  FATAL_ERROR(error);

  // Start watchdog timer
  watchdog_timer = node_handle.createTimer(
      ros::Duration(0.1), [&](const ros::TimerEvent &event) {
        if (cepton_sdk::api::is_end()) {
          NODELET_INFO("[%s] capture replay done", getName().c_str());
          ros::shutdown();
        }
      });
}

void DriverNodelet::parse_transforms_file(const std::string& transforms_path)
{
  Json::Value root;
  std::ifstream file(transforms_path.c_str());
  file >> root;
  
  Json::Value::Members list_of_serial_nos = root.getMemberNames();
  for (auto sensor : list_of_serial_nos) {
    uint64_t serial_no = std::stoi(sensor);
    Json::Value val = root[sensor.c_str()];

    if (val.isMember("frame_id")) {
      frame_ids[serial_no] = val["frame_id"].asString();
    } else {
      frame_ids[serial_no] = "cepton_" + std::to_string(serial_no);
    }
    
    if (!val["translation"].isArray()) {
      ROS_ERROR("Malformed JSON translation array for serial number [%lu]", serial_no);
    } else if (!val["rotation"].isArray()) {
      ROS_ERROR("Malformed JSON rotation array for serial number [%lu]", serial_no);
    } else if (val["translation"].size() != 3) {
      ROS_ERROR("Incorrect number of elements in translation array for serial number [%lu]", serial_no);
    } else {
      geometry_msgs::TransformStamped transform;
      transform.header.frame_id = parent_frame_id;
      transform.child_frame_id = frame_ids[serial_no];
      transform.transform.translation.x = val["translation"][0].asDouble();
      transform.transform.translation.y = val["translation"][1].asDouble();
      transform.transform.translation.z = val["translation"][2].asDouble();
      if (val["rotation"].size() == 4) {
        transform.transform.rotation.x = val["rotation"][0].asDouble();
        transform.transform.rotation.y = val["rotation"][1].asDouble();
        transform.transform.rotation.z = val["rotation"][2].asDouble();
        transform.transform.rotation.w = val["rotation"][3].asDouble();
      } else if (val["rotation"].size() == 3) {
        tf2::Quaternion q;
        q.setRPY(val["rotation"][0].asDouble(), val["rotation"][1].asDouble(), val["rotation"][2].asDouble());
        tf2::convert(q, transform.transform.rotation);
      } else {
        ROS_ERROR("Incorrect number of rotation array elements. Use 3 for RPY, 4 for quaternion");
      }
      tf_broadcaster.sendTransform(transform);
    }

    std::string points_topic;
    std::string info_topic;
    if (val.isMember("topic_name")) {
      points_topic = "cepton/" + val["topic_name"].asString();
      info_topic = "cepton/" + val["topic_name"].asString() + "_info";
    } else {
      points_topic = "cepton/points_" + std::to_string(serial_no);
      info_topic = "cepton/sensor_information_" + std::to_string(serial_no);
    }
    points_publishers[serial_no] = node_handle.advertise<CeptonPointCloud>(points_topic, 1);
    sensor_info_publishers[serial_no] = node_handle.advertise<SensorInformation>(info_topic, 1);
  }
}

void DriverNodelet::set_up_default_transform()
{
  geometry_msgs::TransformStamped transform;
  transform.header.frame_id = parent_frame_id;
  transform.child_frame_id = "cepton_0";
  transform.transform.rotation.w = 1.0;
  tf_broadcaster.sendTransform(transform);
  points_publishers[0] = node_handle.advertise<CeptonPointCloud>("cepton/points", 1);
  sensor_info_publishers[0] = node_handle.advertise<SensorInformation>("cepton/sensor_information", 1);
}

void DriverNodelet::on_image_points(
    cepton_sdk::SensorHandle handle, std::size_t n_points,
    const cepton_sdk::SensorImagePoint *const c_image_points) {
  cepton_sdk::SensorError error;

  // Publish sensor information
  cepton_sdk::SensorInformation sensor_info;
  error = cepton_sdk::get_sensor_information(handle, sensor_info);
  WARN_ERROR(error);
  publish_sensor_information(sensor_info);

  // Publish points
  image_points.reserve(n_points);
  for (std::size_t i = 0; i < n_points; ++i) {
    image_points.push_back(c_image_points[i]);
  }
  publish_points(sensor_info.serial_number);
  image_points.clear();
  points.clear();
}

void DriverNodelet::publish_sensor_information(
    const cepton_sdk::SensorInformation &sensor_info) {
  cepton_ros::SensorInformation msg;
  msg.header.stamp = ros::Time::now();

  msg.handle = sensor_info.handle;
  msg.serial_number = sensor_info.serial_number;
  msg.model_name = sensor_info.model_name;
  msg.model = sensor_info.model;
  msg.firmware_version = sensor_info.firmware_version;

  msg.last_reported_temperature = sensor_info.last_reported_temperature;
  msg.last_reported_humidity = sensor_info.last_reported_humidity;
  msg.last_reported_age = sensor_info.last_reported_age;

  msg.measurement_period = sensor_info.measurement_period;

  msg.ptp_ts = sensor_info.ptp_ts;

  msg.gps_ts_year = sensor_info.gps_ts_year;
  msg.gps_ts_month = sensor_info.gps_ts_month;
  msg.gps_ts_day = sensor_info.gps_ts_day;
  msg.gps_ts_hour = sensor_info.gps_ts_hour;
  msg.gps_ts_min = sensor_info.gps_ts_min;
  msg.gps_ts_sec = sensor_info.gps_ts_sec;

  msg.return_count = sensor_info.return_count;
  msg.segment_count = sensor_info.segment_count;

  msg.is_mocked = sensor_info.is_mocked;
  msg.is_pps_connected = sensor_info.is_pps_connected;
  msg.is_nmea_connected = sensor_info.is_nmea_connected;
  msg.is_ptp_connected = sensor_info.is_ptp_connected;
  msg.is_calibrated = sensor_info.is_calibrated;
  msg.is_over_heated = sensor_info.is_over_heated;

  msg.cepton_sdk_version = CEPTON_SDK_VERSION;
  const uint8_t *const sensor_info_bytes = (const uint8_t *)&sensor_info;
  msg.data = std::vector<uint8_t>(sensor_info_bytes,
                                  sensor_info_bytes + sizeof(sensor_info));

  if (sensor_info_publishers.find(sensor_info.serial_number) == sensor_info_publishers.end()) {
    if (points_publishers.find(0) == points_publishers.end()) {
      // Default settings for serial number "0" not found; create a new publisher for this unexpected sensor
      sensor_info_publishers[sensor_info.serial_number] = node_handle.advertise<SensorInformation>("cepton/sensor_info_" + std::to_string(sensor_info.serial_number), 1);
    } else {
      // Use default settings from serial number "0"
      sensor_info_publishers[0].publish(msg);
    }
  } else {
    sensor_info_publishers[sensor_info.serial_number].publish(msg);
  }
}

void DriverNodelet::publish_points(uint64_t serial_number) {
  // Convert image points to points
  points.clear();
  points.resize(image_points.size());
  for (int i = 0; i < image_points.size(); ++i) {
    cepton_sdk::util::convert_sensor_image_point_to_point(image_points[i],
                                                          points[i]);
  }

  point_cloud.clear();
  point_cloud.header.stamp = rosutil::to_usec(ros::Time::now());
  if (frame_ids.find(serial_number) == frame_ids.end()) {
    point_cloud.header.frame_id = "cepton_0";
  } else {
    point_cloud.header.frame_id = frame_ids[serial_number];
  }
  point_cloud.height = 1;
  point_cloud.width = points.size();
  point_cloud.resize(points.size());
  for (std::size_t i = 0; i < points.size(); ++i) {
    point_cloud.points[i] = points[i];
  }

  if (points_publishers.find(serial_number) == points_publishers.end()) {
    if (points_publishers.find(0) == points_publishers.end()) {
      // Default settings for serial number "0" not found; create a new publisher for this unexpected sensor
      points_publishers[serial_number] = node_handle.advertise<CeptonPointCloud>("cepton/points_" + std::to_string(serial_number), 1);
    } else {
      // Use default settings from serial number "0"
      points_publishers[0].publish(point_cloud);
    }
  } else {
    points_publishers[serial_number].publish(point_cloud);
  }
}

}  // namespace cepton_ros
