;hello-os
;TAB=4

CYLS	EQU		10		;柱面数
SECTIONS  EQU     18	;扇区数
HEAD   EQU     2		;磁头数


	ORG 0x7c00			;指明程序的装载地址

;标注FAT12软盘专用代码

	JMP entry
	DB 0x90
	DB "MYOSRAIN"		;启动区的名字，八字节
	DW 512				;每个扇区的大小必须是512字节
	DB 1				;簇的大小，必须是1扇区
	DW 1				;FAT的起始位置，一般从1扇区开始
	DB 2				;FAT的个数，必须是2
	DW 224				;根目录的大小，一般设成224项
	DW 2880				;该磁盘的大小，必须是2880扇区
	DB 0xf0             ;磁盘的种类，必须是0xf0
	DW 9				;FAT的长度，必须是9扇区
	DW 18				;1个磁道有18个扇区
	DW 2   				;2个磁头
	DD 0				;不使用分区，必须是0
	DD 2880				;重写一次磁盘大小;
	DB 0,0,0x29			;意义不明，固定
	DD 0xffffffff		;可能是卷标号码
	DB "EASYOS     "	;磁盘名称,必须是11字节(不满用空格补)
	DB "FAT12   "		;磁盘格式名称，必须8字节，不满用空格补
	RESB 18				;空出18字节
	
	
	
;程序主体
entry:	
	MOV AX,0			;初始化寄存器
	MOV SS,AX
	MOV SP,0x7c00
	MOV DS,AX
	MOV ES,AX	



;阅读磁盘
	MOV AX,0x0820
	MOV ES,AX
	MOV CH,0			;柱面0
	MOV DH,0			;磁头0
	MOV CL,2			;扇区2,因为1号扇区要留给启动区
	
	
;试错，重复读5次，若仍失败则表示软盘出错
readloop:
	MOV SI,0			;记录失败次数

retry:	
	MOV AH,0x02			;AH=0x02表示读盘
	MOV AL,1			;1个扇区
	MOV BX,0			;ES：BX是缓冲地址，ES*16+BX
						;此处es=0x0820，bx=0，所以软盘的数据被装载到内存中0x8200到0x83ff
						;之所以es=0x0820，是因为0x8000到0x81ff这512字节留给启动区。
	MOV DL,0x00			;A驱动器，软盘驱动器
	INT 0x13			;磁盘BIOS
	JNC next
	ADD SI,1
	CMP SI,5
	JAE error
	MOV AH,0x00			;这三行表示复位
	MOV DH,0x00			;驱动器a
	INT 0x13			;重置驱动器
	JMP retry
	
next:					;循环读18扇区，2磁头，10柱面
	MOV AX,ES
	ADD AX,0x0020
	MOV ES,AX
	ADD CL,1
	CMP CL,SECTIONS
	JBE readloop
	
	MOV CL,1
	ADD DH,1
	CMP DH,HEAD
	JB  readloop
	
	MOV DH,0
	ADD CH,1
	CMP CH,CYLS
	JB  readloop
	
;运行完了读入haribote.sys
	MOV	[0x0ff0],CH		;磁盘装载内容结束地址告诉haribote.sys
	JMP 0xc200			;磁盘映像文件内容写在0x004200处，磁盘内容被加载到0x8000号地址，所以磁盘0x4200地址位于0x8000+0x4200
	
fin:
    HLT
	JMP fin		
		
error:	
	MOV SI,msg

putloop:
    MOV AL,[SI]
	ADD SI,1
	CMP AL,0	
	JE fin
	MOV AH,0x0e		;显示一个文字
	MOV BX,15		;字符颜色
	INT 0x10		;调用显卡BIOS
	JMP putloop
	
	
	
;信息显示部分
msg:
	DB 0x0a,0x0a		;2个换行
	DB "load   error"	 	
	DB 0x0a				;换行
	DB 0
	
	RESB 0x7dfe-$		;有ORG指令后，$不再表示输出文件的第几个字节，而是代表将要读入的内存地址。
	DB 0x55,0xaa        ;必须启动区的最后两个字节是0x55和0xaa。