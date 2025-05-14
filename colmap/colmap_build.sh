#!/usr/bin/env bash
set -euo pipefail

# 并行线程数（可选，根据需要调整）
JOBS=$(nproc)

# 项目根目录
PROJECT_DIR="./"
DB_PATH="$PROJECT_DIR/database.db"
IMAGE_DIR="$PROJECT_DIR/images"
SPARSE_DIR="$PROJECT_DIR/sparse"
DENSE_DIR="$PROJECT_DIR/dense"

# 确保目录存在
mkdir -p "${IMAGE_DIR}"
mkdir -p "${SPARSE_DIR}"
mkdir -p "${DENSE_DIR}"

echo "=== 1. 初始化数据库 ==="
colmap feature_extractor \
    --database_path "${DB_PATH}" \
    --image_path "${IMAGE_DIR}"

echo "=== 2. 匹配图像特征 ==="
colmap exhaustive_matcher \
    --database_path "${DB_PATH}"

echo "=== 3. 稀疏重建 ==="
colmap mapper \
    --database_path "${DB_PATH}" \
    --image_path "${IMAGE_DIR}" \
    --output_path "${SPARSE_DIR}"

echo "=== 4. 图像去畸变（稠密重建前处理）=== "
colmap image_undistorter \
    --image_path "${IMAGE_DIR}" \
    --input_path "${SPARSE_DIR}/0" \
    --output_path "${DENSE_DIR}" \
    --output_type COLMAP \
    --max_image_size 2000

echo "=== 5. 深度图构建 === "
colmap patch_match_stereo \
    --workspace_path "${DENSE_DIR}" \
    --workspace_format COLMAP \
    --PatchMatchStereo.geom_consistency true

echo "=== 6. 生成稠密点云 ==="
colmap stereo_fusion \
    --workspace_path "${DENSE_DIR}" \
    --workspace_format COLMAP \
    --input_type geometric \
    --output_path "${DENSE_DIR}/fused.ply"

echo "=== 7. 将稀疏模型导出为 PLY 查看 ==="
colmap model_converter \
    --input_path "${SPARSE_DIR}/0" \
    --output_path "${SPARSE_DIR}/0.ply" \
    --output_type PLY

echo "=== 8. 使用 Meshlab 打开稠密点云 ==="
meshlab "${DENSE_DIR}/fused.ply" || echo "请先安装 meshlab：sudo apt install meshlab"

echo "=== 全部流程执行完毕 ==="
