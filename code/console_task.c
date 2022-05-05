#include "bootpack.h"
#include <string.h>
#include<stdio.h> 

//键盘输入字符集
static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
};

extern struct MEMORY_MANANGE *memman;


void console_task_main(struct TASK *task)
{
	struct SHEET *sheet = task->sht_task;
	
	struct SCL *scl = sheet->scl;//图层管理表的地址
	
	struct TIMER *timer;
	int i, fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_c = COL8_FFFFFF;
	char s[30],cmdline[30]; // 存储命令行的字符串
	int x, y, height=3;
	
	
	fifo32_init(&task->fifo, 128, fifobuf, task);

	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);
	
	putStrings_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			//task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) { // 光标定时器数据;
				if (i != 0) {
					timer_init(timer, &task->fifo, 0); 
					
						cursor_c = COL8_FFFFFF;
				
				} else {
					timer_init(timer, &task->fifo, 1); 
				
						cursor_c = COL8_000000;
					
				}
				timer_settime(timer, 50);
				
				
			
			}else if (256 <= i && i <= 511){ // 键盘数据
					
				
				if (i == 256+0x0e && cursor_x > 16){ // 退格键;删除一个字符,并将光标位置左移8个像素点;
				
					putStrings_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					cursor_x -= 8;
					
				}else if (i == 256 + 0x1c) { // 回车键
					// 
					putStrings_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					cmdline[cursor_x / 8 - 2] = 0;
					cursor_y = cons_newline(cursor_y, sheet);
					
					if (strcmp(cmdline,"CLS")==0){//如果是cls命令
						for (y = 28; y < 28 + 128; y++) {
							for (x = 8; x < 8 + 240; x++) {
								sheet->buf[x + y * sheet->bxsize] = COL8_000000;
							}
						}
						sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
						cursor_y = 28;
					}else if (strcmp(cmdline, "MEMORYLIST")==0) {
						//创建存管理窗口
						struct SHEET *sht_task_mem;
						unsigned char *buf_task_mem;
						
						sht_task_mem= sheet_alloc(scl);
						
						buf_task_mem = (unsigned char *) memman_alloc_4k(memman, 400*400,"Memory List UI");
						sheet_setbuf(sht_task_mem, buf_task_mem, 400, 400, -1); 
						make_window8(buf_task_mem, 400, 400, "Memory List", 2);
						
						sheet_slide(sht_task_mem, 400, 50);
						//showMemory(sht_task_mem);
						
						if (height>scl->top){
							height = scl->top;
						}
						sheet_updown(sht_task_mem, height);
						showMemory(sht_task_mem);
						
						/*
						struct TASK *task_mem;
						task_mem = task_alloc();
						task_mem->tss.esp = memman_alloc_4k(memman, 64 * 1024, "Task_b") + 64 * 1024 - 8;
						task_mem->tss.eip = (int) &memorylist_task;
						task_mem->tss.es = 1 * 8;
						task_mem->tss.cs = 2 * 8;
						task_mem->tss.ss = 1 * 8;
						task_mem->tss.ds = 1 * 8;
						task_mem->tss.fs = 1 * 8;
						task_mem->tss.gs = 1 * 8;
						*((int *) (task_mem->tss.esp + 4)) = (int) sht_task_mem;
						task_add(task_mem,sht_task_mem);*/
						
					}else if(strcmp(cmdline, "MEM")==0){
						
						
						unsigned int memtotal;
						memtotal = memtest(0x00400000, 0xbfffffff);
						
						sprintf(s, "total   %dMB", memtotal / (1024 * 1024));
						putStrings_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);
						sprintf(s, "free %dKB", memman_total(memman) / 1024);
						putStrings_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);
						
					}else if (strcmp(cmdline, "TIME")==0){
						//分配进程b中的窗口图层
	
						struct SHEET *sht_win_b; // 进程2中的窗口图层
						unsigned char *buf_win_b; // 进程2中的窗口缓冲区
						struct TASK *task_b;
						
						sht_win_b = sheet_alloc(scl);
						buf_win_b = (unsigned char *) memman_alloc_4k(memman, 144 * 52, "time_task UI");
						sheet_setbuf(sht_win_b, buf_win_b, 144, 52, 1); 
						sprintf(s, "time");
						make_window8(buf_win_b, 144, 52, s, 1);
						//make_textbox8(sht_win_b, 8, 28, 144, 16, COL8_FFFFFF);//描绘文本框
						sheet_slide(sht_win_b, 168,  56);
						
						if (height>scl->top){
							height = scl->top;
						}
						sheet_updown(sht_win_b, height);
						
						
						task_b = task_alloc("Time");
						task_b->tss.esp = memman_alloc_4k(memman, 64 * 1024, "Time task") + 64 * 1024 - 8;
						task_b->tss.eip = (int) &time_task_main;
						task_b->tss.es = 1 * 8;
						task_b->tss.cs = 2 * 8;
						task_b->tss.ss = 1 * 8;
						task_b->tss.ds = 1 * 8;
						task_b->tss.fs = 1 * 8;
						task_b->tss.gs = 1 * 8;
						*((int *) (task_b->tss.esp + 4)) = (int) task_b;
						//4.将新进程加到任务管理表末尾
						task_add(task_b,sht_win_b);
						
					}else if (strcmp(cmdline, "TEXT")==0){
						struct SHEET *sht_win; // 进程2中的窗口图层
						unsigned char *buf_win; // 进程2中的窗口缓冲区
						struct TASK *task_text;
						
						
						sht_win = sheet_alloc(scl);
						//2.对于图层中需要显示内容(缓冲区空间)比较大的,用内存分配函数进行分配内存;
						buf_win = (unsigned char *) memman_alloc_4k(memman,160*52,"Text UI");// 缓冲区大小;(160*80,即缓冲区需要多少个像素点,每个像素点对于1字节)
						//3.将该缓冲区连接到对应图层的缓冲区;即让对应图层的缓冲区地址等于该缓冲区;
						// 同时也设定下该图层的画布大小,以及图层背景板的透明色; 就是除了缓冲区外的画布像素是什么颜色;
						sheet_setbuf(sht_win,buf_win,160,52,-1);
						//4.向该图层的缓冲区写入数据(像素点阵信息); 2中只是分配了buf的内存空间大小,其内并没有数据;
						//make_window8(buf_win, 160, 68, "counter");
						make_window8(buf_win, 160, 68, "Text",1);
						make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);//描绘文本框
						cursor_x = 8; //光标的位置;  即为本框内光标输入到哪个字符了; 1个字符8个像素;
						cursor_c = COL8_FFFFFF;
						
						sheet_slide(sht_win, 158,  180);
						if (height>scl->top){
							height = scl->top;
						}
						sheet_updown(sht_win, height);
						
						task_text = task_alloc("Text");
						task_text->tss.esp = memman_alloc_4k(memman, 64 * 1024, "Text task") + 64 * 1024 - 8;
						task_text->tss.eip = (int) &text_task_main;
						task_text->tss.es = 1 * 8;
						task_text->tss.cs = 2 * 8;
						task_text->tss.ss = 1 * 8;
						task_text->tss.ds = 1 * 8;
						task_text->tss.fs = 1 * 8;
						task_text->tss.gs = 1 * 8;
						*((int *) (task_text->tss.esp + 4)) = (int) task_text;
						//4.将新进程加到任务管理表末尾
						task_add(task_text,sht_win);
						
					}else if (strcmp(cmdline, "TASKLIST")==0){
						struct SHEET *sht_task_manage; // 进程2中的窗口图层
						unsigned char *buf_task_manage; // 进程2中的窗口缓冲区
						struct TASK *task_manage;
						
						
						
						
						sht_task_manage = sheet_alloc(scl);
						buf_task_manage = (unsigned char *) memman_alloc_4k(memman, 400 * 400, "Task Management UI");
						sheet_setbuf(sht_task_manage, buf_task_manage, 400, 400, -1); 
						sprintf(s, "Task Management List");
						make_window8(buf_task_manage, 400, 400, s, 2);
						//make_textbox8(sht_win_b, 8, 28, 144, 16, COL8_FFFFFF);//描绘文本框
						sheet_slide(sht_task_manage, 400,  40);
						
						if (height>scl->top){
							height = scl->top;
						}
						sheet_updown(sht_task_manage, height);
						showTask(sht_task_manage);
						
						
						
						task_manage = task_alloc("Task Management");
						task_manage->tss.esp = memman_alloc_4k(memman, 64 * 1024, "Task manage") + 64 * 1024 - 8;
						task_manage->tss.eip = (int) &task_manage_task_main;
						task_manage->tss.es = 1 * 8;
						task_manage->tss.cs = 2 * 8;
						task_manage->tss.ss = 1 * 8;
						task_manage->tss.ds = 1 * 8;
						task_manage->tss.fs = 1 * 8;
						task_manage->tss.gs = 1 * 8;
						*((int *) (task_manage->tss.esp + 4)) = (int) task_manage;
						//4.将新进程加到任务管理表末尾
						task_add(task_manage,sht_task_manage);
						
						
						
					}else if (strcmp(cmdline, "SHUTDOWN")==0){
						shutdown();
						
						
					}else if(cmdline[0] !=0){ // 不是命令,且此行也不为空
						putStrings_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "No this command.", 20);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);
						
					}
					
						
					
					putStrings_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 16;
					
				}else {
					if (i < 0x54+256){
						if (keytable[i-256] != 0 && cursor_x<240){//正常字符
							s[0]=keytable[i-256];
							s[1]=0;
							
							cmdline[cursor_x / 8 - 2] = keytable[i-256]; // 这里使用cursor_x 作为记录输入的下标;
							//如果有退格命令,cursor_x就会减-8; 下次输入的新字符就会覆盖该字符的位置;
							//所以在退格中不用考虑减去字符的情况
							//如果只输入一个字符就退格;在按回车的时候就会进行由有一次更新;见570行
							
							putStrings_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
							cursor_x += 8; // 显示一个字符后,光标右移八个像素点
							
						}
					}
				}
			}
			
			
		
			boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
			sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}			
				
}

