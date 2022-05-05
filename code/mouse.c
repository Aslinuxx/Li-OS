#include "bootpack.h"
#include <stdio.h>


extern struct BOOTINFO *binfo;


struct FIFO32 *mousefifo;
int mousedata0;


void inthandler2c(int *esp)
/* 来自PS/2鼠标的中断 */
{	
	int data;
	io_out8(PIC1_OCW2,0x64);//通知从PIC(PIC1)IRQ12的受理已完毕; IRQ12是鼠标中断的监听电路;
	io_out8(PIC0_OCW2,0x62); //通知IRQ2的受理已完毕,即从PIC连接到主PIC;
	//向鼠标缓冲区FIFO中写入数据;
	data = io_in8(PORT_KEYDAT);
	fifo32_input(mousefifo,data+mousedata0);
	return ;
	
}


void enable_mouse(struct FIFO32 *fifo, int data0, struct mouse_state *mdec){
	//将鼠标fifo区的数据放到全局的fifo中;
	mousefifo=fifo;
	mousedata0=data0;
	
	//鼠标有效
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,MOUSECMD_ENABLE);
	mdec->phase=0;
	
	return ; // 如果激活成功,键盘控制器会返回ack(oxfa);应答
	
	
	
}


//再增加一个鼠标初始化,存满三个字符取一次;
//返回1表示三个字节已经存储好了;返回0表示还没有;
int mouse_decode(struct mouse_state *mdec, unsigned char data){
	char s[2];
	
	//除了开头的0xfa,其余的是三个字节一组
	if (mdec->phase==0){
		if (data==0xfa){
			//这里是显示鼠标的中断启动时候返回的0xfa;
			//但是由于后面是加了图层的;所以如果还直接写到VRAM中就会有错误;
			//需要写到图层0的buf缓冲区中;这里先不改了
			//sprintf(s,"%02x",data);
			//boxfill8(binfo->VRAM, binfo->SCRNX, COL8_008484, 0, 40, 200, 80); 
			//putStrings(binfo->VRAM,binfo->SCRNX,0,40,COL8_FFFFFF,"mouse:");
			//putStrings(binfo->VRAM,binfo->SCRNX,60,40,COL8_FFFFFF,s);// 这里传入的是s的首地址
			
			mdec->phase=1;
			
		}
		return 0;
	}else if(mdec->phase==1){
		if ((data & 0xc8)== 0x08){ //解决鼠标接触不良时候第一字节发送错误的问题;见P149
			mdec->dbuf[0]=data;
			mdec->phase=2;
		}
			return 0;
	}else if(mdec->phase==2){
			mdec->dbuf[1]=data;
			mdec->phase=3;
			return 0;
	}else if (mdec->phase==3){
			mdec->dbuf[2]=data;
			mdec->phase=1;
			
			//下面是对移动信息(x,y),按键状态btn的获取
			mdec->btn = mdec->dbuf[0] & 0x07; //btn是buf[0]的低三位
			mdec->x = mdec->dbuf[1];
			mdec->y = mdec->dbuf[2];
			
			if ((mdec->dbuf[0] & 0x10)!=0){//x有效
				mdec->x  |= 0xffffff00;
				
			}
			if ((mdec->dbuf[0] & 0x20)!=0){//y有效
				mdec->y |= 0xffffff00;
				
				
			}
			mdec->y = - mdec->y; //y需要取反
			
			return 1;
	}
	return -1;
	
}

/*
int mouse_decode(struct mouse_state *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		
		if ((dat & 0xc8) == 0x08) {
			
			mdec->dbuf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		
		mdec->dbuf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		
		mdec->dbuf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->dbuf[0] & 0x07;
		mdec->x = mdec->dbuf[1];
		mdec->y = mdec->dbuf[2];
		if ((mdec->dbuf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->dbuf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; 
		return 1;
	}
	return -1;
}

*/