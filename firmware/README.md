## FPGA updater PCIe Template

### Build

```shell
sbt "runMain updater.top.TopGen --hash=$(git rev-parse --short=8 HEAD) --stamp=$(date +%s) --deviceType=xc7vx690tffg1158-2 --pcieLocation=X0Y1"
source {Your Xilinx Installation Path}/Xilinx/Vivado/2021.1/settings64.sh
vivado -mode batch -source generate.tcl -nojournal -nolog -notrace -tclargs spi 4 66 1024 $(expr 28 \* 1024 \* 1024) xc7vx690tffg1158-2
vivado -mode batch -source compile.tcl -nojournal -nolog build/top/fpga_update.xpr
vivado -mode batch -source deps/tcls/multiboot_address_table.tcl -nojournal -nolog -notrace -tclargs spi 4 66 1024 $(expr 28 \* 1024 \* 1024) build/top/fpga_update.runs/impl_1/Top.bit
```
