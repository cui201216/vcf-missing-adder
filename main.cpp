#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <unistd.h>  // 用于getopt

void replaceGenotypeWithMissing(const std::string& input_vcf, const std::string& output_vcf, float missing_rate, size_t buffer_size) {

    std::mt19937 rng(19991220);  // 使用该种子初始化随机数生成器
    std::uniform_real_distribution<float> dist(0.0, 1.0);
    std::ifstream infile(input_vcf);
    std::ofstream outfile(output_vcf);
    if (!infile.is_open()) {
        std::cerr << "无法打开输入文件: " << input_vcf << std::endl;
        return;
    }
    if (!outfile.is_open()) {
        std::cerr << "无法打开输出文件: " << output_vcf << std::endl;
        return;
    }
    // 设置缓冲区大小
    infile.rdbuf()->pubsetbuf(nullptr, buffer_size);
    outfile.rdbuf()->pubsetbuf(nullptr, buffer_size);

    std::string line;
    while (std::getline(infile, line)) {
        // 处理注释行（以#开头的行）
        if (line[0] == '#') {
            outfile << line << std::endl;
        } else {
            // 处理数据行
            std::stringstream ss(line);
            std::string column;
            std::vector<std::string> columns;

            // 将行按制表符分割成列
            while (std::getline(ss, column, '\t')) {
                columns.push_back(column);
            }

            // 从第10列（索引为9）开始处理基因型列
            for (size_t i = 9; i < columns.size(); ++i) {
                // 随机决定是否将该基因型替换为./.
                if (dist(rng) < missing_rate) {
                    columns[i] = "./.";
                }
            }
            // 将修改后的行写入输出文件
            for (size_t i = 0; i < columns.size(); ++i) {
                if (i != 0) {
                    outfile << "\t";  // 保持制表符分隔
                }
                outfile << columns[i];
            }
            outfile << std::endl;
        }
    }

    infile.close();
    outfile.close();
}

int main(int argc, char* argv[]) {
    std::string input_vcf;
    std::string output_vcf;
    float missing_rate = 0.01f;  // 默认缺失率为1%
    size_t buffer_size = 16 * 1024 * 1024;  // 默认缓冲区为16MB

    // 解析命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "i:o:r:b:")) != -1) {
        switch (opt) {
            case 'i':  // 输入文件
                input_vcf = optarg;
                break;
            case 'o':  // 输出文件
                output_vcf = optarg;
                break;
            case 'r':  // 缺失率
                missing_rate = std::stof(optarg);
                break;
            case 'b':  // 缓冲区大小（单位MB）
                buffer_size = std::stoul(optarg) * 1024 * 1024;  // 将输入的MB转换为字节
                break;
            default:
                std::cerr << "用法: " << argv[0] << " -i <输入文件> -o <输出文件> -r <缺失率> -b <缓冲区大小(单位MB)>" << std::endl;
                return 1;
        }
    }

    if (input_vcf.empty() || output_vcf.empty()) {
        std::cerr << "输入文件和输出文件必须指定。" << std::endl;
        return 1;
    }
    // 调用函数处理VCF文件
    replaceGenotypeWithMissing(input_vcf, output_vcf, missing_rate, buffer_size);
    return 0;
}
