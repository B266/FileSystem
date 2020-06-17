

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

const int BlockSize = 512;//���С, һ����512�ֽ�

const int InodeSize = 128;//inode��С
const int InodeBitMapBlockSum = 50; //���inodeλͼ�Ŀ������

const int InodeSum = InodeBitMapBlockSum*512*8;//inode����

const int InodeSumInOneBlock = BlockSize / InodeSize;//һ����inode��
const int InodeBlockSum = InodeSum /4;//���inode�Ŀ������ 

const int DataBlockSum = InodeSum;//���ݿ������
const int DiskBlockSum = 1 + InodeBitMapBlockSum + InodeBlockSum +1+ DataBlockSum;//������̿�������������+inodeλͼ+inode��+�û���+���ݿ�[��һ��Ϊ�û����ݿ�]


const int InodeBitmapBlockStart = 1;
const int InodeBlockStart = 1 + InodeBitMapBlockSum;
const int UserBlockStart = 1 + InodeBitMapBlockSum + InodeBlockSum;
const int DataBlockStart =  1+InodeBitMapBlockSum+ InodeBlockSum + 1;//1�ŷų����飬2����257�ŷ�inode��258����1282�ŷ����ݿ�



const int FreeBlockSum = DataBlockSum / 100 + 1;
const int MaxFreeBlockCount = 100;

const int MAXPATH_LEN = 200;  //����ַ����
const int MAXUSERNAME_LEN = 20; //����û�������
const int MAXDEVICENAME_LEN = 20;//����豸����

struct block
{
	char byte[512] = { 0 };//ÿһ��ռ512�ֽ� 
	//������bool�����Ȼһ��bool��Ҫһ���ֽ�
};



const int NameLen = 18;
const int PassWordLen = 20;



struct inode {  //128byte
	int inodeId = 0;
	char Name[NameLen] = { 0 };
	char ExtensionName[NameLen] = { 0 };
	char username[14];						//�ļ������û�
	char usergroupname[14];						//�ļ������û���
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


//�����ݿ�ĵ�һ����˻��飬��һ��userΪroot�û�,������Ϊ��ͨ�û�
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

void UserManager(Disk& disk); //�������user�йصĺ�������ʾ


//---------------�������----------------------------------------------------------------------------------
extern superblock SuperBlock;
extern inode Inode[InodeSum];

extern bool InodeBitmap[InodeSum]; //inodeλͼ��0��ʾinode�ڵ�û���� 1��ʾinode�ڵ㱻�� ���֧��512*8��inode�ڵ�����
extern int lastInodePos ;
extern Disk disk;

extern inode* NowPath;//��ǰĿ¼
extern inode* RootPath; //��Ŀ¼

extern char NowPathName[MAXPATH_LEN];
extern char NowUser[MAXUSERNAME_LEN]; 
extern char DeviceName[MAXDEVICENAME_LEN];
extern HANDLE CommandLineHandle;

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


File* OpenFile(Disk &disk, inode *fileInode);  //��ȡĳ��inode������
void SaveFileData(Disk&disk ,inode* fileInode,char *data,int datasize); //����ĳ��inode������




void SaveFolderToBlock(Disk& disk, int index, Folder folder);
Folder* loadFolderFromDisk(Disk& disk, int index);
void InitRootFolder();
void AddItemInFolder(inode* folderInode, char* name, int inodeIndex);
void DeleteItemInFolder(inode* folderInode, inode* fileInode); // ����Ŀ¼�ṹ��ɾ��һ���ļ�
inode* getInodeByPathName(const char* folderPathName, inode* nowPath=NowPath, int mode=1);


void SaveTextBlockToDisk(Disk& disk, int index, TextBlock& textBlock);
TextBlock* LoadTextBlockFromDisk(Disk& disk, int index);
void NewTxt(inode* FolderInode);
void ShowText(char *pathName, inode* nowpath);
void NewFolder(Disk& disk, inode* FatherFolderInode, char* folderName);
void LS(inode* Inode);
void LS(char* folderPathName);
void CD(char* name, inode** nowpath);
void RM(Disk& disk, inode* folderInode, char* name, bool isSonFolder); // ɾ���ļ�
bool Chmod(inode* Inode, int permission);
bool Chmod(char* pathname, int permission, inode* nowpath);
bool Rename(char* filenameandpath, char* name); //������
//Export ��Ӳ���ڵ��ļ� windows�ڵ��ļ�
bool Export(char* pathnameInWindows, char* filepathname);

//Import  windows�ڵ��ļ�
bool Import(char* pathnameInWindows, inode* NowPath);
void Format();


int complier(char* filename, inode* NowPath, Disk& disk);
void MV(inode* NowPath, char* filePath, char* targetPath);
void CP(inode* NowPath, char* filePath, char* targetPath);

void ShowNowPathInfo();
void SetTitle(const char* Title);

void CutArr(char* Arr, char* Arr1, char* Arr2, char* Arr3);


// ��ȡ�ַ������ļ�������չ��
void GetFileNameAndExtensionName(char* AllName, char* FileName, char* ExtensionName);

#endif FILESYSTEM_H