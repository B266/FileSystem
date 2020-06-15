
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

//��Ŀ¼
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


	//д���Լ���inode

	cout << "name:";
	char name[NameLen];
	cin >> name;
	


	strcpy_s(Inode[indexInode].Name, name);
	getchar();
	getchar();
	const int maxsize = 1024*1024;

	//д���Լ���datablock
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
		//��ӻ��з�
		char n = '\n';
		::memcpy(text + strlen(text), &n, 1);
	}

	SaveFileData(disk, &Inode[indexInode], text, strlen(text));

	
	//�޸��ϼ�Ŀ¼
	AddItemInFolder(FolderInode, name, indexInode);

}


void ShowText(char *pathName, inode* nowpath)
{
	inode* fileinode=NULL;
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
	else if (targetpath != NULL && strcmp(targetpath->ExtensionName, "folder") == 0 ){
		memcpy(NowPathName, nowPathName_backup, strlen(nowPathName_backup) + 1);
		cout << "��·�������ļ����޷���" << endl;
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
			int dataBlockIndex2FileIndex = GetOneBlock(disk);//������ӿ�
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

	//�޸��ϼ�Ŀ¼
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
		cout << "��·��Ϊ�ļ�·�����޷�����" << endl;
	}

}

// ����Ŀ¼�ṹ��ɾ��һ���ļ�
void DeleteItemInFolder(inode* folderInode, Folder* folder, char* name, int index) {
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

	// ������ǰĿ¼
	for (int i = 2; i < folder->itemSum; i++) {
		bool haveSuchAFile = false; // �ж��Ƿ���Ҫɾ�����ļ�
		// �������ļ��У���Ĭ��ɾ�������ļ��������д��ļ�
		if (isSonFolder) { haveSuchAFile = true; }
		// ���������ļ��в����ҵ���Ҫɾ�����ļ���
		if (!isSonFolder && strcmp(folder->name[i], name) == 0) { 
			// �ж��Ƿ�Ϊ�ļ���
			if (strcmp(Inode[folder->index[i]].ExtensionName, "folder") == 0) {
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
		}
		// ���д��ļ���ִ��ɾ��
		if (haveSuchAFile) {
			int inodeID = folder->index[i];
			// ���㵱ǰ��block����
			int blockNum = Inode[inodeID].size / (sizeof(block) - sizeof(int)) + 1;
			// ����ǰ�ļ�Ϊ�ļ��У�ɾ�����е������ļ�
			if (strcmp(Inode[inodeID].ExtensionName, "folder") == 0) {
				inode* sonFolderInode = &Inode[inodeID];
				char sonFolderName[20] = ""; // ��ɾ��������ռλ
				// ɾ�����ļ����е������ļ�
				RM(disk, sonFolderInode, sonFolderName, true);
			}
			// �ͷŵ�ǰ�ļ���inode��λͼ1��0
			if (FreeAInode(inodeID)) {
				// �ͷŵ�ǰ�ļ���block��������
				// ��blockNum����<=10����˵������10��ֱ�ӿ��д洢������
				for (int j = 0; j < min(blockNum, 10); j++) {
					int blockID = Inode[inodeID].DataBlockIndex0[j];
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
					DeleteItemInFolder(folderInode, folder, name, i);
					rmFlag = true;
					break;
				}
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
inode* getInodeByPathName(const char* folderPathName, inode* nowPath) {
	/*
		�����м��ַ��ؽ��
		1. ·���е�����һ�������ڣ��򷵻�NULL
		2. ·�����ڣ����һ��·�����ļ��У��򷵻���inode
	*/
	inode* targetPath;
	char nowPathName[MAXPATH_LEN];
	// ���ݵ�ǰ·��
	memcpy(nowPathName, NowPathName, strlen(NowPathName) + 1);

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

	// ���Ե�ַ
	if (*folderPathName == '/') {
		//cout << "���Ե�ַ" << endl;
		memcpy(NowPathName, "/", sizeof("/"));
		targetPath = RootPath;
	}
	// ��Ե�ַ
	else {
		//cout << "��Ե�ַ" << endl;
		targetPath = nowPath;
	}

	for (int p = 0; p < pathNum; p++) {
		Folder* folder = loadFolderFromDisk(disk, targetPath->DataBlockIndex0[0]);
		bool haveSuchAPath = false;
		for (int q = 0; q < folder->itemSum; q++) {
			if (strcmp(path[p], folder->name[q]) == 0) {
				targetPath = &Inode[folder->index[q]];
				haveSuchAPath = true;
				break; // �ҵ�·���˳���ǰ�ļ��еı���
			}

		}
		if (!haveSuchAPath) {
			cout << "û��'" << path[p] << "'�Ǹ�·��" << endl;
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