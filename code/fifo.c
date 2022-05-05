// FIFO缓冲区初始化; 数据写入缓存区; 从缓冲区取数据

# include "bootpack.h"

void fifo32_init (struct FIFO32 *fifo, int size, int *buf, struct TASK *task){
	fifo->size = size;
	fifo->buf = buf; // 因为这里buf的空间随自己分配;所以只需要指向一个自己分配的空间的地址即可;
	fifo->free=size;
	fifo->flags=0; //为0表示没有溢出;
	fifo->p=0;
	fifo->q=0; //当前写入/读出的位置(索引);
	
	fifo->task = task; // 这个fifo是属于哪个进程的
	return ;
	
	
}

//向fifo中写入一字节的数据
int fifo32_input(struct FIFO32 *fifo, int data){
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
	
	//if (fifo->task!=0){
		//if(fifo->task->state != ACTIVE){ //2表示活跃状态
		//	task_add(fifo->task, fifo->task->sht_task); // 唤醒该任务
		//}
	//}
	
	
	return 0; //没有溢出则返回0;
	
	
}


//从fifo中获取一字节的数据
int fifo32_get(struct FIFO32 *fifo){
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
int fifo32_status(struct FIFO32 *fifo){
	
	return fifo->size-fifo->free;
	
}