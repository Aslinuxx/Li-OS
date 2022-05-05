; haribote-os boot asm
; TAB=4

; 切换到保护模式; 也就是切换到32位模式;
;[INSTRSET "i486p"]				; 想使用486的指令的记述
; instrset 指令是为了使用486的指令集,即使用LGDT,EAX,CR0等关键字
[INSTRSET "i486p"]	

VBEMODE	EQU		0x105			; 1024 x  768 x 8bit
; 分辨率对应情况
;	0x100 :  640 x  400 x 8bit
;	0x101 :  640 x  480 x 8bit
;	0x103 :  800 x  600 x 8bit
;	0x105 : 1024 x  768 x 8bit
;	0x107 : 1280 x 1024 x 8bit

BOTPAK	EQU		0x00280000		; bootpack的内存地址编号 加载地址
DSKCAC	EQU		0x00100000		; 磁盘存储的位置
DSKCAC0	EQU		0x00008000		; 磁盘存储的位置(真实模式,即16位模式的实际地址模式)


; 底层参数在asmhead.nas 汇编文件中
; 如下的有关BOOT_INFO的设定信息;
; 各个参数的值都存在内存地址中,所以我们只需调用内存地址处存储的值即可;
; 下面的相当于定义常量 比如LEDS EQU 0x0ff1; 就是 LEDS=0x0ff1
CYLS	EQU		0x0ff0			; 设定启动区; EQU:equal
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; 关于颜色数量的信息。颜色的位数
SCRNX	EQU		0x0ff4			; 分辨率的X(screen x)
SCRNY	EQU		0x0ff6			; 分辨率的Y(screen Y)
VRAM	EQU		0x0ff8			; 图像缓冲区的起始地址

		ORG		0xc200			; 该程序将被装载到内存的什么地址

;画面设定
; 以下是320*200的分辨率设定
		;MOV		AL,0x13			;VGA显卡,320*200*8位彩色
		;MOV		AH,0x00
		;INT		0x10
		;MOV		BYTE [VMODE],8	;记录画面模式 上面VGA显卡的8;感觉是8个通道的意思
		;MOV		WORD [SCRNX],320
		;MOV		WORD [SCRNY],200
		;MOV		DWORD [VRAM],0x000a0000
		
; 1:查询VBE是否存在
		MOV		AX,0x9000
		MOV		ES,AX
		MOV		DI,0
		MOV		AX,0x4f00
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320
; 2:检查VBE的版本
		MOV		AX,[ES:DI+4]
		CMP		AX,0x0200
		JB		scrn320			; if (AX < 0x0200) goto scrn320
;3:取得画面的信息		
		MOV		CX,VBEMODE
		MOV		AX,0x4f01
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320
; 4:画面模式信息的确认 
		CMP		BYTE [ES:DI+0x19],8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b],4
		JNE		scrn320
		MOV		AX,[ES:DI+0x00]
		AND		AX,0x0080
		JZ		scrn320			; 模式属性的bit7是0,所以放弃
; 5:画面模式的切换
		MOV		BX,VBEMODE+0x4000
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8	; 画面モードをメモする（C言語が参照する）
		MOV		AX,[ES:DI+0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14]
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28]
		MOV		[VRAM],EAX
		JMP		keystatus
; 如果画面分辨率模式切换失败就使用320*200的分辨率
scrn320:
		; 以下是320*200的分辨率设定; 
		MOV		AL,0x13			;VGA显卡,320*200*8位彩色
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	;记录画面模式 上面VGA显卡的8;感觉是8个通道的意思
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000
		
	
; 用BIOS获取键盘上各种led指示灯的状态
keystatus:
		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS; 键盘中断
		MOV		[LEDS],AL


		
		
;   PIC禁止一切中断
;	根据AT兼容机的规格,如果要初始化PIC
;	必须在CLI之前进行,否则有时候会挂起(关闭中断允许;CLI是关闭中断允许的意思,STI是恢复中断允许的意思;见P80;)
;	之后进行PIC的初始化

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; 如果连着执行多个out命令,有些机型会无法正常运行; 
								;NOP指令是什么都不做,让cpu休息一个时钟长的时间
		OUT		0xa1,AL

		CLI						; 禁止cpu级别的中断


; 让cpu能够访问1MB以上的内存空间,32位模式能使用的内存空间远大于1MB,16位cpu最多只能指定1MB左右的内存;
; 见P49,P36; 通过使用段寄存器与基址寄存器的组合将 内存由2^16字节=64k扩展到1MB; ESP*16+BX
; 而32位cpu,使用的基址寄存器就是32位寄存器; 2^32 字节= 4096Mb=4Gb 内存;

; 以下程序是在设定A20GATE 
		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout
		
; 切换到保护模式
;[INSTRSET "i486p"]				; 想使用486的指令的记述
; instrset 指令是为了使用486的指令集,即使用LGDT,EAX,CR0等关键字

		LGDT	[GDTR0]			; 设定临时GDT 段号记录表
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; 设置bit31为0(为了禁止分页)
		OR		EAX,0x00000001	; bit0设为0(为了切换到保护模式) 

		MOV		CR0,EAX
		JMP		pipelineflush
		
pipelineflush:
		MOV		AX,1*8			;  壜?幨揑抜 32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; 传送到bootpack

		MOV		ESI,bootpack	; 转送源
		MOV		EDI,BOTPAK		; 转送目的地
		MOV		ECX,512*1024/4
		CALL	memcpy

; 顺便把磁盘数据也传送到本来的位置

; 首先从启动扇区开始

		MOV		ESI,0x7c00		; 转送源
		MOV		EDI,DSKCAC		; 转送目的地
		MOV		ECX,512/4
		CALL	memcpy

; 所有剩下的

		MOV		ESI,DSKCAC0+512	; 转送源
		MOV		EDI,DSKCAC+512	; 转送目的地
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 从柱面数变换为 字节数/4
		SUB		ECX,512/4		; 减去IPL
		CALL	memcpy
		

; 必须交由asmhead来完成的工作,至此已经全部结束;
; 接下来就交由bootpack.c来完成

; bootpackd的启动

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 没有需要传送的内容时
		MOV		ESI,[EBX+20]	; 转送源 old_address
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 转送目的地 new_address
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 栈初始值
		JMP		DWORD 2*8:0x0000001b


waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		
		;以下的IN AL,0x60 是程序中没有,但是书本上有的
		IN		 AL,0x60 		; 空读(为了清空数据接收缓冲区中的垃圾数据)
		
		JNZ		waitkbdout		; ANDの結果が0でなければwaitkbdoutへ
		RET

memcpy: ;复制内存处内容的程序
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 如果运算结果不为0就继续跳转到memcpy
		RET

; memcpy(old_address,new_address, data_size); 

		ALIGNB	16
		
;GDT 和IDT的设置
GDT0:
		RESB	8				; 零选择器 NULL selsctor
		DW		0xffff,0x0000,0x9200,0x00cf	; 可读写的段(segment) 32bit 32位
		DW		0xffff,0x0000,0x9a28,0x0047	; 可执行的段(segment) 32bit（bootpack用）

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack: