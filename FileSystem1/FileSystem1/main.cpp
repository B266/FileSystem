#include"FileSystem.h"


int main()
{
	cout << "DataBlockIndexFile: " << sizeof(DataBlockIndexFile) << endl;
	initInode();
	initGroupLink(disk);
	InitRootFolder();

	//showAll();
	cout << "int: "  << sizeof(int) << endl;
	//cout << sizeof(Folder) << endl;
	//cout << sizeof(int) << endl;
	//cout << sizeof(char)* NameLen << endl;
	cout << "block: " << sizeof(block) << endl;
	cout << "inode: " << sizeof(inode) << endl;

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
		if (strcmp(command, "rm") == 0) {
			char name[20];
			cin >> name;
			RM(disk, NowPath, name, false);
		}
	}

	


	return 0;
}