// FIFO缓冲区初始化; 数据写入缓存区; 从缓冲区取数据

# include "bootpack.h"

void fifo8_init (struct FIFO8 *fifo, int size, unsigned char *buf){
	fifo->size = size;
	fifo->buf = buf; // 因为这里buf的空间随自己分配;所以只需要指向一个自己分配的空间的地址即可;
	fifo->free=size;
	fifo->flags=0; //为0表示没有溢出;
	fifo->p=0;
	fifo->q=0; //当前写入/读出的位置(索引);
	return ;
	
	
}

//向fifo中写入一字节的数据
int fifo8_input(struct FIFO8 *fifo, unsigned char data){
	//先判断是否有溢出
	if (fifo->free==0){
		fifo->flags =1;
		return -1;
	}
	fifo->buf[fifo->p]=data;
	fifo->p++;
	if (fifo->p == fifo->size){
		fifo->p = 0;
	}
	
	fifo->free--;
	return 0; //没有溢出则返回0;
	
	
}


//从fifo中获取一字节的数据
int fifo8_get(struct FIFO8 *fifo){
	int data;
	//如果buf中没有数据,返回-1;
	if (fifo->free==(*fifo).size){return -1;} //(*fifo).size==fifo->size [AX]表示取得的是地址编号为AX处的内容
	
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q==fifo->size){
		fifo->q=0;
	}
	fifo->free++;
	return data;
	
	
}


// 查看当前fifo存储的状态
int fifo8_status(struct FIFO8 *fifo){
	
	return fifo->size-fifo->free;
	
}