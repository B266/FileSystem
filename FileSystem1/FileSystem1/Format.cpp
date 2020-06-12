/*
#include"OS.h"

bool Format()
{
	//格式化超级块
	SuperBlock.totalINodeSum = InodeSum;
	SuperBlock.totalBlockSum = DiskBlockSum;
	SuperBlock.freeBlockSum = 0;
	SuperBlock.freeINodeSum = 0;
	SuperBlock.firstInode = 0;
	SuperBlock.name[20] = { 0 };
	SuperBlock.SuperEmptyBlockList.EmptyBlockList;
	//格式化磁盘块区，按成组链法组织
	initGroupLink(disk);
	memset(&disk.data[SuperBlockStart], 0, sizeof(block));//将磁盘上的一块数据全置零（初始化）
	memcpy(&disk.data[SuperBlockStart], &SuperBlock, sizeof(SuperBlock));

	//格式化inode位图
	/*void *memset(void *str, int c, size_t n)
	str -- 指向要填充的内存块。
	c -- 要被设置的值。该值以 int 形式传递，但是函数在填充内存块时是使用该值的无符号字符形式。
	n -- 要被设置为该值的字节数。*/
	/*memset(&disk.data[InodeBitmapStart], 0, sizeof(block));//将磁盘上的一块数据全置零（初始化）//位图置零
	memcpy(&disk.data[InodeBitmapStart], InodeBitmap, sizeof(InodeBitmap));

	initInode();//格式化inode
	InitRootFolder();//格式化目录
	memset(&disk.data[InodeStart], 0, sizeof(block));//将磁盘上的一块数据全置零（初始化）
	memcpy(&disk.data[InodeStart], Inode, sizeof(Inode));

	//写入到虚拟磁盘文件
	/*int fseek(FILE *stream, long int offset, int whence)
	stream -- 这是指向 FILE 对象的指针，该 FILE 对象标识了流。
	offset -- 这是相对 whence 的偏移量，以字节为单位。
	whence -- 这是表示开始添加偏移 offset 的位置。它一般指定为下列常量之一：*/
	/*fseek(fw, SuperBlockStart, SEEK_SET);
	/*size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
	ptr -- 这是指向要被写入的元素数组的指针。
	size -- 这是要被写入的每个元素的大小，以字节为单位。
	nmemb -- 这是元素的个数，每个元素的大小为 size 字节。
	stream -- 这是指向 FILE 对象的指针，该 FILE 对象指定了一个输出流。*/
	/*for (int i = 0; i < 3; i++)
		for (int j = 0; j < 512 * 8; j++)
			cout << (bool)disk.data[i].byte[j];
	fwrite(&disk, sizeof(disk), 1, fw);
	fflush(fw);




	return true;

}*/