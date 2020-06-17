
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

//��Ŀ¼
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


	//д���Լ���inode

	cout << "name:";
	char name[NameLen];
	cin >> name;
	

	//��дinode
	strcpy_s(Inode[indexInode].Name, name);
	strcpy_s(Inode[indexInode].ExtensionName, "txt");
	strcpy_s(Inode[indexInode].username, NowUser);
	strcpy_s(Inode[indexInode].usergroupname, NowGroupName);
	Inode[indexInode].permissions = 666;
	getchar();
	getchar();
	const int maxsize = 99999;

	//д���Լ���datablock
	char text[maxsize] = { 0 };
	
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
		//��ӻ��з�
		char n = '\n';
		::memcpy(text + strlen(text), &n, 1);
	}

	SaveFileData(disk, &Inode[indexInode], text, strlen(text));

	
	//�޸��ϼ�Ŀ¼
	AddItemInFolder(FolderInode, name, indexInode);

}


void ShowText(char* pathName, inode* nowpath)
{
	inode* fileinode = NULL;
	inode* path = nowpath; // ����ԭ·������
	char nowPathName_backup[MAXPATH_LEN];
	// ����·��
	memcpy(nowPathName_backup, NowPathName, strlen(NowPathName) + 1);
	// ����Ŀ��·����inode
	inode* targetpath = getInodeByPathName(pathName, path);
	// ����ǰinodeΪ�ļ�����Ϊfileinode��ֵ
	if (targetpath != NULL && strcmp(targetpath->ExtensionName, "folder") != 0) {
		fileinode = targetpath;
	}
	// �������ļ�����ָ�·��
	else if (targetpath != NULL && strcmp(targetpath->ExtensionName, "folder") == 0) {
		memcpy(NowPathName, nowPathName_backup, strlen(nowPathName_backup) + 1);
		cout << "open: ��·�������ļ����޷���" << endl;
	}
	else if (targetpath == NULL) {
		cout << "open: " << pathName << ": û���Ǹ��ļ���Ŀ¼" << endl;
		return;
	}


	File* openFile = OpenFile(disk, fileinode);
	
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
	if (fileDataBlockSum > 138)
	{
		DataBlockIndexFile dataBlockIndexFile2 = LoadDataBlockIndexFileFromDisk(disk, fileInode->DataBlockIndex2);
		DataBlockIndexFile dataBlockIndexFile2s;
		for (int i = 0; i < (fileDataBlockSum - 138) / 128 - 1; i++)
		{
			dataBlockIndexFile2s = LoadDataBlockIndexFileFromDisk(disk, dataBlockIndexFile2.index[i]);
			for (int j = 0; j < 128; j++)
			{
				TextBlock* textBlock = LoadTextBlockFromDisk(disk, dataBlockIndexFile2s.index[j]);
				::memcpy(newFile->data + (138 + i * 128 + j) * CharInOneBlockSum, textBlock->data, CharInOneBlockSum);
			}
		}

		int t = (fileDataBlockSum - 138) / 128;
		dataBlockIndexFile2s = LoadDataBlockIndexFileFromDisk(disk, dataBlockIndexFile2.index[t]);
		for (int i = 0; i < (fileDataBlockSum - 138) % 128; i++)
		{
			TextBlock* textBlock = LoadTextBlockFromDisk(disk, dataBlockIndexFile2s.index[i]);
			::memcpy(newFile->data + (138 + t * 128 + i) * CharInOneBlockSum, textBlock->data, CharInOneBlockSum);
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
			int dataBlockIndex2FileIndex = GetOneBlock(disk);//������ӿ�
			for (int i = 0; i < (blockSize - 138) / 128; i++)
			{
				for (int j = 0; j < 128; j++)
				{
					indexBlock = GetOneBlock(disk);
					TextBlock textBlock;
					::memcpy(textBlock.data, &data[138 + 128 * i + j], dataOneBlock);
					textBlock.inodeindex = fileInode->inodeId;
					dataBlockIndexFile1.index[j] = indexBlock;
					SaveTextBlockToDisk(disk, indexBlock, textBlock);
				}
				int dataBlockIndexFileIndex = GetOneBlock(disk);//������ӿ�ָ���Ŀ¼��
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
			int dataBlockIndexFileIndex = GetOneBlock(disk);//������ӿ�ָ���Ŀ¼��
			SaveDataBlockIndexFileToDisk(dataBlockIndexFile1, disk, dataBlockIndexFileIndex);
			dataBlockIndexFile2.index[t] = dataBlockIndexFileIndex;
			SaveDataBlockIndexFileToDisk(dataBlockIndexFile2, disk, dataBlockIndex2FileIndex);
		}
		else
		{
			int dataBlockIndex2FileIndex = GetOneBlock(disk);//������ӿ�
			for (int j = 0; j < (blockSize - 138) % 128; j++)
			{
				indexBlock = GetOneBlock(disk);
				TextBlock textBlock;
				::memcpy(textBlock.data, &data[138 + j], dataOneBlock);
				textBlock.inodeindex = fileInode->inodeId;
				dataBlockIndexFile1.index[j] = indexBlock;
				SaveTextBlockToDisk(disk, indexBlock, textBlock);
			}
			int dataBlockIndexFileIndex = GetOneBlock(disk);//������ӿ�ָ���Ŀ¼��
			SaveDataBlockIndexFileToDisk(dataBlockIndexFile1, disk, dataBlockIndexFileIndex);
			dataBlockIndexFile2.index[0] = dataBlockIndexFileIndex;
			SaveDataBlockIndexFileToDisk(dataBlockIndexFile2, disk, dataBlockIndex2FileIndex);
		}
	}
	
}

void NewFolder(Disk& disk, inode* FatherFolderInode, char* folderName)
{

	Folder folderBlock;

	inode* targetInode = getInodeByPathName(folderName, FatherFolderInode, 2);
	if (targetInode != NULL) {
		char FileName[NameLen] = { 0 };
		char ExtensionName[NameLen] = { 0 };
		GetFileNameAndExtensionName(folderName, FileName, ExtensionName);
		// �鿴�Ƿ���ͬ���ļ�
		inode* haveSameNameFolder = getInodeByPathName(FileName, targetInode);
		if (haveSameNameFolder != NULL) {
			cout << "mkdir: �޷�����\"" << FileName << "\": ��ͬ���ļ��Ѵ���" << endl;
			return;
		}
		::memcpy(folderName, FileName, strlen(FileName) + 1);
	}
	else {
		return;
	}

	sprintf_s(folderBlock.name[0], "..");
	sprintf_s(folderBlock.name[1], ".");
	folderBlock.itemSum = 2;
	int inodeId = GetAInode();

	folderBlock.index[0] = targetInode->inodeId;
	folderBlock.index[1] = inodeId;

	int blockId = GetOneBlock(disk);

	//��дinode����Ϣ
	strcpy_s(Inode[inodeId].Name, folderName);
	Inode[inodeId].DataBlockIndex0[0] = blockId;
	strcpy_s(Inode[inodeId].ExtensionName, "folder");
	strcpy_s(Inode[inodeId].username, NowUser);
	strcpy_s(Inode[inodeId].usergroupname, NowGroupName);
	Inode[inodeId].permissions = 777;
	SaveFolderToBlock(disk, blockId, folderBlock);

	//�޸��ϼ�Ŀ¼
	AddItemInFolder(targetInode, folderName, inodeId);

}

void LS(char* folderPathName)
{
	inode* nowFolderInode = getInodeByPathName(folderPathName);
	if (strcpy_s(nowFolderInode->ExtensionName, "folder") == 0)
	{
		LS(nowFolderInode);
	}
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
			cout << folder->name[i]<<"."<<Inode[folder->index[i]].ExtensionName << "\t";
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
	inode** path = nowpath; // ����nowpath
	inode* targetpath = getInodeByPathName(name, *path); // ��ȡĿ���ַ��inode

	// �鿴��ǰinode�Ƿ��ȡ�ɹ��Լ��Ƿ�Ϊ�ļ��У����������nowpath
	if (targetpath != NULL && strcmp(targetpath->ExtensionName, "folder") == 0) {
		*nowpath = targetpath;
		char path[10][20]; // ���10��·��
		int i = 0, pathNum = 0, k = 0;
		// �и�·��
		while (*(name + i) != '\0') {
			// ������·���ָ��
			if (*(name + i) != '/') {
				path[pathNum][k] = *(name + i);
				k++;
			}
			// ��Ϊ·���ָ����׼����д��һ��·��
			if (*(name + i) == '/' && i != 0) {
				path[pathNum][k] = '\0';
				k = 0;
				pathNum++;
			}
			i++;
		}
		// ���һ���ַ������Ƿָ����·��������1
		if (*(name + i - 1) != '/') {
			path[pathNum][k] = '\0';
			pathNum++;
		}
		// ���Ե�ַ
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
		cout << "cd: ��·��Ϊ�ļ�·�����޷�����" << endl;
	}
	else if (targetpath == NULL) {
		cout << "cd: " << name << ": û���Ǹ��ļ���Ŀ¼" << endl;
	}

}

// ����Ŀ¼�ṹ��ɾ��һ���ļ�
void DeleteItemInFolder(inode* folderInode, inode* fileInode) {
	Folder* folder = loadFolderFromDisk(disk, folderInode->DataBlockIndex0[0]);
	int index = 0;
	for (int i = 0; i < folder->itemSum; i++) {
		if (fileInode->inodeId == folder->index[i]) {
			index = i;
			break;
		}
	}
	// ����ǰ�ļ���Ҫɾ�����ļ���ߵ������ļ���λ��-1
	for (int i = index; i < folder->itemSum - 1; i++) {
		strcpy_s(folder->name[i], folder->name[i + 1]);
		folder->index[i] = folder->index[i + 1];
	}
	// ��������һ��������ļ����ÿ�
	strcpy_s(folder->name[folder->itemSum - 1], "");
	folder->index[folder->itemSum - 1] = NULL;
	// �ļ����ڵ��ļ���������һ
	folder->itemSum--;
	// ��Folder���Block
	SaveFolderToBlock(disk, folderInode->DataBlockIndex0[0], *folder);
}

// ɾ���ļ�����
void RM(Disk& disk, inode* folderInode, char* name, bool isSonFolder) {
	bool rmFlag = false; // Ĭ��δɾ��ָ���ļ�
	bool isFolder = false; // �ж��Ƿ�Ϊ�ļ��У������ͬ����ʾ��Ϣ
	Folder* folder = loadFolderFromDisk(disk, folderInode->DataBlockIndex0[0]);
	inode* path;
	if (!isSonFolder) {
		path = getInodeByPathName(name, folderInode);
		if (path == NULL) {
			cout << "rm: �޷�ɾ��\"" << name << "\": û���Ǹ��ļ���Ŀ¼" << endl;
			return;
		}
	}
	else {
		path = folderInode;
	}

	bool haveSuchAFile = false; // �ж��Ƿ���Ҫɾ�����ļ�
	// �������ļ��У���Ĭ��ɾ�������ļ��������д��ļ�
	if (isSonFolder) { haveSuchAFile = true; }
	// ���������ļ��в����ҵ���Ҫɾ�����ļ���
	if (!isSonFolder) {
		// �ж��Ƿ�Ϊ�ļ���
		if (strcmp(path->ExtensionName, "folder") == 0) {
			isFolder = true;
		}
		if (isFolder) {
			cout << "rm: �Ƿ�ɾ�� Ŀ¼ \"" << name << "\"?(y/n) ";
		}
		else {
			cout << "rm: �Ƿ�ɾ�� һ���ļ� \"" << name << "\"?(y/n) ";
		}
		// ȷ���Ƿ�ɾ�����ļ�
		char answer;
		cin >> answer;
		// ȷ��ɾ���������д��ļ�
		if (answer == 'y') {
			haveSuchAFile = true;
		}
		else if (answer == 'n') {
			return;
		}
		getchar();
	}
	// ���д��ļ���ִ��ɾ��
	if (haveSuchAFile) {
		//int inodeID = folder->index[i];
		// ���㵱ǰ��block����
		int blockNum = path->size / (sizeof(block) - sizeof(int)) + 1;
		// ����ǰ�ļ�Ϊ�ļ��У�ɾ�����е������ļ�
		if (strcmp(path->ExtensionName, "folder") == 0) {
			//inode* sonFolderInode = &Inode[inodeID];
			Folder* sonFolder = loadFolderFromDisk(disk, path->DataBlockIndex0[0]);
			char sonFolderName[20] = ""; // ��ɾ��������ռλ
			// ɾ�����ļ����е������ļ�
			for (int i = 2; i < sonFolder->itemSum; i++) {
				inode* sonFile = &Inode[sonFolder->index[i]];
				RM(disk, sonFile, sonFolderName, true);
			}

		}
		// �ͷŵ�ǰ�ļ���inode��λͼ1��0
		if (FreeAInode(path->inodeId)) {
			// �ͷŵ�ǰ�ļ���block��������
			// ��blockNum����<=10����˵������10��ֱ�ӿ��д洢������
			for (int j = 0; j < min(blockNum, 10); j++) {
				int blockID = path->DataBlockIndex0[j];
				memset(&disk.data[blockID], 0, sizeof(block));
				FreeABlock(disk, blockID);
			}
			// ��blockNum����>10����˵��ʹ����һ����ַ
			if (blockNum > 10) {
				DataBlockIndexFile dataBlockIndexFile = LoadDataBlockIndexFileFromDisk(disk, folderInode->DataBlockIndex1);
				for (int j = 10; j < min(blockNum, 128 + 10); j++)
				{
					int blockID = dataBlockIndexFile.index[j - 10];
					memset(&disk.data[blockID], 0, sizeof(block));
					FreeABlock(disk, blockID);
				}
			}
			// ���������ļ��У�����Ŀ¼�ṹ�����ݵ�ǰĿ¼Inode��folder��ɾ�����ļ����ƣ�ɾ�����ļ���Ŀ¼�е����
			// ��������ļ��У���Ϊ���ȫ��ɾ�ˣ��Ͳ��ø���
			if (!isSonFolder) {
				inode* lastFolderPath = getInodeByPathName(name, folderInode, 2);
				DeleteItemInFolder(lastFolderPath, path);
				rmFlag = true;
			}
		}
	}

	// �������ļ��������δ�ҵ��ļ��������Ϣ
	if (!isSonFolder && !rmFlag) {
		cout << "rm: �޷�ɾ��\"" << name << "\": û���Ǹ��ļ���Ŀ¼" << endl;
	}
}


void SetTitle(const char* Title)
{
	wchar_t title[MAXPATH_LEN];
	size_t len = strlen(Title);
	mbstowcs_s(&len, title,Title, (size_t)MAXPATH_LEN);
	SetConsoleTitle(title);
}

// ͨ��·����ȡInode
inode* getInodeByPathName(const char* folderPathName, inode* nowPath, int mode) {
	/*
		�����м��ַ��ؽ��
		1. ·���е�����һ�������ڣ��򷵻�NULL
		2. ·����ʽ�����Ϲ淶������NULL
		3. ·�����ڣ��򷵻���inode
		   (1)Ĭ��mode=1������·�������һ���ڵ��inode
		   (2)mode=2ʱ�������ж����һ���ڵ��Ƿ���ڣ���ǰ����ѭ��������·���ĵ����ڶ����ڵ��inode
		ʹ�ú������飺ʹ�� if(targetpath != NULL) �ж��Ƿ��ȡ��inode
					ʹ�� if(strcmp(targetpath->ExtensionName, "folder") == 0) �жϵ�ǰinode�Ƿ�Ϊ�ļ��и�ʽ
	*/
	inode* targetPath;

	char path[10][20]; // ���10��·��
	int i = 0, pathNum = 0, k = 0;
	// �и�·��
	while (*(folderPathName + i) != '\0') {
		// ������·���ָ��
		if (*(folderPathName + i) != '/') {
			path[pathNum][k] = *(folderPathName + i);
			k++;
		}
		// ��Ϊ·���ָ����׼����д��һ��·��
		if (*(folderPathName + i) == '/' && i != 0) {
			path[pathNum][k] = '\0';
			k = 0;
			pathNum++;
		}
		i++;
	}
	// ���һ���ַ������Ƿָ����·��������1
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

	// ���Ե�ַ
	if (*folderPathName == '/') {
		targetPath = RootPath;
	}
	// ��Ե�ַ
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
				break; // �ҵ�·���˳���ǰ�ļ��еı���
			}

		}
		if (!haveSuchAPath) {
			return NULL;
		}
		// �������һ��·��֮ǰ������ͨ�ļ������������Ϣ
		if (p != pathNum - 1 && strcmp(targetPath->ExtensionName, "folder") != 0) {
			cout << "·�������Ϲ淶��\"" << path[p] << "\" ����ͨ�ļ���ʽ" << endl;
			return NULL;
		}
		// ���mode=2�����ص����ڶ����ڵ�
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


