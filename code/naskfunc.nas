; naskfunc
; TAB=4

[FORMAT  "WCOFF"] ; 
[BITS 32] ; 
[INSTRSET "i486p"]				; 该程序是给486使用的

; 以下是制作目标文件的信息
[FILE "naskfunc.nas"] ; 源文件名信息
	; 实现内存写入函数
	GLOBAL _io_hlt,_io_cli,_io_sti, _io_stihlt
	GLOBAL _io_load_eflags, _io_store_eflags, _write_tr, _jmpfar
	GLOBAL _io_in8, _io_out8
	GLOBAL _load_gdtr, _load_idtr
	GLOBAL	 _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c ,_asm_inthandler20
	GLOBAL _load_cr0, _store_cr0
	GLOBAL _memtest_sub
	GLOBAL	_asm_end_app, _shutdown 
	; 中断执行完后,不能执行return;(即RET指令);必须执行IRETD指令; 所以借助汇编完成;
	EXTERN	_inthandler21, _inthandler27, _inthandler2c ,_inthandler20
	
	
	
; 以下是HLT函数的实现部分
[SECTION .text]		; 目标文件中写了这些后在写程序
_io_hlt: 			; 函数名;对应c文件头部的 void io_hlt(void)
	HLT
	RET				; RET:return 和c中一样就是返回的意思;该函数没有返回值 相当于return void

_io_load_eflags:   ;int io_load_eflags(void); 返回标志位寄存器的状态
	PUSHFD 			;PUSHFD:push flags double-word;  意思是将标志位的值按照双字长(DW :32位 4字节 int长度)入栈;
	POP EAX 		;
	RET 
	
_io_store_eflags: ; void io_store_eflags(int eflags);
	MOV EAX,[ESP+4]
	PUSH EAX
	POPFD
	RET 

; 向TR寄存器中写入当前执行中的进程编号
_write_tr: ; void write_tr();
 	LTR [ESP+4]
	RET

; 要进行进程切换,必须要使用 far模式的JMP命令
; 即向EPI寄存器(cpu下一指令内存地址寄存器) 和 cs寄存器(代码段寄存器)
_jmpfar: ;切换进程
	JMP		FAR [ESP+4] ;epi cs ;JMP 4*8:0 向EPI寄存器写入下一跳的内存地址,4是段号,*8之后就是段4的地址段;固定写法
										; 0表示向cs代码段寄存器写入0
	RET 

_io_cli: ; void io_cli(void);关闭标志寄存器; 关闭中断允许
	CLI ;固定指令
	RET 
	
_io_sti: ;void io_sti(void); 恢复中断允许
	STI
	RET
_io_stihlt: ; void io_stihlt(void);
	STI
	HLT
	RET 
	
	
; I/O指令; 向设备发送信息Out,取得设备的信息In;
_io_in8: ; int io_in8(int port); //从哪个端口输入的数据; In是设备向cpu进行发送信息;
	mov EDX,[ESP+4]
	mov EAX,0
	IN	AL,DX  
	RET 

; out8是cpu向设备(port,端口)进行发送信息
_io_out8: ; void io_out8(int port, int data); 向内存地址中写入数据
	mov EDX,[ESP+4] ; 端口号 ;ESP+4存放第一个传入参数
	mov AL,[ESP+8] ; 数据;ESP+8存放第2个传入参数;
	OUT DX,AL
	RET 

; 将设置好的GDT和IDT存放到GDTR寄存器和IDTR寄存器中;
_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX ; [ESP+4]存放第一个传入的参数,[ESP+6]是第二个传入的参数;
		LIDT	[ESP+6]
		RET
	
_asm_inthandler20: ; 跳转到21号中断执行程序void inthandler27(int *esp)
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


; 实现读取cr0控制寄存器内容,与写入cr0寄存器内容的汇编程序;
_load_cr0: ; int load_cr0(void);
		mov		EAX,CR0
		RET 
		
_store_cr0: ; void store_cr0(int cr0);
		mov EAX,[ESP+4]
		mov CR0,EAX
		RET
		
		
; 以下是实现内存容量查询的汇编程序
_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; （EBX, ESI, EDI も使いたいので）
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
	MOV AX,0x5301 ;高级电源管理功能,配置连接设备的版本号 
	XOR BX,BX ;系统ＢＩＯＳ设备ＩＤ 
	;MOV CX,0x0102 ;CH主版本号CL从版本号 
	INT 0x15