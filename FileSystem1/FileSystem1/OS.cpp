#include"OS.h"
superblock SuperBlock;
superblock* SuperP = &SuperBlock;
Disk disk;
inode Inode[InodeSum];
bool InodeBitmap[InodeSum] = { 0 };
inode* NowPath = &Inode[SuperBlock.firstInode];
FILE* fw;
FILE* fr;
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


FreeBlock makeFreeBlock(Disk& disk, int index)//在磁盘的index位置上建一个组
{
	FreeBlock freeBlock;
	freeBlock.EmptyBlockList[0] = 0;//将空闲块个数初始化为0
	if (index + 100 < DiskBlockSum)
	{
		freeBlock.EmptyBlockList[1] = index + 100;//初始化下一块地址
	}
	else
	{
		freeBlock.EmptyBlockList[1] = 0;
	}
	for (int i = 1; i < 100; i++)//分配100个空闲块
	{

		if (index + i >= DiskBlockSum)
		{
			break;
		}
		freeBlock.EmptyBlockList[0]++;//空闲块个数+1
		freeBlock.EmptyBlockList[i + 1] = index + i;//将下一个空闲块地址存入成组链
		//cout << "Link " << index + i << endl;
	}
	return freeBlock;
}



void initGroupLink(Disk& disk)//初始化成组链
{

	int FreeBlockSum = DataBlockSum / 100 + 1;//总共十一个组，连成链叫成组链
	SuperBlock.SuperEmptyBlockList.EmptyBlockList[0] = 1;
	SuperBlock.SuperEmptyBlockList.EmptyBlockList[1] = DataBlockStart;//第一块数据块的位置
	for (int i = 0; i < FreeBlockSum; i++)
	{
		FreeBlock freeBlock = makeFreeBlock(disk, i * 100 + DataBlockStart);//在i * 100 + DataBlockStart位置上建一个组
		/*void *memset(void *str, int c, size_t n)
	    str -- 指向要填充的内存块。
	    c -- 要被设置的值。该值以 int 形式传递，但是函数在填充内存块时是使用该值的无符号字符形式。
	    n -- 要被设置为该值的字节数。*/
		memset(&disk.data[i * 100 + DataBlockStart], 0, sizeof(block));//将磁盘上的一块数据全置零（初始化）
		/*void *memcpy(void *str1, const void *str2, size_t n)
		str1 -- 指向用于存储复制内容的目标数组，类型强制转换为 void* 指针。
        str2 -- 指向要复制的数据源，类型强制转换为 void* 指针。
        n -- 要被复制的字节数。*/
		memcpy(&disk.data[i * 100 + DataBlockStart], &freeBlock, sizeof(FreeBlock));//将freeBlock里的数据写入上一步初始化的磁盘块
		//cout << "Make linklist " << i * 100 + DataBlockStart << endl;
	}
}

FreeBlock* LoadBlockAsFreeBlock(Disk& disk, int index)
{
	if (index > DataBlockSum + DataBlockStart)
	{
		return NULL;
	}
	FreeBlock* freeBlock = new FreeBlock;

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
	memcpy(&bitmap, &disk.data[1], sizeof(block));
	memcpy(&inodeList, &disk.data[2], sizeof(inode) * InodeSum);
}

void SaveDisk()
{
	SaveSuperBlockToDisk(SuperBlock, disk);
	SaveInodeToDisk(*InodeBitmap, *Inode, disk);
	ofstream out;
	out.open("data.dat", ios::out | ios::binary);

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

//根目录
void SaveFolderToBlock(Disk& disk, int index, Folder folder)
{
	if (index > DiskBlockSum)
	{
		return;
	}
	memcpy(&disk.data[index], &folder, sizeof(Folder));
}


Folder* loadFolderFromDisk(Disk& disk, int index)
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
	Inode[SuperBlock.firstInode].firstDataBlockIndex = GetOneBlock(disk);
	SuperBlock.firstInode = GetAInode();

	Folder rootF;
	rootF.index[0] = SuperBlock.firstInode;
	rootF.index[1] = SuperBlock.firstInode;

	sprintf_s(rootF.name[0], "..");
	sprintf_s(rootF.name[1], ".");
	rootF.itemSum = 2;
	SaveFolderToBlock(disk, Inode[SuperBlock.firstInode].firstDataBlockIndex, rootF);


}