void CutArr(char* Arr, char* Arr1, char* Arr2, char* Arr3)
{
	int indexL[3] = { 0,0,0 };
	int indexR[3] = { -1,-1,-1 };
	int count = 0;
	bool spaceFlag = false;
	for (int i = 0; i < strlen(Arr); i++)
	{
		//������һ���ո��ʱ��
		if (Arr[i] == ' '&&spaceFlag==false)
		{
			indexR[count] = i - 1;
			count++;
			spaceFlag = true;
		}
		//������һ�����ǿո��ʱ��
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
		memset(Arr1, 0, MAXPATH_LEN);
		memcpy(Arr1, Arr + indexL[0], (indexR[0] - indexL[0] + 1) * sizeof(char));
	}
	if (indexR[1] != -1)
	{
		memset(Arr2, 0, MAXPATH_LEN);
		memcpy(Arr2, Arr + indexL[1], (indexR[1] - indexL[1] + 1) * sizeof(char));
	}
	if (indexR[2] != -1)
	{
		memset(Arr3, 0, MAXPATH_LEN);
		memcpy(Arr3, Arr + indexL[2], (indexR[2] - indexL[2] + 1) * sizeof(char));
	}

}


//��windows�����ļ�
bool Export(char* pathname, inode* FileInde)
{
	ofstream out;
	out.open(pathname, ios::out | ios::binary);
	File *saveFile = OpenFile(disk, FileInde);
	out.write(saveFile->data, sizeof(char) * saveFile->dataSize);
	out.close();
	return true;
}

