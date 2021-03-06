
#include"FileSystem.h"


using namespace std;
superblock SuperBlock;
Disk disk;
inode Inode[InodeSum];
bool InodeBitmap[InodeSum] = { 0 };
inode* NowPath = &Inode[SuperBlock.firstInode];
inode* RootPath = &Inode[SuperBlock.firstInode];
char NowPathName[MAXPATH_LEN] = "/";
char NowUser[MAXUSERNAME_LEN] = "root";
char NowGroupName[NameLen] = "root";
char DeviceName[MAXDEVICENAME_LEN] = "Disk0";

bool isLogin = false;

void ShowNowPathInfo()
{
	SetConsoleTextAttribute(CommandLineHandle, FOREGROUND_GREEN);
	cout << NowUser << "@" << DeviceName << ":";
	SetConsoleTextAttribute(CommandLineHandle, 0x09);
	//cout << ((strcmp(NowPathName, "/") == 0) ? "" : NowPathName);
	cout << NowPathName;
	SetConsoleTextAttribute(CommandLineHandle, 0x07);
	cout << "$ ";
	char title[MAXPATH_LEN + MAXUSERNAME_LEN + MAXDEVICENAME_LEN + 2];
	sprintf_s(title, "%s@%s:%s", NowUser, DeviceName, NowPathName);
	SetTitle(title);
}

