set INSTRUCTIONS "

Usage: [info script]

This Tcl script generate Multi-boot 7 series golden project.

Usage:

Batch mode (all arguments are required in the given order):
    [info script] <flash_type> <data_width> <freq_mhz> <flash_size_mbit> <bitstream_size>

    <flash_type>      : Flash type: spi, bpi
    <data_width>      : Flash data width: 1, 2, 4, 8, 16
    <freq_mhz>        : Frequency of CCLK (enter highest frequency used)
    <flash_size_mbit> : Size of flash device in Mbit (ex: 128, 256, 512, 1024)
    <bitstream_size>  : Size of bitstream in bytes. Get this from UG470 or
                        UG570. If compression is enabled and reading this
                        directly form the bitstream file itself, be aware that
                        subsequent builds can vary significantly in size and
                        this script will need to be re-run.
    <part_name>       : Name of device part name.
"

set MIN_BIT_SIZE    131072 ; # 1 Mbit (in bytes)
set MAX_BIT_SIZE 536870912 ; # 4 Gbit (in bytes)

# GetOptions --
#
# Arguments:
#
# Results: Interactive mode is run or command line arguments are processed for
# valid values
proc GetOptions {argc argv} {
    global INSTRUCTIONS
    global MIN_BIT_SIZE
    global MAX_BIT_SIZE

    if { $argc == 6 } {
        set options {}
        scan [lindex $argv 0] "%s" flash_type
        scan [lindex $argv 1] "%d" data_width
        scan [lindex $argv 2] "%d" freq_mhz
        scan [lindex $argv 3] "%d" flash_size_mbit
        scan [lindex $argv 4] "%d" bit_size_bytes
        scan [lindex $argv 5] "%s" part_name

        lappend options $flash_type
        lappend options $data_width
        lappend options $freq_mhz
        lappend options $flash_size_mbit
        lappend options $bit_size_bytes
        lappend options $part_name

    } else {
        puts "Unrecognized argument list: $argv" 
        puts $INSTRUCTIONS
        return -1
    }

    # puts "DEBUG(GetOptions): Options: $options"
    return $options
}

# NextBitstreamAddress --
#
# Calculate the address for the next bitstream. There is a minimum buffer
# requirement between bitstreams as certain startup conditions can delay
# configuration. If a following bitstream inserts new configuration commands,
# it could cause a configuration failure of the first bitstream
#
# Arguments:
# bits:      size of the bitstream
# start:     start address for the bitstream (0 by default)
# width:     (optional) Configuration width (defaults to 16)
# freq:      (optional) Configuration frequency in MHz (defaults to 134)
# dci_wait:  (optional) Is DCI wait needed. Should be set to 1 if any DCI is
#            being used in the design. Recommended to leave this at the
#            default unless no DCI is used in the design
# family:    (optional) 7: 7 series, 8: UltraScale (defaults to 7)
#            Safe to leave at 7 for UltraScale
#
# results:
# Returns the next address at which it is safe to place a new bitstream (such
# as the timer images)
proc NextBitstreamAddress {bits start {width 16} {freq_mhz 134} {dci_wait 1} \
                           {family 7}} {

    # Constants

    # Always assume at least PLL lock waitup is needed. This is quite small
    set pll_tlockmax_us 200

    # Use 256 kB sectors, smaller sectors may save a little bit of buffer
    # space, but will likely cause problems if used on 256 kB sector devices
    set sector_size [expr 256 * 2**10]

    if {$family == 8} {
        set dcu_wait_ms 4
    } else {
        set dcu_wait_ms 10
    }
    set dcu_wait_us [expr $dcu_wait_ms * 1000]

    # DCI Wait is always larger then pll_tlockmax_us, use it if enabled
    if {$dci_wait} {
        set wait_time [expr $dcu_wait_us * pow(10, -6)]
    } else {
        set wait_time [expr $pll_tlockmax_us * pow(10, -6)]
    }

    set frequency [expr $freq_mhz * 10**6]

    # Find the number CCLK cycles needed for the max wait time
    set wait_cycles [expr int(ceil($wait_time * $frequency))]
#    puts " -- Wait cycles: $wait_cycles"

    # The extra bits needed for the buffer
    set extra_bytes [expr ($wait_cycles * $width) / 8]
    # int(ceil(($frequency * $width / 8)))]

#    puts " -- Extra Bytes: $extra_bytes"

    # Add the extra bytes to the bitstream size and get the end address
    set bits_end [expr $bits + $start + $extra_bytes]

    set end_sector [expr int(ceil((1.0 * $bits_end) / $sector_size))]

    set end_address [expr $end_sector * $sector_size]

#    puts " -- Next available address: [hexpr $end_address]"

    return $end_address
}

# hexpr --
#
# Arguments:
# args: values to return formatted as hexidecimal 
#       All values are passed along to expr
#
# Results:
proc hexpr args {
   uplevel "format \"0x%08X\" \"[expr $args] \""
}

# FirstTimerAddress --
#
# Calculate the first timer address based off the multiboot address
#
# Arguments:
# multiboot_address: Address of the multiboot image. This is the next "safe"
#                    sector after the golden bitstream
# Results:
# returns the address to place the first timer
proc FirstTimerAddress {multiboot_address} {
    set timer_size 1024

    return [expr $multiboot_address - $timer_size]
}

# GetMultibootAddress --
#
# Arguments:
# bitsize_bytes: Size of bitstream in bytes
# type:          Flash type: spi, bpi
# width:         Flash data width: 1, 2, 4, 8, 16
# freq_mhz:      Frequency of CCLK (enter highest frequency used)
#
# Results: 
# return multiboot bit address
proc GetMultibootAddress {bitsize_bytes type width freq_mhz} {
    switch ${type}_${width} {
        "spi_1" {
            set interface "SPIx1"
        }
        "spi_2" {
            set interface "SPIx2"
        }
        "spi_4" {
            set interface "SPIx4"
        }
        "spi_8" {
            set interface "SPIx8"
        }
        "bpi_8" {
            set interface "BPIx8"
            set bitswap 1
        }
        "bpi_16" {
            set interface "BPIx16"
            set bitswap 1
            set byteswap 1
        }
        default {
            error "Unknown width and flash type combo: $type/$width"
        }
    }

    set multiboot_address [NextBitstreamAddress $bitsize_bytes 0 $width $freq_mhz 0]
    if {[string match $interface "BPIx16"]} {
        set multiboot_address [expr $multiboot_address >> 1]
    } 

    return $multiboot_address
}