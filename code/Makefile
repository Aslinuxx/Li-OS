TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/
OBJS_BOOTPACK= bootpack.obj graphic.obj dsctbl.obj int.obj fifo.obj naskfunc.obj hankaku.obj \
 keyboard.obj mouse.obj memory.obj sheet.obj timer.obj mtask.obj time_task.obj console_task.obj \
 text_task.obj task_manage_task.obj
 
 
MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del



default :
	$(MAKE) img



ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

# 一般規則
# 一般規則
# .c->.gas->.nas->.obj 上述过程可以用那个一般性的通用规则来进行生成;而不用具体再写每一个文件名了
# 注意make.exe 会优先找具体的生成规则; 如果没找到就会执行通用的生成规则; 具体的规则优先级更高

%.gas : %.c Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas
%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst


# .obj生成.bim的时候就需要考虑kankaku.txt了;必须也要将其编译成.obj 才能一起链接
# 先将.txt编译成.bin 
hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

#再将.bin 编译成.obj
# 注意这个不能省略 _hankaku 这里估计是他封装了个函数,后面的label参数,应该是文件名
# 这个.obj的生成规则和一般性规则不同,故不能省略
hankaku.obj: hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku 


	
#注意bim文件编译的改变
# 使用$(OBJS_BOOTPACK)来整体表示 bootpack.obj graphic.obj dsctbl.obj naskfunc.obj hankaku.obj
bootpack.bim : $(OBJS_BOOTPACK)  Makefile 
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)
# 3MB+64KB=3136KB

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

haribote.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb haribote.sys

Li_OS.img : ipl10.bin haribote.sys Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		imgout:Li_OS.img



img :
	$(MAKE) Li_OS.img

run :
	$(MAKE) img
	$(COPY) Li_OS.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: Li_OS.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
