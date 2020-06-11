#ifndef OS_H
#define OS_H
#include <fstream>
#include <iostream>
using namespace std;
extern FILE* fw;
extern FILE* fr;

const int FILE_NAME_LEN = 8;
const int FILE_EXT_LEN = 8;

const int BlockSize = 512;//���С��һ����512�ֽ�
const int InodeBlockSum = 256;//���inode�Ŀ������

const int DataBlockSum = 1024;//���ݿ������
const int DiskBlockSum = 1 + 1 + InodeBlockSum + DataBlockSum;//������̿�������1������+1λͼ+256inode��+1024���ݿ�=1282��
const int InodeSize = 128;//inode��С
const int DataBlockStart = InodeBlockSum + 2;//1�ŷų����飬2����257�ŷ�inode��258����1282�ŷ����ݿ�

const int InodeSumInOneBlock = BlockSize / InodeSize;//һ����inode��
const int InodeSum = InodeSumInOneBlock * InodeBlockSum;//inode����

const int FreeBlockSum = DataBlockSum / 100 + 1;
const int MaxFreeBlockCount = 100;

struct block
{
	char byte[512] = { 0 };//ÿһ��ռ512�ֽ�
};



const int NameLen = 18;



struct inode {  //128byte
	int inodeId = 0;
	char Name[NameLen] = { 0 };
	char username[20];						//�ļ������û�
	char usergroupname[20];						//�ļ������û���
	int permissions = 0;

	int size;

	int DataBlockIndex0[10] = { 0 };						//10��ֱ�ӿ顣10*512B = 5120B = 5KB
	int DataBlockIndex1;						//һ����ӿ顣512B/4 * 512B = 128 * 512B = 64KB
	int DataBlockIndex2;			//������ӿ顣(512B/4)*(512B/4) * 512B = 128*128*512B = 8192KB = 8MB
	int DataBlockIndex3;			//������ӿ顣(512B/4)*(512B/4)*(512B/4) * 512B = 128*128*128*512B = 1048576KB = 1024MB = 1G
											//�ļ�ϵͳ̫С������ʡ�Զ�����������ӿ�
};


struct datablock {//���ݿ飬����ļ�����
	int inodeAddr; //Ŀ¼���Ӧ��inode�ڵ��ַ
	char data[512 - sizeof(int)] = { 0 };
};


struct Disk {//�������

	block data[DiskBlockSum];//1282���飬ÿ��512�ֽ�
};


struct FreeBlock//���п��б�
{

	int EmptyBlockList[101];
};

struct superblock {//������
	int totalINodeSum;//inode����
	int freeINodeSum;//����inode��
	int totalBlockSum;//������
	int freeBlockSum;//���п�����
	char name[20] = { 0 };
	int firstInode = 0;
	FreeBlock SuperEmptyBlockList;//���п��б�

};



struct File {
	inode fileInode;
	int dataSize;
	char* data;
};
//======================================================
// text block
struct TextBlock
{
	int inodeindex;
	char data[sizeof(block) - sizeof(int)];
};

//=============================================
struct Folder
{
	int itemSum = 0;
	int index[20];
	char name[20][20];
};



//---------------�������----------------------------------------------------------------------------------
extern superblock SuperBlock;
extern inode Inode[InodeSum];

extern bool InodeBitmap[InodeSum]; //inodeλͼ��0��ʾinode�ڵ�û���� 1��ʾinode�ڵ㱻�� ���֧��512*8��inode�ڵ�����


extern Disk disk;

extern inode* NowPath;//��ǰĿ¼
//----------------��������-----------------------------------------------------------------------------
void initInode();

int GetAInode();
bool FreeAInode(int index);
FreeBlock makeFreeBlock(Disk& disk, int index);
void initGroupLink(Disk& disk);
FreeBlock* LoadBlockAsFreeBlock(Disk& disk, int index);
bool SaveFreeBlockToDisk(Disk& disk, int index, FreeBlock& freeBlock);
void ShowFreeBlock(FreeBlock* freeBlock, Disk& disk, int deep);
void showAll();
int GetOneBlock(Disk& disk);
void FreeABlock(Disk& disk, int index);
void ShowAllBlock(Disk& disk);
void SaveSuperBlockToDisk(superblock& SuperBlock, Disk& disk);
void SaveInodeToDisk(bool& bitmap, inode& inodeList, Disk& disk);
void LoadSuperBlockToDisk(superblock& SuperBlock, Disk& disk);
void LoadInodeFromDisk(bool& bitmap, inode& inodeList, Disk& disk);
void SaveDisk();
void LoadDisk();

bool Format();



void SaveFolderToBlock(Disk& disk, int index, Folder folder);
Folder* loadFolderFromDisk(Disk& disk, int index);
void InitRootFolder();
void AddItemInFolder(inode folderInode, char* name, int inodeIndex);


void SaveTextBlockToDisk(Disk& disk, int index, TextBlock& textBlock);
TextBlock* LoadTextBlockFromDisk(Disk& disk, int index);
void NewTxt(inode* FolderInode);
void ShowText(inode* fileinode);
void NewFolder(Disk& disk, inode* FatherFolderInode, char* folderName);
void LS(inode* Inode);
void CD(char* name, inode** nowpath);

#endif 

