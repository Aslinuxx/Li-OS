# include "bootpack.h"

//控制电路如果已经准备完毕,cpu就可以向其发指令了;
// 控制电路准备完毕时,cpu从设备编号为0x0064处读取的数据的倒数第二位(即从最低位开始数第二位) 应该是0;
//所以如果cpu一直读取该设备的数据,不为0则说明还没准备好


struct FIFO32 *keyfifo;
int keydata0;

void wait_KBC_sendready(void){
	
	for (;;){
		
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY)==0){ //KEYSTA_SEND_NOTREADY=0x02 (0010) 与运算相当于看倒数第二位上是1还是0;
			break;
			
		}
		
	}
	return ;
	
	
}


// 键盘控制电路初始化;
void init_keyboard(struct FIFO32 *fifo, int data0){
	
	//将fifo缓冲区的信息保存到全局变量中
	keyfifo = fifo; // 将两个fifo的地址指向相同
	keydata0=data0; 
	
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,KBC_MODE);
	return ;
	
}

//键盘的端口号 port 即内存地址编号
#define PORT_KEYDAT		0x0060 //0x0060地址就是键盘;见p127



void inthandler21(int *esp)
//来自PS/2键盘的中断 /
{
	int data; 
	
	/*通知主PIC(PIC0),IRQ1已经受理完毕,
	即1路信号的中断已经发生,现在需要一直进行监控该路中断信号;
	IRQ1是键盘中断的信号路*/ 
	io_out8(PIC0_OCW2,0x61); //如果没有向PIC中发送该指令,则PIC会只会受理一次IRQ1中断
	
	data = io_in8(PORT_KEYDAT); // 读取键盘设备的写入信息 (port 即键盘设备的内存地址编号)
	
	//上面已经让cpu一直进行监听键盘设备的输入信息了;
	//这里需要将所有输入的信息存到缓存区内;
	
	fifo32_input(keyfifo,data+keydata0); // 键盘和鼠标输入的数要加上值再存放到fifo中,为了区分是哪个设备的数据;
	
	return ;


}
