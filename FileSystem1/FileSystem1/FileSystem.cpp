
#include"FileSystem.h"

superblock SuperBlock;
Disk disk;
inode Inode[InodeSum];
bool InodeBitmap[InodeSum] = { 0 };
inode* NowPath = &Inode[SuperBlock.firstInode];
inode* RootPath = &Inode[SuperBlock.firstInode];
char NowPathName[MAXPATH_LEN] = "/";
char NowUser[MAXUSERNAME_LEN] = "root";
char DeviceName[MAXDEVICENAME_LEN] = "Disk0";

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

	char a = getchar();
	if (a == '\n')
	{
		char back = '\n';
		cin.putback(back);
	}
}

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
		FreeBlock freeBlock = makeFreeBlock(disk, i * 100 + DataBlockStart);

		memset(&disk.data[i * 100 + DataBlockStart], 0, sizeof(block));
		::memcpy(&disk.data[i * 100 + DataBlockStart], &freeBlock, sizeof(FreeBlock));
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
	::memcpy(&disk.data[1], &bitmap, sizeof(block));
	::memcpy(&disk.data[2], &inodeList, sizeof(inode) * InodeSum);

}

void LoadSuperBlockFromDisk(superblock& SuperBlock, Disk& disk)
{
	::memcpy(&SuperBlock, &disk.data[0], sizeof(superblock));

}