extern struct MTCL *mtcl;
void showMemory(struct SHEET *memoryListSheet)
{
	int curPosY=0,i;
	//make_window8(memoryListSheet,600,200,"Memory List",0);
	char str[70]="   From         To           Status";
	//参数:显示图层 显示的起始坐标(x,y) 字符颜色  背景颜色 字符串内容 字符串长度
	putStrings_sht(memoryListSheet,18,32+curPosY*30,COL8_000000,COL8_C6C6C6,str,50);
	curPosY=1;
	for (i=0;i<128;i++)
	{
		if (memman->used[i].flag==1)
		{
			sprintf (str,"0x%08x  0x%08x  %s",memman->used[i].addr,memman->used[i].addr+memman->used[i].size,memman->used[i].status);
			putStrings_sht(memoryListSheet,18,32+curPosY*30,COL8_000000,COL8_C6C6C6,str,50);
			curPosY++;
		}
	}
}

int cons_newline(int cursor_y, struct SHEET *sheet)
{
	int x, y;
	if (cursor_y < 28 + 112) { //换行
		cursor_y += 16; 
	} else {
		// 滚动
		for (y = 28; y < 28 + 112; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		for (y = 28 + 112; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	}
	return cursor_y;
}


void showTask(struct SHEET *taskListSheet)
{
	int curPosY=0,i, j=0;
	//make_window8(memoryListSheet,600,200,"Memory List",0);
	char str[70]="    ID       Name          Status";
	//参数:显示图层 显示的起始坐标(x,y) 字符颜色  背景颜色 字符串内容 字符串长度
	putStrings_sht(taskListSheet,18,32+curPosY*30,COL8_000000,COL8_C6C6C6,str,50);
	curPosY=1;
	for (i=0;i<10;i++)
	{	
		if (mtcl->tasks[i].state==ACTIVE)
		{	
			sprintf (str,"%d      %s           Running",j,mtcl->tasks[i].name);
			putStrings_sht(taskListSheet,48,52+curPosY*30,COL8_000000,COL8_C6C6C6,str,50);
			curPosY++;
			j++;
			
		}
		
		
	}
	//boxfill8(taskListSheet->buf, 400, COL8_C6C6C6, 0,0,400,400); //先用背景色填充上一次显示的
	//sheet_refresh(taskListSheet,0,0, 400,400);
	
	while (j<6){
		
		boxfill8(taskListSheet->buf, taskListSheet->bxsize, COL8_C6C6C6, 48,52+curPosY*30, 400,52+curPosY*30+30); //先用背景色填充上一次显示的
		sheet_refresh(taskListSheet,48,52+curPosY*30, 400,52+curPosY*30+30);
		curPosY++;
		j++;
	}
	
	//for (i=0; i<5; i++){
		//boxfill8(taskListSheet->buf, taskListSheet->bxsize, COL8_C6C6C6, 48,52+curPosY*30, 48+50,52+curPosY*30); //先用背景色填充上一次显示的
						
		//sheet_refresh(taskListSheet,48,52+curPosY*30, 48+50,52+curPosY*30);
		//curPosY++;
		
	//}
}
