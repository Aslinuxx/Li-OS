#include "bootpack.h"

// 实现对图层的管理:图层分配+图层释放
// 类似之前的内存管理; 
// 计算机中的屏幕显示都是图层叠加的,比如新打开一个图层他就位于最上层;
// 本程序还做不到,比如先打开A,之后打开B;此时B显示在A之上,然后再点击A,A就会显示在B之上了;(这是现代计算机的处理方法)


// 由于图层管理的结构体SCL的大小超过了9KB; 所以如果按照传统的方法先声明一个结构体;再初始化;
// 声明的过程就是在进行内存分配;对于需要内存空间在9KB以上的,我们可以利用之前的内存管理程序来快速对其进行内存分配;
// 声明该结构体的过程时候顺便也初始初始化了;
// 下面是结构体函数;可以理解为是在对一个结构体变量进行初始化;
/*    1. 提高代码阅读性

    2. 分类管理函数及部分属性

    3. 偏向于c++的面向对象思维
	*/

struct SCL *scl_init(struct MEMORY_MANANGE *man, unsigned char *vram, int xsize, int ysize){
	//内存管理参数需要传入,VRAM屏幕的信息需要传入
	
	//声明一个图层管理表结构体
	struct SCL *scl;
	int i;
	//进行内存分配;
	// 内存分配api需要传入的参数有内存管理链表的内存地址,需要分配的内存大小;
	scl = (struct SCL *) memman_alloc_4k(man,sizeof(struct SCL), "Sheet Control List"); 
	
	if (scl==0){//内存分配失败;
		goto err;
		
	}
	//初始化所有图层的map
	scl->map = (unsigned char *) memman_alloc_4k(man, xsize * ysize, "Sheet Map");
	if (scl->map == 0) {
		memman_free_4k(man, (int) scl, sizeof (struct SCL));
		goto err;
	}
	
	scl->vram=vram;
	scl->xsize=xsize;
	scl->ysize=ysize;
	scl->top=-1; // 表示最高图层处于-1层;即现在一个图层都没有
	
	for (i=0;i<MAX_SHEETS;i++){
		//将所有的图层全部标记为未使用
		scl->sht_ls0[i].flags=0;
		scl->sht_ls0[i].scl=scl; //将所有图层的管理表地址都赋值上// 只有结构体指针才能用->;否则都只能用.; 
		// -> 相当于 (*p).xx = p->xx
	}
	
err:
	return scl;
	
	
}


//图层分配:找到空闲的未使用的图层;
// 和之前的内存分配一样:分为内存查找+内存分配;
// 只不过这里不需要内存那边改变的麻烦,找到空闲图层将其标记为已使用即可; flags=1;

struct SHEET *sheet_alloc(struct SCL *scl)
{
	struct SHEET *sht;
	int i;
	for (i = 0; i < MAX_SHEETS; i++) {
		if (scl->sht_ls0[i].flags == 0) {
			sht = &scl->sht_ls0[i]; // 将找到的未使用的图层地址赋给sht;
			sht->flags = SHEET_USE; //标记为正在使用
			sht->height = -1; //此时该图层的具体显示内容还没有设置,所以层高先设置为-1,表示暂时还不能显示
			return sht;
		}
	}
	return 0;	// 找不到可分配图层就返回0
}

//设置图层的缓存区和透明色;
//buf表示图层将要绘画的内容,如鼠标像素点阵等
//这时候该图层已经被传入了需要的信息了
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}


void sheet_refreshmap(struct SCL *scl, int vx0, int vy0, int vx1, int vy1, int h0)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, sid, *map = scl->map;
	struct SHEET *sht;
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > scl->xsize) { vx1 = scl->xsize; }
	if (vy1 > scl->ysize) { vy1 = scl->ysize; }
	for (h = h0; h <= scl->top; h++) {
		sht = scl->sht_ls1[h];
		sid = sht - scl->sht_ls0; /* 番地を引き算してそれを下じき番号として利用 */
		buf = sht->buf;
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		for (by = by0; by < by1; by++) {
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = sht->vx0 + bx;
				if (buf[by * sht->bxsize + bx] != sht->col_inv) {
					map[vy * scl->xsize + vx] = sid;
				}
			}
		}
	}
	return;
}




