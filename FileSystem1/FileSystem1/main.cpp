
#include"FileSystem.h"
#include "simple_vim.h"

using namespace std;
HANDLE CommandLineHandle;

void GetStr()
{
	char name[MAXPATH_LEN] = { 0 };
	bool inputflag = false;
	while (1)
	{
		char c=_getch();
		memcpy(name + strlen(name), &c, sizeof(char));
	}
}


int main()
{

	CommandLineHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetTitle("FileSystem");
	Sleep(500);
	HWND hw = FindWindow(L"ConsoleWindowClass", L"FileSystem");
	SetLayeredWindowAttributes(hw, NULL, (230 * 100) / 100, LWA_ALPHA);
	
	//cout << "DataBlockIndexFile: " << sizeof(DataBlockIndexFile) << endl;
	Format();
	//showAll();
	//cout << "int: "  << sizeof(int) << endl;
	//cout << sizeof(Folder) << endl;
	//cout << sizeof(int) << endl;
	//cout << sizeof(char)* NameLen << endl;
	//cout << "block: " << sizeof(block) << endl;
	//cout << "inode: " << sizeof(inode) << endl;
	cout << sizeof(User) << endl;


	while (1)
	{



		UserManager(disk);

		ShowNowPathInfo();
		char Arr[MAXPATH_LEN * 3] = { 0 };
		char Arr1[MAXPATH_LEN] = { 0 };
		char Arr2[MAXPATH_LEN] = { 0 };
		char Arr3[MAXPATH_LEN] = { 0 };
		cin.clear();
		cin.sync();
		cin.getline(Arr, MAXPATH_LEN * 3);
		CutArr(Arr, Arr1, Arr2, Arr3);
		//切成三个命令参数来计算

		if (strcmp(Arr1, "cd") == 0)
		{
			CD(Arr2,&NowPath);

		}
		else if (strcmp(Arr1, "ls") == 0)
		{
			LS(Arr2, Arr3);
			
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
				cout << "提示：请拖拽文件到本窗口，并按下回车键\n";
				char Path[MAXPATH_LEN] = { 0 };
				cin >> Path;
				Import(Path, NowPath);
			}
		}
		else if (strcmp(Arr1, "rename") == 0)
		{
			Rename(Arr2, Arr3);
		}
		else if (strcmp(Arr1, "format") == 0)
		{
			Format();
		}
		else if (strcmp(Arr1, "mv") == 0) {
			MV(NowPath, Arr2, Arr3);
		}
		else if (strcmp(Arr1, "cp") == 0) {
			CP(NowPath, Arr2, Arr3);
		}
		else if (strcmp(Arr1, "useradd") == 0)
		{
			useradd(Arr2, disk);
		}
		else if (strcmp(Arr1, "passwd") == 0)
		{
			passwd(Arr2, disk);
		}
		else if (strcmp(Arr1, "su") == 0)
		{
			su(Arr2);
		}
		else if (strcmp(Arr1, "vim") == 0) {
			vim(NowPath, Arr2);
		}
		else
		{
			WrongCommand(Arr1);
		}
		
	}

	return 0;
}