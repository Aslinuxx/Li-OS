#include "bootpack.h"



//实现关闭高速缓存的功能;并实现内存容量查询的功能
unsigned int memtest(unsigned int start_addr, unsigned int end_addr){
	//先查询cpu是不是486以上的,486以上的才有缓存
	char cpu486=0;
	unsigned int eflags,cr0,i; 
	
	eflags = io_load_eflags(); //读取EFLAGS寄存器此时的值
	eflags |= EFLAGS_AC_BIT; //将其第十八位置为1;
	io_store_eflags(eflags);//向EFLAGS寄存器中写入值
	eflags = io_load_eflags();
	
	if ((eflags & EFLAGS_AC_BIT)!=0){//如果cpu是386,即使向EFLAGS寄存器中18位写入1也会自动重置为0
		cpu486 = 1;
		
	}
	eflags &= ~EFLAGS_AC_BIT; //将第18位置为0
	io_store_eflags(eflags);
	
	if (cpu486 !=0){
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; //禁止高速缓存 Cache
		store_cr0(cr0);
	}
	
	//执行内容容量检查
	i = memtest_sub(start_addr,end_addr);
	
	//恢复缓存允许
	if (cpu486!=0){
		
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	
	return i;
}







/*实现内存管理:内存分配与内存释放;*/
void memman_init(struct MEMORY_MANANGE *man)
{		int i;
		man->frees=0; //当前可用空闲段数目
		man->maxfrees=0; //历史可用最大空闲段数,用于观察可用状态;
		man->lostsize=0;//释放失败的内存大小总和
		man->losts=0;//释放失败次数
	
		//初始化已使用状态
		for (i=0;i<MEMMAN_FREES;i++)
		{
			man->used[i].flag=0;
			sprintf (man->used[i].status,"System Stack");
		}
	
		return 0;
}

//报告剩余空间大小
unsigned int memman_total(struct MEMORY_MANANGE *man){
	unsigned int i ,t=0;
	for (i=0;i<man->frees; i++){
		t += man->free[i].size;
		
	}
	return t;
}

//实现指定内存大小的分配:查找到合适内存段+执行内存分配
unsigned int memman_alloc(struct MEMORY_MANANGE *man, unsigned int size, char *status){
	unsigned int i,a,j;
	for(i=0;i<man->frees;i++){
		if(man->free[i].size>=size){
			a = man->free[i].addr; //找到可用内存段了;
			//将该段的可用内存减去需要使用的内存
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size==0){//如果某一段可用大小变为0;则需减去该段,并将可用段数组整体进行左移一位;
				man->frees--;
				for(;i<man->frees;i++){
					man->free[i] = man->free[i+1]; //结构体赋值;
				}
				
			}
			
			//增加使用记录
			for (j=0;j<MEMMAN_FREES;j++) 
				if (man->used[j].flag==0)
				{
					man->used[j].addr=a;
					man->used[j].size=size;
					sprintf (man->used[j].status,"%s",status);
					man->used[j].flag=1;
					break;
				}
			
			return a;
		}
		
	}
	return 0; //要是没有可用段,则返回0;代表没有找到可用段的初始地址;
	
}

//进行内存释放
//传入的参数是释放内存的首地址(物理地址),释放的内存大小;
int memman_free(struct MEMORY_MANANGE *man, unsigned int addr, unsigned int size){
	int i,j;
	
	
	//删除已使用的内存记录 
	for (i=0;i<MEMMAN_FREES;i++){
		if (man->used[i].addr==addr)
			man->used[i].flag=0;
	}
	
	
	//首先需要找到释放的地址该插入段管理数组的什么位置合适
	
	for (i=0;i<man->frees;i++){
		if (man->free[i].addr > addr){
			break;
		}
		
	}
	
	//查看当前的释放地址能否与找到的该段地址合并;
	
	if(i>0){
		//如果能与前面一段进行合并
		if (man->free[i-1].addr + man->free[i-1].size == addr){
			man->free[i-1].size += size;
			// 如果也能与后面一段进行合并
			if (i<man->frees){
				if (addr + size == man->free[i].addr){
					man->free[i-1].size += man->free[i].size;
					
					//删除free[i],并将后面的内存数组左移一位
					man->frees--;
					for (;i<man->frees;i++){
						man->free[i] = man->free[i+1];
					}
					
				}
			}
			return 0; //成功进行合并
		}	
	}
	
	// 如果不能与前面的空间进行合并;查看能否与后面的空间进行合并
	if (i<man->frees){
		if (addr + size == man->free[i].addr){
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
		
	}
	
	// 既不能与前面进行合并,也不能与后面进行合并;就需要将其插入数组中;
	//必须保证此时预留的最大数组长度没有用尽
	if(man->frees<MEMMAN_FREES){
		//倒序进行移动; 右移一位
		for (j=man->frees; j>i; j--){
			man->free[j]= man->free[j-1];
			
		}
		man->frees++;
		
		if (man->maxfrees<man->frees){
			man->maxfrees = man->frees;
		}
		// 将该新释放的空间插入进来
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
		
	}
	
	// 如果上面三种都没法实现,则说明此时的内存虽然释放了;但是没法合并,或者新开辟一块进行管理;
	//就将其舍弃
	man->losts++;
	man->lostsize += size;
	
	
	
	return -1;
}


// 以0x1000字节(4KB)为基本单位进行内存分配和释放
unsigned int memman_alloc_4k(struct MEMORY_MANANGE *man, unsigned int size, char *status){
	unsigned int a;
	//将待分配的内存空间进行向上取整
	size = (size+0xfff) & 0xfffff000;
	a = memman_alloc(man,size,status);
	
	return a;
}


int memman_free_4k(struct MEMORY_MANANGE *man, unsigned int addr, unsigned int size){
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man,addr,size);
	
	return i;
}
