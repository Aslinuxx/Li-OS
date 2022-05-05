/*注意.h文件只是用于声明库函数的;或者声明结构体的;不能直接实现函数本体或者创建一个结构体变量*/


/*bootpack.c*/



// 定义传入基础设置信息的结构体
/*asmhead.nas 利用汇编写好基础的系统设置信息文件*/
struct BOOTINFO{
	char CYLS;
	char LEDS;
	char VMODE;
	char reserve;
	short SCRNX,SCRNY;
	char *VRAM;
	
	
};



#define  ADR_BOOTINFO 0x00000ff0 //0x0ff0是启动区地址



//调用的混编的汇编中的函数
/*naskfunc.nas*/
void io_hlt(void); 
void io_sti(void);

void io_cli(void);
void io_stihlt(void);
//I/O指令;向指定端口(设备)写入信息/从指定端口读取信息
void io_out8(int port, int data);
int io_in8(int port);

int io_load_eflags(void);
void io_store_eflags(int eflags);

//读/写CR0寄存器
int load_cr0(void);
void store_cr0(int cr0);
//汇编实现内存容量查询
int memtest_sub(unsigned int start_addr, unsigned int end_addr);

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
//中断结束后的处理IRETD指令
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

void asm_end_app(void);

void shutdown(void);

/* graphic.c */
//画布设置,绘制鼠标图案等等的设置; 
//制作窗体等
void init_palette(void);
void set_palette(int start_color_number, int end_color_number, unsigned char *rgb);
void boxfill8(unsigned char *vram, int x_size, unsigned char color, int x_start, int y_start, int x_end, int y_end);
void init_screen(unsigned char *vram, int xsize, int ysize);
extern char hankaku[4096]; //字符库
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putStrings(char *VRAM, int xsize , int x_start, int y_start, char color, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize);

// 制作窗口
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
//void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
//void putStrings_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);

// 由于0-15号颜色对应的颜色代码实在太难记了,所以使用一个常数项进行记录;
#define COL8_000000 	0 //0号颜色对应的颜色代码 6位16进制
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15





/*dsctbl.c*/
//GDT和IDT的结构体; 全局段号记录表和中断记录表;
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};


//GDT和IDT相关的函数
// 注意这里使用到了结构体SEGMENT_DESCRIPTOR; 就必须要在声明之前先定义;
void init_gdtidt(void);	
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);


#define ADR_IDT			0x0026f800 //中断记录表的起始地址
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000 //全局段号记录表的起始地址
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a

#define AR_INTGATE32	0x008e

#define AR_TSS32		0x0089




