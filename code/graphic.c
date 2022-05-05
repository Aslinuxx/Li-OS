
# include "bootpack.h" //""双引号表示头文件与源文件在同一文件夹; <>表示头文件位于编译器提供的文件夹里;
// #include 就是告诉编译器你先读xx.h中的内容;将其加载进来; 相当于就可以调库函数了; 不同源文件之间可以引用彼此的函数;


void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}



//绘制鼠标图案
void init_mouse_cursor8(char *mouse, char bc)

{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc; //bc是背景色;
			}
		}
	}
	return;
}

//将输出字符串时候:先用背景色覆盖原字符串,再输出新字符的; 直接合并到一个程序中
//直接将字符串输到图层;
void putStrings_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putStrings(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}


void putStrings(char *VRAM, int xsize , int x_start, int y_start, char color, unsigned char *s){
	//字符串都是以转义字符 '\0'结尾的,其对应的ASCII值是0x00; 注意有时候以字符判断是否等于'\0'反而会出错,用内存地址判断不会出错;
	
	extern char hankaku[4096];
	for (; *s!='\0'; s++){ //地址变量s+1就可以遍历到下一字符了,因为字符串是开辟一块连续空间存储所有的字符,每个字符就是一个char(BYTE); 一个字节,对应只要地址变量加1即可;
		putfont8(VRAM,xsize,x_start,y_start,color,hankaku+ *s*16); //s是地址变量;*S=[SI]即是字符了
		x_start+=8; //8*16的像素矩阵显示一个字符; 将字符显示在同一行则需要每显示完一个字符,x轴进行右移8个像素点;
		
	}
	return ;
	
	
}

//将字符的二极管排列进行输出显示;
void putfont8(char *vram, int xsize, int x, int y, char c, char *font) //除了python这种语言,传递数组不需要传递地址;其他的语言都是需要传递地址的;
																									//传递地址也说明了数组在内存中的存储时连续的,只需要传递数组首元素的地址即可;
{
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = *(font+i); // *font 是[font]; font存的是font_A的首地址
		d = font[i]; // font[i] 就是*(p+i); 其是[SI],即内存地址SI存放的内容;
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}



void boxfill8(unsigned char *vram, int x_size, unsigned char color, int x_start,int y_start,int x_end, int y_end){
	int x,y;
	for (y=y_start;y<=y_end;y++){
		for (x=x_start;x<=x_end;x++)
		
			*(vram+x+y*x_size) = color;
			
		
		
	}
	return ;
	
}

void init_screen(unsigned char *vram, int xsize, int ysize){ //传入的参数是不需要再定义的; 
	
	
	boxfill8(vram, xsize, COL8_000084,  0,         0,          xsize -  1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize - 3);
	
	return ;
	
}


void init_palette(void){
	
	//进行设置0-15号颜色的对应的颜色代码(6位16进制);详见P78
	//c中的static char只能用于数据,相当于汇编中的DB指令
	static unsigned char table_rgb[16*3]={ //实际上就是声明一个常量的数组
		0x00, 0x00, 0x00, /*0号颜色:黑*/
		0xff, 0x00, 0x00, /*1号颜色:亮红*/
		0x00, 0xff, 0x00, //2:亮绿色
		0xff, 0xff, 0x00, //3:亮黄
		0x00, 0x00, 0xff,	/*  4:亮蓝色*/
		0xff, 0x00, 0xff,	/*  5:亮紫 */
		0x00, 0xff, 0xff,	/*  6:浅亮蓝 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:亮灰色 */
		0x84, 0x00, 0x00,	/*  9:暗红色 */
		0x00, 0x84, 0x00,	/* 10:暗绿色 */
		0x84, 0x84, 0x00,	/* 11:暗黄色 */
		0x00, 0x00, 0x84,	/* 12:暗青 */
		0x84, 0x00, 0x84,	/* 13:暗紫*/
		0x00, 0x84, 0x84,	/* 14:暗蓝色 */
		0x84, 0x84, 0x84	/* 15:暗灰色 */
	};
	
	// 调用汇编程序,往调色板中写入颜色设置;
	set_palette(0,15,table_rgb);
	return ;
	
}

// 向内存中写入设置好的调色板
void set_palette(int start_color_number,int end_color_number, unsigned char *rgb)
{
	int i,eflags; // EFLAGS 是16位FLAGS寄存器拓展而来的,类似于32位系统中AX拓展到EAX;
					// EFLAGS 是存储进位标志和中断标志等标志的寄存器; 相当于是记录cpu的各个使用状态的寄存器;
	
	// 在VGA的使用手册中,使用汇编进行配置的时候必须按照以下步骤;
	// 1.关闭VGA的中断允许
	// 2.写入配置(先将设定的颜色色号写入0x03c8(端口号),再按照RGB写入0x03c9地址; 如果想继续设置下一色号,可省略色号的传入,只写RGB即可) 固定用法; 见P80；
	// 3.完成后再打开中断允许
	
	//先读取此时EFLAGS寄存器的状态,以便恢复的时候进行赋值
	eflags = io_load_eflags(); // 调用汇编程序获取EFLAGS的状态
	io_cli(); //关闭中断允许;CLI是关闭中断允许的意思,STI是恢复中断允许的意思;见P80；
	
	io_out8(0x03c8,start_color_number);//写入颜色编号
	
	//写入RGB配置
	for (i=start_color_number;i<=end_color_number;i++){
		io_out8(0x03c9,rgb[0]/4);
		io_out8(0x03c9,rgb[1]/4);
		io_out8(0x03c9,rgb[2]/4);
		rgb +=3;
		
	}
	
	io_store_eflags(eflags); //恢复中断允许
	
	return ;
	
	
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act)
{
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	make_wtitle8(buf, xsize, title, act);
	return;
}

void make_wtitle8(unsigned char *buf, int xsize, char *title, char act)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c, tc, tbc;
	if (act != 0) {
		tc = COL8_FFFFFF;
		tbc = COL8_000084;
	} else {
		tc = COL8_C6C6C6;
		tbc = COL8_848484;
	}
	boxfill8(buf, xsize, tbc, 3, 3, xsize - 4, 20);
	putStrings(buf, xsize, 24, 4, tc, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}




/*
//制作窗口
//act 为1时候窗口名称栏不为灰色,为0适变为灰色
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c, tc, tbc;
	if (act != 0) {
		tc = COL8_FFFFFF;
		tbc = COL8_000084;
	} else {
		tc = COL8_C6C6C6;
		tbc = COL8_848484;
	}
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, tbc,         3,         3,         xsize - 4, 20       );
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	putStrings(buf, xsize, 24, 4, tc, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}
*/
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}
