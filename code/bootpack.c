

//����c�еĿ�
#include<stdio.h> // c�е�i/o��
#include "bootpack.h" //""˫���ű�ʾͷ�ļ���Դ�ļ���ͬһ�ļ���; <>��ʾͷ�ļ�λ�ڱ������ṩ���ļ�����;



//struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0; //��Ļ������Ϣ��ַ; //����ŵ�ȫ���������Ǻ���Ҳ�õ��ýṹ����;ʡ���ٴ��ݵ�ַ;
//extern struct TIMER timer1;

struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;

struct MEMORY_MANANGE *memman = (struct MEMORY_MANANGE *) MEMMAN_ADDR;

//������
void HariMain(void){
	
	//struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	
	struct FIFO32 fifo;
	char s[40];
	int fifobuf[128];//ȫ��fifo����ȡ�Ĵ�С�� 128*4�ֽ�
	struct mouse_state mdec;
	struct SCL *scl;//����ͼ������
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, buf_mouse[256]; //Ҳ���õ�fifo//����ͼ������ͼ��Ļ�����;������ͼ�㻭ʲô����,���ͼ�㻭�����ص���
	struct SHEET *sht_cons;
	unsigned char *buf_cons;
	
	
	
	unsigned int memtotal;
	int mx,my,i,count=0;
	
	struct TASK *task_a,  *task_cons;//a������������
	
	
	int j,x,y,mmx=-1,mmy=-1;
	//��������л�����
	//�������,�����ָ�����ƶ�,�����ж�,�ƶ��ľ����������0;��ôͼ��ʹ����ƶ�ģʽ
	struct SHEET *sht=0;
	
	
	//GDT��IDT��ʼ��
	init_gdtidt(); //
	//����PIC(�жϼ�����·); ��·IRQ�������ʲô�ж�(���/����...)
	init_pic();
	io_sti(); /* IDT/PIC�ĳ�ʼ�����������Խ��CPU���жϽ�ֹ */

	//����/��껺������ʼ�� / ��ʱ��fifo��
	//ȫ��fifo��ʼ��
	fifo32_init(&fifo,128,fifobuf,0);
	
	//���ﶨʱ�������ĳ�ʼ���Ѿ�������PIT�ĳ�ʼ��������
	// ����������ڴ���亯��,��ô�ٷ����ڴ�֮ǰһ��Ҫ�ڴ��������;
	//������ڴ����ʧ��;
	//��ʱ���ж���IRQ0,Ҳ������Ϊ0; 
	init_pit();
	//���̿��Ƶ�·��ʼ��
	init_keyboard(&fifo,256); // ���̵�fifo���ݱ������256,�������ֿ���ͬ�豸
	enable_mouse(&fifo,512,&mdec);//���fifo���ݼ���512
	io_out8(PIC0_IMR, 0xf8); /* IRQ0,1,2ȫ����Ч;0�Ƕ�ʱ��,1�Ǽ���,2�Ǵ�PIC��Ч;  */ 
	io_out8(PIC1_IMR, 0xef);/* PIC1������ж����(1110 1111) */
	
	
	

	/*�ڴ��ʼ����������ڴ����֮ǰ*/
	//�����ڴ�������ѯ
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	//���������Ķ�
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	memman->used[0].addr=0x9e000;
	memman->used[0].size=0x00400000-0x9e000;
	memman->used[0].flag=1;
	sprintf (memman->used[0].status,"System");

	

	//�趨��ɫ��
	init_palette(); 


	//��ʼ��ͼ������
	scl = scl_init(memman,binfo->VRAM, binfo->SCRNX,binfo->SCRNY);
	
	task_a = task_init(memman);
	fifo.task = task_a; 
	
	
	
	
	

	//���䱳��ͼ��
	sht_back = sheet_alloc(scl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->SCRNX * binfo->SCRNY,"Backgroud UI" ); 
	sheet_setbuf(sht_back,buf_back,binfo->SCRNX,binfo->SCRNY,-1); //-1��͸��ɫ
	init_screen(buf_back, binfo->SCRNX, binfo->SCRNY);
	
	
	// �������ͼ��
	sht_mouse = sheet_alloc(scl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	//��ʼ��������ص���;
	init_mouse_cursor8(buf_mouse, 99);
	// �������ͼ��;���ͼ�������ڵ�1��;�����ǵ�0��;
	mx = (binfo->SCRNX - 16) / 2; /*�ڻ��������������*/ // ��ȡ������ؾ����ڻ����е���ʼ����;
	my = (binfo->SCRNY - 28 - 16) / 2;
	
	
	//���俪����ӭ����
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
	
	
	

	
	//���������д��ڽ���; 
	//���������д����еĴ���ͼ��
	sht_cons = sheet_alloc(scl);
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165,"Console UI");
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); 
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	
	//�����������������
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
	
	
	
	sheet_slide(sht_back, 0, 0); // ͼ��ƽ��
	sheet_slide(sht_cons, 80, 300);
	sheet_slide(sht_mouse, mx, my);
	
	
	// ������ͼ�����㼶; ע�ⲻ�����ĸ����̵�ͼ�����ͼ�㶼������ͼ������
	sheet_updown(sht_back, 0); 
	sheet_updown(sht_cons, -1);
	sheet_updown(sht_welcome, 1);
	sheet_updown(sht_mouse, 2);
	
	

	
	//��ʾ��ʱ���ڴ�����;
	//sprintf(s,"memory: %dMB   free: %dKB",memtotal/(1024*1024),memman_total(memman)/1024);
	//putStrings(buf_back,binfo->SCRNX,0,84,COL8_FFFFFF,s);
	sheet_refresh(sht_back, 0, 84, binfo->SCRNX, 110);// ˢ������ͼ����ʾ



	
	
	//�����ǲ���ϵͳ����֮��ĳ���;ǰ�涼���ڽ��л�������;
	for (;;){
		
		//sprintf(s,"%010d",tcl.count);
		//boxfill8(buf_win, 160, COL8_C6C6C6, 40, 28, 119, 43);
		//putStrings(buf_win, 160, 40, 28, COL8_000000, s);
		//sheet_refresh(sht_win, 40, 28, 120, 44);
		
		count ++; //��ʱ���ж��Լ���count����
		io_cli(); //�ر��ж�����;CLI�ǹر��ж��������˼,STI�ǻָ��ж��������˼;��P80��
		
		
		if (fifo32_status(&fifo) == 0){
			//task_sleep(task_a);
			//io_sti();
			//ʹ�ö�ʱ��ʱ������cpu����;
			//io_sti();
			
			
			//stihlt();�൱�ڻָ��ж������������;
			//��ʱ��
			io_stihlt(); // ���������û������,˵������û�������ж�; ִ��hlt,���ָ��ж����� sti��
			
		}else{
			//����fifo�����ĸ��豸������
			i = fifo32_get(&fifo); 
			io_sti();
			if (256 <= i && i <= 511){ // ��������
				
				// ���̵���ʵ���ݱ���Ҫ��ȥ���ϵ�256��ʶ
				//ʹ���µ��ַ���ʾapi�������������ʾ��ͼ��ˢ��
				//����:��ʾͼ�� ��ʾ����ʼ����(x,y) �ַ���ɫ  ������ɫ �ַ������� �ַ�������
				//sprintf(s,"keyboard: %02x",i-256); 
				//putStrings_sht(sht_back,260,100,COL8_FFFFFF, COL8_008484,s,13);
				
				
				//ʵ�ְ�F11���������н���
				
					if (i==0x57+256){//F11����ȥ����������0x57
						if(task_cons->state != ACTIVE){ //2��ʾ��Ծ״̬
							int height = 2;
							task_add(task_cons,sht_cons); // ���Ѹ�����
							
							if (height>scl->top){
								height = scl->top;
							}
							sheet_updown(sht_cons, height);
						}
					}
				
				
					fifo32_input(&sht->task->fifo,i);
				
				
				
	
				
				
			} else if(512 <= i && i <= 767){ // �������
				if (mouse_decode(&mdec,i-512)!=0){
						sprintf(s,"[lcr %4d %4d]",mdec.x,mdec.y); //ָ���������ʹ��-> ��ͬ��(*p).xx =p->xx; ��ָ��ṹ��ֱ��p.xx; 
						//��ָ��ṹ������൱����ͨ�Ĵ���;����ֱ�Ӵ��ڼĴ����е� 
						
						//��������if���ж��������������Ƿ��а���;���.�м����.����Ҽ�
						if ((mdec.btn & 0x01) != 0) {
							s[1] = 'L';
						}
						if ((mdec.btn & 0x02) != 0) {
							s[3] = 'R';
						}
						if ((mdec.btn & 0x04) != 0) {
							s[2] = 'C';
						}
						//��������ʽ����������ؾ����С���Ƶ�ͼ����Χ
						//boxfill8(binfo->VRAM, binfo->SCRNX, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
						//putStrings(binfo->VRAM, binfo->SCRNX, 32, 16, COL8_FFFFFF, s);
						
						//�������Լ����Ƶ�ͼ����Χ
						//��Щ������ʾ��ͼ��1;
						//boxfill8(buf_back, binfo->SCRNX, COL8_008484, 0, 40, 200, 64); //���ñ���ɫ�����һ����ʾ��
						//putStrings(buf_back,binfo->SCRNX,0,40,COL8_FFFFFF,"mouse:");
						//putStrings(buf_back,binfo->SCRNX,60,40,COL8_FFFFFF,s);// ���ﴫ�����s���׵�ַ
						
						//sheet_refresh(sht_back, 0, 40, 200, 64);
						
						//����������ؾ���ʵʱ������(mx,my);��ע���ƶ������в��ܳ�����Ļ����ʾ��Χ
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
						
						sheet_slide(sht_mouse, mx, my); //���ͼ��ˢ�º�ƽ�ƾ��Ǵ�����ԭ���Ļ������ؾ���ĺ���putblock8_8
						
						//�൱���Ҽ����µ�ʱ��;���˿�����ʵʱ����(mx,my),Ҳ��ֵ��ָ��ͼ��;
						//�������,����ƶ�����,ָ��ͼ��Ҳ�ƶ���ĳ��Ӧλ��;
						//sheet_slide(sht_mouse, mx, my);
					if ((mdec.btn & 0x01) != 0) {
						
						
						
						if (mmx<0){ // С��0�������ƶ�ģʽ
							for (j = scl->top - 1; j > 0; j--) {
								sht = scl->sht_ls1[j];
								x = mx - sht->vx0;
								y = my - sht->vy0;
								if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
									if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
										sheet_updown(sht, scl->top - 1);
										if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) { // ��λ�ƾ����ƶ�����
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
						}else{ // �����ƶ�ģʽ
							
							x = mx - mmx;	
							y = my - mmy;
							sheet_slide(sht, sht->vx0 + x, sht->vy0 + y);
							mmx = mx;	// �ƶ��������
							mmy = my;
						}
					}else {//û���������;����ͨ��ģʽ
						mmx =-1;
					}
				
				}
				
				
			}
			
			
		}
		
	}
	
	
}
  







