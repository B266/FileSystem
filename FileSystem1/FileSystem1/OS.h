#ifndef OS_H
#define OS_H
#include <fstream>
#include <iostream>
using namespace std;
extern FILE* fw;
extern FILE* fr;

const int FILE_NAME_LEN = 8;
const int FILE_EXT_LEN = 8;

const int BlockSize = 512;//块大小，一块存放512字节
const int InodeBlockSum = 256;//存放inode的块的数量

const int DataBlockSum = 1024;//数据块的数量
const int DiskBlockSum = 1 + 1 + InodeBlockSum + DataBlockSum;//虚拟磁盘块总数：1超级快+1位图+256inode块+1024数据块=1282块
const int InodeSize = 128;//inode大小
const int DataBlockStart = InodeBlockSum + 2;//1号放超级块，2——257号放inode，258——1282号放数据块

const int InodeSumInOneBlock = BlockSize / InodeSize;//一块中inode数
const int InodeSum = InodeSumInOneBlock * InodeBlockSum;//inode总数

const int FreeBlockSum = DataBlockSum / 100 + 1;
const int MaxFreeBlockCount = 100;

struct block
{
	char byte[512] = { 0 };//每一块占512字节
};



const int NameLen = 18;



struct inode {  //128byte
	int inodeId = 0;
	char Name[NameLen] = { 0 };
	char username[20];						//文件所属用户
	char usergroupname[20];						//文件所属用户组
	int permissions = 0;

	int size;

	int DataBlockIndex0[10] = { 0 };						//10个直接块。10*512B = 5120B = 5KB
	int DataBlockIndex1;						//一级间接块。512B/4 * 512B = 128 * 512B = 64KB
	int DataBlockIndex2;			//二级间接块。(512B/4)*(512B/4) * 512B = 128*128*512B = 8192KB = 8MB
	int DataBlockIndex3;			//三级间接块。(512B/4)*(512B/4)*(512B/4) * 512B = 128*128*128*512B = 1048576KB = 1024MB = 1G
											//文件系统太小，暂且省略二级、三级间接块
};


struct datablock {//数据块，存放文件数据
	int inodeAddr; //目录项对应的inode节点地址
	char data[512 - sizeof(int)] = { 0 };
};


struct Disk {//虚拟磁盘

	block data[DiskBlockSum];//1282个块，每块512字节
};


struct FreeBlock//空闲块列表
{

	int EmptyBlockList[101];
};

struct superblock {//超级块
	int totalINodeSum;//inode总数
	int freeINodeSum;//空闲inode数
	int totalBlockSum;//块总数
	int freeBlockSum;//空闲块总数
	char name[20] = { 0 };
	int firstInode = 0;
	FreeBlock SuperEmptyBlockList;//空闲块列表

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



//---------------共享变量----------------------------------------------------------------------------------
extern superblock SuperBlock;
extern inode Inode[InodeSum];

extern bool InodeBitmap[InodeSum]; //inode位图，0表示inode节点没被用 1表示inode节点被用 最大支持512*8个inode节点索引


extern Disk disk;

extern inode* NowPath;//当前目录
//----------------函数声明-----------------------------------------------------------------------------
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

