source deps/tcls/functions.tcl

set options [GetOptions $argc $argv]
if {$options == -1} {
    exit -1
}

proc Prepare {} {
    global options
    set type [lindex $options 0]
    set width [lindex $options 1]
    set freq_mhz [lindex $options 2]
    set flash_size_mbit [lindex $options 3]
    set bitsize_bytes [lindex $options 4]
    set part_name [lindex $options 5]
    set multiboot_address [GetMultibootAddress $bitsize_bytes $type $width $freq_mhz]

    puts "Flash type          : [string toupper $type]"
    puts "Flash width (bits)  : $width"
    puts "CCLK frequency (MHz): $freq_mhz"
    puts "Flash density (Mbit): $flash_size_mbit"
    puts "Bitstream size (B)  : $bitsize_bytes"
    puts "Part name           : $part_name"
    puts "Multiboot address   : [hexpr $multiboot_address]"

    set timer1_address [FirstTimerAddress $multiboot_address]

    return $timer1_address
}

set part_name [lindex $options 5]
set timer1_address [Prepare]

set target_tcl "deps/gen/generate-$part_name.tcl"
puts "deps/gen/generate-$part_name.tcl"
source $target_tcl