#include "bootpack.h"


struct TIMER *task_timer;
struct MTCL *mtcl;



// 管理表初始化
struct TASK *task_init(struct MEMORY_MANANGE *man){
	
	struct TASK *task;
	int i;
	
	//开始用内存分配api进程分配内存;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT; 
	mtcl = (struct MTCL *) memman_alloc_4k(man, sizeof(struct MTCL),"Timer Manage List");
	

	
	
	// 初始化任务管理表
	//将所有的进程注册到GDT中
	for (i=0; i<MAX_TASKS; i++){
		mtcl->tasks[i].state = 0; // 0表示该任务不在使用中
		// 参数:段号 该段空间大小(多少字节) 该段的起始地址 该段的属性设置 
		mtcl->tasks[i].sel = (TASK_GDT0 + i)*8; // GDT中的段地址
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &mtcl->tasks[i].tss, AR_TSS32);
	}
	task = task_alloc("System");
	task->state=ACTIVE; // 活跃中 
	mtcl->running_sum = 1;
	mtcl->now = 0;
	mtcl->tasks_addr[0] = task;
	write_tr(task->sel); // 当前运行的进程号
	task_timer = timer_alloc();
	timer_settime(task_timer,2);// 0.02秒进行一次切换
	return task;
	

}



struct TASK *task_alloc(char *name){
	int i;
	struct TASK *task;
	for (i=0; i<MAX_TASKS; i++){
		if (mtcl->tasks[i].state == 0){
			task = &mtcl->tasks[i];
			task->state = SLEEP; //已占用;进行配置;目前处于休眠状态;(还未开始工作)
			sprintf (task->name,"%s",name); 
			
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; 
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			
			return task;
			
		}
		
		
	}
	return 0;
	
}


// 将新进程添加到任务管理表中
//向尾部追加
void task_add(struct TASK *task, struct SHEET *sht_task){
	
	//该进程所属的图层
	task->sht_task =  sht_task;
	
	//该图层所属的进程
	sht_task->task = task;
	
	task->state = ACTIVE;
	mtcl->tasks_addr[mtcl->running_sum] = task;
	mtcl->running_sum++;
	
	return ;
}


// 切换任务
// 轮转算法
// 这里必须要用到已经初始化的任务管理表; 且没有参数传入;
// 所以只能外部导入了

void task_switch(void){
	
	
	timer_settime(task_timer,2);
	if (mtcl->running_sum>=2){
		mtcl->now++;// 切换到下一进程节点;移位
		if (mtcl->now == mtcl->running_sum){
			mtcl->now=0; // 如果轮转到尾部了就需要重新指向头部;无环结构
		}
		jmpfar(0,mtcl->tasks_addr[mtcl->now]->sel);
		
	}
	return ;
	
}


void task_sleep(struct TASK *task)
{	int i;
	char ts=0;
	
	if (task->state == ACTIVE){//进程处于活跃状态
		if(task == mtcl->tasks_addr[mtcl->now]){
			ts =1; // 要活跃状态直接变为休眠,现需要将进程进行切换
			
		}
		
		//查找该进程在任务管理表中的哪个位置
		for(i=0; i<mtcl->running_sum; i++){
			if(mtcl->tasks_addr[i]==task){
				break;
			}
		}
		
		
		if(i<mtcl->now){
			mtcl->now--;
		}
		mtcl->running_sum--;
		//将该活跃进程后的进程往前移一位;该进程马上变为休眠了;
		for (;i<mtcl->running_sum;i++){
			mtcl->tasks_addr[i]=mtcl->tasks_addr[i+1];
		}
		
		task->state=SLEEP; // 变为休眠状态
		if (ts!=0){
			if(mtcl->now >= mtcl->running_sum){
				mtcl->now=0;
				
			}
			jmpfar(0,mtcl->tasks_addr[mtcl->now]->sel);
		}
		
	}
		
	
	return;
}

