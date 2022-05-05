# include "bootpack.h" //""双引号表示头文件与源文件在同一文件夹; <>表示头文件位于编译器提供的文件夹里;
// #include 就是告诉编译器你先读xx.h中的内容;将其加载进来; 相当于就可以调库函数了; 不同源文件之间可以引用彼此的函数;


/// 初始化GDT和IDT
void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT; //段号记录表
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) ADR_IDT; //中断记录表
	int i;

	/* GDT初始化 */
	for (i = 0; i < LIMIT_GDT/8; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	// 下面的两个语句是为了段号1和段号2两个段进行的设定;
	// 参数:段号 该段空间大小(多少字节) 该段的起始地址 该段的属性设置 
	set_segmdesc(gdt + 1, 0xffffffff,   0x00000000, AR_DATA32_RW);
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);
	load_gdtr(LIMIT_GDT, ADR_GDT); //该处是对GTDR进行赋值; 前面设定好的GDT必须放在GDTR寄存器中才能被cpu读取;

	/* IDT初始化*/
	for (i = 0; i < LIMIT_IDT/8; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(LIMIT_IDT, ADR_IDT);
	
	/* 对于不同中断的设置 */
	//将不同的中断号注册到IDT中;
	set_gatedesc(idt + 0x20, (int) asm_inthandler20, 2 * 8, AR_INTGATE32); 
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32); //0x21是键盘中断
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32);
	

	return;
}

//设定GDT
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

//设定IDT
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
