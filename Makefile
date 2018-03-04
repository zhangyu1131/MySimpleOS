OBJS_BOOTPACK = bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj \
		int.obj fifo.obj keyboard.obj mouse.obj memory.obj sheet.obj timer.obj mtask.obj \
		file.obj console.obj window.obj

TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

#默认

default :
	$(MAKE) img

#文件生成规则

ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)
# 3MB+64KB=3136KB

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0
	
ac.bim : ac.obj ac_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:ac.bim map:ac.map ac.obj ac_nask.obj

ac.hrb : ac.bim Makefile
	$(BIM2HRB) ac.bim ac.hrb 0
	
paixu.bim : paixu.obj ac_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:paixu.bim map:paixu.map paixu.obj ac_nask.obj

paixu.hrb : paixu.bim Makefile
	$(BIM2HRB) paixu.bim paixu.hrb 0

xunhuan1.bim : xunhuan1.obj ac_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:xunhuan1.bim map:xunhuan1.map xunhuan1.obj ac_nask.obj

xunhuan1.hrb : xunhuan1.bim Makefile
	$(BIM2HRB) xunhuan1.bim xunhuan1.hrb 0

xunhuan2.bim : xunhuan2.obj ac_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:xunhuan2.bim map:xunhuan2.map xunhuan2.obj ac_nask.obj

xunhuan2.hrb : xunhuan2.bim Makefile
	$(BIM2HRB) xunhuan2.bim xunhuan2.hrb 0

hello5.bim : hello5.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:hello5.bim stack:1k map:hello5.map hello5.obj

hello5.hrb : hello5.bim Makefile
	$(BIM2HRB) hello5.bim hello5.hrb 0
	
	
openwin.bim : openwin.obj ac_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:openwin.bim stack:1k map:openwin.map \
		openwin.obj ac_nask.obj

openwin.hrb : openwin.bim Makefile
	$(BIM2HRB) openwin.bim openwin.hrb 0
	
crack1.bim : crack1.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:crack1.bim map:crack1.map crack1.obj ac_nask.obj

crack1.hrb : crack1.bim Makefile
	$(BIM2HRB) crack1.bim crack1.hrb 0
	
crack2.hrb : crack2.nas Makefile
	$(NASK) crack2.nas crack2.hrb crack2.lst
	
haribote.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb haribote.sys

haribote.img : ipl10.bin haribote.sys ac.hrb  paixu.hrb xunhuan1.hrb xunhuan2.hrb openwin.hrb hello5.hrb crack1.hrb crack2.hrb Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		copy from:ipl10.nas to:@: \
		copy from:make.bat to:@: \
		copy from:ac.hrb to:@: \
		copy from:xunhuan1.hrb to:@: \
		copy from:xunhuan2.hrb to:@: \
		copy from:crack1.hrb to:@: \
		copy from:crack2.hrb to:@: \
		copy from:hello5.hrb to:@: \
		copy from:openwin.hrb to:@: \
		copy from:paixu.hrb to:@: \
		imgout:haribote.img

#一般规则

%.gas : %.c Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

#命令

img :
	$(MAKE) haribote.img

run :
	$(MAKE) img
	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) *.map
	-$(DEL) *.bim
	-$(DEL) *.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
