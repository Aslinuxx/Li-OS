

//调用c中的库
#include<stdio.h> // c中的i/o库
#include "bootpack.h" //""双引号表示头文件与源文件在同一文件夹; <>表示头文件位于编译器提供的文件夹里;



//struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0; //屏幕设置信息地址; //这里放到全局来定义是后面也用到该结构体了;省得再传递地址;
//extern struct TIMER timer1;

struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;

struct MEMORY_MANANGE *memman = (struct MEMORY_MANANGE *) MEMMAN_ADDR;

//主函数
void HariMain(void){
	
	//struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	
	struct FIFO32 fifo;
	char s[40];
	int fifobuf[128];//全局fifo缓存取的大小是 128*4字节
	struct mouse_state mdec;
	struct SCL *scl;//声明图层管理表
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, buf_mouse[256]; //也是用的fifo//背景图层和鼠标图层的缓冲区;即背景图层画什么内容,鼠标图层画的像素点阵
	struct SHEET *sht_cons;
	unsigned char *buf_cons;
	
	
	
	unsigned int memtotal;
	int mx,my,i,count=0;
	
	struct TASK *task_a,  *task_cons;//a任务是主进程
	
	
	int j,x,y,mmx=-1,mmy=-1;
	//单击左键切换窗口
	//长按左键,且鼠标指针在移动,我们判断,移动的距离如果大于0;那么图层就处于移动模式
	struct SHEET *sht=0;
	
	
	//GDT和IDT初始化
	init_gdtidt(); //
	//设置PIC(中断监听电路); 哪路IRQ负责监听什么中断(鼠标/键盘...)
	init_pic();
	io_sti(); /* IDT/PIC的初始化结束了所以解除CPU的中断禁止 */

	//键盘/鼠标缓冲区初始化 / 定时器fifo等
	//全局fifo初始化
	fifo32_init(&fifo,128,fifobuf,0);
	
	//这里定时器管理表的初始化已经包含了PIT的初始化设置了
	// 这里调用了内存分配函数,那么再分配内存之前一定要内存管理先做;
	//否则会内存分配失败;
	//定时器中断是IRQ0,也必须置为0; 
	init_pit();
	//键盘控制电路初始化
	init_keyboard(&fifo,256); // 键盘的fifo数据必须加上256,依次区分开不同设备
	enable_mouse(&fifo,512,&mdec);//鼠标fifo数据加上512
	io_out8(PIC0_IMR, 0xf8); /* IRQ0,1,2全部有效;0是定时器,1是键盘,2是从PIC有效;  */ 
	io_out8(PIC1_IMR, 0xef);/* PIC1的鼠标中断许可(1110 1111) */
	
	
	

	/*内存初始化必须得在内存分配之前*/
	//进行内存容量查询
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	//开辟两块大的段
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	memman->used[0].addr=0x9e000;
	memman->used[0].size=0x00400000-0x9e000;
	memman->used[0].flag=1;
	sprintf (memman->used[0].status,"System");

	

	//设定调色板
	init_palette(); 


	//初始化图层管理表
	scl = scl_init(memman,binfo->VRAM, binfo->SCRNX,binfo->SCRNY);
	
	task_a = task_init(memman);
	fifo.task = task_a; 
	
	
	
	
	

	//分配背景图层
	sht_back = sheet_alloc(scl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->SCRNX * binfo->SCRNY,"Backgroud UI" ); 
	sheet_setbuf(sht_back,buf_back,binfo->SCRNX,binfo->SCRNY,-1); //-1是透明色
	init_screen(buf_back, binfo->SCRNX, binfo->SCRNY);
	
	
	// 分配鼠标图层
	sht_mouse = sheet_alloc(scl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	//初始化鼠标像素点阵;
	init_mouse_cursor8(buf_mouse, 99);
	// 绘制鼠标图案;鼠标图案绘制在第1层;背景是第0层;
	mx = (binfo->SCRNX - 16) / 2; /*在画面中央坐标计算*/ // 获取鼠标像素矩阵在画布中的起始坐标;
	my = (binfo->SCRNY - 28 - 16) / 2;
	
	
	//分配开机欢迎界面
	struct SHEET *sht_welcome;
	unsigned char *buf_welcome;
	
	sht_welcome = sheet_alloc(scl);
	
	buf_welcome = (unsigned char *) memman_alloc_4k(memman, 300*100,"Welcome UI");
	sheet_setbuf(sht_welcome, buf_welcome, 300, 100, -1); 
	make_window8(buf_welcome, 300, 100, "LI'S OS", 2);
	
	sprintf (s,"Welcome to my OS!");
	putStrings_sht(sht_welcome,30,30,COL8_000000,COL8_C6C6C6,s,20);
	
	sprintf (s,"Press F11 to start cmd window!");
	putStrings_sht(sht_welcome,30,70,COL8_000000,COL8_C6C6C6,s,20);
	
	
	sheet_slide(sht_welcome, 300, 200);
	//showMemory(sht_task_mem);
	
	
	

	
	//分配命令行窗口进程; 
	//分配命令行窗口中的窗口图层
	sht_cons = sheet_alloc(scl);
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165,"Console UI");
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); 
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	
	//分配命令行任务进程
	task_cons = task_alloc("Console");
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024,"Console task") + 64 * 1024 - 8;
	task_cons->tss.eip = (int) &console_task_main;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	//*((int *) (task_cons->tss.esp + 8)) = (int) sht_cons;
	*((int *) (task_cons->tss.esp + 4)) = (int) task_cons;
	
	task_add(task_cons,sht_cons); 
	task_sleep(task_cons);
	
	
	
	sheet_slide(sht_back, 0, 0); // 图层平移
	sheet_slide(sht_cons, 80, 300);
	sheet_slide(sht_mouse, mx, my);
	
	
	// 给各个图层分配层级; 注意不管是哪个进程的图层分配图层都是依靠图层管理表
	sheet_updown(sht_back, 0); 
	sheet_updown(sht_cons, -1);
	sheet_updown(sht_welcome, 1);
	sheet_updown(sht_mouse, 2);
	
	

	
	//显示此时的内存容量;
	//sprintf(s,"memory: %dMB   free: %dKB",memtotal/(1024*1024),memman_total(memman)/1024);
	//putStrings(buf_back,binfo->SCRNX,0,84,COL8_FFFFFF,s);
	sheet_refresh(sht_back, 0, 84, binfo->SCRNX, 110);// 刷新所有图层显示



	
	
	//以下是操作系统启动之后的程序;前面都是在进行基础设置;
	for (;;){
		
		//sprintf(s,"%010d",tcl.count);
		//boxfill8(buf_win, 160, COL8_C6C6C6, 40, 28, 119, 43);
		//putStrings(buf_win, 160, 40, 28, COL8_000000, s);
		//sheet_refresh(sht_win, 40, 28, 120, 44);
		
		count ++; //定时器中断自己的count计数
		io_cli(); //关闭中断允许;CLI是关闭中断允许的意思,STI是恢复中断允许的意思;见P80；
		
		
		if (fifo32_status(&fifo) == 0){
			//task_sleep(task_a);
			//io_sti();
			//使用定时器时候不能让cpu休眠;
			//io_sti();
			
			
			//stihlt();相当于恢复中断允许后再休眠;
			//这时候
			io_stihlt(); // 如果缓冲区没有数据,说明键盘没有输入中断; 执行hlt,并恢复中断允许 sti；
			
		}else{
			//看看fifo区是哪个设备的数据
			i = fifo32_get(&fifo); 
			io_sti();
			if (256 <= i && i <= 511){ // 键盘数据
				
				// 键盘的真实数据必须要减去加上的256标识
				//使用新的字符显示api；这里包含了显示和图层刷新
				//参数:显示图层 显示的起始坐标(x,y) 字符颜色  背景颜色 字符串内容 字符串长度
				//sprintf(s,"keyboard: %02x",i-256); 
				//putStrings_sht(sht_back,260,100,COL8_FFFFFF, COL8_008484,s,13);
				
				
				//实现按F11启动命令行进程
				
					if (i==0x57+256){//F11按下去按键编码是0x57
						if(task_cons->state != ACTIVE){ //2表示活跃状态
							int height = 2;
							task_add(task_cons,sht_cons); // 唤醒该任务
							
							if (height>scl->top){
								height = scl->top;
							}
							sheet_updown(sht_cons, height);
						}
					}
				
				
					fifo32_input(&sht->task->fifo,i);
				
				
				
	
				
				
			} else if(512 <= i && i <= 767){ // 鼠标数据
				if (mouse_decode(&mdec,i-512)!=0){
						sprintf(s,"[lcr %4d %4d]",mdec.x,mdec.y); //指针变量可以使用-> 等同于(*p).xx =p->xx; 非指针结构体直接p.xx; 
						//非指针结构体就是相当于普通寄存器;就是直接存在寄存器中的 
						
						//下面三个if是判断鼠标的三个按键是否有按下;左键.中间滚轮.鼠标右键
						if ((mdec.btn & 0x01) != 0) {
							s[1] = 'L';
						}
						if ((mdec.btn & 0x02) != 0) {
							s[3] = 'R';
						}
						if ((mdec.btn & 0x04) != 0) {
							s[2] = 'C';
						}
						//以下两句式根据鼠标像素矩阵大小绘制的图案范围
						//boxfill8(binfo->VRAM, binfo->SCRNX, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
						//putStrings(binfo->VRAM, binfo->SCRNX, 32, 16, COL8_FFFFFF, s);
						
						//这是我自己绘制的图案范围
						//这些都是显示在图层1;
						//boxfill8(buf_back, binfo->SCRNX, COL8_008484, 0, 40, 200, 64); //先用背景色填充上一次显示的
						//putStrings(buf_back,binfo->SCRNX,0,40,COL8_FFFFFF,"mouse:");
						//putStrings(buf_back,binfo->SCRNX,60,40,COL8_FFFFFF,s);// 这里传入的是s的首地址
						
						//sheet_refresh(sht_back, 0, 40, 200, 64);
						
						//更新鼠标像素矩阵实时的坐标(mx,my);且注意移动过程中不能超过屏幕的显示范围
						mx += mdec.x;
						my += mdec.y;
						if (mx < 0) {
							mx = 0;
						}
						if (my < 0) {
							my = 0;
						}
						if (mx > binfo->SCRNX - 1) {
							mx = binfo->SCRNX - 1;
						}
						if (my > binfo->SCRNY - 1) {
							my = binfo->SCRNY - 1;
						}
						
						//putblock8_8(binfo->VRAM, binfo->SCRNX, 16, 16, mx, my, mcursor, 16);
						
						sheet_slide(sht_mouse, mx, my); //这个图层刷新和平移就是代替了原来的绘制像素矩阵的函数putblock8_8
						
						//相当于右键按下的时候;将此刻鼠标的实时坐标(mx,my),也赋值给指定图层;
						//左键按下,鼠标移动到哪,指定图层也移动到某对应位置;
						//sheet_slide(sht_mouse, mx, my);
					if ((mdec.btn & 0x01) != 0) {
						
						
						
						if (mmx<0){ // 小于0不处于移动模式
							for (j = scl->top - 1; j > 0; j--) {
								sht = scl->sht_ls1[j];
								x = mx - sht->vx0;
								y = my - sht->vy0;
								if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
									if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
										sheet_updown(sht, scl->top - 1);
										if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) { // 有位移就是移动窗口
											mmx = mx;	
											mmy = my;
										}
										if (sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19) {
											
												
												sheet_updown(sht, -1);
												
												//memman_free_4k(memman,memman->used[i].addr,memman->used[i].addr+memman->used[i].size);
												
												if (sht->task!=0){
													for (i=0;i<128;i++)
													{	
												
														if (memman->used[i].addr==sht){
														
															memman_free_4k(memman, memman->used[i].addr, memman->used[i].addr+memman->used[i].size);
															
															
														}
														
														
													}
													io_cli();
													task_sleep(sht->task);
													
													io_sti();
												}
												
												//io_cli();	
												///sht->task->tss.eax = (int) &(sht->task->tss.esp0);
												//sht->task->tss.eip = (int) asm_end_app;
												//io_sti();
											
										}
										
										
										break;
									}
								}
							}
						}else{ // 处于移动模式
							
							x = mx - mmx;	
							y = my - mmy;
							sheet_slide(sht, sht->vx0 + x, sht->vy0 + y);
							mmx = mx;	// 移动后的坐标
							mmy = my;
						}
					}else {//没有左键按下;返回通常模式
						mmx =-1;
					}
				
				}
				
				
			}
			
			
		}
		
	}
	
	
}
  







