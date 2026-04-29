set script_dir [file dirname [file normalize [info script]]]
set build_dir [file join $script_dir build]
set vivado_dir [file join $build_dir vivado]
set out_dir [file join $script_dir overlay]
set project_name hello_overlay
set bd_name hello_system
set part_name xc7z020clg400-1

file mkdir $vivado_dir
file mkdir $out_dir

set ip_dir [file normalize [file join $build_dir hello_hls solution1 impl ip]]
set ip_zips [glob -nocomplain [file join $ip_dir *.zip]]
if {[llength $ip_zips] == 0} {
    error "No packaged HLS IP found in $ip_dir. Run build_hls.tcl first."
}

create_project -force $project_name $vivado_dir -part $part_name

set board_parts [get_board_parts -quiet *pynq-z2*]
if {[llength $board_parts] > 0} {
    set_property board_part [lindex $board_parts 0] [current_project]
}

set_property ip_repo_paths $ip_dir [current_project]
update_ip_catalog

create_bd_design $bd_name

create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7 processing_system7_0
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" apply_board_preset "1" Master "Disable" Slave "Disable"} [get_bd_cells processing_system7_0]

create_bd_cell -type ip -vlnv xilinx.com:hls:sum:1.0 sum_0
create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset proc_sys_reset_0
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect axi_interconnect_0

set_property CONFIG.NUM_MI {1} [get_bd_cells axi_interconnect_0]
set_property CONFIG.NUM_SI {1} [get_bd_cells axi_interconnect_0]

connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins axi_interconnect_0/ACLK]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins axi_interconnect_0/S00_ACLK]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins axi_interconnect_0/M00_ACLK]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins processing_system7_0/M_AXI_GP0_ACLK]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins sum_0/ap_clk]
connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins proc_sys_reset_0/slowest_sync_clk]

connect_bd_net [get_bd_pins processing_system7_0/FCLK_RESET0_N] [get_bd_pins proc_sys_reset_0/ext_reset_in]
connect_bd_net [get_bd_pins proc_sys_reset_0/peripheral_aresetn] [get_bd_pins axi_interconnect_0/ARESETN]
connect_bd_net [get_bd_pins proc_sys_reset_0/peripheral_aresetn] [get_bd_pins axi_interconnect_0/S00_ARESETN]
connect_bd_net [get_bd_pins proc_sys_reset_0/peripheral_aresetn] [get_bd_pins axi_interconnect_0/M00_ARESETN]
connect_bd_net [get_bd_pins proc_sys_reset_0/peripheral_aresetn] [get_bd_pins sum_0/ap_rst_n]

connect_bd_intf_net [get_bd_intf_pins processing_system7_0/M_AXI_GP0] [get_bd_intf_pins axi_interconnect_0/S00_AXI]
connect_bd_intf_net [get_bd_intf_pins axi_interconnect_0/M00_AXI] [get_bd_intf_pins sum_0/s_axi_CTRL]

assign_bd_address
validate_bd_design
save_bd_design

make_wrapper -files [get_files [file join $vivado_dir $project_name.srcs sources_1 bd $bd_name ${bd_name}.bd]] -top
add_files -norecurse [file join $vivado_dir $project_name.gen sources_1 bd $bd_name hdl ${bd_name}_wrapper.v]
update_compile_order -fileset sources_1

launch_runs synth_1 -jobs 4
wait_on_run synth_1
launch_runs impl_1 -to_step write_bitstream -jobs 4
wait_on_run impl_1

set bit_src [file join $vivado_dir $project_name.runs impl_1 ${bd_name}_wrapper.bit]
set hwh_src [file join $vivado_dir $project_name.gen sources_1 bd $bd_name hw_handoff ${bd_name}.hwh]
set bit_dst [file join $out_dir ${project_name}.bit]
set hwh_dst [file join $out_dir ${project_name}.hwh]

file copy -force $bit_src $bit_dst
file copy -force $hwh_src $hwh_dst

puts "Wrote $bit_dst"
puts "Wrote $hwh_dst"
exit
