;忽略操作系统指定的ds，汇编直接将操作系统段地址存入ds
[INSTRSET "i486p"]
[BITS 32]
		MOV EAX,1*8
		MOV DS,AX
		MOV BYTE [0x102600],0
		MOV EDX,4
		INT 0x40