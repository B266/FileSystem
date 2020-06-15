
#include"FileSystem.h"
HANDLE CommandLineHandle;



int main()
{
	CommandLineHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetTitle("FileSystem");
	Sleep(500);
	HWND hw = FindWindow(L"ConsoleWindowClass", L"FileSystem");
	SetLayeredWindowAttributes(hw, NULL, (230 * 100) / 100, LWA_ALPHA);
	
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
			char name[MAXPATH_LEN];
			cin >> name;
			CD(name,&NowPath);

		}
		else if (strcmp(command, "ls") == 0)
		{
			LS(NowPath);
		}
		else if (strcmp(command, "new") == 0)
		{
			NewTxt(NowPath);
		}
		else if (strcmp(command, "open") == 0)
		{
			char name[MAXPATH_LEN];
			cin >> name;

			ShowText(name, NowPath);
		}
		else if (strcmp(command, "save") == 0)
		{
			SaveDisk();
		}
		else if (strcmp(command, "load") == 0)
		{
			LoadDisk();
		}
		else if (strcmp(command, "mkdir") == 0)
		{

			char name[20];
			cin >> name;
			NewFolder(disk, NowPath, name);

		}
		else if (strcmp(command, "rm") == 0) {
			char name[20];
			cin >> name;
			RM(disk, NowPath, name, false);
		}
		else if (strcmp(command, "exit") == 0)
		{
			exit(0);
		}
		else if (strcmp(command, "chmod") == 0)
		{
			char path[MAXPATH_LEN];
			cin >> path;
			int permission;
			cin >> permission;
			
			Chomd(path,  permission, NowPath);
		}
		else
		{
			cout << endl;
		}
	}


	return 0;
}