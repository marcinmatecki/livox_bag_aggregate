# ros1 converter CustomMsg to PointCloud2

## Hint

Please change branch to [Bunker-DVI-Dataset-reg-1]() for quick experiment.  

# Simlified instruction

## Step 1 (prepare code)
```shell
mkdir -p ~/hdmapping-benchmark
cd ~/hdmapping-benchmark
git clone https://github.com/MapsHD/livox_bag_aggregate.git --recursive
```

## Step 2 (build docker)
```shell
cd ~/hdmapping-benchmark/livox_bag_aggregate
docker build -t livox_bag_aggregate_noetic .
```

## Step 3 (run docker)
```shell
cd ~/hdmapping-benchmark/livox_bag_aggregate
chmod +x livox_bag.sh 
./livox_bag.sh <input_bag> <output_folder>
```

## Step 4 (expected outcome)
'reg-1.bag-pc.bag' file should appear in <output_folder>

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
## Workspace

```shell
mkdir -p ~/hdmapping-benchmark
cd ~/hdmapping-benchmark
git clone https://github.com/MapsHD/livox_bag_aggregate.git --recursive
```
## Docker build
```shell
cd ~/hdmapping-benchmark/livox_bag_aggregate
docker build -t livox_bag_aggregate_noetic .
```

## Docker run
```shell
cd ~/hdmapping-benchmark/livox_bag_aggregate
chmod +x livox_bag.sh 
./livox_bag.sh <input_bag> <output_folder>
```