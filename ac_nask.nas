[FORMAT "WCOFF"]		;���ɶ����ļ���ģʽ
[INSTRSET "i486p"]		;��ʾʹ��486����ָ�
[BITS 32]				;����32λģʽ��������
[FILE "ac_nask.nas"]		;ԭ�ļ�����Ϣ


GLOBAL _api_putchar
GLOBAL _api_end
GLOBAL _api_putstr
GLOBAL _api_openwin
GLOBAL _api_closewin


[SECTION .text]

_api_putchar:
		MOV EDX,1
		MOV AL,[ESP+4]
		INT 0x40
		RET
		
_api_end:
		MOV EDX,4
		INT 0x40
			
_api_putstr:		;void api_putstr(char *s)
		PUSH EBX
		MOV  EDX,2
		MOV  EBX,[ESP+8]
		INT  0x40
		POP  EBX
		RET
		
_api_openwin:		;int api_openwin(char *buf,int xsiz,int ysiz,int col_inv,char *title)
		PUSH EDI
		PUSH ESI
		PUSH EBX
		MOV  EDX,5
		MOV  EBX,[ESP+16]
		MOV  ESI,[ESP+20]
		MOV  EDI,[ESP+24]
		MOV  EAX,[ESP+28]
		MOV  ECX,[ESP+32]
		INT  0x40
		POP  EBX
		POP  ESI
		POP  EDI
		RET
		
_api_closewin:		;void api_closewin(int win)
		PUSH EBX
		MOV  EDX,14
		MOV  EBX,[ESP+8]  ;win
		INT  0x40
		POP  EBX
		RET
		