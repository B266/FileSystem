#include"OS.h"
bool Format()
{
	//��ʼ��������
	SuperBlock.totalINodeSum = InodeSum;
	SuperBlock.totalBlockSum = DiskBlockSum;
	SuperBlock.freeBlockSum = 0;
	SuperBlock.freeINodeSum = 0;
	SuperBlock.firstInode = 0;
	SuperBlock.name;
	SuperBlock.SuperEmptyBlockList = {};
	//��ʼ��inodeλͼ

}