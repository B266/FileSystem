
#include"FileSystem.h"

using namespace std;
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



	while (1)
	{
		ShowNowPathInfo();
		char Arr[MAXPATH_LEN * 3] = { 0 };
		char Arr1[MAXPATH_LEN] = { 0 };
		char Arr2[MAXPATH_LEN] = { 0 };
		char Arr3[MAXPATH_LEN] = { 0 };
		cin.getline(Arr, MAXPATH_LEN * 3);
		CutArr(Arr, Arr1, Arr2, Arr3);
		//切成三个命令参数来计算

		if (strcmp(Arr1, "cd") == 0)
		{
			CD(Arr2,&NowPath);

		}
		else if (strcmp(Arr1, "ls") == 0)
		{
			if (strlen(Arr2) > 0)
			{
				LS(Arr2);
			}
			else {
				LS(NowPath);
			}
			
		}
		else if (strcmp(Arr1, "new") == 0)
		{
			NewTxt(NowPath);
		}
		else if (strcmp(Arr1, "open") == 0)
		{
			ShowText(Arr2, NowPath);
		}
		else if (strcmp(Arr1, "save") == 0)
		{
			SaveDisk();
		}
		else if (strcmp(Arr1, "load") == 0)
		{
			LoadDisk();
			NowPath = RootPath;
			strcpy_s(NowPathName, "/");
			
		}
		else if (strcmp(Arr1, "mkdir") == 0)
		{
			NewFolder(disk, NowPath, Arr2);

		}
		else if (strcmp(Arr1, "rm") == 0) {
			RM(disk, NowPath, Arr2, false);
		}
		else if (strcmp(Arr1, "exit") == 0)
		{
			return 0;
		}
		else if (strcmp(Arr1, "chmod") == 0)
		{
			
			Chmod(Arr2, atoi(Arr3),NowPath );
		}
		else if (strcmp(Arr1, "complier") == 0)
		{
			complier(Arr2, NowPath, disk);
		}
		else if (strcmp(Arr1, "export") == 0)
		{
			Export(Arr2, Arr3);
		}
		else if (strcmp(Arr1, "import") == 0)
		{
			if (strlen(Arr2) > 0)
			{
				Import(Arr2, NowPath);
			}
			else {
				cout << "请拖拽文件到本窗口";
				char Path[MAXPATH_LEN] = { 0 };
				cin >> Path;
				Import(Path, NowPath);
			}
		}

	}


	return 0;
}