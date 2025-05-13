#include "UlocalizationPlugins.h"
#include <map>

// 全局变量：存储Ulocalization对象的映射表
//Ulocal::Ulocalization* ulocalization;
std::map<int, Ulocal::Ulocalization*> mUlocalization;

/**
 * 获取映射表中元素数量
 * @return 返回映射表大小
 */
extern "C" int Internal_Map_Length()
{
    std::cout << "[日志] 获取映射表大小: " << mUlocalization.size() << std::endl;
    return mUlocalization.size();
}

/**
 * 清空映射表
 * @return 成功返回1
 */
extern "C" int Internal_Clear_Map()
{
    std::cout << "[日志] 开始清空映射表，当前大小: " << mUlocalization.size() << std::endl;
    mUlocalization.erase(mUlocalization.begin(), mUlocalization.end());
    std::cout << "[日志] 映射表已清空" << std::endl;
    return 1;
}

/**
 * 检查key是否存在于映射表中
 * @param key 要检查的键
 * @return 存在返回1，不存在返回-1
 */
extern "C" int Internal_Check_Key_Existance(const int key)
{
    std::cout << "[日志] 检查键 " << key << " 是否存在" << std::endl;
    std::map<int, Ulocal::Ulocalization*>::iterator iter;
    iter = mUlocalization.find(key);
    if(!(iter == mUlocalization.end())){
        std::cout << "[日志] 键 " << key << " 存在" << std::endl;
        return 1;
    }
    std::cout << "[日志] 键 " << key << " 不存在" << std::endl;
    return -1;
}

/**
 * 初始化Ulocalization对象并添加到映射表
 * @param database_path 数据库路径
 * @param reconstruction_path 重建数据路径
 * @param vocIndex_path 词汇索引路径
 * @param key 映射表键值
 * @return 成功返回1，失败返回-1
 */
extern "C" int Internal_Init_Ulocalization_Map(const char *database_path, const char* reconstruction_path,
                                const char* vocIndex_path, const int key)
{
    std::cout << "[日志] 开始初始化Ulocalization对象，键值: " << key << std::endl;

    // 检查键是否已存在
    std::map<int, Ulocal::Ulocalization*>::iterator iter;
    iter = mUlocalization.find(key);
    if(!(iter == mUlocalization.end())){
        std::cout << "[错误] 初始化失败：键 " << key << " 已存在！" << std::endl;
        return -1;
    }

    std::cout << "[日志] 为键 " << key << " 创建Ulocalization对象" << std::endl;

    // 转换路径字符串
    std::string pathToDatabase(database_path); 
    std::string pathToReconstruction(reconstruction_path);
    std::string pathToVocIndex(vocIndex_path);

    std::cout << "[日志] 路径信息：" << std::endl;
    std::cout << "  数据库路径: " << pathToDatabase << std::endl;
    std::cout << "  重建数据路径: " << pathToReconstruction << std::endl;
    std::cout << "  词汇索引路径: " << pathToVocIndex << std::endl;

    // 检查文件是否存在
    if (is_file_exist(pathToDatabase) && is_file_exist(pathToReconstruction) 
                         && is_file_exist(pathToVocIndex)) {
        // 创建Ulocalization对象
        std::cout << "[日志] 创建Ulocalization对象..." << std::endl;
        Ulocal::Ulocalization* ulocalization = new Ulocal::Ulocalization(pathToDatabase, pathToReconstruction);

        // 加载词汇树用于匹配候选帧
        std::cout << "[日志] 加载词汇树..." << std::endl;
        ulocalization->LoadVocTree(pathToVocIndex);

        // 创建GPU特征匹配器
        std::cout << "[日志] 创建GPU特征匹配器..." << std::endl;
        ulocalization->CreateGPUMatch();

        // 将对象添加到映射表
        mUlocalization.insert(std::map<int, Ulocal::Ulocalization*>::value_type(key, ulocalization));

        std::cout << "[日志] 初始化完成，键 " << key << " 已添加到映射表" << std::endl;
        return 1;
    }
    
    std::cout << "[错误] 初始化失败：文件路径不存在" << std::endl;
    return -1;
}

