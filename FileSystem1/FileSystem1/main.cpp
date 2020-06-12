#include"OS.h"

int main()
{
	errno_t err=0;
	err = fopen_s(&fw, "C:/Users/BlackBird/Desktop/OS.txt", "wb");
	if (err == 0) 
	{
		cout << "openfile" << endl;
		Format();

	}
	

}

/*int main()
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
}*/