/*
* 保存地图关键帧，用于本地定位
* 1. 关键帧数量
* 2. Keyframes -> features 2d
*                          -> feature descriptors
*                          -> world pose 3d
*                          -> camera id 
*/



#ifndef MAP_SAVER_H_LY_
#define MAP_SAVER_H_LY_

#include <fstream> // 引入文件流库

#include "configuration.h" // 引入自定义配置头文件 (假设包含一些配置参数)


namespace BASTIAN
{

using namespace colmap; // 使用 colmap 命名空间，简化代码

// 定义特征描述子类型，Eigen::Dynamic 表示行数在编译时未知，1 表示列数为1
typedef Eigen::Matrix<uint8_t, Eigen::Dynamic, 1> FeatureDescriptor_ly;

class MapSaver // 定义 MapSaver 类
{

public:
        MapSaver(const std::string& database_path, const std::string& recs_path,
                                             bool bViewer_=false); // 构造函数，初始化数据库路径、重建路径和 viewer 标志

        ~MapSaver(); // 析构函数，负责清理资源

public:
        void SaveKeyFrame(image_t framesId, std::ofstream &outputFile); // 保存单个关键帧数据到文件
        void LoadKeyFrame(std::ifstream &inputFile); // 从文件加载单个关键帧数据

        bool SaveMap(const std::string& save_path); // 保存整个地图数据到文件
        bool LoadMap(const std::string& load_path); // 从文件加载整个地图数据

        void SaveKeyframesTxt(const std::string& save_path); // 保存关键帧信息到文本文件 (可能用于调试)

private:
        bool bViewer; // 标记是否使用 viewer (可能是可视化工具)

        // 图像数据库相关成员变量
        int numImage; // 图像数量
        std::vector<image_t> framesIds; // 存储关键帧ID的向量
        Database* database; // 指向数据库对象的指针，用于访问图像数据

        // 相机模型相关成员变量
        int numCamera; // 相机数量

        // 稀疏重建数据相关成员变量
        Reconstruction* reconstruction; // 指向重建对象的指针，用于访问重建结果 (例如 3D 点云、相机位姿)

};
}
#endif
