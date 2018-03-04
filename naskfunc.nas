;naskfunc
;TAB=4

[FORMAT "WCOFF"]			;����Ŀ���ļ���ģʽ		
[INSTRSET "i486p"]			;�����Ǹ�486�õģ�����EAX�Ȼ���ͳɼĴ������������Ϊ�Ǳ�ǩ
[BITS 32]					;����32λģʽ�õĻ�������
;����Ŀ���ļ�����Ϣ

[FILE "naskfunc.nas"]			;Դ�ļ���Ϣ

	GLOBAL _io_hlt,_write_mem8			;�����а����ĺ�������Ҫ��_��ͷ
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
	
;ʵ�ʵĺ���

[SECTION .text]				;Ŀ���ļ���д����Щ֮����д����
	
_io_hlt:						;void io_hlt(void)
		HLT
		RET
		
_write_mem8:					;void write_mem8(int addr,int data)
	MOV ECX,[ESP+4]				;[ESP+4]��ŵ��ǵ�ַ,��һ������
	MOV AL,[ESP+8]				;[ESP+8]��ŵ������ݣ��ڶ�������
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
	PUSHFD				;push eflags��ջ,�����������mov eax,eflags
	POP EAX
	RET
	
_io_store_eflags:		;void io_store_eflags(int eflags)
	MOV EAX,[ESP+4]
	PUSH EAX
	POPFD				;pop eflags��ջ
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
		PUSH EDI	;�ȱ���һЩ�Ĵ�����ֵ��ʹ�ý������ٻָ�
		PUSH ESI
		PUSH EBX
		MOV	 ESI,0xaa55aa55		
		MOV  EDI,0x55aa55aa
		MOV  EAX,[ESP+12+4]		;i=start,��Ϊջ�ﻹ���������Ĵ��������Ե�һ����������esp+3*4+4�ĵط�
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
		PUSH 1			;cons_putchar���һ������
		AND  EAX,0xff  ;��ah��eax��λ��0����eax��Ϊ�Ѵ����ַ������״̬
		PUSH EAX
		PUSH DWORD [0x0fec]	;��ȡ�ڴ沢push����������������
		PUSH DWORD [0x0ff0]
		CALL _cons_putchar
		ADD  ESP,16		;����ջ������
		POPAD
		IRETD
	

_farcall:		;void farcall(int eip,int cs)
		CALL FAR [ESP+4]
		RET
		
_asm_hrb_api:
		STI
		PUSH DS
		PUSH ES
		PUSHAD		;���ڱ����push
		PUSHAD		;������hrb_api��ֵ��push
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
;eaxΪtss.esp0�ĵ�ַ
		MOV ESP,[EAX]
		;MOV DWORD [EAX+4],0
		POPAD
		RET

_asm_end_app:
;eaxΪtss.esp0�ĵ�ַ
		MOV ESP,[EAX]
		MOV DWORD [EAX+4],0
		POPAD
		RET
		
_start_app:			;void start_app(int eip,int cs,int esp,int ds,int *tss_esp0)
		PUSHAD
		MOV EAX,[ESP+36]	;Ӧ�ó�����eip
		MOV ECX,[ESP+40]	;cs
		MOV EDX,[ESP+44]	;esp
		MOV EBX,[ESP+48]	;ds/ss
		MOV	EBP,[ESP+52]	;esp0
		MOV [EBP],ESP		;�������ϵͳ��esp
		MOV [EBP+4],SS		;�������ϵͳss	
		MOV ES,BX
		MOV DS,BX
		MOV FS,BX
		MOV GS,BX
		
;����ջ��������retf��ת��Ӧ�ó���
		OR ECX,3
		OR EBX,3
		PUSH EBX	;Ӧ�ó�����ss
		PUSH EDX	;Ӧ�ó�����esp
		PUSH ECX	;Ӧ�ó�����cs
		PUSH EAX	;Ӧ�ó�����eip
		RETF
		