// 每当有一个图层的显示内容发生改变(缓冲区),比如显示的字符变化了;
// 或者是鼠标移动了位置,图层1该绘制的地方变化了;
// 那么我们只需要针对这些变化的区域或者变化的缓冲区(变化的像素点阵),对这些变化的区域进行重新描绘即可;
// 即现在所有的图层的刷新只需要对这些变化的区域进行从下到上的图层刷新即可;
// 缩减从两个方面,一个是移动位置的像素点阵,一个缓冲区的像素点阵; 只聚焦这些变化的区域进行刷新即可; 
// 不变的像素区域不刷新图层;
void sheet_refreshsub(struct SCL *scl, int vx0, int vy0, int vx1, int vy1, int h0,int h1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, *vram = scl->vram, *map=scl->map,sid; 
	struct SHEET *sht;
	// 如果refresh的范围超出画面外则修正
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > scl->xsize) { vx1 = scl->xsize; }
	if (vy1 > scl->ysize) { vy1 = scl->ysize; }
	for (h = h0; h <= h1; h++) {
		sht = scl->sht_ls1[h];
		buf = sht->buf;
		sid=sht - scl->sht_ls0; //ls1[0]
		//使用vx0-vy1,对bx0-by1进行倒推;
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		for (by = by0; by < by1; by++) {
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = sht->vx0 + bx;
				
				if (map[vy * scl->xsize + vx]==sid) {
					vram[vy * scl->xsize + vx] = buf[by * sht->bxsize + bx];
				}
			}
		}
	}
	return;
}

//将已经分配好的图层显示在指定层数
// 每分配一次图层,所有的图层都需要从上到下全部重新描画一遍;
 void sheet_updown(struct SHEET *sht, int height)
{
	struct SCL *scl = sht->scl;
	int h, old = sht->height; 

	// 如果指定的图层超过当前最高层数+1或者小于-1(表示没有使用),就进行调整hight;
	if (height > scl->top + 1) {
		height = scl->top + 1;
	}
	if (height < -1) {
		height = -1;
	}
	sht->height = height; //

	
	if (old > height) {
		if (height >= 0) {
			
			for (h = old; h > height; h--) {
				scl->sht_ls1[h] = scl->sht_ls1[h - 1];
				scl->sht_ls1[h]->height = h;
			}
			scl->sht_ls1[height] = sht;
			//从height+1层开始刷新起;
			sheet_refreshmap(scl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
			sheet_refreshsub(scl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize,height+1,old);
		} else {	
			if (scl->top > old) {
			
				for (h = old; h < scl->top; h++) {
					scl->sht_ls1[h] = scl->sht_ls1[h + 1];
					scl->sht_ls1[h]->height = h;
				}
			}
			scl->top--; 
			//从最底层开始刷新起;
			sheet_refreshmap(scl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
			sheet_refreshsub(scl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize,0,old-1);
		}	
	} else if (old < height) {
		if (old >= 0) {
			
			for (h = old; h < height; h++) {
				scl->sht_ls1[h] = scl->sht_ls1[h + 1];
				scl->sht_ls1[h]->height = h;
			}
			scl->sht_ls1[height] = sht;
		} else {	
			// 如果新加的图层
			for (h = scl->top; h >= height; h--) {
				scl->sht_ls1[h + 1] = scl->sht_ls1[h];
				scl->sht_ls1[h + 1]->height = h + 1;
			}
			scl->sht_ls1[height] = sht;
			scl->top++; 
		}
		//sheet_refresh(scl); 
		//sheet_refreshsub(scl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize,height);
		
		sheet_refreshmap(scl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
		sheet_refreshsub(scl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
	}
	return;
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if (sht->height >= 0) { //如果正在显示,则按照新图层的信息刷新画面
		
		sheet_refreshsub(sht->scl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1,sht->height, sht->height);
		
	}
	return;
}



//不改变图层显示的层数,即图层依旧显示在该层; 但上下左右移动图层的函数

void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
	struct STL *scl = sht->scl;
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) { // height不为0表示其正在显示
		sheet_refreshmap(scl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
		sheet_refreshmap(scl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
	
		// 某一层窗口平移时候,移开后原位置的坑需要先补上,所以原位置区域需要从最底层到最上层开始刷新;刷新全部层
		sheet_refreshsub(sht->scl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize,0,sht->height);
		// 新的位置区域,只需刷新移动的层级及以上的即可;
		sheet_refreshsub(sht->scl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize,sht->height,sht->height);
	}
	return;
}

// 释放不需再使用的图层
void sheet_free(struct SHEET *sht)
{
	if (sht->height >= 0) {
		sheet_updown(sht, -1); //如果处于显示状态,将其先隐藏起来; 即在该图层关闭后,置为-1;将在该图层之下的图层依次移动上来;
	}
	sht->flags = 0; 
	return;
}



