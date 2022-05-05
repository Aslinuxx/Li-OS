#include "bootpack.h"
#include<stdio.h> 


static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
};



void text_task_main(struct TASK *task){
	
	struct SHEET *sht_win = task->sht_task;
	
	//struct FIFO32 fifo = task->fifo;
	char s[40];
	int i,cursor_x=8,cursor_c,fifobuf[128];
	struct TIMER *timer3;
	
	
	fifo32_init(&task->fifo, 128, fifobuf, task);
	
	//定时器3:模拟光标;设置为0.5秒闪烁一次
	timer3 = timer_alloc();
	timer_init(timer3,&task->fifo,1); //光标定时器
	timer_settime(timer3,50);
	
	
	
	//以下是操作系统启动之后的程序;前面都是在进行基础设置;
	for (;;){
		
		
		
		//count ++; //定时器中断自己的count计数
		io_cli(); //关闭中断允许;CLI是关闭中断允许的意思,STI是恢复中断允许的意思;见P80；
		
		
		if (fifo32_status(&task->fifo) == 0){
			//task_sleep(task_a);
			//io_sti();
			//使用定时器时候不能让cpu休眠;
			//io_sti();
			
			
			//stihlt();相当于恢复中断允许后再休眠;
			//这时候
			io_stihlt(); // 如果缓冲区没有数据,说明键盘没有输入中断; 执行hlt,并恢复中断允许 sti；
			
		}else{
			//看看fifo区是哪个设备的数据
			i = fifo32_get(&task->fifo); 
			io_sti();
			if (256 <= i && i <= 511){ // 键盘数据
					if (i < 0x54+256){
						if (keytable[i-256] != 0 && cursor_x<144){//正常字符
							s[0]=keytable[i-256];
							s[1]=0;
							putStrings_sht(sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF,s,1);
							cursor_x += 8; // 显示一个字符后,光标右移八个像素点
						}
					}
					
					if (i == 256+0x0e && cursor_x > 8){ // 退格键;删除一个字符,并将光标位置左移8个像素点;
					
						putStrings_sht(sht_win,cursor_x,28,COL8_000000,COL8_FFFFFF," ",1);
						cursor_x -= 8;
					}
					// 每次显示完字符或者删除字符后,再重新显示光标的位置;
					boxfill8(sht_win->buf,sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x+7,43);
					sheet_refresh(sht_win,cursor_x, 28, cursor_x+8, 44);
				
			}else if (i<= 1){ //光标定时器的fifo内数据 是0或1
				if (i!=0){
					//设置光标颜色
					timer_init(timer3,&task->fifo,0);
					cursor_c = COL8_000000;
			
				
				}else if (i==0){
					timer_init(timer3,&task->fifo,1);
					cursor_c = COL8_FFFFFF;
					
				
			}
			//光标闪烁
			timer_settime(timer3, 50);
			boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
			sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			
			}
		}
		
	}
	
}