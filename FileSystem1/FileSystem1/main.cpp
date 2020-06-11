#include <fstream>
#include <iostream>
using namespace std;


const int FILE_NAME_LEN = 8;
const int FILE_EXT_LEN = 8;

const int BlockSize = 512;
const int InodeBlockSum = 256;

const int DataBlockSum = 1024;
const int DiskBlockSum = 1 + 1+ InodeBlockSum + DataBlockSum;
const int InodeSize = 128;
const int DataBlockStart = InodeBlockSum+2;

const int InodeSumInOneBlock = BlockSize / InodeSize;
const int InodeSum = InodeSumInOneBlock * InodeBlockSum;

const int FreeBlockSum = DataBlockSum / 100 + 1;
const int MaxFreeBlockCount = 100;

struct block
{
	bool bit[512 * 8] = { 0 };
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


struct datablock {
	int inodeAddr; //目录项对应的inode节点地址
	char data[512 - sizeof(int)] = { 0 };
};


struct Disk {

	block data[DiskBlockSum];
};


struct FreeBlock
{

	int EmptyBlockList[101];
};

struct superblock {
	int totalINodeSum;
	int freeINodeSum;
	int totalBlockSum;
	int freeBlockSum;
	char name[20] = { 0 };
	int firstInode = 0;
	FreeBlock SuperEmptyBlockList;

};


struct File {
	inode fileInode;
	int dataSize;
	char* data;
};




superblock SuperBlock;
inode Inode[InodeSum];

bool InodeBitmap[InodeSum] = { 0 }; //0表示inode节点没被用 1表示inode节点被用 最大支持512*8个inode节点索引


Disk disk;


inode *NowPath = &Inode[SuperBlock.firstInode];

void initInode()
{
	for (int i = 0; i < InodeSum; i++)
	{
		Inode[i].inodeId = i;
	}
}

int GetAInode()
{
	static int lastPos = 0;

	for (int i = lastPos; i < InodeSum; i++)
	{
		if (InodeBitmap[i] == 0)
		{
			InodeBitmap[i] = 1;
			lastPos = i;
			return i;

		}
	}
	for (int i = 0; i < lastPos; i++)
	{
		if (InodeBitmap[i] == 0)
		{
			InodeBitmap[i] = 1;
			lastPos = i;
			return i;

		}
	}

	return -1;
}

bool FreeAInode(int index)
{
	if (index < InodeSum)
	{
		InodeBitmap[index] = 0;
		return true;
	}
	return false;
}



FreeBlock makeFreeBlock(Disk &disk, int index)
{
	FreeBlock freeBlock;
	freeBlock.EmptyBlockList[0] = 0;
	if (index + 100 < DiskBlockSum)
	{
		freeBlock.EmptyBlockList[1] = index + 100;
	}
	else
	{
		freeBlock.EmptyBlockList[1] = 0;
	}
	for (int i = 1; i < 100; i++)
	{
		
		if (index + i >= DiskBlockSum)
		{
			break;
		}
		freeBlock.EmptyBlockList[0]++;
		freeBlock.EmptyBlockList[i+1] = index + i;
		//cout << "Link " << index + i << endl;
	}
	return freeBlock;
}



void initGroupLink(Disk& disk)
{

	int FreeBlockSum = DataBlockSum / 100+1;
	SuperBlock.SuperEmptyBlockList.EmptyBlockList[0] = 1;
	SuperBlock.SuperEmptyBlockList.EmptyBlockList[1] = DataBlockStart;
	for (int i = 0; i < FreeBlockSum; i++)
	{
		FreeBlock freeBlock = makeFreeBlock(disk, i * 100+DataBlockStart);

		memset(&disk.data[i * 100 + DataBlockStart], 0, sizeof(block));
		memcpy(&disk.data[i * 100 + DataBlockStart], &freeBlock, sizeof(FreeBlock));
		//cout << "Make linklist " << i * 100 + DataBlockStart << endl;
	}
}

FreeBlock* LoadBlockAsFreeBlock(Disk& disk, int index)
{
	if (index > DataBlockSum + DataBlockStart)
	{
		return NULL;
	}
	FreeBlock *freeBlock=new FreeBlock;
	
	memcpy(freeBlock, &disk.data[index], sizeof(FreeBlock));
	return freeBlock;
}

bool SaveFreeBlockToDisk(Disk& disk, int index, FreeBlock& freeBlock)
{
if (index >= DiskBlockSum)
{
	return false;
}

memset(&disk.data[index], 0, sizeof(block));
memcpy(&disk.data[index], &freeBlock, sizeof(FreeBlock));
return true;
}

void ShowFreeBlock(FreeBlock* freeBlock, Disk& disk, int deep)
{
	if (freeBlock->EmptyBlockList[1] != 0)
	{
		FreeBlock* freeBlockNext = LoadBlockAsFreeBlock(disk, freeBlock->EmptyBlockList[1]);
		ShowFreeBlock(freeBlockNext, disk, deep + 1);
	}
	int ShowSum = freeBlock->EmptyBlockList[0];
	if (freeBlock->EmptyBlockList[1] != 0)
	{
		ShowSum--;
	}
	for (int i = 0; i < ShowSum; i++)
	{
		for (int j = 0; j <= deep; j++)
		{
			cout << "\t";
		}
		cout << freeBlock->EmptyBlockList[i + 2] << endl;
	}

}

void showAll()
{
	for (int i = 0; i < SuperBlock.SuperEmptyBlockList.EmptyBlockList[0]; i++)
	{
		FreeBlock* freeBlock = LoadBlockAsFreeBlock(disk, SuperBlock.SuperEmptyBlockList.EmptyBlockList[i + 1]);
		ShowFreeBlock(freeBlock, disk, 0);
	}
	for (int i = 0; i <= SuperBlock.SuperEmptyBlockList.EmptyBlockList[0]; i++)
	{
		cout << SuperBlock.SuperEmptyBlockList.EmptyBlockList[i] << endl;
	}
}




int GetOneBlock(Disk& disk)
{

	if (SuperBlock.SuperEmptyBlockList.EmptyBlockList[0] == 1)
	{
		if (SuperBlock.SuperEmptyBlockList.EmptyBlockList[1] == 0)
		{
			return -1;
		}
		else
		{
			int index = SuperBlock.SuperEmptyBlockList.EmptyBlockList[1];
			SuperBlock.SuperEmptyBlockList = *LoadBlockAsFreeBlock(disk, index);
			return index;
		}
	}
	else {
		int index = SuperBlock.SuperEmptyBlockList.EmptyBlockList[SuperBlock.SuperEmptyBlockList.EmptyBlockList[0]];
		SuperBlock.SuperEmptyBlockList.EmptyBlockList[0]--;
		return index;
	}
}

void FreeABlock(Disk& disk, int index)
{
	if (SuperBlock.SuperEmptyBlockList.EmptyBlockList[0] == MaxFreeBlockCount)
	{
		SaveFreeBlockToDisk(disk, index, SuperBlock.SuperEmptyBlockList);
		SuperBlock.SuperEmptyBlockList.EmptyBlockList[0] = 1;
		SuperBlock.SuperEmptyBlockList.EmptyBlockList[1] = index;
	}
	else
	{
		SuperBlock.SuperEmptyBlockList.EmptyBlockList[0]++;
		SuperBlock.SuperEmptyBlockList.EmptyBlockList[SuperBlock.SuperEmptyBlockList.EmptyBlockList[0]] = index;
	}
}

void ShowAllBlock(Disk& disk)
{
	cout << "Total Block:" << DiskBlockSum << endl;

}


void SaveSuperBlockToDisk(superblock& SuperBlock, Disk& disk)
{
	memcpy(&disk.data[0], &SuperBlock, sizeof(block));
}
void SaveInodeToDisk(bool& bitmap, inode& inodeList, Disk& disk)
{
	memcpy(&disk.data[1], &bitmap, sizeof(block));
	memcpy(&disk.data[2], &inodeList, sizeof(inode) * InodeSum);

}

void LoadSuperBlockToDisk(superblock& SuperBlock, Disk& disk)
{
	memcpy(&SuperBlock, &disk.data[0], sizeof(superblock));

}

void LoadInodeFromDisk(bool& bitmap, inode& inodeList, Disk& disk)
{
	memcpy(&bitmap , &disk.data[1], sizeof(block));
	memcpy(&inodeList , &disk.data[2], sizeof(inode) * InodeSum);
}

void SaveDisk()
{
	SaveSuperBlockToDisk(SuperBlock, disk);
	SaveInodeToDisk(*InodeBitmap, *Inode, disk);
	ofstream out;
	out.open("data.dat", ios::out|ios::binary);

	char* buffer = (char*)malloc(sizeof(Disk));
	memcpy(buffer, &disk, sizeof(Disk));
	out.write(buffer, sizeof(disk));
	out.close();
	free(buffer);
}


void LoadDisk()
{
	ifstream in;
	in.open("data.dat", ios::in | ios::binary);
	char* buffer = (char*)malloc(sizeof(Disk));
	in.read(buffer, sizeof(disk));
	in.close();
	memcpy(&disk, buffer, sizeof(Disk));
	free(buffer);

	LoadSuperBlockToDisk(SuperBlock, disk);
	LoadInodeFromDisk(*InodeBitmap, *Inode, disk);
}


//=============================================
struct Folder
{
	int itemSum = 0;
	int index[20];
	char name[20][20];
};

void SaveFolderToBlock(Disk& disk, int index, Folder folder)
{
	if (index > DiskBlockSum)
	{
		return;
	}
	memcpy(&disk.data[index], &folder, sizeof(Folder));
}


Folder *loadFolderFromDisk(Disk& disk, int index)
{
	if (index > DiskBlockSum)
	{
		return NULL;
	}
	Folder* folder = new Folder;
	memcpy(folder, &disk.data[index], sizeof(Folder));
	return folder;
	
}

//根目录
void InitRootFolder()
{
	strcpy_s(Inode[SuperBlock.firstInode].Name, "root");
	Inode[SuperBlock.firstInode].DataBlockIndex0[0] = GetOneBlock(disk);
	SuperBlock.firstInode = GetAInode();
	
	Folder rootF;
	rootF.index[0] = SuperBlock.firstInode;
	rootF.index[1] = SuperBlock.firstInode;
	
	sprintf_s(rootF.name[0], "..");
	sprintf_s(rootF.name[1],".");
	rootF.itemSum = 2;
	SaveFolderToBlock(disk, Inode[SuperBlock.firstInode].DataBlockIndex0[0], rootF);
	

}

void AddItemInFolder(inode *folderInode, char* name, int inodeIndex)
{
	Folder *folder = loadFolderFromDisk(disk, folderInode->DataBlockIndex0[0]);
	strcpy_s(folder->name[folder->itemSum], name);
	folder->index[folder->itemSum] = inodeIndex;
	folder->itemSum++;
	SaveFolderToBlock(disk, folderInode->DataBlockIndex0[0], *folder);
}


//======================================================
// text block
struct TextBlock
{
	int inodeindex;
	char data[sizeof(block)-sizeof(int)];
};

void SaveTextBlockToDisk(Disk& disk, int index, TextBlock& textBlock)
{
	if (index > DiskBlockSum)
	{
		return;
	}
	memcpy(&disk.data[index], &textBlock, sizeof(TextBlock));
}

TextBlock* LoadTextBlockFromDisk(Disk& disk, int index)
{
	if (index > DiskBlockSum)
	{
		return NULL;
	}
	TextBlock* textBlock = new TextBlock;
	memcpy(textBlock, &disk.data[index], sizeof(TextBlock));
	return textBlock;
}

void NewTxt(inode *FolderInode)
{
	int indexInode = GetAInode();
	int indexBlock;


	//写入自己的inode
	
	cout << "name:";
	char name[NameLen];
	cin >> name;


	strcpy_s(Inode[indexInode].Name, name);

	const int maxsize = 99999;

	//写入自己的datablock
	char text[99999] = { 0 };
	cout << "data:";
	cin >> text;
	Inode[indexInode].size = strlen(text);
	
	int dataOneBlock = (sizeof(TextBlock) - sizeof(int));
	int blockSize = Inode[indexInode].size / dataOneBlock +1;
	

	
	for (int i = 0; i < blockSize; i++)
	{
		indexBlock = GetOneBlock(disk);
		TextBlock textBlock;
		memcpy(textBlock.data, &text[i * dataOneBlock], dataOneBlock);
		textBlock.inodeindex = indexInode;
		Inode[indexInode].DataBlockIndex0[i]= indexBlock;
		SaveTextBlockToDisk(disk, indexBlock, textBlock);
		
	}


	//修改上级目录
	AddItemInFolder(FolderInode, name, indexInode);

}


void ShowText(inode *fileinode)
{
	cout << "filename:" << fileinode->Name << endl;

	int CharInOneBlockSum = sizeof(TextBlock) - sizeof(int);
	int fileDataBlockSum = fileinode->size / CharInOneBlockSum + 1;

	for (int i = 0; i < fileDataBlockSum; i++)
	{
		TextBlock* textBlock = LoadTextBlockFromDisk(disk, fileinode->DataBlockIndex0[i]);
		cout << textBlock->data << endl;
	}

}

void NewFolder(Disk& disk,inode *FatherFolderInode,char *folderName)
{

	Folder folderBlock;



	sprintf_s(folderBlock.name[0], "..");
	sprintf_s(folderBlock.name[1], ".");
	folderBlock.itemSum = 2;
	int inodeId = GetAInode();

	folderBlock.index[0] = FatherFolderInode->inodeId;
	folderBlock.index[1] = inodeId;

	int blockId = GetOneBlock(disk);
	strcpy_s(Inode[inodeId].Name, folderName);
	Inode[inodeId].DataBlockIndex0[0] = blockId;
	SaveFolderToBlock(disk, blockId, folderBlock);

	//修改上级目录
	AddItemInFolder(FatherFolderInode, folderName, inodeId);

}

void LS(inode *Inode)
{
	Folder* folder = loadFolderFromDisk(disk, Inode->DataBlockIndex0[0]);
	for (int i = 0; i < folder->itemSum; i++)
	{
		cout << folder->index[i] << "\t";
		cout << folder->name[i] << endl;
	}
}

void CD(char *name,inode **nowpath)
{
	Folder* folder = loadFolderFromDisk(disk, (*nowpath)->DataBlockIndex0[0]);
	for (int i = 0; i < folder->itemSum; i++)
	{
		if (strcmp(name, folder->name[i])==0)
		{
			*nowpath = &Inode[ folder->index[i]];

		}
	}

}

int main()
{
	cout << sizeof(TextBlock) << endl;
	initInode();
	initGroupLink(disk);
	InitRootFolder();

	//showAll();
	//cout << sizeof(Folder) << endl;
	//cout << sizeof(int) << endl;
	//cout << sizeof(char)* NameLen << endl;
	cout << sizeof(inode) << endl;
	LS(NowPath);
	while (1)
	{
		char command[20] = { 0 };
		cin >> command;
		if (strcmp(command, "cd") == 0)
		{
			char name[20];
			cin >> name;
			CD(name,&NowPath);
			LS(NowPath);
		}
		if (strcmp(command, "ls") == 0)
		{
			LS(NowPath);
		}
		if (strcmp(command, "new") == 0)
		{
			NewTxt(NowPath);
		}
		if (strcmp(command, "open") == 0)
		{
			int index;
			cin >> index;
			ShowText(&Inode[index]);
		}
		if (strcmp(command, "save") == 0)
		{
			SaveDisk();
		}
		if (strcmp(command, "load") == 0)
		{
			LoadDisk();
		}
		if (strcmp(command, "mkdir") == 0)
		{

			char name[20];
			cin >> name;
			NewFolder(disk, NowPath, name);
			LS(NowPath);
		}
	}

	


	return 0;
}