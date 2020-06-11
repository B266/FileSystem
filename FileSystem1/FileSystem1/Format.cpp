#include"OS.h"
bool Format()
{
	//初始化超级块
	SuperBlock.totalINodeSum = InodeSum;
	SuperBlock.totalBlockSum = DiskBlockSum;
	SuperBlock.freeBlockSum = 0;
	SuperBlock.freeINodeSum = 0;
	SuperBlock.firstInode = 0;
	SuperBlock.name;
	SuperBlock.SuperEmptyBlockList = {};
	//初始化inode位图

}