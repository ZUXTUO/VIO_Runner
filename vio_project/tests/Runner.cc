/**
*    综合启动器
*
*    ZUXTUO
*
*
*/

#include <vector>
#include <string>
#include <dirent.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <memory>
#include <functional>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

// 包含必要的定位相关头文件
#include "LocalizationLY.h"
#include "MapSaver.h"
#include "test_utils.h"
#include "UlocalizationPlugins.h"

using namespace colmap;

// 自定义线程池类，重命名为CustomThreadPool以避免与colmap::ThreadPool冲突
/**
 * @brief 自定义线程池实现，用于并行处理客户端请求
 * 线程池创建固定数量的工作线程，并维护一个任务队列
 */
class CustomThreadPool {
public:
    /**
     * @brief 构造函数，初始化指定数量的工作线程
     * @param threads 线程数量
     */
    CustomThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i < threads; ++i)
            workers.emplace_back([this] {
                while(true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        // 等待条件：要么线程池停止，要么有新任务
                        this->condition.wait(lock, [this] { 
                            return this->stop || !this->tasks.empty(); 
                        });
                        if(this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
    }

    /**
     * @brief 向线程池添加新任务
     * @param f 待执行的函数或可调用对象
     */
    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    /**
     * @brief 析构函数，停止所有线程并等待它们完成
     */
    ~CustomThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker: workers)
            worker.join();
    }
    
    /**
     * @brief 获取线程池中的线程数量
     * @return 线程数量
     */
    size_t getThreadCount() const {
        return workers.size();
    }

private:
    std::vector<std::thread> workers;        // 工作线程集合
    std::queue<std::function<void()>> tasks; // 任务队列
    std::mutex queue_mutex;                  // 队列互斥锁
    std::condition_variable condition;       // 条件变量，用于线程同步
    bool stop;                               // 停止标志
};

/**
 * @brief Socket读取器类，用于优化网络数据读取
 * 实现缓冲区读取，提高网络IO效率
 */
class SocketReader {
public:
    static constexpr size_t BUFFER_SIZE = 8192; // 8KB缓冲区
    
    /**
     * @brief 构造函数
     * @param socket 客户端套接字描述符
     */
    SocketReader(int socket) : socket_(socket), buffer_pos_(0), buffer_size_(0) {
        buffer_ = new char[BUFFER_SIZE];
    }
    
    /**
     * @brief 析构函数，释放缓冲区
     */
    ~SocketReader() {
        delete[] buffer_;
    }
    
    /**
     * @brief 从套接字读取指定长度的数据
     * @param dest 目标缓冲区
     * @param length 需要读取的字节数
     * @return 是否成功读取
     */
    bool readExactly(char* dest, size_t length) {
        size_t bytesRead = 0;
        while (bytesRead < length) {
            // 如果缓冲区为空，从套接字读取新数据
            if (buffer_pos_ >= buffer_size_) {
                buffer_pos_ = 0;
                ssize_t result = read(socket_, buffer_, BUFFER_SIZE);
                if (result <= 0) return false;  // 读取失败或连接关闭
                buffer_size_ = result;
            }
            
            // 计算从缓冲区可复制的字节数
            size_t canCopy = std::min(buffer_size_ - buffer_pos_, length - bytesRead);
            memcpy(dest + bytesRead, buffer_ + buffer_pos_, canCopy);
            buffer_pos_ += canCopy;
            bytesRead += canCopy;
        }
        return true;
    }
    
private:
    int socket_;           // 套接字描述符
    char* buffer_;         // 缓冲区
    size_t buffer_pos_;    // 当前缓冲区位置
    size_t buffer_size_;   // 缓冲区中有效数据大小
};

/**
 * @brief 定位服务器类
 * 提供基于套接字的图像定位服务
 */
class LocalizationServer {
public:
    /**
     * @brief 构造函数
     * @param db_path 数据库路径
     * @param sparse_map_path 稀疏地图路径
     * @param voc_indices_path 词汇树索引路径
     * @param port 服务器端口，默认为8080
     * @param num_threads 工作线程数，默认为4
     */
    LocalizationServer(const std::string& db_path, const std::string& sparse_map_path, 
                       const std::string& voc_indices_path, int port = 8080, int num_threads = 4)
        : port_(port), running_(false), thread_pool_(num_threads) {
        
        // 初始化定位模块
        std::cout << "【信息】初始化定位模块..." << std::endl;
        localizer_ = std::make_shared<Ulocal::LocalizationLY>(
            db_path.c_str(), sparse_map_path.c_str(), voc_indices_path.c_str(), true, true);
        std::cout << "【信息】定位模块初始化完成" << std::endl;
    }
    
    /**
     * @brief 析构函数，停止服务器
     */
    ~LocalizationServer() {
        stop();
    }
    
