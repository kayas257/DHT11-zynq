# DHT11 Interfacing Zynq PL.  

## Tool Requirement
  1) Xilinx Vivado
  2) Xilinx SDK (for standalone software)
  3) Any C/C++ IDE for (Linux software)
  4) Linux kernel source 
## Hardware
1) Connect DHT11  data pin to any pin of zynq device 
2) Create a Design in Vivado with zynq processor.
3) Add local repo as ip_repo folder

## Software
1) Xilinx SDK for bare metal coding
2) C/C++ using mmap to map physical address
3) Kernel code with character driver / misc platform driver(my case)

## NOTE
* This repo includes in source file, you may have to create ip block out of it and spawn it in ur design.
* To use kernel driver you need device tree which has to be included while building kernel.
* Please remember to chnage baseaddress according to your design

Any issue mail at 
Kayasdev@gmail.com or raise an issue
