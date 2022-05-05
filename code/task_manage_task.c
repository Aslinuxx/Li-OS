
#include "bootpack.h"

// 一个进程就相当于是一个独立的mian函数
// 所以其中的变量和另一个进程是完全独立的;即使说两个进程中的变量名完全一样,也不是同一变量;
//因为变量开辟的内存地址完全不一样;
void task_manage_task_main(struct TASK *task){
	
	
	
	struct SHEET *sht = task->sht_task;
	struct FIFO32 fifo;
	struct TIMER *timer1;
	int i, fifobuf[128];
	
	
	

   
		
	fifo32_init(&fifo,128,fifobuf,0);
	

	
	
	//showTask(sht);


	
	timer1 = timer_alloc();
	timer_init(timer1,&fifo,1);//1秒定时器
	timer_settime(timer1,200);//10秒超时
	
	
	
	while (1)
	{	
		
		io_cli();
		if (fifo32_status(&fifo) == 0){
			//task_sleep();
			//io_sti();
			//如果要全力卯足劲开始计数;就一定不要用io_stihlt();
			io_stihlt();
			
		}else{
			i = fifo32_get(&fifo);
			io_sti();
			
			if (i==1){
				showTask(sht);
				timer_settime(timer1,200);
				
			
			}
		
		}
	
	}
}

