#include <ros/ros.h>
#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/point_cloud2_iterator.h>
#include <livox_ros_driver/CustomMsg.h>
#include <boost/foreach.hpp>
#include <cstdio>
#include <cstdlib>


using livox_ros_driver::CustomMsg;
using livox_ros_driver::CustomPoint;

static std::vector<float> xs, ys, zs, intens, times_within;
static std::vector<uint8_t> tags, lines;

static uint64_t win_start_ns = 0, win_end_ns = 0;
static bool window_active = false;

static void clear_buffers() {
  xs.clear(); ys.clear(); zs.clear(); intens.clear(); times_within.clear();
  tags.clear(); lines.clear();
}

static void reserve_for(size_t extra) {
  xs.reserve(xs.size()+extra); ys.reserve(ys.size()+extra); zs.reserve(zs.size()+extra);
  intens.reserve(intens.size()+extra); times_within.reserve(times_within.size()+extra);
  tags.reserve(tags.size()+extra); lines.reserve(lines.size()+extra);
}

static void flush_window(rosbag::Bag& outbag,
                         const std::string& out_topic,
                         const std::string& frame_id)
{
  if (!window_active) return;
  if (xs.empty()) return;

  sensor_msgs::PointCloud2 cloud;
  cloud.header.stamp.fromNSec(win_start_ns);
  cloud.header.frame_id = frame_id;
  cloud.height = 1;
  cloud.width  = xs.size();

  sensor_msgs::PointCloud2Modifier mod(cloud);
  mod.setPointCloud2Fields(7,
    "x", 1, sensor_msgs::PointField::FLOAT32,
    "y", 1, sensor_msgs::PointField::FLOAT32,
    "z", 1, sensor_msgs::PointField::FLOAT32,
    "intensity", 1, sensor_msgs::PointField::FLOAT32,
    "tag", 1, sensor_msgs::PointField::UINT8,
    "line", 1, sensor_msgs::PointField::UINT8,
    "time", 1, sensor_msgs::PointField::FLOAT32);
  mod.resize(cloud.width);

  sensor_msgs::PointCloud2Iterator<float> it_x(cloud, "x");
  sensor_msgs::PointCloud2Iterator<float> it_y(cloud, "y");
  sensor_msgs::PointCloud2Iterator<float> it_z(cloud, "z");
  sensor_msgs::PointCloud2Iterator<float> it_i(cloud, "intensity");
  sensor_msgs::PointCloud2Iterator<float> it_t(cloud, "time");
  sensor_msgs::PointCloud2Iterator<uint8_t> it_tag(cloud, "tag");
  sensor_msgs::PointCloud2Iterator<uint8_t> it_line(cloud, "line");

  const size_t N = xs.size();
  for (size_t k = 0; k < N; ++k, ++it_x, ++it_y, ++it_z, ++it_i, ++it_tag, ++it_line, ++it_t) {
    *it_x = xs[k]; *it_y = ys[k]; *it_z = zs[k];
    *it_i = intens[k]; *it_tag = tags[k]; *it_line = lines[k]; *it_t = times_within[k];
  }

  outbag.write(out_topic, cloud.header.stamp, cloud);
  clear_buffers();
}

int main(int argc, char** argv)
{
  // args
  if (argc < 4) {
    std::fprintf(stderr,
      "Usage:\n  %s <in.bag> <out.bag> <in_topic> <out_topic> \n",
      argv[0]);
    return 1;
  }
  const std::string in_bag_path  = argv[1];
  const std::string out_bag_path = argv[2];
  const std::string in_topic     = argv[3];
  const std::string out_topic    = argv[4];
  const double agg_secs          = 0.1;
  const std::string frame_id     = (argc > 6) ? argv[6] : "livox_frame";

  ros::Time::init(); // no roscore needed

  rosbag::Bag inbag, outbag;
  try {
    inbag.open(in_bag_path, rosbag::bagmode::Read);
  } catch (const rosbag::BagException& e) {
    std::fprintf(stderr, "Failed to open input bag: %s\n", e.what());
    return 2;
  }
  try {
    outbag.open(out_bag_path, rosbag::bagmode::Write);
  } catch (const rosbag::BagException& e) {
    std::fprintf(stderr, "Failed to open output bag: %s\n", e.what());
    return 3;
  }

  rosbag::View view(inbag);
  const uint64_t window_ns = static_cast<uint64_t>(agg_secs * 1e9);

  clear_buffers();
  window_active = false;

  size_t msg_count = 0, pt_count = 0, out_clouds = 0;

  BOOST_FOREACH(rosbag::MessageInstance const m, view) {
    CustomMsg::ConstPtr msg = m.instantiate<CustomMsg>();
    sensor_msgs::Imu::ConstPtr msgImu = m.instantiate<sensor_msgs::Imu>();
    if (msgImu)
    {
        
        outbag.write(m.getTopic(), m.getTime(), msgImu);

    }

    if (!msg) continue;
    if (m.getTopic() != in_topic) continue;
    ++msg_count;
    if (msg->point_num == 0 || msg->points.empty()) continue;

    const uint64_t first_pt_ns = msg->timebase + static_cast<uint64_t>(msg->points.front().offset_time);

    if (!window_active) {
      window_active = true;
      win_start_ns = first_pt_ns;
      win_end_ns   = win_start_ns + window_ns;
      reserve_for(msg->point_num);
    }

    // new message may start after the current window -> flush windows until aligned
    while (first_pt_ns >= win_end_ns) {
      flush_window(outbag, out_topic, frame_id);
      ++out_clouds;
      win_start_ns = win_end_ns;
      win_end_ns   = win_start_ns + window_ns;
    }

    for (const auto& p : msg->points) {
      const uint64_t t_ns = msg->timebase + static_cast<uint64_t>(p.offset_time);

      // roll windows if point belongs to future window
      while (t_ns >= win_end_ns) {
        flush_window(outbag, out_topic, frame_id);
        ++out_clouds;
        win_start_ns = win_end_ns;
        win_end_ns   = win_start_ns + window_ns;
      }

      xs.push_back(p.x);
      ys.push_back(p.y);
      zs.push_back(p.z);
      intens.push_back(static_cast<float>(p.reflectivity));
      tags.push_back(p.tag);
      lines.push_back(p.line);
      const double t_off = static_cast<double>(t_ns - win_start_ns) * 1e-9;
      times_within.push_back(static_cast<float>(t_off));
      ++pt_count;
    }
  }

  // flush trailing window
  if (window_active && !xs.empty()) {
    flush_window(outbag, out_topic, "livox_frame");
    ++out_clouds;
  }

  inbag.close();
  outbag.close();

  std::printf("Done.\nInput msgs: %zu  Points: %zu  Output clouds: %zu\n",
              msg_count, pt_count, out_clouds);
  return 0;
}
