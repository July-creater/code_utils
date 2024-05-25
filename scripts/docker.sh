#!/bin/zsh
# 获取今天的日期
TODAY=$(date +%Y-%m-%d)

# 定义镜像名称和容器名称
IMAGE_NAME="wq_env:${TODAY}"
CONTAINER_NAME="code_utils"

# 检查镜像是否存在的函数
function check_image_exists {
    docker images -q $IMAGE_NAME
}

# 检查容器是否存在的函数
function check_container_exists {
    docker ps -aq -f name=$CONTAINER_NAME
}

# 构建Docker镜像的函数
function build_image {
    echo "正在构建Docker镜像..."
    docker build -t $IMAGE_NAME .
    if [ $? -ne 0 ]; then
        echo "Docker镜像构建失败!"
        exit 1
    fi
    echo "Docker镜像构建成功!"
}

# 创建Docker容器的函数
function create_container {
    echo "正在创建Docker容器..."
    docker run \
    -v $(pwd):/root/project \
    -v /mnt/e:/root/data \
    -itd --gpus all --name $CONTAINER_NAME $IMAGE_NAME zsh
    
    if [ $? -ne 0 ]; then
        echo "Docker容器创建失败!"
        exit 1
    fi
    echo "Docker容器创建成功!"
}

# 进入Docker容器的函数
function enter_container {
    echo "正在检查Docker容器状态..."
    if ! docker ps -f name=$CONTAINER_NAME --format '{{.Names}}' | grep -q $CONTAINER_NAME; then
        echo "容器未在运行中，正在启动容器..."
        docker start $CONTAINER_NAME;
    fi
    echo "正在进入Docker容器..."
    if [ -d "build" ]; then
        docker exec -it $CONTAINER_NAME zsh -c "chsh -s /bin/zsh \
        && git config --global --add safe.directory /root/project \
        && git config --global user.email "wqjuly@qq.com" \
        && git config --global user.name "wq"  \
        && cd /root/project/build && ./hello_world && zsh"
    else
        docker exec -it $CONTAINER_NAME zsh -c "chsh -s /bin/zsh \
        && git config --global --add safe.directory /root/project \
        && git config --global user.email "wqjuly@qq.com" \
        && git config --global user.name "wq"  \
        && cd /root/project && if [ ! -d build ]; then mkdir build; fi \
        && cd build && cmake .. && make -j24 && ./hello_world && zsh"
    fi
    echo ' ' | sudo -S chmod -R 777 .
}


# 处理命令行参数
if [ "$1" == "init" ]; then
    if [ $(check_container_exists) ]; then
        echo "正在删除现有的容器..."
        docker rm -f $CONTAINER_NAME
    fi

    if [ ! $(check_image_exists) ]; then
        echo "镜像不存在，创建镜像..."
        build_image
    fi
    create_container
    enter_container
else
   if [ ! $(check_container_exists) ]; then
        echo "容器不存在，创建容器..."
        create_container
    fi
    enter_container
fi
