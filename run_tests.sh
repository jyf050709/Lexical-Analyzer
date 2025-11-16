#!/bin/bash

# ToyC 语法分析器测试脚本

echo "编译语法分析器..."
g++ -std=c++11 -Wall -o compiler parser.cpp
if [ $? -ne 0 ]; then
    echo "编译失败！"
    exit 1
fi
echo "编译成功！"
echo ""

# 测试函数
test_file() {
    local file=$1
    local description=$2
    echo "=== $description ==="
    echo "测试文件: $file"
    echo "--- 文件内容 ---"
    cat "$file"
    echo "--- 输出结果 ---"
    ./compiler < "$file"
    echo ""
}

# 运行测试
test_file "test_correct.c" "测试1: 简单正确代码"
test_file "test_correct_complex.c" "测试2: 复杂正确代码（递归）"
test_file "test_error.c" "测试3: 多种语法错误"
test_file "test_multi_error.c" "测试4: 多个函数的错误"
test_file "test1.c" "测试5: 函数参数缺少右括号"

echo "所有测试完成！"
