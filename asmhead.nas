

[INSTRSET "i486p"]

VBEMODE	EQU		0x105

BOTPAK	EQU		0x00280000		
DSKCAC	EQU		0x00100000		
DSKCAC0	EQU		0x00008000	

;有关BOOT_INFO
CYLS	EQU	0x0ff0			;设定启动区,这附近的地址没有被使用，所以选择这个地址
LEDS	EQU	0x0ff1
VMODE	EQU	0x0ff2			;颜色位数
SCRNX	EQU 0x0ff4			;x坐标
SCRNY	EQU 0x0ff6			;y坐标
VRAM	EQU 0x0ff8 			;图像缓冲区的开始地址

	ORG 0xc200			;0x8000+0x4200

;画面设定
	MOV AL,0x13			;VGA显卡，320*200*8位彩色
	MOV AH,0x00
	INT 0x10

;记录画面模式,320*200*8位彩色	
	MOV BYTE [VMODE],8	
	MOV WORD [SCRNX],320
	MOV WORD [SCRNY],200
	
	MOV DWORD [VRAM],0x000a0000  ;这种画面模式下，VRAM是0xa0000到0xaffff的64KB
	
;用BIOS去的键盘上各种LED指示灯的状态
keystatus:
	MOV AH,0x02
	INT 0x16
	MOV [LEDS],AL




;------------------------------分割线--------------------------------------------------

;PIC关闭一切中断
;	根据AT兼容机的规格，如果要初始化PIC必须在CLI（禁止中断）之前进行，否则有时会挂起
;	随后进行PIC的初始化
		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; 如果连续使用OUT，有些机种会无法正常运行
		OUT		0xa1,AL

		CLI						; 禁止CPU级别的中断

; 为了让cpu能够访问1mb以上的内存空间，设定A20GATE

		CALL	waitkbdout
		MOV		AL,0xd1			;	1101 0001
		OUT		0x64,AL			;	0110 0100
		CALL	waitkbdout
		MOV		AL,0xdf			; 1101 1111 enable A20
		OUT		0x60,AL			;
		CALL	waitkbdout

;切换到保护模式

[INSTRSET "i486p"]				; 使用486命令，如lgdt、eax、cr0（寄存器）

		LGDT	[GDTR0]			; 设定临时GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; 设bit31位0（为了禁止分页）
		OR		EAX,0x00000001	; 设bit0位1（切换到保护模式）
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			;  可读写段，32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack的转送

		MOV		ESI,bootpack	; 转送源
		MOV		EDI,BOTPAK		; 转送目的地
		MOV		ECX,512*1024/4
		CALL	memcpy			;复制内存函数，转送数据大小是以双字为单位，数据大小需字节除以4

; 磁盘数据最终转送到它本来的位置去

; 首先从启动扇区开始

		MOV		ESI,0x7c00		; 转送源
		MOV		EDI,DSKCAC		; 转送目的地
		MOV		ECX,512/4
		CALL	memcpy

; 剩下所有

		MOV		ESI,DSKCAC0+512	; 转送源
		MOV		EDI,DSKCAC+512	; 转送目的地
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 从柱面数变换为字节数/4
		SUB		ECX,512/4		; 减去IPL
		CALL	memcpy

; 必须由asmhead来完成的工作，至此全部完毕
;以后就交由bootpack来完成

; bootpack启动

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]	;hrb文件内部数据部分的大小
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			;没有要转送的东西
		MOV		ESI,[EBX+20]	; 转送源，hrb文件数据部分从哪里开始
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 转送目的地，esp初始值&数据部分传送目的地址
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 栈初始值
		JMP		DWORD 2*8:0x0000001b	;2*8放入cs里

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; and 的结果如果不是0，就跳转到waitkbdout
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			
		RET


		ALIGNB	16				;一直添加DBO，知道时机合适 的时候为止。地址能被16整除的时候就是时机合适的时候
GDT0:							;gdt0号不指定段，是特定的gdt，先初始化
		RESB	8				; null selecor
		DW		0xffff,0x0000,0x9200,0x00cf	; 可以读写的段 32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; 可以执行的段 32bit

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:	