/**
 * 从映射表中销毁指定键的Ulocalization对象
 * @param key 要销毁的对象的键值
 * @return 成功返回1，失败返回-1
 */
extern "C" int Internal_Destroy_Ulocalization_Map(const int key)
{
    std::cout << "[日志] 尝试销毁键 " << key << " 的Ulocalization对象" << std::endl;
    
    std::map<int, Ulocal::Ulocalization*>::iterator iter;
    iter = mUlocalization.find(key);
    if(iter == mUlocalization.end()){
        std::cout << "[错误] 销毁失败：未找到键 " << key << std::endl;
        return -1;
    }
    
    mUlocalization.erase(iter);
    std::cout << "[日志] 键 " << key << " 的对象已成功销毁" << std::endl;
    return 1;
}

/**
 * 使用指定键的Ulocalization对象进行图像跟踪定位
 * @param inputImage 输入图像数据
 * @param bufferLength 图像数据长度
 * @param focus_length 焦距
 * @param deviceType 设备类型
 * @param key 映射表键值
 * @return 位姿矩阵（浮点数数组）
 */
extern "C" float* Internal_Track_Ulocalization_Map(unsigned char* inputImage, int bufferLength,
                             double focus_length, int deviceType, const int key)
{
    std::cout << "[日志] 开始使用键 " << key << " 的对象进行图像跟踪" << std::endl;
    
    // 获取对象
    Ulocal::Ulocalization* ulocalization;
    std::map<int, Ulocal::Ulocalization*>::iterator iter;
    iter = mUlocalization.find(key);
    if(iter == mUlocalization.end()){
        std::cout << "[错误] 跟踪失败：未找到键 " << key << std::endl;
        return NULL;
    }
    
    ulocalization = iter->second;

    std::cout << "[日志] 图像缓冲区长度: " << bufferLength << ", 焦距: " << focus_length << std::endl;
    
    // 转换字节为cv::Mat
    cv::Mat inputImageMat;
    if(deviceType == SHADOW_CREATOR) {
        std::cout << "[日志] 设备类型：SHADOW_CREATOR，进行图像解码" << std::endl;
        inputImageMat = cv::imdecode(cv::Mat(1, bufferLength, CV_8UC3, inputImage), 1);
    }
    else if(deviceType == META_20) {
        std::cout << "[日志] 设备类型：META_20，直接使用原始数据" << std::endl;
        inputImageMat = cv::Mat(480, 640, CV_8UC1, inputImage);
    }
    else {
        std::cout << "[错误] 不支持的设备类型: " << deviceType << std::endl;
        return NULL;
    }

    int width = inputImageMat.cols;
    int height = inputImageMat.rows;
    std::cout << "[日志] 图像尺寸: " << width << "x" << height << std::endl;

    // 计时开始
    TicToc tictoc;

    cv::Mat mTcw = cv::Mat::zeros(0, 0, CV_32F);

    // GPU特征提取
    std::cout << "[日志] 开始GPU特征提取..." << std::endl;
    FeatureKeypoints* keypoints1 = new FeatureKeypoints;
    FeatureDescriptors* descriptors1 = new FeatureDescriptors;
    cv::imwrite(std::to_string(key) + "temp.png", inputImageMat);
    if(!Ulocal::SIFTextractionTestGPU(std::to_string(key) + "temp.png", keypoints1, descriptors1,
                                      width, height)){
        std::cout << "[错误] 特征提取失败" << std::endl;        
    }
    else {
        std::cout << "[日志] 特征提取成功，提取了 " << keypoints1->size() << " 个特征点" << std::endl;
    }

    // 词汇树检索
    std::cout << "[日志] 使用词汇树进行检索..." << std::endl;
    Ulocal::Retrieval retrieval = ulocalization->MatchVocTreeReturnAll(*keypoints1, *descriptors1);
    std::cout << "[日志] 检索到 " << retrieval.image_scores.size() << " 个候选图像" << std::endl;

    // 遍历候选图像进行位姿估计
    int tmp = 1; 
    int maxTrival = 10;
    cv::Mat output;
    bool ifSuccess = false;
    for(auto image_score : retrieval.image_scores){
        std::cout << "[日志] 测试第 " << tmp << " 个候选图像 (ID: " << image_score.image_id << ")" << std::endl;
        
        // GPU特征匹配
        FeatureMatches matches = ulocalization->MatchWithImageGPU(image_score.image_id, *descriptors1);
        std::cout << "[日志] 匹配到 " << matches.size() << " 对特征点" << std::endl;
        
        // 位姿估计
        std::cout << "[日志] 进行位姿估计..." << std::endl;
        output = ulocalization->PoseEstimationCvMat(*keypoints1, image_score.image_id, matches, 
                                                  focus_length, width, height);

        if(output.at<float>(3,3) > 0.5){
            std::cout << "[日志] 位姿估计成功" << std::endl;
            ifSuccess = true;
            break;
        }
        
        std::cout << "[日志] 位姿估计失败，尝试下一个候选" << std::endl;
        tmp++;
        
        if(tmp > maxTrival){
            std::cout << "[日志] 已达到最大尝试次数: " << maxTrival << std::endl;
            std::cout << "[失败] 定位失败" << std::endl;
            ifSuccess = false;
            break;
        }
    }

    // 输出总耗时
    std::cout << "[日志] 总耗时: " << tictoc.Now() << " 秒" << std::endl;

    // 转换位姿矩阵为浮点数数组
    std::cout << "[日志] 转换位姿矩阵为浮点数数组" << std::endl;
    float* mModelview_matrix = CvMatToFloatArray(output, ifSuccess);
    
    if (ifSuccess) {
        std::cout << "[日志] 返回成功的位姿结果" << std::endl;
    } else {
        std::cout << "[日志] 返回零矩阵（定位失败）" << std::endl;
    }

    return mModelview_matrix;
}

