; haribote-os boot asm
; TAB=4

; �л�������ģʽ; Ҳ�����л���32λģʽ;
;[INSTRSET "i486p"]				; ��ʹ��486��ָ��ļ���
; instrset ָ����Ϊ��ʹ��486��ָ�,��ʹ��LGDT,EAX,CR0�ȹؼ���
[INSTRSET "i486p"]	

VBEMODE	EQU		0x105			; 1024 x  768 x 8bit
; �ֱ��ʶ�Ӧ���
;	0x100 :  640 x  400 x 8bit
;	0x101 :  640 x  480 x 8bit
;	0x103 :  800 x  600 x 8bit
;	0x105 : 1024 x  768 x 8bit
;	0x107 : 1280 x 1024 x 8bit

BOTPAK	EQU		0x00280000		; bootpack���ڴ��ַ��� ���ص�ַ
DSKCAC	EQU		0x00100000		; ���̴洢��λ��
DSKCAC0	EQU		0x00008000		; ���̴洢��λ��(��ʵģʽ,��16λģʽ��ʵ�ʵ�ַģʽ)


; �ײ������asmhead.nas ����ļ���
; ���µ��й�BOOT_INFO���趨��Ϣ;
; ����������ֵ�������ڴ��ַ��,��������ֻ������ڴ��ַ���洢��ֵ����;
; ������൱�ڶ��峣�� ����LEDS EQU 0x0ff1; ���� LEDS=0x0ff1
CYLS	EQU		0x0ff0			; �趨������; EQU:equal
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; ������ɫ��������Ϣ����ɫ��λ��
SCRNX	EQU		0x0ff4			; �ֱ��ʵ�X(screen x)
SCRNY	EQU		0x0ff6			; �ֱ��ʵ�Y(screen Y)
VRAM	EQU		0x0ff8			; ͼ�񻺳�������ʼ��ַ

		ORG		0xc200			; �ó��򽫱�װ�ص��ڴ��ʲô��ַ

;�����趨
; ������320*200�ķֱ����趨
		;MOV		AL,0x13			;VGA�Կ�,320*200*8λ��ɫ
		;MOV		AH,0x00
		;INT		0x10
		;MOV		BYTE [VMODE],8	;��¼����ģʽ ����VGA�Կ���8;�о���8��ͨ������˼
		;MOV		WORD [SCRNX],320
		;MOV		WORD [SCRNY],200
		;MOV		DWORD [VRAM],0x000a0000
		
; 1:��ѯVBE�Ƿ����
		MOV		AX,0x9000
		MOV		ES,AX
		MOV		DI,0
		MOV		AX,0x4f00
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320
; 2:���VBE�İ汾
		MOV		AX,[ES:DI+4]
		CMP		AX,0x0200
		JB		scrn320			; if (AX < 0x0200) goto scrn320
;3:ȡ�û������Ϣ		
		MOV		CX,VBEMODE
		MOV		AX,0x4f01
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320
; 4:����ģʽ��Ϣ��ȷ�� 
		CMP		BYTE [ES:DI+0x19],8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b],4
		JNE		scrn320
		MOV		AX,[ES:DI+0x00]
		AND		AX,0x0080
		JZ		scrn320			; ģʽ���Ե�bit7��0,���Է���
; 5:����ģʽ���л�
		MOV		BX,VBEMODE+0x4000
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8	; �����`�ɤ��⤹�루C���Z�����դ��룩
		MOV		AX,[ES:DI+0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14]
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28]
		MOV		[VRAM],EAX
		JMP		keystatus
; �������ֱ���ģʽ�л�ʧ�ܾ�ʹ��320*200�ķֱ���
scrn320:
		; ������320*200�ķֱ����趨; 
		MOV		AL,0x13			;VGA�Կ�,320*200*8λ��ɫ
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	;��¼����ģʽ ����VGA�Կ���8;�о���8��ͨ������˼
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000
		
	
; ��BIOS��ȡ�����ϸ���ledָʾ�Ƶ�״̬
keystatus:
		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS; �����ж�
		MOV		[LEDS],AL


		
		
;   PIC��ֹһ���ж�
;	����AT���ݻ��Ĺ��,���Ҫ��ʼ��PIC
;	������CLI֮ǰ����,������ʱ������(�ر��ж�����;CLI�ǹر��ж��������˼,STI�ǻָ��ж��������˼;��P80;)
;	֮�����PIC�ĳ�ʼ��

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; �������ִ�ж��out����,��Щ���ͻ��޷���������; 
								;NOPָ����ʲô������,��cpu��Ϣһ��ʱ�ӳ���ʱ��
		OUT		0xa1,AL

		CLI						; ��ֹcpu������ж�


; ��cpu�ܹ�����1MB���ϵ��ڴ�ռ�,32λģʽ��ʹ�õ��ڴ�ռ�Զ����1MB,16λcpu���ֻ��ָ��1MB���ҵ��ڴ�;
; ��P49,P36; ͨ��ʹ�öμĴ������ַ�Ĵ�������Ͻ� �ڴ���2^16�ֽ�=64k��չ��1MB; ESP*16+BX
; ��32λcpu,ʹ�õĻ�ַ�Ĵ�������32λ�Ĵ���; 2^32 �ֽ�= 4096Mb=4Gb �ڴ�;

; ���³��������趨A20GATE 
		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout
		
; �л�������ģʽ
;[INSTRSET "i486p"]				; ��ʹ��486��ָ��ļ���
; instrset ָ����Ϊ��ʹ��486��ָ�,��ʹ��LGDT,EAX,CR0�ȹؼ���

		LGDT	[GDTR0]			; �趨��ʱGDT �κż�¼��
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; ����bit31Ϊ0(Ϊ�˽�ֹ��ҳ)
		OR		EAX,0x00000001	; bit0��Ϊ0(Ϊ���л�������ģʽ) 

		MOV		CR0,EAX
		JMP		pipelineflush
		
pipelineflush:
		MOV		AX,1*8			;  ��?�ʓI�i 32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; ���͵�bootpack

		MOV		ESI,bootpack	; ת��Դ
		MOV		EDI,BOTPAK		; ת��Ŀ�ĵ�
		MOV		ECX,512*1024/4
		CALL	memcpy

; ˳��Ѵ�������Ҳ���͵�������λ��

; ���ȴ�����������ʼ

		MOV		ESI,0x7c00		; ת��Դ
		MOV		EDI,DSKCAC		; ת��Ŀ�ĵ�
		MOV		ECX,512/4
		CALL	memcpy

; ����ʣ�µ�

		MOV		ESI,DSKCAC0+512	; ת��Դ
		MOV		EDI,DSKCAC+512	; ת��Ŀ�ĵ�
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; ���������任Ϊ �ֽ���/4
		SUB		ECX,512/4		; ��ȥIPL
		CALL	memcpy
		

; ���뽻��asmhead����ɵĹ���,�����Ѿ�ȫ������;
; �������ͽ���bootpack.c�����

; bootpackd������

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; û����Ҫ���͵�����ʱ
		MOV		ESI,[EBX+20]	; ת��Դ old_address
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; ת��Ŀ�ĵ� new_address
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; ջ��ʼֵ
		JMP		DWORD 2*8:0x0000001b


waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		
		;���µ�IN AL,0x60 �ǳ�����û��,�����鱾���е�
		IN		 AL,0x60 		; �ն�(Ϊ��������ݽ��ջ������е���������)
		
		JNZ		waitkbdout		; AND�νY����0�Ǥʤ����waitkbdout��
		RET

memcpy: ;�����ڴ洦���ݵĳ���
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; �����������Ϊ0�ͼ�����ת��memcpy
		RET

; memcpy(old_address,new_address, data_size); 

		ALIGNB	16
		
;GDT ��IDT������
GDT0:
		RESB	8				; ��ѡ���� NULL selsctor
		DW		0xffff,0x0000,0x9200,0x00cf	; �ɶ�д�Ķ�(segment) 32bit 32λ
		DW		0xffff,0x0000,0x9a28,0x0047	; ��ִ�еĶ�(segment) 32bit��bootpack�ã�

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack: