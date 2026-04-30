set script_dir [file dirname [file normalize [info script]]]
set build_dir [file join $script_dir build]

file mkdir $build_dir

open_project -reset [file join $build_dir matmul_parallel_hls]
set_top matmul_parallel
add_files [file join $script_dir matmul_parallel.cpp]
add_files -tb [file join $script_dir tb_matmul_parallel.cpp]

open_solution -reset solution1 -flow_target vivado
set_part {xc7z020clg400-1}
create_clock -period 10 -name default

csim_design
csynth_design
export_design -format ip_catalog
exit