void LoadInodeFromDisk(bool& bitmap, inode& inodeList, Disk& disk)
{
	::memcpy(&bitmap, &disk.data[1], sizeof(block));
	::memcpy(&inodeList, &disk.data[2], sizeof(inode) * InodeSum);
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

	LoadSuperBlockFromDisk(SuperBlock, disk);
	LoadInodeFromDisk(*InodeBitmap, *Inode, disk);
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
	strcpy_s(Inode[SuperBlock.firstInode].Name, "root");
	Inode[SuperBlock.firstInode].DataBlockIndex0[0] = GetOneBlock(disk);
	SuperBlock.firstInode = GetAInode();

	Folder rootF;
	rootF.index[0] = SuperBlock.firstInode;
	rootF.index[1] = SuperBlock.firstInode;

	sprintf_s(rootF.name[0], "..");
	sprintf_s(rootF.name[1], ".");
	rootF.itemSum = 2;
	strcpy_s(Inode[SuperBlock.firstInode].ExtensionName, "folder");
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


	//写入自己的inode

	cout << "name:";
	char name[NameLen];
	cin >> name;
	


	strcpy_s(Inode[indexInode].Name, name);
	getchar();
	getchar();
	const int maxsize = 1024*1024;

	//写入自己的datablock
	char* text = NULL;
	text = (char*)malloc(512 * sizeof(char));
	cout << "data:";

	while (1)
	{
		char lineBuffer[99999] = { 0 };
		cin.getline(lineBuffer, 99999);
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

	
	//修改上级目录
	AddItemInFolder(FolderInode, name, indexInode);

}


void ShowText(char *pathName, inode* nowpath)
{
	inode* fileinode=NULL;
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
	else if (targetpath != NULL && strcmp(targetpath->ExtensionName, "folder") == 0 ){
		memcpy(NowPathName, nowPathName_backup, strlen(nowPathName_backup) + 1);
		cout << "该路径不是文件，无法打开" << endl;
	}


	if (fileinode == NULL)
	{
		return;
	}
	File* openFile = OpenFile(disk,fileinode);
	
	cout << "filename:" << fileinode->Name << endl;
	cout << "data:" << endl;
	cout << openFile->data;

	
}

File* OpenFile(Disk &disk, inode* fileInode)
{
	File* newFile = new File;

	newFile->dataSize = fileInode->size;
	newFile->fileInode = fileInode;
	newFile->data = new char[newFile->dataSize];

	int CharInOneBlockSum = sizeof(block) - sizeof(int);
	int fileDataBlockSum = fileInode->size / CharInOneBlockSum + 1;

	for (int i = 0; i < min(fileDataBlockSum, 10); i++)
	{
		TextBlock* textBlock = LoadTextBlockFromDisk(disk, fileInode->DataBlockIndex0[i]);
		::memcpy(newFile->data+i* CharInOneBlockSum, textBlock->data, CharInOneBlockSum);

	}
	if (fileDataBlockSum > 10)
	{
		DataBlockIndexFile dataBlockIndexFile = LoadDataBlockIndexFileFromDisk(disk, fileInode->DataBlockIndex1);
		for (int i = 10; i < min(fileDataBlockSum, 128 + 10); i++)
		{
			TextBlock* textBlock = LoadTextBlockFromDisk(disk, dataBlockIndexFile.index[i - 10]);
			::memcpy(newFile->data + i * CharInOneBlockSum, textBlock->data, CharInOneBlockSum);
			
		}
	}
	return newFile;

}

void SaveFileData(Disk &disk,inode* fileInode, char* data, int datasize)
{
	fileInode->size = strlen(data);

	int dataOneBlock = (sizeof(block) - sizeof(int));
	int blockSize = fileInode->size / dataOneBlock + 1;
	int indexBlock;


	for (int i = 0; i < min(blockSize, 10); i++)
	{
		indexBlock = GetOneBlock(disk);
		TextBlock textBlock;
		::memcpy(textBlock.data, &data[i * dataOneBlock], dataOneBlock);
		textBlock.inodeindex = fileInode->inodeId;
		fileInode->DataBlockIndex0[i] = indexBlock;
		SaveTextBlockToDisk(disk, indexBlock, textBlock);

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
		}
		int dataBlockIndex1FileIndex = GetOneBlock(disk);
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
					::memcpy(textBlock.data, &data[138+128*i+j], dataOneBlock);
					textBlock.inodeindex = fileInode->inodeId;
					dataBlockIndexFile1.index[j] = indexBlock;
					SaveTextBlockToDisk(disk, indexBlock, textBlock);
				}
				int dataBlockIndexFileIndex = GetOneBlock(disk);//二级间接块指向的目录块
				SaveDataBlockIndexFileToDisk(dataBlockIndexFile1, disk, dataBlockIndexFileIndex);
				dataBlockIndexFile2.index[i] = dataBlockIndexFileIndex;
			} 
			int t = (blockSize - 138) / 128;
			for (int j = 0; j < (blockSize - 138) % 128; j++)
			{
				indexBlock = GetOneBlock(disk);
				TextBlock textBlock;
				::memcpy(textBlock.data, &data[138 + 128 * t + j], dataOneBlock);
				textBlock.inodeindex = fileInode->inodeId;
				dataBlockIndexFile1.index[j] = indexBlock;
				SaveTextBlockToDisk(disk, indexBlock, textBlock);
			}
			int dataBlockIndexFileIndex = GetOneBlock(disk);//二级间接块指向的目录块
			SaveDataBlockIndexFileToDisk(dataBlockIndexFile1, disk, dataBlockIndexFileIndex);
			dataBlockIndexFile2.index[t] = dataBlockIndexFileIndex;
			SaveDataBlockIndexFileToDisk(dataBlockIndexFile2, disk, dataBlockIndex2FileIndex);
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
			}
			int dataBlockIndexFileIndex = GetOneBlock(disk);//二级间接块指向的目录块
			SaveDataBlockIndexFileToDisk(dataBlockIndexFile1, disk, dataBlockIndexFileIndex);
			dataBlockIndexFile2.index[0] = dataBlockIndexFileIndex;
			SaveDataBlockIndexFileToDisk(dataBlockIndexFile2, disk, dataBlockIndex2FileIndex);
		}
	}
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
	Inode[inodeId].DataBlockIndex0[0] = blockId;
	strcpy_s(Inode[inodeId].ExtensionName, "folder");
	SaveFolderToBlock(disk, blockId, folderBlock);

	//修改上级目录
	AddItemInFolder(FatherFolderInode, folderName, inodeId);

}

void LS(inode* FolderInode)
{
	Folder* folder = loadFolderFromDisk(disk, FolderInode->DataBlockIndex0[0]);

	for (int i = 2; i < folder->itemSum; i++)
	{
		
		if (strcmp(Inode[folder->index[i]].ExtensionName, "folder") == 0)
		{
			SetConsoleTextAttribute(CommandLineHandle, 0x09);
			cout << folder->name[i] << "\t";
			SetConsoleTextAttribute(CommandLineHandle, 0x0f);
		}
		else
		{
			cout << folder->name[i] << "\t";
		}

	}
	if (folder->itemSum > 2)
	{
		cout << endl;
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

	// 查看当前inode是否获取成功以及是否为文件夹，若是则更改nowpath
	if (targetpath != NULL && strcmp(targetpath->ExtensionName, "folder") == 0) {
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
				memcpy(NowPathName + strlen(NowPathName), path[p], sizeof(path[p]));
				if (strcmp(NowPathName, n) != 0 || strlen(NowPathName) != 1)
				{
					memcpy(NowPathName + strlen(NowPathName), &n, sizeof(n));
				}
			}
		}
	}
	else if (targetpath != NULL && strcmp(targetpath->ExtensionName, "folder") != 0){
		cout << "该路径为文件路径，无法进入" << endl;
	}

}

