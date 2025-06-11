/**
 * @file update_ctrl.hpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @date 2024-09-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <xdma_engine.hpp>
#include <micron_spi_flash.hpp>

namespace update_np
{
    class update_ctrl
    {
    private:
        xdma_np::xdma_engine xdma;
        micronnp::micron_spi_flash spi_flash;

    public:
        update_ctrl();
        ~update_ctrl();
        bool init(std::string slot_alias, int timeout);
        bool update(std::string mcs_file_path, std::string prm_file_path);
    };

} // namespace update_np
