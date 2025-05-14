#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port> <image_path> [focal_length]\n";
        return 1;
    }

    // 解析参数
    std::string server_ip = argv[1];
    int port = std::stoi(argv[2]);
    std::string image_path = argv[3];
    double focal_length = 1000.0;  // 默认焦距
    if (argc > 4) {
        focal_length = std::stod(argv[4]);
    }

    // 读取图像
    cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        std::cerr << "Failed to load image: " << image_path << std::endl;
        return 1;
    }

    // 确保图像是BGR格式
    if (image.channels() != 3) {
        std::cerr << "Image must be in BGR format with 3 channels\n";
        return 1;
    }

    // 创建socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error\n";
        return 1;
    }

    // 设置服务器地址
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    // 将IP地址从文本转换为二进制形式
    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address or address not supported\n";
        close(sock);
        return 1;
    }

    // 连接到服务器
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed\n";
        close(sock);
        return 1;
    }

    // 发送焦距
    if (write(sock, &focal_length, sizeof(focal_length)) != sizeof(focal_length)) {
        std::cerr << "Failed to send focal length\n";
        close(sock);
        return 1;
    }

    // 发送图像尺寸
    int dims[2] = {image.cols, image.rows};
    if (write(sock, dims, sizeof(dims)) != sizeof(dims)) {
        std::cerr << "Failed to send image dimensions\n";
        close(sock);
        return 1;
    }

    // 发送图像数据
    size_t image_size = image.total() * image.elemSize();
    if (write(sock, image.data, image_size) != image_size) {
        std::cerr << "Failed to send image data\n";
        close(sock);
        return 1;
    }

    // 接收JSON响应长度
    uint32_t response_length;
    if (read(sock, &response_length, sizeof(response_length)) != sizeof(response_length)) {
        std::cerr << "Failed to receive response length\n";
        close(sock);
        return 1;
    }

    // 接收JSON响应
    if (response_length > 10000000) {  // 安全检查，防止分配过大内存
        std::cerr << "Response length too large: " << response_length << std::endl;
        close(sock);
        return 1;
    }

    std::string json_response(response_length, '\0');
    if (read(sock, &json_response[0], response_length) != response_length) {
        std::cerr << "Failed to receive complete JSON response\n";
        close(sock);
        return 1;
    }

    // 关闭连接
    close(sock);

    // 显示结果
    std::cout << "Server response:\n" << json_response << std::endl;

    return 0;
}