    /**
     * @brief 启动服务器
     * @return 是否成功启动
     */
    bool start() {
        signal(SIGPIPE, SIG_IGN); // 防止客户端断开连接导致服务器崩溃
        
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) {
            std::cerr << "【错误】套接字创建失败\n";
            return false;
        }
        
        // 设置套接字选项，允许端口重用
        int opt = 1;
        setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        // 绑定地址和端口
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port_);
        
        if (bind(server_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "【错误】绑定端口失败\n";
            close(server_fd_);
            return false;
        }
        
        // 开始监听
        if (listen(server_fd_, 30) < 0) {
            std::cerr << "【错误】监听失败\n";
            close(server_fd_);
            return false;
        }
        
        running_ = true;
        acceptor_thread_ = std::thread(&LocalizationServer::acceptLoop, this);
        
        std::cout << "【信息】服务器已在端口 " << port_ << " 启动" << std::endl;
        std::cout << "【信息】使用 " << thread_pool_.getThreadCount() << " 个工作线程" << std::endl;
        
        return true;
    }
    
    /**
     * @brief 停止服务器
     */
    void stop() {
        if (!running_) return;
        
        std::cout << "【信息】正在停止服务器..." << std::endl;
        
        running_ = false;
        if (acceptor_thread_.joinable()) {
            acceptor_thread_.join();
        }
        
        if (server_fd_ >= 0) {
            close(server_fd_);
            server_fd_ = -1;
        }
        
        std::cout << "【信息】服务器已停止" << std::endl;
    }
    
private:
    /**
     * @brief 构建JSON响应
     * @param success 定位是否成功
     * @param qvec 四元数向量
     * @param tvec 平移向量
     * @param process_time 处理时间
     * @return JSON格式的响应字符串
     */
    std::string buildJsonResponse(bool success, const Eigen::Vector4d& qvec, 
                                 const Eigen::Vector3d& tvec, double process_time) {
        std::ostringstream json;
        json.precision(12); // 使用高精度输出
        
        json << "{";
        json << "\"success\":" << (success ? "true" : "false");
        
        if (success) {
            json << ",\"position\":[" << tvec(0) << "," << tvec(1) << "," << tvec(2) << "]";
            json << ",\"quaternion\":[" << qvec(0) << "," << qvec(1) << "," << qvec(2) << "," << qvec(3) << "]";
        }
        
        json << ",\"time\":" << process_time;
        json << "}";
        
        return json.str();
    }
    
    /**
     * @brief 处理客户端连接
     * @param client_socket 客户端套接字描述符
     */
    void handleClient(int client_socket) {
        // 使用智能指针管理资源
        std::unique_ptr<SocketReader> reader = std::make_unique<SocketReader>(client_socket);
        
        try {
            std::cout << "【信息】处理新的客户端连接..." << std::endl;
            
            // 接收焦距
            double focus_length;
            if (!reader->readExactly((char*)&focus_length, sizeof(double))) {
                std::cerr << "【错误】读取焦距失败\n";
                close(client_socket);
                return;
            }
            std::cout << "【信息】接收到焦距: " << focus_length << std::endl;
            
            // 接收图像尺寸
            int dims[2];
            if (!reader->readExactly((char*)dims, 2 * sizeof(int))) {
                std::cerr << "【错误】读取图像尺寸失败\n";
                close(client_socket);
                return;
            }
            
            int width = dims[0];
            int height = dims[1];
            if (width <= 0 || height <= 0 || width > 10000 || height > 10000) {
                std::cerr << "【错误】无效的图像尺寸: " << width << "x" << height << std::endl;
                close(client_socket);
                return;
            }
            std::cout << "【信息】接收到图像尺寸: " << width << "x" << height << std::endl;
            
            // 读取原始图像数据
            size_t image_size = (size_t)width * height * 3;
            cv::Mat original_frame(height, width, CV_8UC3);
            if (!reader->readExactly((char*)original_frame.data, image_size)) {
                std::cerr << "【错误】读取图像数据失败\n";
                close(client_socket);
                return;
            }
            std::cout << "【信息】成功接收图像数据" << std::endl;
            
            // 将图像调整为原始大小的1/3（与Test_video_images保持一致）
            cv::Mat resized_frame;
            cv::resize(original_frame, resized_frame, cv::Size(width/3, height/3));
            std::cout << "【信息】图像已调整为 " << width/3 << "x" << height/3 << std::endl;
            
            // 调用定位模块
            std::cout << "【信息】开始定位计算..." << std::endl;
            TicToc timer;
            Eigen::Vector4d qvec;
            Eigen::Vector3d tvec;
            bool success = localizer_->LocalizeImage(resized_frame, focus_length, qvec, tvec);
            double process_time = timer.Now();
            
            // 构建JSON响应
            std::string json_response = buildJsonResponse(success, qvec, tvec, process_time);
            
            // 首先发送JSON响应长度（4字节整数）
            uint32_t resp_len = json_response.size();
            if (write(client_socket, &resp_len, sizeof(resp_len)) < 0) {
                std::cerr << "【错误】发送响应长度失败\n";
                close(client_socket);
                return;
            }
            
            // 发送JSON响应
            if (write(client_socket, json_response.c_str(), json_response.size()) < 0) {
                std::cerr << "【错误】发送JSON响应失败\n";
            }
            
            std::cout << "【信息】处理图像 " << width << "x" << height 
                      << ", 结果: " << (success ? "成功" : "失败")
                      << ", 耗时: " << process_time << "秒" << std::endl;
            
            // 以与Test_video_images相同的格式输出调试信息
            if (success) {
                std::cout << "【定位结果】位姿: q: " << qvec(0) << " " << qvec(1) << " " 
                          << qvec(2) << " " << qvec(3) << " t: " << tvec(0) << " " 
                          << tvec(1) << " " << tvec(2) << std::endl;
                std::cout << "【定位结果】耗时: " << process_time << " 秒" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "【异常】" << e.what() << std::endl;
        }
        
        close(client_socket);
        std::cout << "【信息】客户端连接已关闭" << std::endl;
    }
    
    /**
     * @brief 接受连接循环
     * 在单独的线程中运行，接受新的客户端连接
     */
    void acceptLoop() {
        std::cout << "【信息】开始接受客户端连接..." << std::endl;
        
        while (running_) {
            sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);
            int client_sock = accept(server_fd_, (sockaddr*)&client_addr, &len);
            
            if (client_sock < 0) {
                if (running_) {
                    std::cerr << "【警告】接受连接失败，继续等待...\n";
                }
                continue;
            }
            
            // 设置客户端套接字接收超时
            struct timeval tv;
            tv.tv_sec = 10;  // 10秒超时
            tv.tv_usec = 0;
            setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
            
            // 在线程池中处理客户端
            std::cout << "【信息】接受新的客户端连接，分配给工作线程处理" << std::endl;
            thread_pool_.enqueue([this, client_sock]() {
                handleClient(client_sock);
            });
        }
    }
    
    int port_;                                       // 服务器端口
    int server_fd_ = -1;                             // 服务器套接字描述符
    std::atomic<bool> running_;                      // 运行标志
    std::thread acceptor_thread_;                    // 接受连接线程
    std::shared_ptr<Ulocal::LocalizationLY> localizer_; // 定位模块
    CustomThreadPool thread_pool_;                   // 线程池（使用重命名的类）
};

