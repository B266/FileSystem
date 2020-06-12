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


FreeBlock makeFreeBlock(Disk& disk, int index)//�ڴ��̵�indexλ���Ͻ�һ����
{
	FreeBlock freeBlock;
	freeBlock.EmptyBlockList[0] = 0;//�����п������ʼ��Ϊ0
	if (index + 100 < DiskBlockSum)
	{
		freeBlock.EmptyBlockList[1] = index + 100;//��ʼ����һ���ַ
	}
	else
	{
		freeBlock.EmptyBlockList[1] = 0;
	}
	for (int i = 1; i < 100; i++)//����100�����п�
	{

		if (index + i >= DiskBlockSum)
		{
			break;
		}
		freeBlock.EmptyBlockList[0]++;//���п����+1
		freeBlock.EmptyBlockList[i + 1] = index + i;//����һ�����п��ַ���������
		//cout << "Link " << index + i << endl;
	}
	return freeBlock;
}



void initGroupLink(Disk& disk)//��ʼ��������
{

	int FreeBlockSum = DataBlockSum / 100 + 1;//�ܹ�ʮһ���飬�������г�����
	SuperBlock.SuperEmptyBlockList.EmptyBlockList[0] = 1;
	SuperBlock.SuperEmptyBlockList.EmptyBlockList[1] = DataBlockStart;//��һ�����ݿ��λ��
	for (int i = 0; i < FreeBlockSum; i++)
	{
		FreeBlock freeBlock = makeFreeBlock(disk, i * 100 + DataBlockStart);//��i * 100 + DataBlockStartλ���Ͻ�һ����
		/*void *memset(void *str, int c, size_t n)
	    str -- ָ��Ҫ�����ڴ�顣
	    c -- Ҫ�����õ�ֵ����ֵ�� int ��ʽ���ݣ����Ǻ���������ڴ��ʱ��ʹ�ø�ֵ���޷����ַ���ʽ��
	    n -- Ҫ������Ϊ��ֵ���ֽ�����*/
		memset(&disk.data[i * 100 + DataBlockStart], 0, sizeof(block));//�������ϵ�һ������ȫ���㣨��ʼ����
		/*void *memcpy(void *str1, const void *str2, size_t n)
		str1 -- ָ�����ڴ洢�������ݵ�Ŀ�����飬����ǿ��ת��Ϊ void* ָ�롣
        str2 -- ָ��Ҫ���Ƶ�����Դ������ǿ��ת��Ϊ void* ָ�롣
        n -- Ҫ�����Ƶ��ֽ�����*/
		memcpy(&disk.data[i * 100 + DataBlockStart], &freeBlock, sizeof(FreeBlock));//��freeBlock�������д����һ����ʼ���Ĵ��̿�
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

//��Ŀ¼
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

//��Ŀ¼
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


	//д���Լ���inode

	cout << "name:";
	char name[NameLen];
	cin >> name;

	Inode[indexInode].firstDataBlockIndex = indexBlock;
	strcpy_s(Inode[indexInode].Name, name);


	//д���Լ���datablock
	char text[BlockSize] = { 0 };
	cout << "data:";
	cin >> text;


	TextBlock textBlock;
	sprintf_s(textBlock.data, text);
	textBlock.next = -1;
	SaveTextBlockToDisk(disk, indexBlock, textBlock);

	//�޸��ϼ�Ŀ¼
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

	//�޸��ϼ�Ŀ¼
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
	//��ʽ��������
	SuperBlock.totalINodeSum = InodeSum;
	SuperBlock.totalBlockSum = DiskBlockSum;
	SuperBlock.freeBlockSum = 0;
	SuperBlock.freeINodeSum = 0;
	SuperBlock.firstInode = 0;
	SuperBlock.name[20] = {0};
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
	memset(&disk.data[InodeBitmapStart], 0, sizeof(block));//�������ϵ�һ������ȫ���㣨��ʼ����//λͼ����
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
	fseek(fw, SuperBlockStart, SEEK_SET);
	/*size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
	ptr -- ����ָ��Ҫ��д���Ԫ�������ָ�롣
    size -- ����Ҫ��д���ÿ��Ԫ�صĴ�С�����ֽ�Ϊ��λ��
    nmemb -- ����Ԫ�صĸ�����ÿ��Ԫ�صĴ�СΪ size �ֽڡ�
    stream -- ����ָ�� FILE �����ָ�룬�� FILE ����ָ����һ���������*/
	for (int i = 0; i < 3; i++)
		for(int j=0;j<512*8;j++)
		    cout << (bool)disk.data[i].byte[j];
	fwrite(&disk, sizeof(disk), 1, fw);
	fflush(fw);

	
	
	
	return true;

}