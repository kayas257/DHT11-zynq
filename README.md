# DHT11 Interfacing Zynq PL.  
DHT11 FPGA VHDL implementation
## Tool Requirement
  * Xilinx Vivado
  * Xilinx SDK (for standalone software)
  * Any C/C++ IDE for (Linux software)
  * Linux kernel source 
## Hardware
* Connect DHT11  data pin to any pin of zynq device 
* Create a Design in Vivado with zynq processor.
* Add local repo as ipblock folder
* Add DHT11 from ip catalog


## Software
* Xilinx SDK for bare metal coding
* C/C++ using mmap to map physical address
* Kernel code with character driver / misc platform driver(my case)

## NOTE
* To use kernel driver you need device tree which has to be included while building kernel.
* Please remember to change base address(AXI lite interface) according to your design.

Any issue mail at 
Kayasdev@gmail.com or raise an issue
