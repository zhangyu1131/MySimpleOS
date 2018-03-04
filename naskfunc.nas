;naskfunc
;TAB=4

[FORMAT "WCOFF"]			;制作目标文件的模式		
[INSTRSET "i486p"]			;声明是给486用的，遇到EAX等会解释成寄存器，否则会认为是标签
[BITS 32]					;制作32位模式用的机器语言
;制作目标文件的信息

[FILE "naskfunc.nas"]			;源文件信息

	GLOBAL _io_hlt,_write_mem8			;程序中包含的函数名，要以_开头
	GLOBAL _io_cli,_io_sti,_io_stihlt
	GLOBAL _io_in8,_io_in16,_io_in32
	GLOBAL _io_out8,_io_out16,_io_out32
	GLOBAL _io_load_eflags,_io_store_eflags
	GLOBAL	_load_gdtr, _load_idtr
	GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c, _asm_inthandler20,_asm_inthandler0d
	GLOBAL	_asm_inthandler0c
	GLOBAL	_load_cr0, _store_cr0, _memtest_sub,_load_tr,_taskswitch
	GLOBAL	_asm_cons_putchar,_farcall,_asm_hrb_api,_start_app,_asm_end_app
	EXTERN	_inthandler21, _inthandler2c,_inthandler27,_inthandler20,_inthandler0d,_inthandler0c
	EXTERN	_cons_putchar,_hrb_api
	
;实际的函数

[SECTION .text]				;目标文件中写了这些之后再写程序
	
_io_hlt:						;void io_hlt(void)
		HLT
		RET
		
_write_mem8:					;void write_mem8(int addr,int data)
	MOV ECX,[ESP+4]				;[ESP+4]存放的是地址,第一个参数
	MOV AL,[ESP+8]				;[ESP+8]存放的是数据，第二个参数
	MOV [ECX],AL
	RET
	
	
	
_io_cli:
	CLI
	RET
	
_io_sti:
	STI
	RET
	
	
_io_stihlt:
	STI
	HLT
	RET
	
_io_in8:			;int io_in8(int port)
	MOV EDX,[ESP+4]
	MOV EAX,0
	IN  AL,DX
	RET
	
_io_in16:
	MOV EDX,[ESP+4]
	MOV EAX,0
	IN  AX,DX
	RET
	
_io_in32:
	MOV EDX,[ESP+4]
	IN  EAX,DX
	RET	
	
	
_io_out8:			;void io_out8(int port,int data)
	MOV EDX,[ESP+4]
	MOV AL,[ESP+8]
	OUT DX,AL
	RET
	
_io_out16:			;void io_out16(int port,int data)
	MOV EDX,[ESP+4]
	MOV AL,[ESP+8]
	OUT DX,AX
	RET
	
	
_io_out32:			;void io_out32(int port,int data)
	MOV EDX,[ESP+4]
	MOV AL,[ESP+8]
	OUT DX,EAX
	RET
		
		
_io_load_eflags:		;int in_load_eflags()
	PUSHFD				;push eflags入栈,这两句代替了mov eax,eflags
	POP EAX
	RET
	
_io_store_eflags:		;void io_store_eflags(int eflags)
	MOV EAX,[ESP+4]
	PUSH EAX
	POPFD				;pop eflags出栈
	RET
	
_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET	
		
_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
		
		
		
_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
		
_asm_inthandler0d:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		CMP		EAX,0
		JNE		end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4
		IRETD		

_asm_inthandler0c:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0c
		CMP		EAX,0
		JNE		end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4
		IRETD

_load_cr0:			;int load_cr0()
		MOV EAX,CR0
		RET
		
_store_cr0:			;void store_cr0(int cr0)
		MOV EAX,[ESP+4]
		MOV CR0,EAX
		RET
_memtest_sub:		;unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH EDI	;先保存一些寄存器的值，使用结束后再恢复
		PUSH ESI
		PUSH EBX
		MOV	 ESI,0xaa55aa55		
		MOV  EDI,0x55aa55aa
		MOV  EAX,[ESP+12+4]		;i=start,因为栈里还存了三个寄存器，所以第一个参数存在esp+3*4+4的地方
mts_loop:
		MOV EBX,EAX		
		ADD EBX,0xffc	;p=i+0xffc
		MOV EDX,[EBX]	;old=*p
		MOV [EBX],ESI	;*p=test0
		XOR DWORD [EBX],0xffffffff
		CMP EDI,[EBX]
		JNE mts_fin
		XOR DWORD [EBX],0xffffffff
		CMP ESI,[EBX]
		JNE mts_fin
		MOV [EBX],EDX	;*p=old
		ADD EAX,0x1000
		CMP EAX,[ESP+12+8]
		
		JBE mts_loop
		POP EBX
		POP ESI
		POP EDI
		RET
mts_fin:
		MOV [EBX],EDX
		POP EBX
		POP ESI
		POP EDI
		RET

		
_load_tr:		;void loar_tr(int tr)
		LTR [ESP+4]
		RET
		
_taskswitch:	;void taskswitch(int eip, int cs)
		JMP FAR [ESP+4]
		RET
		
_asm_cons_putchar:
		STI
		PUSHAD
		PUSH 1			;cons_putchar最后一个参数
		AND  EAX,0xff  ;将ah和eax高位置0，将eax置为已存入字符编码的状态
		PUSH EAX
		PUSH DWORD [0x0fec]	;读取内存并push，倒数第三个参数
		PUSH DWORD [0x0ff0]
		CALL _cons_putchar
		ADD  ESP,16		;丢弃栈中数据
		POPAD
		IRETD
	

_farcall:		;void farcall(int eip,int cs)
		CALL FAR [ESP+4]
		RET
		
_asm_hrb_api:
		STI
		PUSH DS
		PUSH ES
		PUSHAD		;用于保存的push
		PUSHAD		;用于向hrb_api传值的push
		MOV AX,SS
		MOV DS,AX
		MOV ES,AX
		CALL _hrb_api
		CMP EAX,0
		JNE end_app
		ADD ESP,32
		POPAD
		POP ES
		POP DS
		IRETD

end_app:
;eax为tss.esp0的地址
		MOV ESP,[EAX]
		;MOV DWORD [EAX+4],0
		POPAD
		RET

_asm_end_app:
;eax为tss.esp0的地址
		MOV ESP,[EAX]
		MOV DWORD [EAX+4],0
		POPAD
		RET
		
_start_app:			;void start_app(int eip,int cs,int esp,int ds,int *tss_esp0)
		PUSHAD
		MOV EAX,[ESP+36]	;应用程序用eip
		MOV ECX,[ESP+40]	;cs
		MOV EDX,[ESP+44]	;esp
		MOV EBX,[ESP+48]	;ds/ss
		MOV	EBP,[ESP+52]	;esp0
		MOV [EBP],ESP		;保存操作系统用esp
		MOV [EBP+4],SS		;保存操作系统ss	
		MOV ES,BX
		MOV DS,BX
		MOV FS,BX
		MOV GS,BX
		
;调整栈，以免用retf跳转到应用程序
		OR ECX,3
		OR EBX,3
		PUSH EBX	;应用程序用ss
		PUSH EDX	;应用程序用esp
		PUSH ECX	;应用程序用cs
		PUSH EAX	;应用程序用eip
		RETF
		