/**
 * @brief 保存地图函数（来自Save_map.cc）
 * @param database_path 数据库路径
 * @param sparse_map_path 稀疏地图路径
 * @param save_path 地图保存路径
 * @param save_path_txt 关键帧文本保存路径
 */
void saveMap(const std::string& database_path, const std::string& sparse_map_path, 
             const std::string& save_path, const std::string& save_path_txt) {
    
    std::cout << "【信息】开始保存地图..." << std::endl;
    
    BASTIAN::MapSaver *pMapSaver = new BASTIAN::MapSaver(database_path.c_str(), sparse_map_path.c_str(), true);

    // 将地图保存到二进制文件
    pMapSaver->SaveMap(save_path);
    std::cout << "【信息】地图已保存至: " << save_path << std::endl;

    // 将关键帧保存到文本文件
    pMapSaver->SaveKeyframesTxt(save_path_txt);
    std::cout << "【信息】关键帧已保存至: " << save_path_txt << std::endl;

    // 验证加载地图
    pMapSaver->LoadMap(save_path);
    std::cout << "【信息】已成功加载地图进行验证。" << std::endl;
    
    delete pMapSaver;
}

/**
 * @brief 创建词汇树索引函数（来自make_index.cc）
 * @param database_path 数据库路径
 * @param sparse_map_path 稀疏地图路径
 * @param voc_indices_path 词汇树索引保存路径
 * @param vocab_path 词汇树路径
 */
void makeIndex(const std::string& database_path, const std::string& sparse_map_path,
               const std::string& voc_indices_path, const std::string& vocab_path) {
    
    std::cout << "【信息】开始为服务器定位创建索引..." << std::endl;

    // 加载预先构建的colmap地图文件
    std::cout << "【信息】加载地图文件..." << std::endl;
    Ulocal::Ulocalization ulocalization(database_path, sparse_map_path);

    // 保存PLY点云
    std::string ply_path = sparse_map_path + "point_cloud.ply";
    ulocalization.SavePLYCLoud(ply_path);
    std::cout << "【信息】点云已保存至: " << ply_path << std::endl;

    // 制作词汇树索引
    std::cout << "【信息】正在创建词汇树索引，这可能需要一些时间..." << std::endl;
    ulocalization.MakeVocTreeIndex(vocab_path, voc_indices_path);
    std::cout << "【信息】词汇树索引已创建于: " << voc_indices_path << std::endl;

    // 启动查看器
    std::cout << "【信息】启动地图查看器..." << std::endl;
    ulocalization.View();
    std::cout << "【信息】地图查看器已关闭" << std::endl;
}

