#include "bootpack.h"


void memorylist_task(struct TASK *task)
{
	struct SHEET *sheet = task->sht_task;
	/*
	struct TIMER *timer;
	int i, fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_c = COL8_FFFFFF;
	char s[30],cmdline[30]; // 存储命令行的字符串
	int x, y;



	fifo32_init(&task->fifo, 128, fifobuf, task);

	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);
	*/
	
	
	putStrings_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);
	
	sheet_refresh(sheet, 8, 28, 16, 44);
	
}