// 更改目录结构，删除一个文件
void DeleteItemInFolder(inode* folderInode, Folder* folder, char* name, int index) {
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

	// 遍历当前目录
	for (int i = 2; i < folder->itemSum; i++) {
		bool haveSuchAFile = false; // 判断是否有要删除的文件
		// 若是子文件夹，则默认删除所有文件，设置有此文件
		if (isSonFolder) { haveSuchAFile = true; }
		// 若不是子文件夹并且找到了要删除的文件，
		if (!isSonFolder && strcmp(folder->name[i], name) == 0) { 
			// 判断是否为文件夹
			if (strcmp(Inode[folder->index[i]].ExtensionName, "folder") == 0) {
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
		}
		// 若有此文件，执行删除
		if (haveSuchAFile) {
			int inodeID = folder->index[i];
			// 计算当前的block数量
			int blockNum = Inode[inodeID].size / (sizeof(block) - sizeof(int)) + 1;
			// 若当前文件为文件夹，删除其中的所有文件
			if (strcmp(Inode[inodeID].ExtensionName, "folder") == 0) {
				inode* sonFolderInode = &Inode[inodeID];
				char sonFolderName[20] = ""; // 在删除函数中占位
				// 删除子文件夹中的所有文件
				RM(disk, sonFolderInode, sonFolderName, true);
			}
			// 释放当前文件的inode，位图1变0
			if (FreeAInode(inodeID)) {
				// 释放当前文件的block区的内容
				// 若blockNum数量<=10，则说明是在10个直接块中存储的数据
				for (int j = 0; j < min(blockNum, 10); j++) {
					int blockID = Inode[inodeID].DataBlockIndex0[j];
					memset(&disk.data[blockID], 0, sizeof(block));
					FreeABlock(disk, blockID);
				}
				// 若blockNum数量>10，则说明使用了一级间址
				if (blockNum > 10) {
					DataBlockIndexFile dataBlockIndexFile = LoadDataBlockIndexFileFromDisk(disk, folderInode->DataBlockIndex1);
					for (int j = 10; j < min(blockNum, 128 + 10); j++)
					{
						int blockID = dataBlockIndexFile.index[j - 10];
						memset(&disk.data[blockID], 0, sizeof(block));
						FreeABlock(disk, blockID);
					}
				}
				// 若不是子文件夹，更改目录结构，传递当前目录Inode，folder，删除的文件名称，删除的文件在目录中的序号
				// 如果是子文件夹，因为最后全都删了，就不用改了
				if (!isSonFolder) {
					DeleteItemInFolder(folderInode, folder, name, i);
					rmFlag = true;
					break;
				}
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
inode* getInodeByPathName(const char* folderPathName, inode* nowPath) {
	/*
		函数有几种返回结果
		1. 路径中的任意一环不存在，则返回NULL
		2. 路径存在，最后一个路径是文件夹，则返回其inode
	*/
	inode* targetPath;
	char nowPathName[MAXPATH_LEN];
	// 备份当前路径
	memcpy(nowPathName, NowPathName, strlen(NowPathName) + 1);

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

	// 绝对地址
	if (*folderPathName == '/') {
		//cout << "绝对地址" << endl;
		memcpy(NowPathName, "/", sizeof("/"));
		targetPath = RootPath;
	}
	// 相对地址
	else {
		//cout << "相对地址" << endl;
		targetPath = nowPath;
	}

	for (int p = 0; p < pathNum; p++) {
		Folder* folder = loadFolderFromDisk(disk, targetPath->DataBlockIndex0[0]);
		bool haveSuchAPath = false;
		for (int q = 0; q < folder->itemSum; q++) {
			if (strcmp(path[p], folder->name[q]) == 0) {
				targetPath = &Inode[folder->index[q]];
				haveSuchAPath = true;
				break; // 找到路径退出当前文件夹的遍历
			}

		}
		if (!haveSuchAPath) {
			cout << "没有'" << path[p] << "'那个路径" << endl;
			return NULL;
		}
	}
	
	return targetPath;
}

bool Chmod(char* pathname, int permission,inode*nowpath)
{
	inode* Inode = getInodeByPathName(pathname, nowpath);
	if (Inode != NULL)
	{
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