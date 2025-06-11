/**
 * @file mcs_parser_test.cc
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief 
 * @date 2024-09-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>

#include <cxxopts.hpp>
#include <mcs_parser.hpp>

using namespace tools_np;

int main(int argc, char const *argv[]) {
    std::string check_mcs_file = "";
    std::string check_prm_file = "";
    try {
        cxxopts::Options options("mcs_parser_test", "Check a raw file is valid mcs file");

        options.add_options()(
            "m,mcs_file", "Target check mcs file", cxxopts::value<std::string>())(
            "p,prm_file", "Target check prm file", cxxopts::value<std::string>())(
            "h,help", "Print help");
        options.show_positional_help();

        auto parsers = options.parse(argc, argv);
        if (parsers.count("help") || !parsers.count("mcs_file") || !parsers.count("prm_file")) {
            printf("%s\n", options.help().c_str());
            return 0;
        }
        if (parsers.count("mcs_file"))
            check_mcs_file = parsers["mcs_file"].as<std::string>();
        if (parsers.count("prm_file"))
            check_prm_file = parsers["prm_file"].as<std::string>();
    } catch (const cxxopts::exceptions::exception &e) {
        printf("%s\n", e.what());
        return 1;
    }

    auto mcs_result = check_valid(check_mcs_file);
    printf("check result: %s, bytes: %ld\n", mcs_result.valid ? "true" : "false", mcs_result.bytes);

    auto prm_result = parse_prm(check_prm_file);
    if (prm_result.size() <= 0) {
        printf("unrecognized prm file\n");
        return 1;
    }

    printf("================ start get mcs fragment.\n");

    if (mcs_result.valid) {
        MCSFragment fragment;
        std::ifstream file(check_mcs_file);
        if (!file.is_open())
            return 1;

        size_t index = 0;
        while (index < prm_result.size()) {
            int cnt = 0;
            auto cur_partition = prm_result[index];
            printf("================ get partition %ld.\n", index);
            cur_partition.print();
            uint32_t partition_size = cur_partition.size;
            uint32_t partition_addr = cur_partition.addr_start;
            while (!file.eof() && partition_size > 0) {
                size_t fragment_size = partition_size > sizeof(fragment.buffer) ? sizeof(fragment.buffer) : partition_size;
                auto ret = get_mcs_fragment(file, fragment.buffer, sizeof(fragment.buffer), fragment_size);
                if (ret.second != -1) {
                    fragment.addr = ret.first;
                    fragment.bytes = ret.second;
                    partition_size -= ret.second;
                    printf("fragment: %d addr: %08x, ext_linear_addr: %04x size: %ld, left bytes: %u\n", cnt++, partition_addr, ret.first, ret.second, partition_size);
                    partition_addr += fragment.bytes;
                } else {
                    printf("fragment: %d ret: %ld\n", cnt++, ret.second);
                    break;
                }
            }
            index++;
        }

        file.close();
    }

    printf("================ finish get mcs fragment.\n");

    return 0;
}
