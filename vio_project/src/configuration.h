#ifndef CONFIGURATION_LY_H_
#define CONFIGURATION_LY_H_

// 引入文件流，用于文件读写
#include <fstream>

// Pangolin 是一个用于可视化的库，常用于SLAM系统中
#include <pangolin/pangolin.h>

// 引入 Eigen 库，用于线性代数计算
#include <Eigen/Core>
#include <Eigen/Dense>

// 引入项目中的日志记录和选项管理工具
#include "util/logging.h"
#include "util/option_manager.h"

// 引入重建、数据库、投影和三角化相关模块
#include "base/reconstruction.h"
#include "base/database.h"
#include "base/projection.h"
#include "base/triangulation.h"

// 位姿估计模块
#include "estimators/pose.h"

// 工具模块：位图操作、杂项工具、PLY文件处理
#include "util/bitmap.h"
#include "util/misc.h"
#include "util/bitmap.h"  // 重复包含，但通常编译器会忽略重复包含
#include "util/ply.h"

// 特征提取与匹配模块（SIFT、匹配器、特征工具）
#include "feature/sift.h"
#include "feature/matching.h"
#include "feature/utils.h"

// 视觉词袋索引（视觉检索）
#include "visual_index/visual_index_liuye.h"

// OpenCV 库，仅用于示例中的图像处理和显示
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d/features2d.hpp>

// 枚举类型：设备类型定义
enum DeviceType
{
   SHADOW_CREATOR = 0,  // 阴影创建设备
   META_20 = 1          // Meta 2.0设备
};

// 计时器类：用于计算代码运行耗时
class TicToc{
  public:
    TicToc() {t1 = std::chrono::steady_clock::now();}  // 构造函数：记录起始时间
    ~TicToc(){}  // 析构函数

    // 获取当前时间与起始时间之间的差值（秒）
    double Now() {
      std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
      return std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1).count();
    }

  private:
    std::chrono::steady_clock::time_point t1;  // 起始时间点
};

// 判断文件是否存在的函数声明
bool is_file_exist(const std::string fileName);

// 自定义命名空间：Ulocal
namespace Ulocal{

// 表示从colmap voc树中检索的图像结果结构体
struct Retrieval {
  image_t image_id = kInvalidImageId;  // 当前图像的 ID，初始化为无效ID
  std::vector<retrieval::ImageScore> image_scores;  // 与其他图像的相似度得分
};

// 以下是注释掉的图像得分结构体定义
// struct ImageScore {
//   int image_id = -1;
//   float score = 0.0f;
// };

}

#endif // CONFIGURATION_LY_H_
