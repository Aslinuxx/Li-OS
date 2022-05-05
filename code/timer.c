#include "bootpack.h"

#include <stdio.h>



//PIT初始化:需要向指定内存地址发送三次指令进行更改始终周期;
//1:需要向内存0x43处发送0x34; 固定用法
//2.将中断周期的低八位发送到内存0x40处; 
//3.将中断周期的高八位发送到内存0x40处；
//比如我们将中断周期为11932,就是100HZ,对应就是1秒会发生100次中断;
// 11932对应十六进制是0x2e9c;

/* struct TIMER timer1;
void init_pit(void){
	io_out8(PIT_CTRL,0x34);
	io_out8(PIT_CNT0,0x9c);
	io_out8(PIT_CNT0,0x2e);
	timer1.count=0;
	timer1.timeout=0;
	
	return ;
}
*/

#define TIMER_FLAGS_ALLOC		1	// 已分配
#define TIMER_FLAGS_USING		2 //正在使用

//extern struct BOOTINFO *binfo;

struct TCL tcl;

void init_pit(void){
	int i;
	struct TIMER *t;
	
	io_out8(PIT_CTRL,0x34);
	io_out8(PIT_CNT0,0x9c);
	io_out8(PIT_CNT0,0x2e);
	
	for (i=0;i<MAX_TIMERS;i++){
		tcl.timers[i].flags=0;//该图层未使用
		tcl.timers[i].tcl=&tcl;
		
	}
	
	t = timer_alloc(); // 分配一个新定时器;将其的timeout值设置成最大; 所以其就是哨兵;会一直位于链表尾部;保证链表不为空
	t->timeout = 0xffffffff;
	t->flags = TIMER_FLAGS_USING;
	t->next = 0;
	
	// 将哨兵加入链表
	tcl.t0=t;
	tcl.next_count=0xffffffff;// 
	
	
	
	
	return ;
}

/*
// 自己写一下定时器管理表的初始化;
struct TCL *tcl_init(struct MEMORY_MANANGE *man){//因为tcl所需空间也不小;所以也使用内存分配函数进行初始化
	
	
	struct TCL *tcl;
	int i;
	
	tcl = (struct TCL *) memman_alloc_4k(man,sizeof(struct TCL));
	// 这里有个大坑; 但凡调用内存分配函数之前,必须保证内存容量已经初始化了,内存检查完成;
	
	//char s[400];
	//sprintf(s,"tcl: %d size: %d",tcl,sizeof(struct TCL));
	//boxfill8(binfo->VRAM, binfo->SCRNX, COL8_008484, 0, 40, 200, 80); 
	//putStrings(binfo->VRAM,binfo->SCRNX,60,200,COL8_FFFFFF,s);
			//putStrings(binfo->VRAM,binfo->SCRNX,60,40,COL8_FFFFFF,s);// 这里传入的是s的首地址
	
	if (tcl==0){//内存分配失败;
		goto err;
	}
	
	
	tcl.count = 0;
	tcl.next=0xffffffff;// 
	tcl.using_sum=0;//最初一个在使用的定时器都没有
	
	for (i=0;i<MAX_TIMERS;i++){
		tcl.timers[i].flags=0;//该图层未使用
		tcl.timers[i].tcl=tcl;
		
	}
	
	
err:
	 return tcl;
	
	
}
*/


// 分配单个图层
struct TIMER *timer_alloc(){
	
	struct TIMER *timer;
	int i;
	//首次适应算法
	for (i=0;i<MAX_TIMERS;i++){
		if (tcl.timers[i].flags==0){
			timer = &tcl.timers[i];
			timer->flags=TIMER_FLAGS_ALLOC;
			return timer;
		}
	}
	return 0; //没有空闲的定时器了;
	
	
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0; /* 未使用 */
	return;
}


//对单个定时器进行初始化;
//指定单个定时的缓冲区地址(fifo); 
// 指定当timeout减为0时候向该定时器fifo中写入的数据;
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;

	return;
}

//新加入的定时器插入到链表中
void timer_settime(struct TIMER *timer, unsigned int timeout){
	
	int e;
	
	struct TIMER *t, *s;
	timer->timeout = timeout + tcl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	t = tcl.t0;
	
	//加入哨兵后,链表中一定是有节点的;
	
	//新加入的节点的count值小于最大节点的值;加在头部
	if (timer->timeout <= t->timeout) {
		
		tcl.t0 = timer;
		timer->next = t; // 原头部节点移到第二位
		tcl.next_count = timer->timeout;
		io_store_eflags(e);
		return;
	}
	
	// 新加入的节点插入到中间的合适位置
	for (;;) {
		s = t;
		t = t->next;
		
		if (timer->timeout <= t->timeout) {
			//在中间位置 s和t之间
			s->next = timer; 
			timer->next = t; 
			io_store_eflags(e);
			return;
		}
		
	}
	// 加入哨兵后,哨兵肯定在链表尾部;
	return ;
	
}


// 20号定时器中断处理程序;
// 这里加上多任务管理的时候,必须额外判断此时超时的是不是进程切换定时器;
//如果是就启动进程切换
void inthandler20(int *esp)
{	
	char ts = 0; // ts表示是否进行任务切换;之所以先将ts置为1,等最后再判断ts是否为1进行任务切换;
	// 是因为如果直接进行任务切换回出现中断没有处理完成的情况; 见P301

	struct TIMER *timer; 
	io_out8(PIC0_OCW2,0x60); 
	//通知主PIC(PIC0),IRQ0已经受理完毕,
	//即0路信号的中断已经发生,现在需要一直进行监控该路中断信号;
	//IRQ0是定时器中断的信号路
	//如果没有向PIC中发送该指令,则PIC会只会受理一次IRQ0中断
	// 这里IRQ0--IRQ7;对应0x60-0x67;
	// IRQ8-IRQ15也对应0x60-0x67;
	tcl.count++;
	if (tcl.next_count > tcl.count) { // count是当前计时器中断的计数; next_count 是当前最小的超时定时器的数值;
		return; // next_count即下一超时的时刻;
	}
	timer = tcl.t0; //将活跃中的定时器链表的头结点地址赋给timer;
	
	for (;;) {
		
		if (timer->timeout > tcl.count) {
			break;
		}
		
		timer->flags = TIMER_FLAGS_ALLOC;
		
		//这里判断是不是任务管理定时器
		if (timer != task_timer){
			fifo32_input(timer->fifo, timer->data);
		}
		else{
			ts = 1;
		}
		
		//该定时器超时了,所以不属于活跃中的定时器了;将其从链表中去掉;
		//即将链表指向该节点的下一节点;
		timer = timer->next; 
		
	}

	
	tcl.t0 = timer;
	tcl.next_count = timer->timeout;
	
	//进行任务切换
	if (ts !=0){
		task_switch();
	}

	
	return;
}
