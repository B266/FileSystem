/*
#include"OS.h"

bool Format()
{
	//��ʽ��������
	SuperBlock.totalINodeSum = InodeSum;
	SuperBlock.totalBlockSum = DiskBlockSum;
	SuperBlock.freeBlockSum = 0;
	SuperBlock.freeINodeSum = 0;
	SuperBlock.firstInode = 0;
	SuperBlock.name[20] = { 0 };
	SuperBlock.SuperEmptyBlockList.EmptyBlockList;
	//��ʽ�����̿�����������������֯
	initGroupLink(disk);
	memset(&disk.data[SuperBlockStart], 0, sizeof(block));//�������ϵ�һ������ȫ���㣨��ʼ����
	memcpy(&disk.data[SuperBlockStart], &SuperBlock, sizeof(SuperBlock));

	//��ʽ��inodeλͼ
	/*void *memset(void *str, int c, size_t n)
	str -- ָ��Ҫ�����ڴ�顣
	c -- Ҫ�����õ�ֵ����ֵ�� int ��ʽ���ݣ����Ǻ���������ڴ��ʱ��ʹ�ø�ֵ���޷����ַ���ʽ��
	n -- Ҫ������Ϊ��ֵ���ֽ�����*/
	/*memset(&disk.data[InodeBitmapStart], 0, sizeof(block));//�������ϵ�һ������ȫ���㣨��ʼ����//λͼ����
	memcpy(&disk.data[InodeBitmapStart], InodeBitmap, sizeof(InodeBitmap));

	initInode();//��ʽ��inode
	InitRootFolder();//��ʽ��Ŀ¼
	memset(&disk.data[InodeStart], 0, sizeof(block));//�������ϵ�һ������ȫ���㣨��ʼ����
	memcpy(&disk.data[InodeStart], Inode, sizeof(Inode));

	//д�뵽��������ļ�
	/*int fseek(FILE *stream, long int offset, int whence)
	stream -- ����ָ�� FILE �����ָ�룬�� FILE �����ʶ������
	offset -- ������� whence ��ƫ���������ֽ�Ϊ��λ��
	whence -- ���Ǳ�ʾ��ʼ���ƫ�� offset ��λ�á���һ��ָ��Ϊ���г���֮һ��*/
	/*fseek(fw, SuperBlockStart, SEEK_SET);
	/*size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
	ptr -- ����ָ��Ҫ��д���Ԫ�������ָ�롣
	size -- ����Ҫ��д���ÿ��Ԫ�صĴ�С�����ֽ�Ϊ��λ��
	nmemb -- ����Ԫ�صĸ�����ÿ��Ԫ�صĴ�СΪ size �ֽڡ�
	stream -- ����ָ�� FILE �����ָ�룬�� FILE ����ָ����һ���������*/
	/*for (int i = 0; i < 3; i++)
		for (int j = 0; j < 512 * 8; j++)
			cout << (bool)disk.data[i].byte[j];
	fwrite(&disk, sizeof(disk), 1, fw);
	fflush(fw);




	return true;

}*/