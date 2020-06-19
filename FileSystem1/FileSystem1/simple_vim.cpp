#include "simple_vim.h"
using namespace std;

void gotoxy(HANDLE hOut, int x, int y)	//移动光标到指定位置
{
	COORD pos;
	pos.X = x;             //横坐标
	pos.Y = y;            //纵坐标
	SetConsoleCursorPosition(hOut, pos);
}

void vim(inode* nowPath, char* fileName) {
	// 获取目标文件Inode
	inode* fileInode = getInodeByPathName(fileName, nowPath);
	inode* fatherFolderInode = getInodeByPathName(fileName, nowPath, 2);
	File* openFile;
	// 判断文件是否存在
	if (fatherFolderInode == NULL) {
		cout << "vim: 无法打开\"" << fileName << "\": 没有那个文件或目录" << endl;
		return;
	}
	// 新建文件
	if (fileInode == NULL && fatherFolderInode != NULL) {
		int indexInode = GetAInode();
		char FileName[NameLen] = { 0 };
		char ExtensionName[NameLen] = { 0 };
		GetFileNameAndExtensionName(fileName, FileName, ExtensionName);
		//填写inode
		strcpy_s(Inode[indexInode].Name, FileName);
		strcpy_s(Inode[indexInode].ExtensionName, ExtensionName);
		strcpy_s(Inode[indexInode].username, NowUser);
		strcpy_s(Inode[indexInode].usergroupname, NowGroupName);
		Inode[indexInode].size = 0;
		Inode[indexInode].permissions = 666;

		//修改上级目录
		AddItemInFolder(fatherFolderInode, FileName, indexInode);

		fileInode = &Inode[indexInode];
	}
	
	openFile = OpenFile(disk, fileInode);

	//cout << openFile->data << endl;
	int maxlen = 0; // 达到过的最大长度
	
	char buf[99999];	//最大100K

	//初始化vi
	int cnt = 0;
	system("cls");	//清屏

	int winx, winy, curx, cury;

	HANDLE handle_out;                              //定义一个句柄  
	CONSOLE_SCREEN_BUFFER_INFO screen_info;         //定义窗口缓冲区信息结构体  
	COORD pos = { 0, 0 };                             //定义一个坐标结构体

	//将文件内容读取出来，显示在，窗口上
	int i = 0;
	int sumlen = fileInode->size;	//文件长度

	cnt = strlen(openFile->data);
	strcpy_s(buf, openFile->data);
	maxlen = sumlen;

	//获得输出之后的光标位置
	handle_out = GetStdHandle(STD_OUTPUT_HANDLE);   //获得标准输出设备句柄  
	GetConsoleScreenBufferInfo(handle_out, &screen_info);   //获取窗口信息  
	winx = screen_info.srWindow.Right - screen_info.srWindow.Left + 1;
	winy = screen_info.srWindow.Bottom - screen_info.srWindow.Top + 1;
	curx = screen_info.dwCursorPosition.X;
	cury = screen_info.dwCursorPosition.Y;

	//进入vi
	//先用vi读取文件内容

	int mode = 0;	//vi模式，一开始是命令模式
	unsigned char c;
	while (1) {
		if (mode == 0) {	//命令行模式
			c = _getch();

			if (c == 'i' || c == 'a') {	//插入模式
				if (c == 'a') {
					curx++;
					if (curx == winx) {
						curx = 0;
						cury++;

						/*
						if(cury>winy-2 || cury%(winy-1)==winy-2){
							//超过这一屏，向下翻页
							if(cury%(winy-1)==winy-2)
								printf("\n");
							SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
							int i;
							for(i=0;i<winx-1;i++)
								printf(" ");
							gotoxy(handle_out,0,cury+1);
							printf(" - 插入模式 - ");
							gotoxy(handle_out,0,cury);
						}
						*/
					}
				}

				if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
					//超过这一屏，向下翻页
					if (cury % (winy - 1) == winy - 2)
						printf("\n");
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, cury + 1);
					printf(" - 插入模式 - ");
					gotoxy(handle_out, 0, cury);
				}
				else {
					//显示 "插入模式"
					gotoxy(handle_out, 0, winy - 1);
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, winy - 1);
					printf(" - 插入模式 - ");
					gotoxy(handle_out, curx, cury);
				}

				gotoxy(handle_out, curx, cury);
				mode = 1;


			}
			else if (c == ':') {
				//system("color 09");//设置文本为蓝色
				if (cury - winy + 2 > 0)
					gotoxy(handle_out, 0, cury + 1);
				else
					gotoxy(handle_out, 0, winy - 1);
				_COORD pos;
				if (cury - winy + 2 > 0)
					pos.X = 0, pos.Y = cury + 1;
				else
					pos.X = 0, pos.Y = winy - 1;
				SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
				int i;
				for (i = 0; i < winx - 1; i++)
					printf(" ");

				if (cury - winy + 2 > 0)
					gotoxy(handle_out, 0, cury + 1);
				else
					gotoxy(handle_out, 0, winy - 1);

				WORD att = BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_INTENSITY; // 文本属性
				//FillConsoleOutputAttribute(handle_out, att, winx, pos, NULL);	//控制台部分着色 
				//SetConsoleTextAttribute(handle_out, FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_GREEN);	//设置文本颜色
				printf(":");

				char pc;
				int tcnt = 1;	//命令行模式输入的字符计数
				while (c = _getch()) {
					if (c == '\r') {	//回车
						break;
					}
					else if (c == '\b') {	//退格，从命令条删除一个字符 
						//SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
						tcnt--;
						if (tcnt == 0)
							break;
						printf("\b");
						printf(" ");
						printf("\b");
						continue;
					}
					pc = c;
					printf("%c", pc);
					tcnt++;
				}
				if (pc == 'q') {
					buf[maxlen] = '\0';
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					system("cls");
					break;	//vi >>>>>>>>>>>>>> 退出出口
				}
				else {
					if (cury - winy + 2 > 0)
						gotoxy(handle_out, 0, cury + 1);
					else
						gotoxy(handle_out, 0, winy - 1);
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");

					if (cury - winy + 2 > 0)
						gotoxy(handle_out, 0, cury + 1);
					else
						gotoxy(handle_out, 0, winy - 1);
					//SetConsoleTextAttribute(handle_out, FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_GREEN);	//设置文本颜色
					//FillConsoleOutputAttribute(handle_out, att, winx, pos, NULL);	//控制台部分着色
					printf(" 错误命令");
					//getch();
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					gotoxy(handle_out, curx, cury);
				}
			}
			else if (c == 27) {	//ESC，命令行模式，清状态条
				gotoxy(handle_out, 0, winy - 1);
				SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
				int i;
				for (i = 0; i < winx - 1; i++)
					printf(" ");
				gotoxy(handle_out, curx, cury);

			}

		}
		else if (mode == 1) {	//插入模式

			gotoxy(handle_out, winx / 4 * 3, winy - 1);
			int i = winx / 4 * 3;
			while (i < winx - 1) {
				printf(" ");
				i++;
			}
			if (cury > winy - 2)
				gotoxy(handle_out, winx / 4 * 3, cury + 1);
			else
				gotoxy(handle_out, winx / 4 * 3, winy - 1);
			printf("[行:%d,列:%d]", curx == -1 ? 0 : curx, cury);
			gotoxy(handle_out, curx, cury);

			c = _getch();
			if (c == 27) {	// ESC，进入命令模式
				mode = 0;
				//清状态条
				gotoxy(handle_out, 0, winy - 1);
				SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
				int i;
				for (i = 0; i < winx - 1; i++)
					printf(" ");
				continue;
			}
			else if (c == '\b') {	//退格，删除一个字符
				if (cnt == 0)	//已经退到最开始
					continue;
				printf("\b");
				printf(" ");
				printf("\b");
				curx--;
				cnt--;	//删除字符
				if (buf[cnt] == '\n') {
					//要删除的这个字符是回车，光标回到上一行
					if (cury != 0)
						cury--;
					int k;
					curx = 0;
					for (k = cnt - 1; buf[k] != '\n' && k >= 0; k--)
						curx++;
					gotoxy(handle_out, curx, cury);
					printf(" ");
					gotoxy(handle_out, curx, cury);
					if (cury - winy + 2 >= 0) {	//翻页时
						gotoxy(handle_out, curx, 0);
						gotoxy(handle_out, curx, cury + 1);
						SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
						int i;
						for (i = 0; i < winx - 1; i++)
							printf(" ");
						gotoxy(handle_out, 0, cury + 1);
						printf(" - 插入模式 - ");

					}
					gotoxy(handle_out, curx, cury);

				}
				else
					buf[cnt] = ' ';
				continue;
			}
			else if (c == 224) {	//判断是否是箭头
				c = _getch();
				if (c == 75) {	//左箭头
					if (cnt != 0) {
						cnt--;
						curx--;
						if (buf[cnt] == '\n') {
							//上一个字符是回车
							if (cury != 0)
								cury--;
							int k;
							curx = 0;
							for (k = cnt - 1; buf[k] != '\n' && k >= 0; k--)
								curx++;
						}
						gotoxy(handle_out, curx, cury);
					}
				}
				else if (c == 77) {	//右箭头
					cnt++;
					if (cnt > maxlen)
						maxlen = cnt;
					curx++;
					if (curx == winx) {
						curx = 0;
						cury++;

						if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
							//超过这一屏，向下翻页
							if (cury % (winy - 1) == winy - 2)
								printf("\n");
							SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
							int i;
							for (i = 0; i < winx - 1; i++)
								printf(" ");
							gotoxy(handle_out, 0, cury + 1);
							printf(" - 插入模式 - ");
							gotoxy(handle_out, 0, cury);
						}

					}
					gotoxy(handle_out, curx, cury);
				}
				continue;
			}
			if (c == '\r') {	//遇到回车
				printf("\n");
				curx = 0;
				cury++;

				if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
					//超过这一屏，向下翻页
					if (cury % (winy - 1) == winy - 2)
						printf("\n");
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, cury + 1);
					printf(" - 插入模式 - ");
					gotoxy(handle_out, 0, cury);
				}

				buf[cnt++] = '\n';
				if (cnt > maxlen)
					maxlen = cnt;
				continue;
			}
			else {
				printf("%c", c);
			}
			//移动光标
			curx++;
			if (curx == winx) {
				curx = 0;
				cury++;

				if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
					//超过这一屏，向下翻页
					if (cury % (winy - 1) == winy - 2)
						printf("\n");
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, cury + 1);
					printf(" - 插入模式 - ");
					gotoxy(handle_out, 0, cury);
				}

				buf[cnt++] = '\n';
				if (cnt > maxlen)
					maxlen = cnt;
				if (cury == winy) {
					printf("\n");
				}
			}
			//记录字符 
			buf[cnt++] = c;
			if (cnt > maxlen)
				maxlen = cnt;
		}
		else {	//其他模式
		}
	}

	// 将buf内容写回文件的磁盘块
	SaveFileData(disk, fileInode, buf, strlen(buf));
}
