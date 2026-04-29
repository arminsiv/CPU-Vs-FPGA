set script_dir [file dirname [file normalize [info script]]]
set build_dir [file join $script_dir build]

file mkdir $build_dir

open_project -reset [file join $build_dir hello_hls]
set_top sum
add_files [file join $script_dir hello.c]

open_solution -reset solution1 -flow_target vivado
set_part {xc7z020clg400-1}
create_clock -period 10 -name default

csynth_design
export_design -format ip_catalog
exit