void AddItemInFolder(inode* folderInode, char* name, int inodeIndex)
{
	Folder* folder = loadFolderFromDisk(disk, folderInode->firstDataBlockIndex);
	strcpy_s(folder->name[folder->itemSum], name);
	folder->index[folder->itemSum] = inodeIndex;
	folder->itemSum++;
	SaveFolderToBlock(disk, folderInode->firstDataBlockIndex, *folder);
}

//text block func

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

void NewTxt(inode* FolderInode)
{
	int indexInode = GetAInode();
	int indexBlock = GetOneBlock(disk);


	//写入自己的inode

	cout << "name:";
	char name[NameLen];
	cin >> name;

	Inode[indexInode].firstDataBlockIndex = indexBlock;
	strcpy_s(Inode[indexInode].Name, name);


	//写入自己的datablock
	char text[BlockSize] = { 0 };
	cout << "data:";
	cin >> text;


	TextBlock textBlock;
	sprintf_s(textBlock.data, text);
	textBlock.next = -1;
	SaveTextBlockToDisk(disk, indexBlock, textBlock);

	//修改上级目录
	AddItemInFolder(FolderInode, name, indexInode);

}


void ShowText(inode* fileinode)
{
	cout << "filename:" << fileinode->Name << endl;
	TextBlock* textBlock = LoadTextBlockFromDisk(disk, fileinode->firstDataBlockIndex);
	cout << textBlock->data << endl;
}

void NewFolder(Disk& disk, inode* FatherFolderInode, char* folderName)
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
	Inode[inodeId].firstDataBlockIndex = blockId;
	SaveFolderToBlock(disk, blockId, folderBlock);

	//修改上级目录
	AddItemInFolder(FatherFolderInode, folderName, inodeId);

}

void LS(inode* Inode)
{
	Folder* folder = loadFolderFromDisk(disk, Inode->firstDataBlockIndex);
	for (int i = 0; i < folder->itemSum; i++)
	{
		cout << folder->index[i] << "\t";
		cout << folder->name[i] << endl;
	}
}

void CD(char* name, inode** nowpath)
{
	Folder* folder = loadFolderFromDisk(disk, (*nowpath)->firstDataBlockIndex);
	for (int i = 0; i < folder->itemSum; i++)
	{
		if (strcmp(name, folder->name[i]) == 0)
		{
			*nowpath = &Inode[folder->index[i]];

		}
	}

}

bool Format()
{
	//格式化超级块
	SuperBlock.totalINodeSum = InodeSum;
	SuperBlock.totalBlockSum = DiskBlockSum;
	SuperBlock.freeBlockSum = 0;
	SuperBlock.freeINodeSum = 0;
	SuperBlock.firstInode = 0;
	SuperBlock.name[20] = {0};
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
	memset(&disk.data[InodeBitmapStart], 0, sizeof(block));//将磁盘上的一块数据全置零（初始化）//位图置零
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
	fseek(fw, SuperBlockStart, SEEK_SET);
	/*size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
	ptr -- 这是指向要被写入的元素数组的指针。
    size -- 这是要被写入的每个元素的大小，以字节为单位。
    nmemb -- 这是元素的个数，每个元素的大小为 size 字节。
    stream -- 这是指向 FILE 对象的指针，该 FILE 对象指定了一个输出流。*/
	for (int i = 0; i < 3; i++)
		for(int j=0;j<512*8;j++)
		    cout << (bool)disk.data[i].byte[j];
	fwrite(&disk, sizeof(disk), 1, fw);
	fflush(fw);

	
	
	
	return true;

}