# ros1 converter CustomMsg to PointCloud2

# Intended use

```shell
This tool converts Livox `/livox/lidar` (`livox_ros_driver/CustomMsg`) directly into
`/livox/pointcloud` (`sensor_msgs/PointCloud2`).
```

## Dependencies

```shell
sudo apt update
sudo apt install -y docker.io
sudo usermod -aG docker $USER
```

```shell
mkdir -p ~/livox_converter_ws
cd ~/livox_converter_ws
git clone https://github.com/MapsHD/livox_bag_aggregate.git --recursive
```
## Docker build
```shell
cd ~/livox_converter_ws/livox_bag_aggregate
docker build -t livox_bag_aggregate_noetic .
```

## Docker run
```shell
cd ~/livox_converter_ws/livox_bag_aggregate
chmod +x livox_bag.sh 
livox_bag.sh <input_bag> <output_folder>
```