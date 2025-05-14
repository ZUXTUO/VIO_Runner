#include "MapSaver.h" 

 namespace BASTIAN 
 { 

 // 构造函数，初始化MapSaver对象
 // database_path: 数据库文件路径
 // recs_path: 重建数据文件路径
 // bViewer_: 是否启用查看器标志
 MapSaver::MapSaver(const std::string& database_path, const std::string& recs_path, 
                       bool bViewer_) 
 { 
     bViewer = bViewer_; // 设置查看器标志

     database = new Database(database_path); // 初始化数据库对象
     reconstruction = new Reconstruction; // 初始化重建对象
     reconstruction->ReadBinary(recs_path); // 从二进制文件读取重建数据

     // 读取相机信息 (这部分代码被注释掉了)
     // camera = reconstruction->Camera(1); 
     numCamera = reconstruction->NumCameras(); // 获取相机数量

     // 获取所有关键帧的ID
     framesIds.clear(); // 清空framesIds容器
     framesIds = reconstruction->RegImageIds(); // 获取已注册图像的ID列表

     // 输出稀疏特征地图信息
     std::cout << std::endl << " [地图信息] 稀疏特征地图  " << std::endl;  
     std::cout << "            相机数量 : "<< reconstruction->NumCameras() << std::endl; 
     std::cout << "            3D点数量 : "<< reconstruction->NumPoints3D() << std::endl; 
     std::cout << "            图像数量 : "<< reconstruction->NumImages() << std::endl << std::endl; 
 } 

 // 析构函数，释放资源
 MapSaver::~MapSaver() 
 { 
     if(database) // 如果数据库对象存在
         database->Close(); // 关闭数据库连接
 } 

 // 保存单个关键帧信息到文件
 // framesId: 要保存的关键帧ID
 // outputFile: 输出文件流
 void MapSaver::SaveKeyFrame(image_t framesId, std::ofstream &outputFile) 
 { 
     // 检查图像是否存在于数据库中
     if(!database->ExistsImage(framesId)){ 
         return; // 如果不存在，则直接返回
     } 

     Image& keyframe = reconstruction->Image(framesId); // 获取关键帧对象
     // 写入关键帧位姿
     { 
         Eigen::Matrix3x4d pose = keyframe.InverseProjectionMatrix(); // 获取逆投影矩阵作为位姿
         // 写入位姿矩阵的行数和列数
         int rows = pose.rows(), cols = pose.cols(); 
         outputFile.write((char*) (&rows), sizeof(int)); 
         outputFile.write((char*) (&cols), sizeof(int)); 
         // 写入位姿矩阵数据
         outputFile.write((char*) pose.data(), rows*cols*sizeof(double) ); 
     } 

     std::vector<FeatureKeypoint> keypoints = database->ReadKeypoints(framesId); // 从数据库读取关键点
     FeatureDescriptors descriptors = database->ReadDescriptors(framesId); // 从数据库读取描述子

     // 写入关键点数量
     int n_pt = keypoints.size(); 
     outputFile.write((char*)&n_pt, sizeof(int)); 

     // 写入描述子的列数 (每个描述子的维度)
     int cols = descriptors.cols(); 
     outputFile.write((char*) (&cols), sizeof(int)); 

     //std::cout << "  ==> " << n_pt << " 个关键点。\n"; // 原始英文输出: "  ==> " << n_pt << " key points.\n";

     int pt_with_3d = 0; // 记录具有3D对应的点的数量
     // 遍历所有关键点
     for(int i = 0; i < n_pt; i++){ 
         const Point2D& corr_point2D = keyframe.Point2D(i); // 获取当前关键点对应的2D点
       
         bool has_3d = true; // 标志位，指示当前2D点是否有对应的3D点
         // 检查2D点是否有对应的3D点
         if (!corr_point2D.HasPoint3D()) { 
             has_3d = false; // 如果没有，则设置标志位为false
             outputFile.write((char*)&has_3d, sizeof(bool)); // 写入标志位
             continue; // 继续下一个关键点的处理
         } 

         outputFile.write((char*)&has_3d, sizeof(bool)); // 写入标志位 (true)
         const Point3D& point3D = reconstruction->Point3D(corr_point2D.Point3DId()); // 获取对应的3D点
         // 写入3D点坐标
         { 
             Eigen::Vector3d pt3d = point3D.XYZ(); // 获取3D点坐标
             outputFile.write((char*) pt3d.data(), 3*sizeof(double) ); // 写入3D点坐标数据
             //std::cout << pt3d.transpose() << std::endl; 
         } 

         // 写入关键点信息 (x, y, 仿射参数)
         outputFile.write((char*)&keypoints[i].x, sizeof(float)); 
         outputFile.write((char*)&keypoints[i].y, sizeof(float)); 
         outputFile.write((char*)&keypoints[i].a11, sizeof(float)); 
         outputFile.write((char*)&keypoints[i].a12, sizeof(float)); 
         outputFile.write((char*)&keypoints[i].a21, sizeof(float)); 
         outputFile.write((char*)&keypoints[i].a22, sizeof(float)); 

         // 写入描述子
         FeatureDescriptor_ly descriptor_i = descriptors.row(i); // 获取当前关键点的描述子
         outputFile.write((char*)descriptor_i.data(), cols*sizeof(uint8_t)); // 写入描述子数据
         //if(pt_with_3d == 10) 
         //    std::cout << descriptors.row(i) << std::endl; 
         pt_with_3d++; // 增加具有3D对应的点的计数
     } 
 } 

 // 从文件加载单个关键帧信息
 // inputFile: 输入文件流
 void MapSaver::LoadKeyFrame(std::ifstream &inputFile) 
 { 
     // 读取关键帧位姿
     { 
         Eigen::Matrix3x4d pose; // 用于存储位姿
         // 读取位姿矩阵的行数和列数
         int rows, cols; 
         inputFile.read((char*) (&rows), sizeof(int)); 
         inputFile.read((char*) (&cols), sizeof(int)); 
         // 读取位姿矩阵数据
         inputFile.read((char*) pose.data(), rows*cols*sizeof(double) ); 
     } 

     // 读取关键点数量
     int n_pt; 
     inputFile.read((char*)&n_pt, sizeof(int)); 
   
     //std::cout << "  ==> " << n_pt << " 个关键点。\n"; // 原始英文输出: "  ==> " << n_pt << " key points.\n";

     // 读取描述子的列数 (每个描述子的维度)
     int cols; 
     inputFile.read((char*) (&cols), sizeof(int)); 

     std::vector<FeatureKeypoint> keypoints; // 存储读取的关键点
     std::vector<FeatureDescriptors> descriptors; // 存储读取的描述子 (这里类型可能是 FeatureDescriptor_ly，但代码用的是FeatureDescriptors，需注意)
     keypoints.reserve(n_pt); // 预分配空间
     descriptors.reserve(n_pt); // 预分配空间

     //int num_pt_with_3d = 0; // 记录具有3D对应的点的数量 (未使用)
     // 遍历所有关键点
     for(int i = 0; i < n_pt; i++){   
         bool has_3d; // 标志位，指示当前2D点是否有对应的3D点
         inputFile.read((char*)&has_3d, sizeof(bool)); // 读取标志位
         // 如果没有对应的3D点
         if (!has_3d) { 
             continue; // 继续下一个关键点的处理
         } 
         // 读取3D点坐标
         { 
             Eigen::Vector3d pt3d; // 用于存储3D点坐标
             inputFile.read((char*) pt3d.data(), 3*sizeof(double) ); // 读取3D点坐标数据
             //std::cout << pt3d.transpose() << std::endl; 
         } 

         FeatureKeypoint keypoint; // 用于存储当前关键点信息
         // 读取关键点信息 (x, y, 仿射参数)
         inputFile.read((char*)&keypoint.x, sizeof(float)); 
         inputFile.read((char*)&keypoint.y, sizeof(float)); 
         inputFile.read((char*)&keypoint.a11, sizeof(float)); 
         inputFile.read((char*)&keypoint.a12, sizeof(float)); 
         inputFile.read((char*)&keypoint.a21, sizeof(float)); 
         inputFile.read((char*)&keypoint.a22, sizeof(float)); 
         keypoints.push_back(keypoint); // 将读取的关键点添加到容器

         // 读取描述子
         FeatureDescriptors descriptor_i; // 用于存储当前描述子 (这里应该是 FeatureDescriptor_ly 类型或者调整大小的 FeatureDescriptors)
         descriptor_i.resize(1, cols); // 调整大小以存储一个描述子
         inputFile.read((char*)descriptor_i.data(), cols*sizeof(uint8_t)); // 读取描述子数据
         descriptors.push_back(descriptor_i); // 将读取的描述子添加到容器
     } 

     //std::cout << descriptors[10] << std::endl; // 调试输出 (注释掉了)

     // 这部分代码被注释掉了，原意可能是整体读取所有描述子
     /* { 
          int rows, cols; 
          inputFile.read((char*) (&rows), sizeof(int)); 
          inputFile.read((char*) (&cols), sizeof(int)); 
          descriptors.resize(rows, cols); 
          inputFile.read((char*) descriptors.data(), rows*cols*sizeof(uint8_t)); 
      } 
      //std::cout << descriptors.block(10,0,1,20) << std::endl; 
     */ 
 } 

 // 保存整个地图到二进制文件
 // save_path: 保存文件的路径
 // 返回值: bool, 表示保存是否成功
 bool MapSaver::SaveMap(const std::string& save_path) 
 { 
     // 1. 关键帧数量
     // 2. 关键帧 -> 2D特征点
     //             -> 特征描述子
     //             -> 世界坐标系位姿 3D
     //             -> 相机ID (代码中没有显式保存相机ID，但通过位姿和关键点信息间接关联)
     std::ofstream outFile (save_path, std::ios::binary); // 以二进制模式打开输出文件流
     // 检查文件是否成功打开
     if (!outFile){ 
         std::cout << " [保存地图] 无法打开输出文件夹: " << save_path << std::endl; // 原始英文输出: " [SAVE MAP] cannot open output folder: "
         return false; // 打开失败，返回false
     } 
     std::cout << " [保存地图] 地图保存至 : " << save_path << std::endl; // 原始英文输出: " [SAVE MAP] save map to : "

     // 写入关键帧数量
     int n_kf = framesIds.size(); 
     outFile.write((char*)&n_kf, sizeof(int)); 

     // 逐个写入关键帧信息
     for(image_t framdId : framesIds){ 
         SaveKeyFrame(framdId, outFile); //调用SaveKeyFrame保存单个关键帧
         //break; // (调试用，注释掉了)
     } 

     outFile.close(); // 关闭文件流
     return true; // 保存成功
 } 

 // 从二进制文件加载整个地图
 // load_path: 加载文件的路径
 // 返回值: bool, 表示加载是否成功
 bool MapSaver::LoadMap(const std::string& load_path) 
 { 
     std::ifstream inFile(load_path, std::ios::in|std::ios::binary); // 以二进制读模式打开输入文件流
     // 检查文件是否成功打开
     if (!inFile){ 
         std::cout << " [加载地图] 无法打开输入文件: " << load_path << std::endl; // 原始英文输出: " [LOAD MAP] cannot open output folder: " (应为 input file)
         return false; // 打开失败，返回false
     } 
     std::cout << " [加载地图] 从以下路径加载地图 : " << load_path << std::endl; // 原始英文输出: " [LOAD MAP] load map from : "
   
     // 读取关键帧数量
     int n_kf; 
     inFile.read((char*)&n_kf, sizeof(int)); 

     std::cout << "  ==> " << n_kf << " 个关键帧。\n"; // 原始英文输出: "  ==> " << n_kf << " keyframes.\n";

     // 逐个加载关键帧信息
     for(int i = 0; i < n_kf ; i++){ 
         LoadKeyFrame(inFile); // 调用LoadKeyFrame加载单个关键帧
         //break; // (调试用，注释掉了)
     } 

     inFile.close(); // 关闭文件流
     return true; // 加载成功
 } 

 // 将关键帧信息保存为文本文件 (主要保存图像路径和位姿)
 // save_path: 保存文件的路径
 void MapSaver::SaveKeyframesTxt(const std::string& save_path) 
 { 
     std::cout << "  ==> 保存关键帧到文本文件 " << save_path << std::endl; // 原始英文输出: "  ==> save keyframes to "
     std::ofstream runtimefile; // 定义输出文件流
     runtimefile.open(save_path.c_str()); // 打开文件
     runtimefile << std::fixed; // 设置浮点数输出格式为定点表示

     // 遍历所有关键帧ID
     for(image_t idx : framesIds){ 
         // 检查图像是否存在于数据库中
         if(!database->ExistsImage(idx)){ 
             continue; // 如果不存在，则跳过
         } 

         std::string imagePath = reconstruction->Image(idx).Name(); // 获取图像名称(路径)
         Eigen::Matrix3x4d pose = reconstruction->Image(idx).InverseProjectionMatrix(); // 获取关键帧位姿

         runtimefile << imagePath << std::endl; // 写入图像路径
         // 写入位姿矩阵 (3x4)
         for(int i = 0; i < 3; i++) 
             for(int j =0; j < 4; j++) 
                 runtimefile << " " << pose(i,j); 
         runtimefile << std::endl; 
     } 

     runtimefile.close(); // 关闭文件流
 } 
 }