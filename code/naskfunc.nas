; naskfunc
; TAB=4

[FORMAT  "WCOFF"] ; 
[BITS 32] ; 
[INSTRSET "i486p"]				; �ó����Ǹ�486ʹ�õ�

; ����������Ŀ���ļ�����Ϣ
[FILE "naskfunc.nas"] ; Դ�ļ�����Ϣ
	; ʵ���ڴ�д�뺯��
	GLOBAL _io_hlt,_io_cli,_io_sti, _io_stihlt
	GLOBAL _io_load_eflags, _io_store_eflags, _write_tr, _jmpfar
	GLOBAL _io_in8, _io_out8
	GLOBAL _load_gdtr, _load_idtr
	GLOBAL	 _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c ,_asm_inthandler20
	GLOBAL _load_cr0, _store_cr0
	GLOBAL _memtest_sub
	GLOBAL	_asm_end_app, _shutdown 
	; �ж�ִ�����,����ִ��return;(��RETָ��);����ִ��IRETDָ��; ���Խ���������;
	EXTERN	_inthandler21, _inthandler27, _inthandler2c ,_inthandler20
	
	
	
; ������HLT������ʵ�ֲ���
[SECTION .text]		; Ŀ���ļ���д����Щ����д����
_io_hlt: 			; ������;��Ӧc�ļ�ͷ���� void io_hlt(void)
	HLT
	RET				; RET:return ��c��һ�����Ƿ��ص���˼;�ú���û�з���ֵ �൱��return void

_io_load_eflags:   ;int io_load_eflags(void); ���ر�־λ�Ĵ�����״̬
	PUSHFD 			;PUSHFD:push flags double-word;  ��˼�ǽ���־λ��ֵ����˫�ֳ�(DW :32λ 4�ֽ� int����)��ջ;
	POP EAX 		;
	RET 
	
_io_store_eflags: ; void io_store_eflags(int eflags);
	MOV EAX,[ESP+4]
	PUSH EAX
	POPFD
	RET 

; ��TR�Ĵ�����д�뵱ǰִ���еĽ��̱��
_write_tr: ; void write_tr();
 	LTR [ESP+4]
	RET

; Ҫ���н����л�,����Ҫʹ�� farģʽ��JMP����
; ����EPI�Ĵ���(cpu��һָ���ڴ��ַ�Ĵ���) �� cs�Ĵ���(����μĴ���)
_jmpfar: ;�л�����
	JMP		FAR [ESP+4] ;epi cs ;JMP 4*8:0 ��EPI�Ĵ���д����һ�����ڴ��ַ,4�Ƕκ�,*8֮����Ƕ�4�ĵ�ַ��;�̶�д��
										; 0��ʾ��cs����μĴ���д��0
	RET 

_io_cli: ; void io_cli(void);�رձ�־�Ĵ���; �ر��ж�����
	CLI ;�̶�ָ��
	RET 
	
_io_sti: ;void io_sti(void); �ָ��ж�����
	STI
	RET
_io_stihlt: ; void io_stihlt(void);
	STI
	HLT
	RET 
	
	
; I/Oָ��; ���豸������ϢOut,ȡ���豸����ϢIn;
_io_in8: ; int io_in8(int port); //���ĸ��˿����������; In���豸��cpu���з�����Ϣ;
	mov EDX,[ESP+4]
	mov EAX,0
	IN	AL,DX  
	RET 

; out8��cpu���豸(port,�˿�)���з�����Ϣ
_io_out8: ; void io_out8(int port, int data); ���ڴ��ַ��д������
	mov EDX,[ESP+4] ; �˿ں� ;ESP+4��ŵ�һ���������
	mov AL,[ESP+8] ; ����;ESP+8��ŵ�2���������;
	OUT DX,AL
	RET 

; �����úõ�GDT��IDT��ŵ�GDTR�Ĵ�����IDTR�Ĵ�����;
_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX ; [ESP+4]��ŵ�һ������Ĳ���,[ESP+6]�ǵڶ�������Ĳ���;
		LIDT	[ESP+6]
		RET
	
_asm_inthandler20: ; ��ת��21���ж�ִ�г���void inthandler27(int *esp)
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
		
_asm_end_app:

		MOV		ESP,[EAX]
		MOV		DWORD [EAX+4],0
		POPAD
		RET				


; ʵ�ֶ�ȡcr0���ƼĴ�������,��д��cr0�Ĵ������ݵĻ�����;
_load_cr0: ; int load_cr0(void);
		mov		EAX,CR0
		RET 
		
_store_cr0: ; void store_cr0(int cr0);
		mov EAX,[ESP+4]
		mov CR0,EAX
		RET
		
		
; ������ʵ���ڴ�������ѯ�Ļ�����
_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; ��EBX, ESI, EDI ��ʹ�������Τǣ�
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;
mts_loop:
		MOV		EBX,EAX
		ADD		EBX,0xffc				; p = i + 0xffc;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,0x1000				; i += 0x1000;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET
		
		
_shutdown:
	MOV AX,0x5301 ;�߼���Դ������,���������豸�İ汾�� 
	XOR BX,BX ;ϵͳ�£ɣϣ��豸�ɣ� 
	;MOV CX,0x0102 ;CH���汾��CL�Ӱ汾�� 
	INT 0x15