void initInode()
{
	for (int i = 0; i < InodeSum; i++)
	{
		Inode[i].inodeId = i;
	}
}
int lastInodePos = 0;
int GetAInode()
{


	for (int i = lastInodePos; i < InodeSum; i++)
	{
		if (InodeBitmap[i] == 0)
		{
			InodeBitmap[i] = 1;
			lastInodePos = i;
			return i;

		}
	}
	for (int i = 0; i < lastInodePos; i++)
	{
		if (InodeBitmap[i] == 0)
		{
			InodeBitmap[i] = 1;
			lastInodePos = i;
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



FreeBlock makeFreeBlock(Disk& disk, int index)
{
	FreeBlock freeBlock;
	freeBlock.EmptyBlockList[0] = 0;
	if (index + 100 < DiskBlockSum)
	{
		freeBlock.EmptyBlockList[1] = index + 100;
		freeBlock.EmptyBlockList[0]++;
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
		freeBlock.EmptyBlockList[i + 1] = index + i;
		//cout << "Link " << index + i << endl;
	}
	return freeBlock;
}



void initGroupLink(Disk& disk)
{

	int FreeBlockSum = DataBlockSum / 100 + 1;
	SuperBlock.SuperEmptyBlockList.EmptyBlockList[0] = 1;
	SuperBlock.SuperEmptyBlockList.EmptyBlockList[1] = DataBlockStart;
	for (int i = 0; i < FreeBlockSum; i++)
	{

		if (i * 100 + DataBlockStart < DiskBlockSum)
		{
			FreeBlock freeBlock = makeFreeBlock(disk, i * 100 + DataBlockStart);

			::memset(&disk.data[i * 100 + DataBlockStart], 0, sizeof(block));
			::memcpy(&disk.data[i * 100 + DataBlockStart], &freeBlock, sizeof(FreeBlock));
			//cout << "Make linklist " << i * 100 + DataBlockStart << endl;
		}

	}
}

FreeBlock* LoadBlockAsFreeBlock(Disk& disk, int index)
{
	if (index > DataBlockSum + DataBlockStart)
	{
		return NULL;
	}
	FreeBlock* freeBlock = new FreeBlock;

	::memcpy(freeBlock, &disk.data[index], sizeof(FreeBlock));
	return freeBlock;
}

bool SaveFreeBlockToDisk(Disk& disk, int index, FreeBlock& freeBlock)
{
	if (index >= DiskBlockSum)
	{
		return false;
	}

	memset(&disk.data[index], 0, sizeof(block));
	::memcpy(&disk.data[index], &freeBlock, sizeof(FreeBlock));
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

void SaveDataBlockIndexFileToDisk(DataBlockIndexFile& dataBlockIndexFile, Disk& disk, int index)
{
	::memcpy(&disk.data[index], &dataBlockIndexFile, sizeof(DataBlockIndexFile));
}

DataBlockIndexFile LoadDataBlockIndexFileFromDisk(Disk& disk, int index)
{
	DataBlockIndexFile dataBlockIndexFile;
	::memcpy(&dataBlockIndexFile, &disk.data[index], sizeof(block));
	return dataBlockIndexFile;
}


void SaveSuperBlockToDisk(superblock& SuperBlock, Disk& disk)
{
	::memcpy(&disk.data[0], &SuperBlock, sizeof(block));
}
void SaveInodeToDisk(bool& bitmap, inode& inodeList, Disk& disk)
{
	::memcpy(&disk.data[InodeBitmapBlockStart], &bitmap,sizeof(block) * InodeBitMapBlockSum);
	::memcpy( &disk.data[InodeBlockStart], &inodeList, sizeof(inode) * InodeSum);

}

void LoadSuperBlockFromDisk(superblock& SuperBlock, Disk& disk)
{
	::memcpy(&SuperBlock, &disk.data[0], sizeof(superblock));

}

void LoadInodeFromDisk(bool& bitmap, inode& inodeList, Disk& disk)
{
	::memcpy(&bitmap, &disk.data[InodeBitmapBlockStart], sizeof(block)* InodeBitMapBlockSum);
	::memcpy(&inodeList, &disk.data[InodeBlockStart], sizeof(inode) * InodeSum);
}

void SaveDisk()
{
	SaveSuperBlockToDisk(SuperBlock, disk);
	SaveInodeToDisk(*InodeBitmap, *Inode, disk);
	ofstream out;
	out.open("data.dat", ios::out | ios::binary);

	char* buffer = (char*)malloc(sizeof(Disk));
	::memcpy(buffer, &disk, sizeof(Disk));
	out.write(buffer, sizeof(disk));
	out.close();
	free(buffer);
	cout << "save: save disk successfully" << endl;
}


void LoadDisk()
{
	ifstream in;
	in.open("data.dat", ios::in | ios::binary);
	char* buffer = (char*)malloc(sizeof(Disk));
	in.read(buffer, sizeof(disk));
	in.close();
	::memcpy(&disk, buffer, sizeof(Disk));
	free(buffer);
	isLogin = false;

	LoadSuperBlockFromDisk(SuperBlock, disk);
	LoadInodeFromDisk(*InodeBitmap, *Inode, disk);
	cout << "load: load disk successfully" << endl;
}




void SaveFolderToBlock(Disk& disk, int index, Folder folder)
{
	if (index > DiskBlockSum)
	{
		return;
	}
	::memcpy(&disk.data[index], &folder, sizeof(Folder));
}


Folder* loadFolderFromDisk(Disk& disk, int index)
{
	if (index > DiskBlockSum)
	{
		return NULL;
	}
	Folder* folder = new Folder;
	::memcpy(folder, &disk.data[index], sizeof(Folder));
	return folder;

}

//根目录
void InitRootFolder()
{
	
	Inode[SuperBlock.firstInode].DataBlockIndex0[0] = GetOneBlock(disk);
	SuperBlock.firstInode = GetAInode();

	Folder rootF;
	rootF.index[0] = SuperBlock.firstInode;
	rootF.index[1] = SuperBlock.firstInode;

	sprintf_s(rootF.name[0], "..");
	sprintf_s(rootF.name[1], ".");
	rootF.itemSum = 2;
	strcpy_s(Inode[SuperBlock.firstInode].Name, "root");
	strcpy_s(Inode[SuperBlock.firstInode].ExtensionName, "folder");



	strcpy_s(Inode[SuperBlock.firstInode].username, NowUser);
	strcpy_s(Inode[SuperBlock.firstInode].usergroupname, NowGroupName);
	Inode[SuperBlock.firstInode].permissions = 755;
	SaveFolderToBlock(disk, Inode[SuperBlock.firstInode].DataBlockIndex0[0], rootF);

}

void AddItemInFolder(inode* folderInode, char* name, int inodeIndex)
{
	Folder* folder = loadFolderFromDisk(disk, folderInode->DataBlockIndex0[0]);
	strcpy_s(folder->name[folder->itemSum], name);
	folder->index[folder->itemSum] = inodeIndex;
	folder->itemSum++;
	SaveFolderToBlock(disk, folderInode->DataBlockIndex0[0], *folder);
}



void SaveTextBlockToDisk(Disk& disk, int index, TextBlock& textBlock)
{
	if (index > DiskBlockSum)
	{
		return;
	}
	::memcpy(&disk.data[index], &textBlock, sizeof(TextBlock));
}

TextBlock* LoadTextBlockFromDisk(Disk& disk, int index)
{
	if (index > DiskBlockSum)
	{
		return NULL;
	}
	TextBlock* textBlock = new TextBlock;
	::memcpy(textBlock, &disk.data[index], sizeof(TextBlock));
	return textBlock;
}


void NewTxt(inode* FolderInode)
{
	int indexInode = GetAInode();
	int indexBlock;


	if (JudgePermission(FolderInode, 1) == false)
	{
		cout << "new: permission denied!" << endl;
		return;
	}

	//写入自己的inode

	cout << "name:";
	char name[NameLen];
	cin >> name;
	

	//填写inode
	strcpy_s(Inode[indexInode].Name, name);
	strcpy_s(Inode[indexInode].ExtensionName, "txt");
	strcpy_s(Inode[indexInode].username, NowUser);
	strcpy_s(Inode[indexInode].usergroupname, NowGroupName);
	Inode[indexInode].permissions = 666;
	getchar();
	getchar();


	//写入自己的datablock
	char* text = new char[MaxFileSize];
	memset(text, 0, MaxFileSize * sizeof(char));
	
	cout << "data:";
	const int MaxLineSize = MaxFileSize / 100;

	while (1)
	{
		char lineBuffer[MaxLineSize] = { 0 };
		cin.getline(lineBuffer, MaxLineSize);
		if (strcmp(lineBuffer, ":q") == 0)
		{
			break;
		}
		::memcpy(text+strlen(text), lineBuffer, strlen(lineBuffer));
		//添加换行符
		char n = '\n';
		::memcpy(text + strlen(text), &n, 1);
	}

	SaveFileData(disk, &Inode[indexInode], text, strlen(text));

	delete[]text;
	//修改上级目录
	AddItemInFolder(FolderInode, name, indexInode);

}


void ShowText(char* pathName, inode* nowpath)
{
	inode* fileinode = NULL;
	inode* path = nowpath; // 保存原路径不变
	char nowPathName_backup[MAXPATH_LEN];
	// 备份路径
	memcpy(nowPathName_backup, NowPathName, strlen(NowPathName) + 1);
	// 计算目标路径的inode
	inode* targetpath = getInodeByPathName(pathName, path);
	// 若当前inode为文件，则为fileinode赋值
	if (targetpath != NULL && strcmp(targetpath->ExtensionName, "folder") != 0) {
		fileinode = targetpath;
	}
	// 若不是文件，则恢复路径
	else if (targetpath != NULL && strcmp(targetpath->ExtensionName, "folder") == 0) {
		memcpy(NowPathName, nowPathName_backup, strlen(nowPathName_backup) + 1);
		cout << "open: 该路径不是文件，无法打开" << endl;
	}
	else if (targetpath == NULL) {
		cout << "open: " << pathName << ": 没有那个文件或目录" << endl;
		return;
	}


	if (JudgePermission(fileinode, 2) == false)
	{
		cout << "open: permission denied!" << endl;
		return;
	}


	File* openFile = OpenFile(disk, fileinode);
	cout << "filename:" << fileinode->Name << endl;
	cout << "ExtensionName:" << fileinode->ExtensionName << endl;
	cout << "data:" << endl;

	if (strcmp(fileinode->ExtensionName, "txt") == 0||strcmp(fileinode->ExtensionName,"c")==0||strcmp(fileinode->ExtensionName,"tm")==0)
	{
		//输出ascii模式
		

		cout << openFile->data << endl;


	}
	else
	{
		//输出十六进制
		cout << "index\tdata" << endl;
		int linesize = 32;
		for (int i = 0; i < openFile->dataSize; i++)
		{
			if (i % linesize == 0)
			{
				cout << endl;
				cout << i << "\t";
			}
			char buffer[3] = { 0 };
			unsigned char ch = openFile->data[i];
			sprintf_s(buffer, "%02x", ch);
			
			cout << buffer <<" ";
			

		}
		cout << endl;
	}

	
}

File* OpenFile(Disk &disk, inode* fileInode)
{
	File* newFile = new File;
	 
	newFile->dataSize = fileInode->size;
	newFile->fileInode = fileInode;
	newFile->data = new char[newFile->dataSize+1];
	memset(newFile->data, 0, newFile->dataSize + 1);
	//cout << strlen(newFile->data) << " " << newFile->dataSize << endl;
	

	int CharInOneBlockSum = sizeof(block) - sizeof(int);
	int fileDataBlockSum = fileInode->size / CharInOneBlockSum + 1;
	//int n[800];
	//int m[800];
	//cout << fileDataBlockSum << endl;
	//cout << fileInode->DataBlockIndex1 << endl;
	for (int i = 0; i < min(fileDataBlockSum, 10); i++)
	{
		TextBlock* textBlock = LoadTextBlockFromDisk(disk, fileInode->DataBlockIndex0[i]);
		::memcpy(newFile->data+i* CharInOneBlockSum, textBlock->data, CharInOneBlockSum);
		//n[i] = i;
		//m[i] = fileInode->DataBlockIndex0[i];

	}
	if (fileDataBlockSum > 10)
	{
		DataBlockIndexFile dataBlockIndexFile = LoadDataBlockIndexFileFromDisk(disk, fileInode->DataBlockIndex1);
		for (int i = 10; i < min(fileDataBlockSum, 128 + 10); i++)
		{
			TextBlock* textBlock = LoadTextBlockFromDisk(disk, dataBlockIndexFile.index[i - 10]);
			::memcpy(newFile->data + i * CharInOneBlockSum, textBlock->data, CharInOneBlockSum);
			//n[i] = i;
			//m[i] = dataBlockIndexFile.index[i - 10];
		}
	}
	if (fileDataBlockSum > 138)
	{
		DataBlockIndexFile dataBlockIndexFile2 = LoadDataBlockIndexFileFromDisk(disk, fileInode->DataBlockIndex2);
		DataBlockIndexFile dataBlockIndexFile2s;
		for (int i = 0; i < (fileDataBlockSum - 138) / 128 ; i++)
		{
			dataBlockIndexFile2s = LoadDataBlockIndexFileFromDisk(disk, dataBlockIndexFile2.index[i]);
			//cout << "dataBlockIndexFile2.index[i]" << dataBlockIndexFile2.index[i] << endl;
			for (int j = 0; j < 128; j++)
			{
				TextBlock* textBlock = LoadTextBlockFromDisk(disk, dataBlockIndexFile2s.index[j]);
				::memcpy(newFile->data + (138 + i * 128 + j) * CharInOneBlockSum, textBlock->data, CharInOneBlockSum);
				//n[(138 + i * 128 + j)] = 138 + i * 128 + j;
				//m[138 + i * 128 + j] = dataBlockIndexFile2s.index[j];
			}
		}

		int t = (fileDataBlockSum - 138) / 128;
		dataBlockIndexFile2s = LoadDataBlockIndexFileFromDisk(disk, dataBlockIndexFile2.index[t]);
		for (int i = 0; i < (fileDataBlockSum - 138) % 128; i++)
		{
			TextBlock* textBlock = LoadTextBlockFromDisk(disk, dataBlockIndexFile2s.index[i]);
			::memcpy(newFile->data + (138 + t * 128 + i) * CharInOneBlockSum, textBlock->data, CharInOneBlockSum);
			//n[138 + t * 128 + i] = 138 + t * 128 + i;
			//m[138 + t * 128 + i] = dataBlockIndexFile2s.index[i];
		}

	}
	//for (int i = 0; i < fileDataBlockSum; i++)
		//cout << n[i] << ":" << m[i] << endl;
	return newFile;

}

void SaveFileData(Disk &disk,inode* fileInode, char* data, int datasize)
{
	fileInode->size = datasize+1;

	int dataOneBlock = (sizeof(block) - sizeof(int));
	int blockSize = fileInode->size / dataOneBlock + 1;
	int indexBlock;
	//cout << blockSize << endl;

	for (int i = 0; i < min(blockSize, 10); i++)
	{ 
		indexBlock = GetOneBlock(disk);
		TextBlock textBlock;
		::memcpy(textBlock.data, &data[i * dataOneBlock], dataOneBlock);
		textBlock.inodeindex = fileInode->inodeId;
		fileInode->DataBlockIndex0[i] = indexBlock;
		SaveTextBlockToDisk(disk, indexBlock, textBlock);  
		//cout << i << " "<< indexBlock << endl;

	}

	DataBlockIndexFile dataBlockIndexFile;

	if (blockSize > 10)
	{
		for (int i = 10; i < min(blockSize, 128 + 10); i++)
		{
			indexBlock = GetOneBlock(disk);
			TextBlock textBlock;
			::memcpy(textBlock.data, &data[i * dataOneBlock], dataOneBlock);
			textBlock.inodeindex = fileInode->inodeId;
			dataBlockIndexFile.index[i - 10] = indexBlock;
			SaveTextBlockToDisk(disk, indexBlock, textBlock);
			//cout << i << " " << indexBlock << endl;
		}
		int dataBlockIndex1FileIndex = GetOneBlock(disk);
		//
		//cout << "dataBlockIndex1FileIndex" << ":" << dataBlockIndex1FileIndex << endl;
		SaveDataBlockIndexFileToDisk(dataBlockIndexFile, disk, dataBlockIndex1FileIndex);

		fileInode->DataBlockIndex1 = dataBlockIndex1FileIndex;
	}
	DataBlockIndexFile dataBlockIndexFile1;
	DataBlockIndexFile dataBlockIndexFile2;

	if (blockSize > 138)
	{
		if (blockSize > 138 + 128)
		{
			int dataBlockIndex2FileIndex = GetOneBlock(disk);//二级间接块
			for (int i = 0; i < (blockSize - 138) / 128; i++)
			{
				for (int j = 0; j < 128; j++)
				{
					indexBlock = GetOneBlock(disk);
					TextBlock textBlock;
					::memcpy(textBlock.data, &data[(138 + 128 * i + j)* dataOneBlock], dataOneBlock);
					textBlock.inodeindex = fileInode->inodeId;
					dataBlockIndexFile1.index[j] = indexBlock;
					SaveTextBlockToDisk(disk, indexBlock, textBlock);
					//cout << (138 + 128 * i + j) << dataBlockIndexFile1.index[j]<<" " << indexBlock << endl;
				}
				int dataBlockIndexFileIndex = GetOneBlock(disk);//二级间接块指向的目录块
				
				SaveDataBlockIndexFileToDisk(dataBlockIndexFile1, disk, dataBlockIndexFileIndex);
				dataBlockIndexFile2.index[i] = dataBlockIndexFileIndex;
				//cout << "dataBlockIndexFileIndex" << ":" << dataBlockIndexFileIndex << ":"<<dataBlockIndexFile2.index[i] << endl;
			}
			int t = (blockSize - 138) / 128;
			for (int j = 0; j < (blockSize - 138) % 128; j++)
			{
				indexBlock = GetOneBlock(disk);
				TextBlock textBlock;
				::memcpy(textBlock.data, &data[(138 + 128 * t + j)* dataOneBlock], dataOneBlock);
				textBlock.inodeindex = fileInode->inodeId;
				dataBlockIndexFile1.index[j] = indexBlock;
				
				SaveTextBlockToDisk(disk, indexBlock, textBlock);
				//cout << (138 + 128 * t + j) << " " << dataBlockIndexFile1.index[j] << indexBlock << endl;
			}
			int dataBlockIndexFileIndex = GetOneBlock(disk);//二级间接块指向的目录块
			SaveDataBlockIndexFileToDisk(dataBlockIndexFile1, disk, dataBlockIndexFileIndex);
			dataBlockIndexFile2.index[t] = dataBlockIndexFileIndex;
			SaveDataBlockIndexFileToDisk(dataBlockIndexFile2, disk, dataBlockIndex2FileIndex);
			//cout << "dataBlockIndex2FileIndex :" << dataBlockIndex2FileIndex<<":"<< dataBlockIndexFile2.index[t]<<    " "<<dataBlockIndexFileIndex << endl;
			fileInode->DataBlockIndex2 = dataBlockIndex2FileIndex;
		}
		else
		{
			int dataBlockIndex2FileIndex = GetOneBlock(disk);//二级间接块
			for (int j = 0; j < (blockSize - 138) % 128; j++)
			{
				indexBlock = GetOneBlock(disk);
				TextBlock textBlock;
				::memcpy(textBlock.data, &data[138 + j], dataOneBlock);
				textBlock.inodeindex = fileInode->inodeId;
				dataBlockIndexFile1.index[j] = indexBlock;
				SaveTextBlockToDisk(disk, indexBlock, textBlock);
				//cout << 138 + j << " " << indexBlock << endl;
			}
			int dataBlockIndexFileIndex = GetOneBlock(disk);//二级间接块指向的目录块
			SaveDataBlockIndexFileToDisk(dataBlockIndexFile1, disk, dataBlockIndexFileIndex);
			dataBlockIndexFile2.index[0] = dataBlockIndexFileIndex;
			SaveDataBlockIndexFileToDisk(dataBlockIndexFile2, disk, dataBlockIndex2FileIndex);
			fileInode->DataBlockIndex2 = dataBlockIndex2FileIndex;
		}
	}
	
}

bool NewFolder(Disk& disk, inode* FatherFolderInode, char* folderName)
{

	Folder folderBlock;

	inode* targetInode = getInodeByPathName(folderName, FatherFolderInode, 2);
	if (targetInode != NULL) {
		char FileName[NameLen] = { 0 };
		char ExtensionName[NameLen] = { 0 };
		GetFileNameAndExtensionName(folderName, FileName, ExtensionName);
		// 查看是否有同名文件
		inode* haveSameNameFolder = getInodeByPathName(FileName, targetInode);
		if (haveSameNameFolder != NULL) {
			cout << "mkdir: 无法创建\"" << FileName << "\": 有同名文件已存在" << endl;
			return false;
		}
		::memcpy(folderName, FileName, strlen(FileName) + 1);
	}
	else {
		return false;
	}

	//权限判断
	if (JudgePermission(targetInode, 1) == false)
	{
		cout << "mkdir: permission denied!" << endl;
		return false;
	}


	sprintf_s(folderBlock.name[0], "..");
	sprintf_s(folderBlock.name[1], ".");
	folderBlock.itemSum = 2;
	int inodeId = GetAInode();

	folderBlock.index[0] = targetInode->inodeId;
	folderBlock.index[1] = inodeId;

	int blockId = GetOneBlock(disk);

	//填写inode的信息
	strcpy_s(Inode[inodeId].Name, folderName);
	Inode[inodeId].DataBlockIndex0[0] = blockId;
	strcpy_s(Inode[inodeId].ExtensionName, "folder");
	strcpy_s(Inode[inodeId].username, NowUser);
	strcpy_s(Inode[inodeId].usergroupname, NowGroupName);
	Inode[inodeId].permissions = 755;
	SaveFolderToBlock(disk, blockId, folderBlock);

	//修改上级目录
	AddItemInFolder(targetInode, folderName, inodeId);

	return true;

}



//mode 0 缩略版 1 详细版
void LS(inode* FolderInode, int mode)
{
	//权限判断
	if (JudgePermission(FolderInode, 2) == false)
	{
		cout << "ls: permission denied!" << endl;
		return;
	}


	Folder* folder = loadFolderFromDisk(disk, FolderInode->DataBlockIndex0[0]);

	for (int i = 2; i < folder->itemSum; i++)
	{

		if (strcmp(Inode[folder->index[i]].ExtensionName, "folder") == 0)
		{
			SetConsoleTextAttribute(CommandLineHandle, 0x09);
			cout << folder->name[i] << "\t";
			if (mode == 1)
			{
				cout << folder->index[i] << "\t";
			}
			SetConsoleTextAttribute(CommandLineHandle, 0x0f);

		}
		else
		{
			cout << folder->name[i] << "." << Inode[folder->index[i]].ExtensionName << "\t";
			if (mode == 1)
			{
				cout << folder->index[i] << "\t";
			}
		}

	}
	if (folder->itemSum > 2)
	{
		cout << endl;
	}

}


void LS(char* folderPathName, char* mode)
{
	
	inode* nowFolderInode = getInodeByPathName(folderPathName);
	if (nowFolderInode == NULL)
	{
		nowFolderInode = NowPath;
	}

	if (strcpy_s(nowFolderInode->ExtensionName, "folder") == 0)
	{
		if (strcmp(folderPathName, "-i") == 0 || strcmp(mode, "-i") == 0)
		{
			LS(nowFolderInode, 1);
		}
		else
		{
			LS(nowFolderInode, 0);
		}
		
	}
}

void CutPath(char* name)
{
	int count = 0;
	for (int i = strlen(name)-1; i >= 0; i--)
	{
		if (name[i] == '/')
		{
			count++;
		}
		if (count == 2)
		{
			name[i + 1] = '\0';
			break;
		}
	}
}

void CD(char* name, inode** nowpath)
{
	inode** path = nowpath; // 备份nowpath
	inode* targetpath = getInodeByPathName(name, *path); // 获取目标地址的inode

	//权限判断

	// 查看当前inode是否获取成功以及是否为文件夹，若是则更改nowpath
	if (targetpath != NULL && strcmp(targetpath->ExtensionName, "folder") == 0) {
		//权限判断
		if (JudgePermission(targetpath, 2) == false)
		{
			cout << "cd: permission denied!" << endl;
			return;
		}
		*nowpath = targetpath;
		char path[10][20]; // 最多10层路径
		int i = 0, pathNum = 0, k = 0;
		// 切割路径
		while (*(name + i) != '\0') {
			// 若不是路径分割符
			if (*(name + i) != '/') {
				path[pathNum][k] = *(name + i);
				k++;
			}
			// 若为路径分割符则准备填写下一个路径
			if (*(name + i) == '/' && i != 0) {
				path[pathNum][k] = '\0';
				k = 0;
				pathNum++;
			}
			i++;
		}
		// 最后一个字符若不是分割符则路径数量加1
		if (*(name + i - 1) != '/') {
			path[pathNum][k] = '\0';
			pathNum++;
		}
		// 绝对地址
		if (*name == '/') {
			::memcpy(NowPathName, "/", sizeof("/"));
		}
		for (int p = 0; p < pathNum; p++) {
			char n[20] = "/";
			char n1[20] = "..";
			char n2[20] = ".";
			if (strcmp(path[p], n1) == 0)
			{
				CutPath(NowPathName);
			}
			else if (strcmp(path[p], n2) == 0)
			{

			}
			else
			{
				::memcpy(NowPathName + strlen(NowPathName), path[p], sizeof(path[p]));
				if (strcmp(NowPathName, n) != 0 || strlen(NowPathName) != 1)
				{
					::memcpy(NowPathName + strlen(NowPathName), &n, sizeof(n));
				}
			}
		}
	}
	else if (targetpath != NULL && strcmp(targetpath->ExtensionName, "folder") != 0){
		cout << "cd: 该路径为文件路径，无法进入" << endl;
	}
	else if (targetpath == NULL) {
		cout << "cd: " << name << ": 没有那个文件或目录" << endl;
	}

}

// 更改目录结构，删除一个文件
void DeleteItemInFolder(inode* folderInode, inode* fileInode) {
	Folder* folder = loadFolderFromDisk(disk, folderInode->DataBlockIndex0[0]);
	int index = 0;
	for (int i = 0; i < folder->itemSum; i++) {
		if (fileInode->inodeId == folder->index[i]) {
			index = i;
			break;
		}
	}
	// 将当前文件夹要删除的文件后边的所有文件的位置-1
	for (int i = index; i < folder->itemSum - 1; i++) {
		strcpy_s(folder->name[i], folder->name[i + 1]);
		folder->index[i] = folder->index[i + 1];
	}
	// 清除掉最后一个多余的文件，置空
	strcpy_s(folder->name[folder->itemSum - 1], "");
	folder->index[folder->itemSum - 1] = NULL;
	// 文件夹内的文件总数量减一
	folder->itemSum--;
	// 将Folder存回Block
	SaveFolderToBlock(disk, folderInode->DataBlockIndex0[0], *folder);
}

// 删除文件操作
void RM(Disk& disk, inode* folderInode, char* name, bool isSonFolder) {



	bool rmFlag = false; // 默认未删除指定文件
	bool isFolder = false; // 判断是否为文件夹，输出不同的提示信息
	Folder* folder = loadFolderFromDisk(disk, folderInode->DataBlockIndex0[0]);
	inode* path;
	if (!isSonFolder) {
		path = getInodeByPathName(name, folderInode);
		if (path == NULL) {
			cout << "rm: 无法删除\"" << name << "\": 没有那个文件或目录" << endl;
			return;
		}
	}
	else {
		path = folderInode;
	}

	if (JudgePermission(path, 1) == false)
	{
		cout << "rm: permission denied!" << endl;
		return;
	}

	bool haveSuchAFile = false; // 判断是否有要删除的文件
	// 若是子文件夹，则默认删除所有文件，设置有此文件
	if (isSonFolder) { haveSuchAFile = true; }
	// 若不是子文件夹并且找到了要删除的文件，
	if (!isSonFolder) {
		// 判断是否为文件夹
		if (strcmp(path->ExtensionName, "folder") == 0) {
			isFolder = true;
		}
		if (isFolder) {
			cout << "rm: 是否删除 目录 \"" << name << "\"?(y/n) ";
		}
		else {
			cout << "rm: 是否删除 一般文件 \"" << name << "\"?(y/n) ";
		}
		// 确认是否删除该文件
		char answer;
		cin >> answer;
		// 确认删除，设置有此文件
		if (answer == 'y') {
			haveSuchAFile = true;
		}
		else if (answer == 'n') {
			return;
		}
		getchar();
	}
	// 若有此文件，执行删除
	if (haveSuchAFile) {
		//int inodeID = folder->index[i];
		// 计算当前的block数量
		int blockNum = path->size / (sizeof(block) - sizeof(int)) + 1;
		// 若当前文件为文件夹，删除其中的所有文件
		if (strcmp(path->ExtensionName, "folder") == 0) {
			//inode* sonFolderInode = &Inode[inodeID];
			Folder* sonFolder = loadFolderFromDisk(disk, path->DataBlockIndex0[0]);
			char sonFolderName[20] = ""; // 在删除函数中占位
			// 删除子文件夹中的所有文件
			for (int i = 2; i < sonFolder->itemSum; i++) {
				inode* sonFile = &Inode[sonFolder->index[i]];
				RM(disk, sonFile, sonFolderName, true);
			}

		}
		// 释放当前文件的inode，位图1变0
		if (FreeAInode(path->inodeId)) {
			// 释放当前文件的block区的内容
			// 若blockNum数量<=10，则说明是在10个直接块中存储的数据
			for (int j = 0; j < min(blockNum, 10); j++) {
				int blockID = path->DataBlockIndex0[j];
				::memset(&disk.data[blockID], 0, sizeof(block));
				FreeABlock(disk, blockID);
			}
			// 若blockNum数量>10，则说明使用了一级间址
			if (blockNum > 10) {
				DataBlockIndexFile dataBlockIndexFile = LoadDataBlockIndexFileFromDisk(disk, folderInode->DataBlockIndex1);
				for (int j = 10; j < min(blockNum, 128 + 10); j++)
				{
					int blockID = dataBlockIndexFile.index[j - 10];
					::memset(&disk.data[blockID], 0, sizeof(block));
					FreeABlock(disk, blockID);
				}
				int blockID = path->DataBlockIndex1;
				::memset(&disk.data[blockID], 0, sizeof(block));
				FreeABlock(disk, blockID);
			}
			// 二级间址
			if (blockNum > 138) {
				if (blockNum > 138 + 128) {
					DataBlockIndexFile dataBlockIndexFile1 = LoadDataBlockIndexFileFromDisk(disk, folderInode->DataBlockIndex2);
					for (int i = 0; i < (blockNum - 138) / 128; i++) {
						DataBlockIndexFile dataBlockIndexFile2 = LoadDataBlockIndexFileFromDisk(disk, dataBlockIndexFile1.index[i]);
						for (int j = 0; j < 128; j++) {
							int blockID = dataBlockIndexFile2.index[j];
							::memset(&disk.data[blockID], 0, sizeof(block));
							FreeABlock(disk, blockID);
						}
						int blockID = dataBlockIndexFile1.index[i];
						::memset(&disk.data[blockID], 0, sizeof(block));
						FreeABlock(disk, blockID);
					}
					int t = (blockNum - 138) / 128;
					DataBlockIndexFile dataBlockIndexFile2 = LoadDataBlockIndexFileFromDisk(disk, dataBlockIndexFile1.index[t]);
					for (int j = 0; j < (blockNum - 138) % 128; j++) {
						int blockID = dataBlockIndexFile2.index[j];
						::memset(&disk.data[blockID], 0, sizeof(block));
						FreeABlock(disk, blockID);
					}
					int blockID = dataBlockIndexFile1.index[t];
					::memset(&disk.data[blockID], 0, sizeof(block));
					FreeABlock(disk, blockID);
				}
				else {
					DataBlockIndexFile dataBlockIndexFile = LoadDataBlockIndexFileFromDisk(disk, folderInode->DataBlockIndex2);
					for (int j = 0; j < (blockNum - 138) % 128; j++) {
						int blockID = dataBlockIndexFile.index[j];
						::memset(&disk.data[blockID], 0, sizeof(block));
						FreeABlock(disk, blockID);
					}
				}
				int blockID = path->DataBlockIndex2;
				::memset(&disk.data[blockID], 0, sizeof(block));
				FreeABlock(disk, blockID);
			}
			// 若不是子文件夹，更改目录结构，传递当前目录Inode，folder，删除的文件名称，删除的文件在目录中的序号
			// 如果是子文件夹，因为最后全都删了，就不用改了
			if (!isSonFolder) {
				inode* lastFolderPath = getInodeByPathName(name, folderInode, 2);
				DeleteItemInFolder(lastFolderPath, path);
				rmFlag = true;
			}
		}
	}

	// 不是子文件的情况下未找到文件，输出信息
	if (!isSonFolder && !rmFlag) {
		cout << "rm: 无法删除\"" << name << "\": 没有那个文件或目录" << endl;
	}
}


void SetTitle(const char* Title)
{
	wchar_t title[MAXPATH_LEN];
	size_t len = strlen(Title);
	mbstowcs_s(&len, title,Title, (size_t)MAXPATH_LEN);
	SetConsoleTitle(title);
}

// 通过路径获取Inode
inode* getInodeByPathName(const char* folderPathName, inode* nowPath, int mode) {
	/*
		函数有几种返回结果
		1. 路径中的任意一环不存在，则返回NULL
		2. 路径格式不符合规范，返回NULL
		3. 路径存在，则返回其inode
		   (1)默认mode=1，返回路径的最后一个节点的inode
		   (2)mode=2时，不会判断最后一个节点是否存在，提前结束循环，返回路径的倒数第二个节点的inode
		使用函数建议：使用 if(targetpath != NULL) 判断是否获取了inode
					使用 if(strcmp(targetpath->ExtensionName, "folder") == 0) 判断当前inode是否为文件夹格式
	*/
	inode* targetPath;

	char path[10][20]; // 最多10层路径
	int i = 0, pathNum = 0, k = 0;
	// 切割路径
	while (*(folderPathName + i) != '\0') {
		// 若不是路径分割符
		if (*(folderPathName + i) != '/') {
			path[pathNum][k] = *(folderPathName + i);
			k++;
		}
		// 若为路径分割符则准备填写下一个路径
		if (*(folderPathName + i) == '/' && i != 0) {
			path[pathNum][k] = '\0';
			k = 0;
			pathNum++;
		}
		i++;
	}
	// 最后一个字符若不是分割符则路径数量加1
	if (*(folderPathName + i - 1) != '/') {
		path[pathNum][k] = '\0';
		pathNum++;
	}
	char allName[MAXPATH_LEN];
	memcpy(allName, folderPathName, strlen(folderPathName) + 1);
	char FileName[NameLen] = { 0 };
	char ExtensionName[NameLen] = { 0 };
	GetFileNameAndExtensionName(allName, FileName, ExtensionName);
	strcpy_s(path[pathNum - 1], FileName);

	// 绝对地址
	if (*folderPathName == '/') {
		targetPath = RootPath;
	}
	// 相对地址
	else {
		targetPath = nowPath;
	}

	if (pathNum == 1 && mode == 2) {
		return nowPath;
	}

	for (int p = 0; p < pathNum; p++) {
		Folder* folder = loadFolderFromDisk(disk, targetPath->DataBlockIndex0[0]);
		bool haveSuchAPath = false;
		for (int q = 0; q < folder->itemSum; q++) {
			if (strcmp(path[p], folder->name[q]) == 0) {
				if (p == pathNum - 1 && strcmp(Inode[folder->index[q]].ExtensionName, ExtensionName) != 0) {
					continue;
				}
				targetPath = &Inode[folder->index[q]];
				haveSuchAPath = true;
				break; // 找到路径退出当前文件夹的遍历
			}

		}
		if (!haveSuchAPath) {
			return NULL;
		}
		// 若在最后一个路径之前出现普通文件，输出错误信息
		if (p != pathNum - 1 && strcmp(targetPath->ExtensionName, "folder") != 0) {
			cout << "路径不符合规范，\"" << path[p] << "\" 是普通文件格式" << endl;
			return NULL;
		}
		// 如果mode=2，返回倒数第二个节点
		if (mode == 2 && p == pathNum - 2) {
			break;
		}
	}
	
	return targetPath;
}

bool Chmod(char* pathname, int permission,inode*nowpath)
{
	
	inode* Inode = getInodeByPathName(pathname, nowpath, 1);
	if (Inode != NULL)
	{

		if (JudgePermission(Inode, 1) == false)
		{
			cout << "rm: permission denied!" << endl;
			return false;
		}

		return Chmod(Inode, permission);
		
	}
	return false;

}

bool Chmod(inode* Inode,int permission)
{
	if (Inode != NULL)
	{
		Inode->permissions = permission;
		return true;
	}
	return false;
		

}


int complier(char* filename,inode* NowPath,Disk&disk)
{
	inode* FileInode = getInodeByPathName(filename,NowPath,1);
	inode*folderInode= getInodeByPathName(filename, NowPath,2);

	if (FileInode != NULL)
	{
		if (JudgePermission(folderInode, 1) == false)
		{
			cout << "complier: permission denied!" << endl;
			return -1;
		}

		File* file = OpenFile(disk, FileInode);
		char name[NameLen + 1 + NameLen];
		sprintf_s(name, "%s.%s", FileInode->Name, FileInode->ExtensionName);

		char *Code=complier(file->data,FileInode->size, name);
		
		File CodeFile;
		CodeFile.data = Code;
		CodeFile.dataSize = strlen(Code);
		inode *CodeInode = &Inode[GetAInode()];
		CodeFile.fileInode = CodeInode;
		strcpy_s(CodeInode->Name, "temp");
		strcpy_s(CodeInode->ExtensionName, "tm");
		SaveFileData(disk, CodeInode, Code, CodeFile.dataSize);
		
		AddItemInFolder(folderInode, CodeInode->Name, CodeInode->inodeId);
	}
	return 0;
}

int tm_f(char* filename, inode* NowPath,Disk& disk) {
	inode* FileInode = getInodeByPathName(filename, NowPath, 1);
	inode* folderInode = getInodeByPathName(filename, NowPath, 2);
	if (FileInode != NULL)
	{
		if (JudgePermission(folderInode, 2) == false)
		{
			cout << "complier: permission denied!" << endl;
			return -1;
		}
		File* file = OpenFile(disk, FileInode);
		tm_c(file->data, FileInode->size);
	}
	return 0;
}



void CutArr(char* Arr, char* Arr1, char* Arr2, char* Arr3)
{
	int indexL[3] = { 0,0,0 };
	int indexR[3] = { -1,-1,-1 };
	int count = 0;
	bool spaceFlag = false;
	for (int i = 0; i < strlen(Arr); i++)
	{
		//遇到第一个空格的时候
		if (Arr[i] == ' '&&spaceFlag==false)
		{
			indexR[count] = i - 1;
			count++;
			spaceFlag = true;
		}
		//遇到第一个不是空格的时候
		if (Arr[i] != ' ' && spaceFlag == true)
		{
			indexL[count] = i;
			spaceFlag = false;
		}
		if (count == 3)
		{
			break;
		}
	}
	if (indexR[count] == -1)
	{
		indexR[count] = strlen(Arr) - 1;
	}
	
	if (indexR[0] != -1)
	{
		::memset(Arr1, 0, MAXPATH_LEN);
		memcpy(Arr1, Arr + indexL[0], (indexR[0] - indexL[0] + 1) * sizeof(char));
	}
	if (indexR[1] != -1)
	{
		::memset(Arr2, 0, MAXPATH_LEN);
		memcpy(Arr2, Arr + indexL[1], (indexR[1] - indexL[1] + 1) * sizeof(char));
	}
	if (indexR[2] != -1)
	{
		::memset(Arr3, 0, MAXPATH_LEN);
		memcpy(Arr3, Arr + indexL[2], (indexR[2] - indexL[2] + 1) * sizeof(char));
	}
}


//向windows导出文件
bool Export(char* pathname, inode* FileInde)
{
	if (strlen(pathname) == 0)
	{
		char fullname[NameLen + 1 + NameLen] = { 0 };
		sprintf_s(fullname, "%s.%s", FileInde->Name, FileInde->ExtensionName);
		strcpy_s(pathname,sizeof(fullname) ,fullname);
	}
	ofstream out;
	out.open(pathname, ios::out | ios::binary);
	File *saveFile = OpenFile(disk, FileInde);
	out.write(saveFile->data, sizeof(char) * saveFile->dataSize);
	out.close();
	return true;
}

//向windows导出文件
//Export 本硬盘内的文件 windows内的文件
bool Export(char* filepathname, char* pathnameInWindows)
{
	inode* FileInode = getInodeByPathName(filepathname);
	return Export(pathnameInWindows, FileInode);
}

/*
 获取字符串的文件名和扩展名
 使用样例
		char name[MAXPATH_LEN*2+1];
		char FileName[MAXPATH_LEN];
		char ExtensionName[MAXPATH_LEN];
		cin >> name;

		GetFileNameAndExtensionName(name, FileName, ExtensionName);
		cout << FileName << " " << ExtensionName << endl;

*/
void GetFileNameAndExtensionName(char* AllName, char* FileName, char* ExtensionName)
{
	::memset(FileName, 0, strlen(FileName));
	::memset(ExtensionName, 0, strlen(ExtensionName));
	if (strcmp(AllName, "..") == 0| strcmp(AllName, ".") == 0)
	{
		memcpy(FileName, AllName, strlen(AllName));
		memcpy(ExtensionName, "folder", sizeof("folder"));
		return;
	}

	int pointPos = -1;
	int LastFolderPos = -1;
	for (int i = 0; i < strlen(AllName); i++)
	{
		if (AllName[i] == '.')
		{
			pointPos = i;
		}
		if (AllName[i] == '/'|| AllName[i]==92)
		{
			LastFolderPos = i;
		}
	}

	//如果找到扩展名
	if (pointPos != -1)
	{
		memcpy(FileName, AllName + LastFolderPos + 1, sizeof(char) * (pointPos - LastFolderPos-1));
		memcpy(ExtensionName, &AllName[pointPos + 1], sizeof(char) * (strlen(AllName) - pointPos));
	}
	else
	{
		memcpy(FileName, AllName + LastFolderPos + 1, sizeof(char) * (strlen(AllName) - LastFolderPos));
		memcpy(ExtensionName, "folder", sizeof("folder"));
	}


	
}

bool Import(char* pathnameInWindows, inode* folderInode)
{
	if (JudgePermission(folderInode, 1) == false)
	{
		cout << "import: permission denied!" << endl;
		return false;
	}

	//读取全部数据到新的File
	FILE* In =new  FILE;
	fopen_s(&In, pathnameInWindows, "rb");
	if (In == NULL)
	{
		return NULL;
	}
	fseek(In, 0, SEEK_END);
	int length = ftell(In);
	char* InData = (char*)malloc((length + 1) * sizeof(char));
	rewind(In);
	length = fread(InData, 1, length, In);
	InData[length] = '\0';
	fclose(In);



	//新建inode和数据块
	File newFile;

	
	int fileInodeIndex = GetAInode();

	char FileNameInWindows[MAXPATH_LEN] = { 0 };
	char ExtensionNameInWindows[MAXPATH_LEN] = { 0 };
	GetFileNameAndExtensionName(pathnameInWindows, FileNameInWindows, ExtensionNameInWindows);
	memcpy(Inode[fileInodeIndex].Name, FileNameInWindows, strlen(FileNameInWindows));
	memcpy(Inode[fileInodeIndex].ExtensionName, ExtensionNameInWindows, strlen(ExtensionNameInWindows));
	Inode[fileInodeIndex].permissions = 666;
	
	SaveFileData(disk, &Inode[fileInodeIndex], InData, length);

	AddItemInFolder(folderInode, FileNameInWindows, fileInodeIndex);

	return true;
}

bool Rename(char* filenameandpath, char* name)
{
	inode* FileInode = getInodeByPathName(filenameandpath, NowPath, 1);
	if (FileInode == NULL)
	{
		return false;
	}

	//权限判断
	if (JudgePermission(FileInode, 1) == false)
	{
		cout << "rename: permission denied!" << endl;
		return false;
	}


	char FileName[NameLen] = { 0 };
	char ExtensionName[NameLen] = { 0 };
	GetFileNameAndExtensionName(name, FileName, ExtensionName);
	memcpy(FileInode->Name, FileName, sizeof(FileName));
	memcpy(FileInode->ExtensionName, ExtensionName, sizeof(ExtensionName));
	return true;
}

void MV(inode* NowPath, char* fileName, char* targetName) {
	inode* filePath = getInodeByPathName(fileName, NowPath);
	inode* fileLastPath = getInodeByPathName(fileName, NowPath, 2);
	inode* targetPath = getInodeByPathName(targetName, NowPath);
	//inode* targetLastPath = getInodeByPathName(targetName, NowPath, 2);
	


	if (filePath == NULL || targetPath == NULL) {
		cout << "mv: 无法将\"" << fileName << "\" 移动至\"" << targetName << "\": 没有那个文件或目录" << endl;
		return;
	}
	if (strcmp(targetPath->ExtensionName, "folder") != 0) {
		cout << "mv: \"" << targetName << "\" 应该为目录" << endl;
		return;
	}

	//权限判断
	if (JudgePermission(filePath, 1) == false)
	{
		cout << "mv: permission denied!" << endl;
		return;
	}
	//权限判断
	if (JudgePermission(targetPath, 1) == false)
	{
		cout << "mv: permission denied!" << endl;
		return;
	}
	// 移动
	char FileName[NameLen] = { 0 };
	char ExtensionName[NameLen] = { 0 };
	GetFileNameAndExtensionName(fileName, FileName, ExtensionName);
	// 查看是否有同名文件
	inode* haveThatFolder = getInodeByPathName(FileName, targetPath);
	if (haveThatFolder != NULL) {
		cout << "mv: 无法将\"" << fileName << "\" 移动至\"" << targetName << "\": 有同名文件已存在" << endl;
		return;
	}
	//修改上级目录
	AddItemInFolder(targetPath, FileName, filePath->inodeId);
	DeleteItemInFolder(fileLastPath, filePath);

}

void CP(inode* NowPath, char* fileName, char* targetName) {
	/*cout << "fileName:" << fileName << endl;
	cout << "targetName: " << targetName << endl;*/
	inode* fileInode = getInodeByPathName(fileName, NowPath);
	inode* fileLastInode = getInodeByPathName(fileName, NowPath, 2);
	inode* targetInode = getInodeByPathName(targetName, NowPath);
	//inode* targetLastInode = getInodeByPathName(targetName, NowPath, 2);

	if (fileInode == NULL || targetInode == NULL) {
		cout << "cp: 无法将\"" << fileName << "\" 复制至\"" << targetName << "\": 没有那个文件或目录" << endl;
		return;
	}
	if (strcmp(targetInode->ExtensionName, "folder") != 0) {
		cout << "cp: \"" << targetName << "\" 应该为目录" << endl;
		return;
	}

	//权限判断
	if (JudgePermission(fileInode, 0) == false)
	{
		cout << "cp: permission denied!" << endl;
		return;
	}
	//权限判断
	if (JudgePermission(targetInode, 1) == false)
	{
		cout << "Rename: permission denied!" << endl;
		return;
	}

	// 移动
	char FileName[NameLen] = { 0 };
	char ExtensionName[NameLen] = { 0 };
	GetFileNameAndExtensionName(fileName, FileName, ExtensionName);
	// 查看是否有同名文件
	inode* haveSameNameFolder = getInodeByPathName(fileName, targetInode);
	if (haveSameNameFolder != NULL) {
		cout << "cp: 无法复制\"" << FileName << "." << ExtensionName << "\": 有同名文件已存在" << endl;
		return;
	}
	// 原路径为文件夹
	if (strcmp(fileInode->ExtensionName, "folder") == 0) {

		NewFolder(disk, targetInode, FileName);

		Folder* nowfolder = loadFolderFromDisk(disk, fileInode->DataBlockIndex0[0]);
		char newTargetName[MAXPATH_LEN];
		strcpy_s(newTargetName, targetName);
		strcat_s(newTargetName, "/");
		strcat_s(newTargetName, FileName);
		for (int i = 2; i < nowfolder->itemSum; i++) {
			char nowFileName[20];
			strcpy_s(nowFileName, fileName);
			strcat_s(nowFileName, "/");
			strcat_s(nowFileName, nowfolder->name[i]);
			if (strcmp(Inode[nowfolder->index[i]].ExtensionName, "folder") != 0)
			{
				strcat_s(nowFileName, ".");
				strcat_s(nowFileName, Inode[nowfolder->index[i]].ExtensionName);
			}
			CP(NowPath, nowFileName, newTargetName);
		}
	}
	// 原路径为文件
	else {
		File* newFile = OpenFile(disk, fileInode);
		int indexInode = GetAInode();
		memcpy(&Inode[indexInode], fileInode, sizeof(inode));
		Inode[indexInode].inodeId = indexInode;
		SaveFileData(disk, &Inode[indexInode], newFile->data, newFile->dataSize);
		AddItemInFolder(targetInode, FileName, indexInode);
	}
}

void Format()
{
	if (strcmp(NowUser, "root") != 0)
	{
		cout << "format: permission denied!\ntry: su root " << endl;

		return;
	}
	//格式化磁盘文件
	::memset(&disk, 0, sizeof(disk));
	//格式化超级块
	::memset(&SuperBlock, 0, sizeof(SuperBlock));
	//格式化Inode节点
	::memset(Inode, 0, sizeof(Inode));
	//格式化位图
	::memset(InodeBitmap, 0, sizeof(InodeBitmap));
	lastInodePos = 0;
	NowPath = &Inode[SuperBlock.firstInode];
	RootPath = &Inode[SuperBlock.firstInode];
	
	isLogin = false;
	memcpy(NowPathName, "/", 2);
	memcpy(NowUser, "root", 5);
	memcpy(NowGroupName, "root", 5);
	memcpy(DeviceName, "Disk0", 6);

	initInode();//初始化inode
	initGroupLink(disk);//初始化磁盘块区，按成组链法组织
	InitRootFolder();//初始化目录
	initUserBlock(disk);

	return;

}


bool Login(char* name, char* password,Disk&disk)
{
	User user = LoadUserFromDisk(disk);
	for (int i = 0; i < user.userSum; i++)
	{
		if (strcmp(user.name[i], name) == 0)
		{
			if (strcmp(user.password[i], password) == 0)
			{
				isLogin = true;
				strcpy_s(NowUser, name);

				return true;
			}
		}
	}

	return false;
}

void initUserBlock(Disk &disk)
{
	char name[] = "root";
	useradd(name, disk);
}

bool Logout()
{
	isLogin = false;
	return true;
}

bool useradd(char* name,Disk&disk)
{
	
	//名字不能为空
	if (strlen(name) == 0)
	{
		return false;
	}

	//不能重名
	User user = LoadUserFromDisk(disk);
	if (user.userSum >= 8)
	{
		cout << "useradd:用户已满，无法继续注册" << endl;
		return false;
	}
	bool isNameUsed = false;
	for (int i = 0; i < user.userSum; i++)
	{
		if (strcmp(name, user.name[i]) == 0)
		{
			isNameUsed = true;
			break;
		}
	}
	if (isNameUsed)
	{
		return false;
	}
	strcpy_s(user.name[user.userSum], name);
	strcpy_s(user.GroupName[user.userSum], name);
	::memset(user.password[user.userSum], 0, sizeof(char) * PassWordLen);
	user.userSum++;
	SaveUserToDisk(disk, user);
	if (NewFolder(disk, RootPath, name) == false)
	{
		inode *folderInode = getInodeByPathName(name, RootPath, 1);
		strcpy_s(folderInode->username, user.name[user.userSum - 1]);
		strcpy_s(folderInode->usergroupname, user.GroupName[user.userSum - 1]);
	}
	return true;


}

User LoadUserFromDisk(Disk& disk)
{
	User user;
	memcpy(&user, &disk.data[UserBlockStart], sizeof(User));
	return user;
}
void SaveUserToDisk(Disk& disk, User user)
{
	memcpy(&disk.data[UserBlockStart], &user, sizeof(User));
}

void UserManager(Disk &disk)
{
	while (1)
	{
		if (isLogin == true)
		{
			break;
		}
		SetConsoleTextAttribute(CommandLineHandle, FOREGROUND_GREEN);
		cout << "login as: ";
		SetConsoleTextAttribute(CommandLineHandle, 0x07);
		char name[NameLen];
		cin.getline(name,NameLen);
		SetConsoleTextAttribute(CommandLineHandle, FOREGROUND_GREEN);
		cout << name << "@" << DeviceName << "'s password:";
		SetConsoleTextAttribute(CommandLineHandle, 0x07);
		char* password = GetPasswd();
		cout << endl;
		if (Login(name, password, disk) == false)
		{
			SetConsoleTextAttribute(CommandLineHandle, FOREGROUND_RED);
			cout << "Login: login failed!"<<endl;
			SetConsoleTextAttribute(CommandLineHandle, 0x07);
		}
	}
}

bool useradd()
{
	char name[NameLen] = { 0 };
	cin.getline(name,NameLen);

	useradd(name,disk);
	return true;
}

void passwd(char* username,Disk&disk)
{
	User user = LoadUserFromDisk(disk);
	char findname[NameLen] = { 0 };
	if (strlen(username) == 0)
	{
		strcpy_s(findname, NowUser);
	}
	else
	{
		strcpy_s(findname, username);
	}
	for (int i = 0; i < user.userSum; i++)
	{
		if (strcmp(user.name[i], findname) == 0)
		{
			SetConsoleTextAttribute(CommandLineHandle, 0x0c);
			//cout << ((strcmp(NowPathName, "/") == 0) ? "" : NowPathName);
			cout << "Changing password for " << findname << endl;
			SetConsoleTextAttribute(CommandLineHandle, 0x07);
			char passowrdOld[PassWordLen] = { 0 };
			cout << "(current) password:";
			strcpy_s(passowrdOld, GetPasswd());
			cout << endl;
			char passwordNew[PassWordLen] = { 0 };
			cout << "Enter new password:";
			strcpy_s(passwordNew, GetPasswd());
			cout << endl;
			char passwordNew1[PassWordLen] = { 0 };
			cout << "Retype new password:";
			strcpy_s(passwordNew1, GetPasswd());
			cout << endl;
			if (strcmp(passowrdOld, user.password[i]) != 0)
			{
				cout << "passwd: old password wrong! password update failed" << endl;
				return;
			}
			if (strcmp(passwordNew, passwordNew1) == 0)
			{
				cout << "passwd: password updated successfully" << endl;
				strcpy_s(user.password[i], passwordNew);
			}
			else
			{
				cout << "passwd: new password not the same! password update failed " << endl;
			}

		}
	}
	SaveUserToDisk(disk, user);
}

char* GetPasswd()
{
	char* Password = (char*)malloc(sizeof(char) * PassWordLen);
	::memset(Password, 0, sizeof(char) * PassWordLen);
	while (1)
	{
		char ch = _getch();
		if (ch == '\n'||ch=='\r')
		{
			break;
		}
		else if (ch == '\b') //退格
		{
			if (strlen(Password) > 0)
			{
				Password[strlen(Password) - 1] = '\0';
			}
			
		}
		else if (strlen(Password) < PassWordLen)
		{
			memcpy(Password + strlen(Password), &ch, sizeof(char));
		}

		else
		{
			break;
		}
	}
	return Password;
}


//判断是否可执行操作 可以执行返回true
// file待判断的文件
// mode 模式 2 读 1 写 0 执行
// 读 写 执行
// r=4  w=2   x=1  
bool JudgePermission(inode* file, int mode)
{
	if (strcmp(NowUser, "root") == 0)
	{
		return true;
	}
	int nowPermision = 0;
	if (strcmp(NowUser, file->username) == 0)
	{
		nowPermision = file->permissions / 100;
	}
	else if (strcmp(NowGroupName, file->usergroupname) == 0)
	{
		nowPermision = file->permissions / 10;
		nowPermision = nowPermision % 10;
	}
	else
	{
		nowPermision = file->permissions % 10;
	}
	char wantPermision = pow(2, mode);
	char cPermision = nowPermision;

	if ((cPermision &wantPermision) != 0)
	{
		return true;
	}
	else
	{
		return false;
	}

}

//切换用户
void su(char* username)
{
	if (strcmp(NowUser, "root") == 0)
	{
		strcpy_s(NowUser, username);
		User user = LoadUserFromDisk(disk);
		for (int i = 0; i < user.userSum; i++)
		{
			if (strcmp(username, user.name[i]) == 0)
			{
				strcpy_s(NowGroupName, user.GroupName[i]);
				
			}
		}
		
	}
	else
	{
		cout << "password:";
		char Password[PassWordLen] = { 0 };
		strcpy_s(Password, GetPasswd());
		User user = LoadUserFromDisk(disk);
		for (int i = 0; i < user.userSum; i++)
		{
			if (strcmp(username, user.name[i]) == 0)
			{
				if (strcmp(Password, user.password[i]) == 0)
				{
					strcpy_s(NowUser, username);
					strcpy_s(NowGroupName, user.GroupName[i]);

				}
				else
				{
					SetConsoleTextAttribute(CommandLineHandle, FOREGROUND_RED);
					cout << "su: su failed!" << endl;
					SetConsoleTextAttribute(CommandLineHandle, 0x07);
				}
			}
		}
		cout << endl;
	}
}


void WrongCommand(char *Arr1)
{
	if (strlen(Arr1) != 0)
	{
		cout << "Command '" << Arr1 << "' not found" << endl;
		cout << "Try:help " << endl;
	}
	
}

bool SeemsLike(const char* name1, char* name2)
{

	int len1 = strlen(name1);
	int len2 = strlen(name2);
	int CurPos = 0;
	int RightNum = 0;

	for (int i = 0; i < len1; i++)
		for (int j = CurPos; j < len2; j++)
		{

			if (name2[j] == name1[i]) {
				CurPos = j + 1;
				RightNum++;
				break;
			}

			else continue;
		}

	if (RightNum == len1)
		return true;
	else return false;

}

void Find(const char* fileName, inode* Path)
{
	//cout << "find!" << endl;
	Folder* CurFolder;//当前文件夹
	File* CurFile;//当前文件
	inode* CurInode;//当前inode
	char CurName[20];//当前文件/文件夹名
	char CurExtentionName[NameLen];//当前扩展名
	memcpy(CurExtentionName, Path->ExtensionName, sizeof(Path->ExtensionName));
	char TxtName[4] = "txt";

	int CurIndex = Path->DataBlockIndex0[0];//当前索引
	//cout << CurIndex;
	static char CurPath[20][20];//当前路径，CurPath[x][y],x是第x层
	static int PathIndex = 0;

	if (strcmp(CurExtentionName, TxtName) != 0)//如果当前处理的不是txt文件
	{
		CurFolder = loadFolderFromDisk(disk, CurIndex); //Folder* loadFolderFromDisk(Disk& disk, int index)
		for (int i = 2; i < CurFolder->itemSum; i++)//遍历当前文件夹下的子文件名
		{
			memcpy(CurName, CurFolder->name[i], sizeof(CurFolder->name[i]));//取CurFolder下的第i个文件的文件名

			memcpy(CurPath[PathIndex], CurName, sizeof(CurName));//
			PathIndex++;

			CurIndex = CurFolder->index[i];//第0项上级目录，第1项它本身
			//cout << "run" << endl;
			CurInode = &Inode[CurIndex];

			if (SeemsLike(fileName, CurName))
				//if(strcmp(fileName,CurName)==0)
			{
				for (int j = 0; j < PathIndex; j++) {
					if (j < PathIndex - 1)
						cout << CurPath[j] << "/";
					else cout << CurPath[j] << "." << CurInode->ExtensionName;

				}
				cout << endl;

			}

			Find(fileName, CurInode);//递归
			memset(CurPath, 0, sizeof(CurPath));
			PathIndex = 0;

		}
	}
}

void help() {
	cout << "**************************************************" << endl;
	cout << "--操作系统课程设计：模拟实现UNIX的文件系统" << endl;
	cout << "--组长：余斐然 20174439 物联网1701班" << endl;
	cout << "--组员：王理庚 20174601 物联网1701班" << endl;
	cout << "--      刘  源 20174602 物联网1701班" << endl;
	cout << "--      李  勐 20174717 物联网1701班" << endl;
	cout << "--2020/6/19" << endl;
	cout << "**************************************************" << endl;
	cout << "--功能：" << endl;
	cout << "--------------------------------------------------" << endl;
	cout << "--添加用户：useradd [用户名]" << endl;
	cout << "--修改用户密码：passwd [用户名]" << endl;
	cout << "--切换用户：su [用户名]" << endl;
	cout << "--------------------------------------------------" << endl;
	cout << "--格式化：format" << endl;
	cout << "--加载硬盘：load" << endl;
	cout << "--保存硬盘：save" << endl;
	cout << "--------------------------------------------------" << endl;
	cout << "--新建目录：mkdir [目录]" << endl;
	cout << "--打开目录：cd [目录]" << endl;
	cout << "--列出目录文件：ls [目录]" << endl;
	cout << "--------------------------------------------------" << endl;
	cout << "--新建文件：new" << endl;
	cout << "--从windows导入文件：import [文件/空]" << endl;
	cout << "--向windows导出文件：export [文件] [windows路径]" << endl;
	cout << "--显示文件内容：open [文件]" << endl;
	cout << "--------------------------------------------------" << endl;
	cout << "--修改文件或目录名称：rename [文件/目录] [新名字]" << endl;
	cout << "--复制文件或目录：cp [文件/目录] [目录]" << endl;
	cout << "--移动文件或目录：mv [文件/目录] [目录]" << endl;
	cout << "--删除文件或目录：rm [文件/目录]" << endl;
	cout << "--更改文件或目录权限：chmod [文件/目录] [权限]" << endl;
	cout << "--查找文件或目录（支持模糊查找）：find [文件/目录]" << endl;
	cout << "--------------------------------------------------" << endl;
	cout << "--文本编辑器：vim [文件]" << endl;
	cout << "--编译文件：compiler [文件]" << endl;
	cout << "--运行虚拟机：tm [文件]" << endl;
	cout << "--显示图片功能：showpic [文件]" << endl;
	cout << "--------------------------------------------------" << endl;
	cout << "--退出程序：exit" << endl;
	cout << "--------------------------------------------------" << endl;
}