//��windows�����ļ�
//Export ��Ӳ���ڵ��ļ� windows�ڵ��ļ�
bool Export(char* filepathname, char* pathnameInWindows)
{
	inode* FileInode = getInodeByPathName(filepathname);
	return Export(pathnameInWindows, FileInode);
}

/*
 ��ȡ�ַ������ļ�������չ��
 ʹ������
		char name[MAXPATH_LEN*2+1];
		char FileName[MAXPATH_LEN];
		char ExtensionName[MAXPATH_LEN];
		cin >> name;

		GetFileNameAndExtensionName(name, FileName, ExtensionName);
		cout << FileName << " " << ExtensionName << endl;

*/
void GetFileNameAndExtensionName(char* AllName, char* FileName, char* ExtensionName)
{
	memset(FileName, 0, strlen(FileName));
	memset(ExtensionName, 0, strlen(ExtensionName));
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

	//����ҵ���չ��
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
	//��ȡȫ�����ݵ��µ�File
	FILE* In =new  FILE;
	fopen_s(&In, pathnameInWindows, "r");
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



	//�½�inode�����ݿ�
	File newFile;

	
	int fileInodeIndex = GetAInode();

	char FileNameInWindows[MAXPATH_LEN] = { 0 };
	char ExtensionNameInWindows[MAXPATH_LEN] = { 0 };
	GetFileNameAndExtensionName(pathnameInWindows, FileNameInWindows, ExtensionNameInWindows);
	memcpy(Inode[fileInodeIndex].Name, FileNameInWindows, strlen(FileNameInWindows));
	memcpy(Inode[fileInodeIndex].ExtensionName, ExtensionNameInWindows, strlen(ExtensionNameInWindows));
	
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
		cout << "mv: �޷���\"" << fileName << "\" �ƶ���\"" << targetName << "\": û���Ǹ��ļ���Ŀ¼" << endl;
		return;
	}
	if (strcmp(targetPath->ExtensionName, "folder") != 0) {
		cout << "mv: \"" << targetName << "\" Ӧ��ΪĿ¼" << endl;
		return;
	}
	// �ƶ�
	char FileName[NameLen] = { 0 };
	char ExtensionName[NameLen] = { 0 };
	GetFileNameAndExtensionName(fileName, FileName, ExtensionName);
	// �鿴�Ƿ���ͬ���ļ�
	inode* haveThatFolder = getInodeByPathName(FileName, targetPath);
	if (haveThatFolder != NULL) {
		cout << "mv: �޷���\"" << fileName << "\" �ƶ���\"" << targetName << "\": ��ͬ���ļ��Ѵ���" << endl;
		return;
	}
	//�޸��ϼ�Ŀ¼
	AddItemInFolder(targetPath, FileName, filePath->inodeId);
	DeleteItemInFolder(fileLastPath, filePath);

}

