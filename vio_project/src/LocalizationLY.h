#ifndef LOCALIZATION_LY_H_
#define LOCALIZATION_LY_H_

// 可视化模块头文件
#include "Viewer.h"

// 配置头文件
#include "configuration.h"

// SiftGPU 库，用于特征提取和匹配
#include "SiftGPU/SiftGPU.h"

// 是否保存运行时间的标志宏
#define SAVE_TIME 0

namespace Ulocal{

using namespace colmap;

// 图像定位主类
class LocalizationLY
{

public:
    // 构造函数
    // 参数：数据库路径、稀疏重建路径、词袋索引路径、是否使用可视化器、是否保存定位结果
    LocalizationLY(const std::string& database_path, const std::string& recs_path, const std::string& index_path,
                   bool bViewer_=false, bool bSaveResult_=false);
    ~LocalizationLY();  // 析构函数

    // 可视化当前状态
    void View();

    // 图像定位主函数
    // 输入：图像、焦距、输出姿态（四元数和平移）、是否使用初始位姿估计
    bool LocalizeImage(cv::Mat &image, double focus_length, Eigen::Vector4d &qvec, Eigen::Vector3d &tvec, bool bAsInitEstimate = false);

    // 加载视觉词袋树
    void LoadVocTree(const std::string& index_path);

private:

    // 使用GPU提取SIFT特征
    bool SIFTextractionGPU(cv::Mat &image, FeatureKeypoints* keypoints,
                           FeatureDescriptors* descriptors);

    // 使用CPU提取SIFT特征
    bool SIFTextractionCPU(cv::Mat &image, FeatureKeypoints* keypoints,
                           FeatureDescriptors* descriptors);

    // 基于视觉词袋树匹配图像，返回所有匹配项
    Retrieval MatchVocTreeReturnAll(FeatureKeypoints &keypoints, FeatureDescriptors &descriptors);

    // 比较两个四元数之间的夹角
    double CompareQuaternions(Eigen::Vector4d &qvec_1, Eigen::Vector4d &qvec_2);

    // 使用初始位姿估计进行匹配
    Retrieval MatchWithInitialPoseEstimation(Eigen::Vector4d &qvec, Eigen::Vector3d &tvec);

    // 使用GPU与数据库中的图像进行匹配
    FeatureMatches MatchWithImageGPU(image_t idx, FeatureDescriptors &descriptors);

    // 使用CPU与数据库中的图像进行匹配
    FeatureMatches MatchWithImageCPU(image_t idx, FeatureDescriptors &descriptors);

    // 位姿估计函数
    // 输入关键点、参考图像索引、匹配对、焦距、图像宽高，输出姿态
    bool PoseEstimation(FeatureKeypoints &keypoints, image_t refIdx, FeatureMatches matches, 
                        double focus_length, int width, int height, Eigen::Vector4d &qvec, Eigen::Vector3d &tvec);

public:
    // 图像数据库相关成员
    int numImage;           // 图像数量
    Database* database;     // 图像数据库指针

    // 相机模型数量
    int numCamera;

    // 稀疏重建数据
    Reconstruction* reconstruction;

    // 视觉词袋树索引结构
    VisualIndex_LY<> visual_index;

    // SIFT匹配器（GPU）
    bool siftMatcherGPUcreated = false;  // 是否已创建GPU匹配器
    SiftMatchingOptions match_options;   // 匹配参数
    SiftMatchGPU* sift_match_gpu;        // GPU匹配器指针

    // 使用初始位姿进行匹配时相关参数
    std::vector<image_t> framesIds;  // 候选帧图像ID列表
    double Threshold_angle = 60;     // 角度阈值（度）
    double Threshold_distance = 1;   // 距离阈值

    // 匹配尝试次数限制
    int maxTrival = 10;              // 普通匹配最大尝试次数
    int maxTrival_init_pose = 20;    // 初始位姿匹配最大尝试次数

public:
    bool bVerbose = true;            // 是否输出详细信息

    // 保存定位结果
    bool bSaveResult;
    std::string runtimeFilename;     // 保存运行时结果的文件名
    std::ofstream runtimefile;       // 保存运行时结果的文件输出流

    // 可视化器相关
    bool bViewer;                    // 是否启用可视化器
    ViewerLY *pViewerLY;             // 可视化器指针
};

}  // namespace Ulocal

#endif // LOCALIZATION_LY_H_