/**
 * @brief 启动定位服务器函数（来自Test_images_server.cc）
 * @param database_path 数据库路径
 * @param sparse_map_path 稀疏地图路径
 * @param voc_indices_path 词汇树索引路径
 * @param port 服务器端口
 * @param num_threads 工作线程数
 */
void startServer(const std::string& database_path, const std::string& sparse_map_path,
                 const std::string& voc_indices_path, int port, int num_threads) {
    
    try {
        std::cout << "【信息】准备启动定位服务器..." << std::endl;
        std::cout << "【配置信息】" << std::endl;
        std::cout << "  - 数据库路径: " << database_path << std::endl;
        std::cout << "  - 稀疏地图路径: " << sparse_map_path << std::endl;
        std::cout << "  - 词汇树索引路径: " << voc_indices_path << std::endl;
        std::cout << "  - 端口: " << port << std::endl;
        std::cout << "  - 线程数: " << num_threads << std::endl;
        
        // 创建并启动服务器
        LocalizationServer server(database_path, sparse_map_path, voc_indices_path, port, num_threads);
        if (!server.start()) {
            std::cerr << "【错误】启动服务器失败。" << std::endl;
            return;
        }
        
        // 等待用户输入以停止服务器
        std::cout << "【信息】服务器运行中。按回车键停止。" << std::endl;
        std::cin.get();
        
        server.stop();
        std::cout << "【信息】服务器已停止。" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "【错误】" << e.what() << std::endl;
    }
}

/**
 * @brief 打印使用信息
 */
void printUsage() {
    std::cout << "统一定位工具" << std::endl;
    std::cout << "===========" << std::endl;
    std::cout << "用法:" << std::endl;
    std::cout << "  1. 服务器模式:" << std::endl;
    std::cout << "     ./Runner server 数据库路径 稀疏地图路径 词汇树索引路径 [端口] [线程数]" << std::endl;
    std::cout << std::endl;
    std::cout << "  2. 地图保存模式:" << std::endl;
    std::cout << "     ./Runner save 数据库路径 稀疏地图路径 保存路径 关键帧文本保存路径" << std::endl;
    std::cout << std::endl;
    std::cout << "  3. 索引创建模式:" << std::endl;
    std::cout << "     ./Runner index 数据库路径 稀疏地图路径 词汇树索引路径 词汇树路径" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief 主函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 执行结果
 */
int main(int argc, char** argv) {
    std::cout << "【信息】统一定位工具 v1.0" << std::endl;
    std::cout << "【信息】启动时间: " << __DATE__ << " " << __TIME__ << std::endl;
    
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];
    
    if (command == "server") {
        // 服务器模式
        if (argc < 5) {
            std::cerr << "【错误】服务器模式缺少参数。" << std::endl;
            printUsage();
            return 1;
        }
        
        std::string database_path = argv[2];
        std::string sparse_map_path = argv[3];
        std::string voc_indices_path = argv[4];
        
        int port = 8080; // 默认端口
        if (argc >= 6) port = std::stoi(argv[5]);
        
        int num_threads = std::thread::hardware_concurrency(); // 默认：使用所有可用CPU核心
        if (argc >= 7) num_threads = std::stoi(argv[6]);
        
        startServer(database_path, sparse_map_path, voc_indices_path, port, num_threads);
    }
    else if (command == "save") {
        // 地图保存模式
        if (argc != 6) {
            std::cerr << "【错误】保存模式参数无效。" << std::endl;
            printUsage();
            return 1;
        }
        
        std::string database_path = argv[2];
        std::string sparse_map_path = argv[3];
        std::string save_path = argv[4];
        std::string save_path_txt = argv[5];
        
        saveMap(database_path, sparse_map_path, save_path, save_path_txt);
    }
    else if (command == "index") {
        // 索引创建模式
        if (argc != 6) {
            std::cerr << "【错误】索引模式参数无效。" << std::endl;
            printUsage();
            return 1;
        }
        
        std::string database_path = argv[2];
        std::string sparse_map_path = argv[3];
        std::string voc_indices_path = argv[4];
        std::string vocab_path = argv[5];
        
        makeIndex(database_path, sparse_map_path, voc_indices_path, vocab_path);
    }
    else {
        std::cerr << "【错误】未知命令: " << command << std::endl;
        printUsage();
        return 1;
    }
    
    std::cout << "【信息】程序正常退出" << std::endl;
    return 0;
}