/**
 * 将OpenCV矩阵转换为浮点数数组（用于返回位姿矩阵）
 * @param cvMatInput 输入的OpenCV矩阵
 * @param ifSuccess 是否成功标志
 * @return 转换后的浮点数数组（4x4矩阵）
 */
extern "C" float* CvMatToFloatArray(cv::Mat cvMatInput, bool ifSuccess)
{
    std::cout << "[日志] 开始转换OpenCV矩阵为浮点数数组" << std::endl;
    
    float* mModelview_matrix = new float[16];
    if (ifSuccess)
    {
        std::cout << "[日志] 转换成功的位姿矩阵" << std::endl;
        cv::Mat Rcw(3, 3, CV_32F);
        cv::Mat tcw(3, 1, CV_32F);

        // 提取旋转矩阵和平移向量
        Rcw = cvMatInput.rowRange(0, 3).colRange(0, 3).t();
        tcw = - Rcw * cvMatInput.rowRange(0, 3).col(3);

        // 填充4x4矩阵（列主序）
        mModelview_matrix[0] = Rcw.at<float>(0, 0);
        mModelview_matrix[1] = Rcw.at<float>(1, 0);
        mModelview_matrix[2] = Rcw.at<float>(2, 0);
        mModelview_matrix[3] = 0.0;

        mModelview_matrix[4] = Rcw.at<float>(0, 1);
        mModelview_matrix[5] = Rcw.at<float>(1, 1);
        mModelview_matrix[6] = Rcw.at<float>(2, 1);
        mModelview_matrix[7] = 0.0;

        mModelview_matrix[8] = Rcw.at<float>(0, 2);
        mModelview_matrix[9] = Rcw.at<float>(1, 2);
        mModelview_matrix[10] = Rcw.at<float>(2, 2);
        mModelview_matrix[11] = 0.0;

        mModelview_matrix[12] = tcw.at<float>(0);
        mModelview_matrix[13] = tcw.at<float>(1);
        mModelview_matrix[14] = tcw.at<float>(2);
        mModelview_matrix[15] = 1.0;

        std::cout << "[日志] 矩阵转换完成，返回结果" << std::endl;
        return mModelview_matrix;
    } else {
        std::cout << "[日志] 定位失败，返回零矩阵" << std::endl;
        int i = 0;
        for (i = 0; i < 16; i++) {
            mModelview_matrix[i] = 0;
        }
        return mModelview_matrix;
    }
}