include $(MY_BUILD_BASE)/config.mk
include ../hal_include.mk

ADD_INCLUDE +=$(HAL_INCLUDE) -include hal_config.h

#要编译的文件列表
OBJS = screen.o pixel.o dotfnt.o cursor.o console.o
# PARTS = 

console.o: console_internal.c console_c_interface.h
	gcc $(ADD_INCLUDE) -I/d/GridOS/include -o console.o console_c_interface.h console_internal.c

all:  subs_parts $(OBJS)
	$(LD) -r -o ../console.o *.o $(ARCH_LD_FLAGS)


include $(MY_BUILD_BASE)/cc.mk