void CP(inode* NowPath, char* fileName, char* targetName) {
	inode* fileInode = getInodeByPathName(fileName, NowPath);
	inode* fileLastInode = getInodeByPathName(fileName, NowPath, 2);
	inode* targetInode = getInodeByPathName(targetName, NowPath);
	//inode* targetLastInode = getInodeByPathName(targetName, NowPath, 2);

	if (fileInode == NULL || targetInode == NULL) {
		cout << "cp: �޷���\"" << fileName << "\" �ƶ���\"" << targetName << "\": û���Ǹ��ļ���Ŀ¼" << endl;
		return;
	}
	if (strcmp(targetInode->ExtensionName, "folder") != 0) {
		cout << "cp: \"" << targetName << "\" Ӧ��ΪĿ¼" << endl;
		return;
	}
	// �ƶ�
	char FileName[NameLen] = { 0 };
	char ExtensionName[NameLen] = { 0 };
	GetFileNameAndExtensionName(fileName, FileName, ExtensionName);
	// ԭ·��Ϊ�ļ���
	if (strcmp(fileInode->ExtensionName, "folder") == 0) {

	}
	// ԭ·��Ϊ�ļ�
	else {
		File* newFile = OpenFile(disk, fileInode);
		int indexInode = GetAInode();
		memcpy(&Inode[indexInode], fileInode, sizeof(inode));
		SaveFileData(disk, &Inode[indexInode], newFile->data, newFile->dataSize);
		AddItemInFolder(targetInode, FileName, indexInode);
	}
	//�޸��ϼ�Ŀ¼
	//AddItemInFolder(targetPath, path[pathNum - 1], filePath->inodeId);

}

