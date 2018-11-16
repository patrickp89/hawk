ifeq ($(KERNELRELEASE),)
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	-@$(RM) hawk.ko
	-@$(RM) .hawk.ko.cmd
	-@$(RM) hawk.mod.c
	-@$(RM) hawk.mod.o
	-@$(RM) .hawk.mod.o.cmd
	-@$(RM) hawk.o
	-@$(RM) .hawk.o.cmd
	-@$(RM) modules.order
	-@$(RM) Module.symvers
	-@$(RM) -rf .tmp_versions

.PHONY: modules modules_install clean

else
    obj-m := hawk.o
endif


