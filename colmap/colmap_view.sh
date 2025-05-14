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

echo "=== 使用 Meshlab 打开稠密点云 ==="
meshlab "${DENSE_DIR}/fused.ply" || echo "请先安装 meshlab：sudo apt install meshlab"

echo "=== 全部流程执行完毕 ==="
