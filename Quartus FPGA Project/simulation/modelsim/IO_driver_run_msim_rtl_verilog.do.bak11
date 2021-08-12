transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -vlog01compat -work work +incdir+C:/0\ Nalith\ Drive/FYP/FPGA/IO_driver_rev3 {C:/0 Nalith Drive/FYP/FPGA/IO_driver_rev3/IO_driver.v}
vlog -vlog01compat -work work +incdir+C:/0\ Nalith\ Drive/FYP/FPGA/IO_driver_rev3 {C:/0 Nalith Drive/FYP/FPGA/IO_driver_rev3/fifo_2clk.v}
vlog -vlog01compat -work work +incdir+C:/0\ Nalith\ Drive/FYP/FPGA/IO_driver_rev3 {C:/0 Nalith Drive/FYP/FPGA/IO_driver_rev3/fifo_1clk.v}

vlog -vlog01compat -work work +incdir+C:/0\ Nalith\ Drive/FYP/FPGA/IO_driver_rev3/simulation/modelsim {C:/0 Nalith Drive/FYP/FPGA/IO_driver_rev3/simulation/modelsim/tb.v}

vsim -t 1ps -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L cycloneive_ver -L rtl_work -L work -voptargs="+acc"  tb

add wave *
view structure
view signals
run 100 ps
