/**
 * @file updater.cc
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @date 2024-09-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <update_ctrl.hpp>
#include <cxxopts.hpp>

using namespace update_np;

int main(int argc, char const *argv[]) {
    std::string mcs_file_path = "";
    std::string prm_file_path = "";
    std::string slot_alias = "";
    int timeout = 4;

    std::string description = "Fpga mcs updater\n";

#ifdef VERSION
    description += std::string(VERSION) + " ";
#endif
#ifdef STAMP
    char stamp[20], format[] = "%Y-%m-%d %H:%M:%S";
    double timeval;
    sscanf(STAMP, "%lf", &timeval);
    time_t tt = static_cast<time_t>(static_cast<unsigned int>(timeval));
    strftime(stamp, sizeof(stamp), format, localtime(&tt));
    description += std::string(stamp) + " ";
#endif
#ifdef HASH
    description += std::string(HASH) + "\n";
#endif
    cxxopts::Options options("updater", description);
    try {
        options.add_options()(
            "m,mcs_file", "Target mcs file", cxxopts::value<std::string>())(
            "p,prm_file", "Target prm file", cxxopts::value<std::string>())(
            "s,slot", "Slot alias name", cxxopts::value<std::string>())(
            "t,timeout", "Operations timeout(seconds) - default: 4 s\nMust be greater than 0.", cxxopts::value<int>())(
            "h,help", "Print help");
        options.show_positional_help();

        auto parsers = options.parse(argc, argv);
        if (parsers.count("help") || !parsers.count("mcs_file") || !parsers.count("prm_file") || !parsers.count("slot")) {
            printf("%s\n", options.help().c_str());
            return 0;
        }

        if (parsers.count("mcs_file"))
            mcs_file_path = parsers["mcs_file"].as<std::string>();

        if (parsers.count("prm_file"))
            prm_file_path = parsers["prm_file"].as<std::string>();

        if (parsers.count("slot"))
            slot_alias = parsers["slot"].as<std::string>();

        if (parsers.count("timeout")) {
            timeout = parsers["timeout"].as<int>();
            if (timeout <= 0)
                throw cxxopts::exceptions::exception("timeout " + std::to_string(timeout) + " is too small");
        }
    } catch (const cxxopts::exceptions::exception &e) {
        printf("%s\n", e.what());
        printf("%s\n", options.help().c_str());
        return 1;
    }

    update_ctrl ctrl;

    if (!ctrl.init(slot_alias, timeout)) {
        logf_err("initialize failed.\n");
        return 1;
    }

    if (!ctrl.update(mcs_file_path, prm_file_path)) {
        logf_err("update %s failed.\n", mcs_file_path.c_str());
        return 1;
    } else {
        logf_info("update %s success.\n", mcs_file_path.c_str());
        return 0;
    }
}
