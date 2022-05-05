# include "bootpack.h" 
# include <stdio.h>

/*PIC的初始化*/
void init_pic(void){
	
	//IMR是8位的中断屏蔽寄存器,8位对应8路中断信号IRQ,为1的时候说明中断信号被屏蔽,PIC自动忽略该路中断信号
	io_out8(PIC0_IMR,0xff); /*禁止主PIC的所有中断*/
	io_out8(PIC1_IMR,0xff);  // 禁止从PIC的所有中断
	
	//进行设定主PIC的设置
	//设定PIC就是设置PIC的4个8位的ICW初始化控制数据寄存器;
	// ICW3是设定主从PIC的连接;ICW1和ICW4与PIC主板配线方式,中断信号的电气特性等有关;
	// ICW2是设定中断发生时候,对应调用哪一类中断指令;
	io_out8(PIC0_ICW1,0x11);/*边沿触发模式(edge trigger mode)*/
	io_out8(PIC0_ICW2,0x20);/*IRQ0-7由INT 20-27接收*/ //INT表示中断,比如INT 0x10是显卡中断; INT 0x13是磁盘中断;
	io_out8(PIC0_ICW3,1 << 2);/*从PIC由IRQ2连接到主PIC*/
	io_out8(PIC0_ICW4,0x01); /*无缓冲区模式*/
	
	//设定从PIC设置
	io_out8(PIC1_ICW1,0x11);/*边沿触发模式(edge trigger mode)*/
	io_out8(PIC1_ICW2,0x28);/*IRQ8-15由INT 28-2f接收*/ //INT表示中断,比如INT 0x10是显卡中断; INT 0x13是磁盘中断;
	io_out8(PIC1_ICW3,2);/*从PIC由IRQ2连接到主PIC*/
	io_out8(PIC1_ICW4,0x01);/*无缓冲区模式*/
	
	io_out8(PIC0_IMR,0xfb); /*1111 1011 PIC1以外全部禁止*/ // 说明只用了一路IRQ
	io_out8(PIC1_IMR,0xff);/*1111 1111禁止所有中断*/ 
	
	return ;
	
	
}


//中断程序不要忘记注册到IDT中

void inthandler27(int *esp)
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07受付完了をPICに通知(7-1参照) */
	return;
}