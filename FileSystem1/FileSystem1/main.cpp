
#include"FileSystem.h"
HANDLE CommandLineHandle;



int main()
{
	CommandLineHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	SetLayeredWindowAttributes(GetActiveWindow(), NULL, 200, LWA_ALPHA);
	
	//cout << "DataBlockIndexFile: " << sizeof(DataBlockIndexFile) << endl;
	initInode();
	initGroupLink(disk);
	InitRootFolder();

	//showAll();
	//cout << "int: "  << sizeof(int) << endl;
	//cout << sizeof(Folder) << endl;
	//cout << sizeof(int) << endl;
	//cout << sizeof(char)* NameLen << endl;
	//cout << "block: " << sizeof(block) << endl;
	//cout << "inode: " << sizeof(inode) << endl;

	//让输入缓冲区里有一个字符，为了输入效果
	char n = '\n';
	cin.putback(n);

	while (1)
	{
		char command[20] = { 0 };
		ShowNowPathInfo();
		cin>>command;

		if (strcmp(command, "cd") == 0)
		{
			char name[20];
			cin >> name;
			CD(name,&NowPath);

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
			char name[20];
			cin >> name;

			ShowText(name,NowPath);
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

		}
		if (strcmp(command, "rm") == 0) {
			char name[20];
			cin >> name;
			RM(disk, NowPath, name, false);
		}
		if (strcmp(command, "exit") == 0)
		{
			exit(0);
		}
	}


	return 0;
}