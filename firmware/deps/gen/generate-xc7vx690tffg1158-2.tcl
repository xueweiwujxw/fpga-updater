# create project
create_project fpga_update build/top/ -part xc7vx690tffg1158-2

# ip generate
source generated/top/generate.tcl

# add files
import_files -fileset constrs_1 generated/top/constraint.xdc
set constraintId [open build/top/fpga_update.srcs/constrs_1/imports/top/constraint.xdc "a"]
puts $constraintId "set_property BITSTREAM.CONFIG.CONFIGFALLBACK ENABLE \[current_design\]"
puts $constraintId "set_property BITSTREAM.CONFIG.NEXT_CONFIG_ADDR [hexpr $timer1_address] \[current_design\]"
close $constraintId
import_files -flat -norecurse generated/top/

# update and set top
update_compile_order -fileset sources_1
set_property top Top [current_fileset]
set_property STEPS.POST_ROUTE_PHYS_OPT_DESIGN.IS_ENABLED true [get_runs impl_1]