void Format()
{
	if (strcmp(NowUser, "root") != 0)
	{
		cout << "Ȩ�޲��㣬���л���root�û���ʹ�ø�����" << endl;
		return;
	}
	//��ʽ�������ļ�
	memset(&disk, 0, sizeof(disk));
	//��ʽ��������
	memset(&SuperBlock, 0, sizeof(SuperBlock));
	//��ʽ��Inode�ڵ�
	memset(Inode, 0, sizeof(Inode));
	//��ʽ��λͼ
	memset(InodeBitmap, 0, sizeof(InodeBitmap));
	lastInodePos = 0;
	NowPath = &Inode[SuperBlock.firstInode];
	RootPath = &Inode[SuperBlock.firstInode];
	
	isLogin = false;
	memcpy(NowPathName, "/", 2);
	memcpy(NowUser, "root", 5);
	memcpy(DeviceName, "Disk0", 6);

	initInode();//��ʼ��inode
	initGroupLink(disk);//��ʼ�����̿�����������������֯
	InitRootFolder();//��ʼ��Ŀ¼
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
	//���ֲ���Ϊ��
	if (strlen(name) == 0)
	{
		return false;
	}

	//��������
	User user = LoadUserFromDisk(disk);
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
	memset(user.password[user.userSum], 0, sizeof(char) * PassWordLen);
	user.userSum++;
	SaveUserToDisk(disk, user);

	NewFolder(disk, RootPath, name);
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
	memset(Password, 0, sizeof(char) * PassWordLen);
	while (1)
	{
		char ch = _getch();
		if (ch == '\n'||ch=='\r')
		{
			break;
		}
		else if (ch == '\b') //�˸�
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