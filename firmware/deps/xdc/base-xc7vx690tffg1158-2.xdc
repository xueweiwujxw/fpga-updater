# Device Type: xc7vx690tffg1158-2
# Flash Type: mt25qu01g-spi-x1_x2_x4

create_clock -name pcie_sys_clk -period 10 [get_ports pcie_sys_clk_p]
set_property PACKAGE_PIN AB6 [get_ports pcie_sys_clk_p]
set_property PACKAGE_PIN AB5 [get_ports pcie_sys_clk_n]

set_false_path -from [get_ports pcie_sys_rst_n]
set_property PULLUP true [get_ports pcie_sys_rst_n]
set_property IOSTANDARD LVCMOS18 [get_ports pcie_sys_rst_n]
set_property PACKAGE_PIN C13 [get_ports pcie_sys_rst_n]

set_property PACKAGE_PIN AE3 [get_ports {pci_exp_rxn[0]}]
set_property PACKAGE_PIN AE4 [get_ports {pci_exp_rxp[0]}]
set_property PACKAGE_PIN AD1 [get_ports {pci_exp_txn[0]}]
set_property PACKAGE_PIN AD2 [get_ports {pci_exp_txp[0]}]

set_property PACKAGE_PIN AF5 [get_ports {pci_exp_rxn[1]}]
set_property PACKAGE_PIN AF6 [get_ports {pci_exp_rxp[1]}]
set_property PACKAGE_PIN AF1 [get_ports {pci_exp_txn[1]}]
set_property PACKAGE_PIN AF2 [get_ports {pci_exp_txp[1]}]

set_property PACKAGE_PIN AG3 [get_ports {pci_exp_rxn[2]}]
set_property PACKAGE_PIN AG4 [get_ports {pci_exp_rxp[2]}]
set_property PACKAGE_PIN AH1 [get_ports {pci_exp_txn[2]}]
set_property PACKAGE_PIN AH2 [get_ports {pci_exp_txp[2]}]

set_property PACKAGE_PIN AH5 [get_ports {pci_exp_rxn[3]}]
set_property PACKAGE_PIN AH6 [get_ports {pci_exp_rxp[3]}]
set_property PACKAGE_PIN AJ3 [get_ports {pci_exp_txn[3]}]
set_property PACKAGE_PIN AJ4 [get_ports {pci_exp_txp[3]}]

set_property PACKAGE_PIN AK5 [get_ports {pci_exp_rxn[4]}]
set_property PACKAGE_PIN AK6 [get_ports {pci_exp_rxp[4]}]
set_property PACKAGE_PIN AK1 [get_ports {pci_exp_txn[4]}]
set_property PACKAGE_PIN AK2 [get_ports {pci_exp_txp[4]}]

set_property PACKAGE_PIN AL3 [get_ports {pci_exp_rxn[5]}]
set_property PACKAGE_PIN AL4 [get_ports {pci_exp_rxp[5]}]
set_property PACKAGE_PIN AM1 [get_ports {pci_exp_txn[5]}]
set_property PACKAGE_PIN AM2 [get_ports {pci_exp_txp[5]}]

set_property PACKAGE_PIN AM5 [get_ports {pci_exp_rxn[6]}]
set_property PACKAGE_PIN AM6 [get_ports {pci_exp_rxp[6]}]
set_property PACKAGE_PIN AN3 [get_ports {pci_exp_txn[6]}]
set_property PACKAGE_PIN AN4 [get_ports {pci_exp_txp[6]}]

set_property PACKAGE_PIN AP5 [get_ports {pci_exp_rxn[7]}]
set_property PACKAGE_PIN AP6 [get_ports {pci_exp_rxp[7]}]
set_property PACKAGE_PIN AP1 [get_ports {pci_exp_txn[7]}]
set_property PACKAGE_PIN AP2 [get_ports {pci_exp_txp[7]}]

set_property PACKAGE_PIN C24 [get_ports {m_spi_flash_ss[0]}]
set_property PACKAGE_PIN A23 [get_ports m_spi_flash_mosi]
set_property PACKAGE_PIN A24 [get_ports m_spi_flash_miso]
set_property IOSTANDARD LVCMOS18 [get_ports {m_spi_flash_ss[0]}]
set_property IOSTANDARD LVCMOS18 [get_ports m_spi_flash_mosi]
set_property IOSTANDARD LVCMOS18 [get_ports m_spi_flash_miso]

set_property PACKAGE_PIN AE11 [get_ports fan_pwm]
set_property IOSTANDARD LVCMOS18 [get_ports fan_pwm]

set_property C_USER_SCAN_CHAIN 1 [get_debug_cores dbg_hub]
connect_debug_port dbg_hub/clk [get_nets xdma_inst/axi_aclk]

set_property BITSTREAM.CONFIG.SPI_FALL_EDGE YES [current_design]
set_property BITSTREAM.CONFIG.CONFIGRATE 66 [current_design]
set_property CONFIG_VOLTAGE 1.8 [current_design]
set_property CFGBVS GND [current_design]
set_property BITSTREAM.CONFIG.SPI_32BIT_ADDR YES [current_design]
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]
set_property CONFIG_MODE SPIx4 [current_design]
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