/*FIFO.c*/
// FIFO
struct FIFO32{
	int *buf;
	int p,q,size,free,flags;
	struct TASK *task;
	
	
};
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
int fifo32_input(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

/* int.c */
// 主从PIC的设置; 具体见P117
// PIC芯片中有两种8位寄存器:IMR是中断屏蔽寄存器,ICW是初始化控制数据,ICW共有4个;
// ICW3是设定主从PIC的连接;ICW1和ICW4与PIC主板配线方式,中断信号的电气特性等有关;
// ICW2是设定中断发生时候,对应调用哪一类中断指令;

void init_pic(void);
//中断结束后的处理IRETD指令
void inthandler27(int *esp);
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1



/*keyboard.c*/
//
void inthandler2c(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(struct FIFO32 *fifo, int data0);

# define PORT_KEYDAT 0x0060
# define PORT_KEYSTA 0x0064 
# define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47



/*mouse.c*/

struct mouse_state{
	unsigned char phase,dbuf[3];
	int x,y,btn;
	
};
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4
void inthandler21(int *esp);
void enable_mouse(struct FIFO32 *fifo, int data0, struct mouse_state *mdec);
int mouse_decode(struct mouse_state *mdec, unsigned char data);



/*memory.c*/
//内存容量检查
#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

//内存管理:内存分配和内存释放
#define MEMMAN_FREES 4096 //预留出来4096个段供管理用; //本身占据32KB大小
#define MEMMAN_ADDR			0x003c0000
struct FREEINFO{
	unsigned int addr, size; //每一段内存页包含的内容:起始地址(真实物理地址),该段的大小; 
	
};

//已分配的内存记录表
struct USEDINFO{
	unsigned int addr,size,flag;
	char status[30];
	
};

// 全部的空闲页管理
// 类似Redis的中 字典的结构;用该结构来管理多个哈希表,更为方便;
struct MEMORY_MANANGE {
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
	struct USEDINFO used[MEMMAN_FREES];
	
};

// 内存容量检查
unsigned int memtest(unsigned int start_addr, unsigned int end_addr);
//内存初始化
void memman_init(struct MEMORY_MANANGE *man);
unsigned int memman_total(struct MEMORY_MANANGE *man);
unsigned int meman_alloc(struct MEMORY_MANANGE *man, unsigned int size, char *status);
unsigned int memman_alloc_4k(struct MEMORY_MANANGE *man, unsigned int size, char *status);
int memman_free(struct MEMORY_MANANGE *man, unsigned int addr, unsigned int size);
int memman_free_4k(struct MEMORY_MANANGE *man, unsigned int addr, unsigned int size);


/*sheet.c*/
#define MAX_SHEETS		256
#define SHEET_USE		1
//单个图层的结构体
struct SHEET{
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags; 
	struct SCL *scl;// 增加一个图层管理的结构体地址,这个每个图层就都知道该管理表地址在哪里了,无需再做参数传入
	// buf是该层图层需要描画的内容的地址; 即[buf]就是该图层需要显示的内容,比如鼠标像素矩阵;
	// bxsize*bysize表示该图层的大小,也就是开辟一块新幕布的大小; ( 这块新画布上显示新内容)
	// vx0,xy0是图层在总画布上的起始坐标; col_inv表示透明色号 color(颜色),invisible(透明);
	// height表示图层高度,即图层处于第几层; Flags表示该图层是否已经使用
	
	struct TASK *task;// 该图层所属的进程
};

//和内存管理一样; 使用链表结构进行管理多个图层;
struct SCL{ //SCL:sheet control
	unsigned char *vram, *map;  // map表示该块内存上的像素点应该属于哪个图层; 
	int xsize,ysize,top; //top表示最上面图层的高度,最上面的图层处于第几层
	struct SHEET *sht_ls1[MAX_SHEETS]; //记录图层的内存地址
	struct SHEET sht_ls0[MAX_SHEETS];
	//使用两个链表进行管理;
	//ls0中存放各个图层的具体设置和内容显示; 但是其的
	// 
	
};
struct SCL *scl_init(struct MEMORY_MANANGE *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SCL *scl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);


/*timer.c*/
//定时器中断
#define PIT_CTRL 0x0043 
#define PIT_CNT0 0x0040

#define  MAX_TIMERS 50
struct TIMER {
	struct TIMER *next; // 该定时器的下一定时器; 链表的下一节点; next指针;
	unsigned int timeout,flags; // flags表示当前的定时器是否在使用中;
	struct FIFO32 *fifo; // 使用fifo必须放在fifo结构体后面
	int data;
	struct TCL *tcl; // 和图层管理一样,每个定时器都内部存一个定时器管理表的内存地址;避免后序函数操作重复传参;
	
};

struct TCL {//timer control list 定时器管理表
	unsigned int count,next_count; // used_sum是总共有多少个定时器在使用; 类似之前图层管理表的top
	struct TIMER *t0; // 存放表头定时器的地址
	struct TIMER timers[MAX_TIMERS]; //最多五百个定时器
	
};
extern struct TCL tcl;

void init_pit(void);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void inthandler20(int *esp);


/*mtask.c*/


#define MAX_TASKS	128
#define TASK_GDT0	3 //从GDT的第几号开始分给TSS; 即进程编号(任务编号)是哪几段;
#define ACTIVE 	2
#define SLEEP	1

// TSS:进程状态段; 这是所有进程都需要初始化时候设置的
struct TSS{
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3; // 保存与进程设置相关的信息
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;	//该行也是保存与进程设置相关的信息
	
};

// 单个任务的结构体
struct  TASK{
	int sel, state; // sel是selector(段地址); 即GDT中的编号; state表示该任务的状态; 是在使用中还是在休眠;
	struct TSS tss;
	struct SHEET *sht_task;
	struct FIFO32 fifo;
	char name[30];
	
};


//任务管理表
struct MTCL { //multitask control list 多任务管理表
	int running_sum; // 正在活跃的进程数
	int now; // 当前运行的是哪个进程
	struct TASK *tasks_addr[MAX_TASKS]; // 所有进程的地址信息
	struct TASK	 tasks[MAX_TASKS]; //所有的进程的具体结构体内容
	
}; 

extern struct TIMER *task_timer;
struct TASK *task_init(struct MEMORY_MANANGE *memman);
struct TASK *task_alloc(char *name);
void task_add(struct TASK *task, struct SHEET *sheet);
void task_switch(void);
void task_sleep(struct TASK *task);




/*console_task.c*/

void console_task_main(struct TASK *task);

void showMemory(struct SHEET *Sheet);
int cons_newline(int cursor_y, struct SHEET *sheet);

/*time_task.c*/

void time_task_main(struct TASK *task);


/*text_task.c*/
void text_task_main(struct TASK *task);


/*task_manage_task.c*/
void showTask(struct SHEET *taskListSheet);
void task_manage_task_main(struct TASK *task);

