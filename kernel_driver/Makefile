KERN_SRC=<path to linux kernel>/linux-xlnx/


obj-m += dht11.o

all:
	make -C ${KERN_SRC} M=${PWD} modules
clean:
	rm -f *.ko *.o *.order *.sym* *.mod.*

