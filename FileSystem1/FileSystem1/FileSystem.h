

#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include <fstream>
#include <iostream>
#include <windows.h>
#include<wchar.h>
#include<conio.h> //for getch()

#include"cminicomplier.h"




const int FILE_NAME_LEN = 8;
const int FILE_EXT_LEN = 8;

const int BlockSize = 512;//块大小, 一块存放512字节

const int InodeSize = 128;//inode大小
const int InodeBitMapBlockSum = 50; //存放inode位图的块的数量

const int InodeSum = InodeBitMapBlockSum*512*8;//inode总数

const int InodeSumInOneBlock = BlockSize / InodeSize;//一块中inode数
const int InodeBlockSum = InodeSum /4;//存放inode的块的数量 

const int DataBlockSum = InodeSum;//数据块的数量
const int DiskBlockSum = 1 + InodeBitMapBlockSum + InodeBlockSum +1+ DataBlockSum;//虚拟磁盘块总数：超级块+inode位图+inode块+用户块+数据块[第一块为用户数据块]


const int InodeBitmapBlockStart = 1;
const int InodeBlockStart = 1 + InodeBitMapBlockSum;
const int UserBlockStart = 1 + InodeBitMapBlockSum + InodeBlockSum;
const int DataBlockStart =  1+InodeBitMapBlockSum+ InodeBlockSum + 1;//1号放超级块，2——257号放inode，258——1282号放数据块



const int FreeBlockSum = DataBlockSum / 100 + 1;
const int MaxFreeBlockCount = 100;

const int MAXPATH_LEN = 200;  //最大地址长度
const int MAXUSERNAME_LEN = 20; //最大用户名长度
const int MAXDEVICENAME_LEN = 20;//最大设备名称

struct block
{
	char byte[512] = { 0 };//每一块占512字节 
	//这里用bool来存居然一个bool就要一个字节
};



const int NameLen = 18;
const int PassWordLen = 20;



struct inode {  //128byte
	int inodeId = 0;
	char Name[NameLen] = { 0 };
	char ExtensionName[NameLen] = { 0 };
	char username[14];						//文件所属用户
	char usergroupname[14];						//文件所属用户组
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
	inode *fileInode;
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

struct DataBlockIndexFile
{
	int index[sizeof(block) / sizeof(int)];
};


//在数据块的第一块存账户块，第一个user为root用户,其他都为普通用户
struct User
{
	int userSum = 0;
	char name[8][NameLen] = { 0 };
	char password[8][PassWordLen] = { 0 };
};

extern bool isLogin;
bool Login(char* name, char* password,Disk&disk);
bool Logout();

bool useradd(char* name,Disk&disk);
void passwd(char* username,Disk&disk);
void initUserBlock(Disk& disk);

void UserManager(Disk& disk); //负责调用user有关的函数来显示


//---------------共享变量----------------------------------------------------------------------------------
extern superblock SuperBlock;
extern inode Inode[InodeSum];

extern bool InodeBitmap[InodeSum]; //inode位图，0表示inode节点没被用 1表示inode节点被用 最大支持512*8个inode节点索引
extern int lastInodePos ;
extern Disk disk;

extern inode* NowPath;//当前目录
extern inode* RootPath; //根目录

extern char NowPathName[MAXPATH_LEN];
extern char NowUser[MAXUSERNAME_LEN]; 
extern char DeviceName[MAXDEVICENAME_LEN];
extern HANDLE CommandLineHandle;

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
void SaveDataBlockIndexFileToDisk(DataBlockIndexFile& dataBlockIndexFile, Disk& disk, int index);
DataBlockIndexFile LoadDataBlockIndexFileFromDisk(Disk& disk, int index);
void SaveSuperBlockToDisk(superblock& SuperBlock, Disk& disk);
void SaveInodeToDisk(bool& bitmap, inode& inodeList, Disk& disk);
void LoadSuperBlockFromDisk(superblock& SuperBlock, Disk& disk);
void LoadInodeFromDisk(bool& bitmap, inode& inodeList, Disk& disk);
User LoadUserFromDisk(Disk& disk);
void SaveUserToDisk(Disk& disk, User user);
void SaveDisk();
void LoadDisk();


File* OpenFile(Disk &disk, inode *fileInode);  //读取某个inode的数据
void SaveFileData(Disk&disk ,inode* fileInode,char *data,int datasize); //保存某个inode的数据




void SaveFolderToBlock(Disk& disk, int index, Folder folder);
Folder* loadFolderFromDisk(Disk& disk, int index);
void InitRootFolder();
void AddItemInFolder(inode* folderInode, char* name, int inodeIndex);
void DeleteItemInFolder(inode* folderInode, inode* fileInode); // 更改目录结构，删除一个文件
inode* getInodeByPathName(const char* folderPathName, inode* nowPath=NowPath, int mode=1);


void SaveTextBlockToDisk(Disk& disk, int index, TextBlock& textBlock);
TextBlock* LoadTextBlockFromDisk(Disk& disk, int index);
void NewTxt(inode* FolderInode);
void ShowText(char *pathName, inode* nowpath);
void NewFolder(Disk& disk, inode* FatherFolderInode, char* folderName);
void LS(inode* Inode);
void LS(char* folderPathName);
void CD(char* name, inode** nowpath);
void RM(Disk& disk, inode* folderInode, char* name, bool isSonFolder); // 删除文件
bool Chmod(inode* Inode, int permission);
bool Chmod(char* pathname, int permission, inode* nowpath);
bool Rename(char* filenameandpath, char* name); //重命名
//Export 本硬盘内的文件 windows内的文件
bool Export(char* pathnameInWindows, char* filepathname);

//Import  windows内的文件
bool Import(char* pathnameInWindows, inode* NowPath);
void Format();


int complier(char* filename, inode* NowPath, Disk& disk);
void MV(inode* NowPath, char* filePath, char* targetPath);
void CP(inode* NowPath, char* filePath, char* targetPath);

void ShowNowPathInfo();
void SetTitle(const char* Title);

void CutArr(char* Arr, char* Arr1, char* Arr2, char* Arr3);


// 获取字符串的文件名和扩展名
void GetFileNameAndExtensionName(char* AllName, char* FileName, char* ExtensionName);

#endif FILESYSTEM_H