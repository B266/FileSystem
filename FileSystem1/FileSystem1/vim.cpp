#include"vim.h"
#include"FileSystem.h"
#pragma once
#include<iostream>
#include<fstream>
#include<Windows.h>
#include<conio.h>

using namespace std;

const int MAX_SIZE = 8500000;
const int OneLineCharSum = 60;
const int LineSum = MAX_SIZE / OneLineCharSum;
HANDLE hOut;

CONSOLE_SCREEN_BUFFER_INFO bInfo;
CONSOLE_CURSOR_INFO cci;

char* buffer;
int bufferlen;
char lineBuffer[LineSum][OneLineCharSum + 1] = { 0 };
ifstream infile;
ofstream outfile;
COORD nowCusorPos;
int Editmode = 0; //浏览模式


int nowLine = 0;

int lineCount = 0;
char command[20] = { 0 };


void init()
{
	buffer = (char*)malloc(sizeof(char) * MAX_SIZE);
	memset(buffer, 0, MAX_SIZE);
	SetConsoleOutputCP(65001);
}

void Load(const char* path)
{
	infile.open(path);
	infile.seekg(0, std::ios::end);
	bufferlen = infile.tellg();
	infile.seekg(0, std::ios::beg);
	infile.read(buffer, bufferlen);
	infile.close();

}

void Save(const char* path)
{
	outfile.open(path);
	outfile.write(buffer, bufferlen);
	outfile.close();
}

int nowBufferIndex = 0;

//返回大小 小于60就说明有回车
int getOneLineIndexFromBuffer(int& index)
{
	int i = nowBufferIndex;
	int oldi = index;
	for (; i < nowBufferIndex + OneLineCharSum; i++)
	{
		if (buffer[i] == '\n')
		{
			index = i + 1;
			nowBufferIndex = i + 1;
			return index - oldi;
		}
	}
	index = i;
	nowBufferIndex = i;
	return index - oldi;
}

void fillLineBuffer()
{
	int oldBufferIndex = 0;
	int newBufferIndex = 0;
	lineCount = 0;
	nowBufferIndex = 0;

	memset(lineBuffer, 0, MAX_SIZE * sizeof(char));
	while (newBufferIndex < bufferlen)
	{
		int nowLineSize;
		nowLineSize = getOneLineIndexFromBuffer(newBufferIndex);

		memcpy(lineBuffer[lineCount], &buffer[oldBufferIndex], nowLineSize);
		memset(lineBuffer[lineCount] + nowLineSize + 1, 0, 1);
		//cout << nowLineSize << " ";
		//cout << lineBuffer[lineCount];
		if (nowLineSize == 60)
		{
			//cout << endl;
		}
		oldBufferIndex = newBufferIndex;
		lineCount++;
	}
	if (lineCount == 0)
	{
		lineCount = 1;
	}
}

void Show()
{
	system("cls");
	cout << buffer;
}

void LineBufferToBuffer()
{
	memset(buffer, 0, MAX_SIZE);
	bufferlen = 0;

	int nowIndex = 0;
	for (int i = 0; i < lineCount; i++)
	{
		//cout << strlen(lineBuffer[i]) << "\t";

		memcpy(buffer + nowIndex, lineBuffer[i], strlen(lineBuffer[i]));
		nowIndex += strlen(lineBuffer[i]);
		bufferlen += strlen(lineBuffer[i]);
	}
}

COORD GetPos()
{

	GetConsoleScreenBufferInfo(hOut, &bInfo);
	return bInfo.dwCursorPosition;
}

void SetPos(COORD pos)
{

	SetConsoleCursorPosition(hOut, pos);
}
void SetPos(int x, int y)
{
	COORD temp;
	temp.X = x;
	temp.Y = y;
	SetConsoleCursorPosition(hOut, temp);
}




void ShowBuffer(int lineStart, const char* filename)
{

	COORD backupPos = nowCusorPos;
	system("cls");

	SetPos(0, 0);
	cout << filename << "[" << lineStart + nowCusorPos.Y << "/" << lineCount << ":" << nowCusorPos.X << "] Editmode:";
	if (Editmode == 0)
	{
		cout << "View" << endl;
	}
	else if (Editmode == 1)
	{
		cout << "Edit" << endl;
	}
	else if (Editmode == 2)
	{
		cout << "Command" << endl;
	}
	for (int i = lineStart; i < lineStart + bInfo.srWindow.Bottom - 2; i++)
	{
		int lineSize = strlen(lineBuffer[i]);
		//cout << strlen(lineBuffer[i]) << "\t";
		cout << lineBuffer[i];
		if (lineSize == 60)
		{
			cout << endl;
		}
	}
	SetPos(0, bInfo.srWindow.Bottom - 1);
	cout << command;
	SetPos(backupPos);
}

void ChangeLineBuffer(int x, int y, char ch)
{
	lineBuffer[y][x] = ch;
	nowCusorPos.X++;
	if (nowCusorPos.X > OneLineCharSum)
	{
		nowCusorPos.X = 0;
		nowCusorPos.Y++;
	}
	SetPos(nowCusorPos);
}



void VIM_(const char* path, const char* filename)
{
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleCursorInfo(hOut, &cci);

	nowCusorPos = GetPos();
	nowCusorPos.Y = 1;
	SetPos(nowCusorPos);
	init();
	Load(path);
	//Show();
	fillLineBuffer();
	LineBufferToBuffer();
	ShowBuffer(0, filename);

	while (1)
	{
		nowCusorPos = GetPos();

		unsigned char ch = _getch();
		//cout << ch << endl;
		if (Editmode == 0)
		{
			if (ch == 'i')
			{
				nowCusorPos.Y = 1;
				nowCusorPos.X = 0;
				SetPos(1, 0);
				Editmode = 1; //修改模式
			}
			if (ch == 'h' || ch == 'w')
			{
				nowCusorPos.Y--;
				if (nowCusorPos.Y <= 1)
				{
					nowCusorPos.Y = 1;
					if (nowLine > 0)
					{
						nowLine--;
					}
				}
			}
			else if (ch == 'j' || ch == 'a')
			{
				if (nowCusorPos.X > 0)
				{
					nowCusorPos.X--;
				}

			}
			else if (ch == 'k' || ch == 'd')
			{
				if (nowCusorPos.X < OneLineCharSum)
				{
					nowCusorPos.X++;
				}

			}
			else if (ch == 'l' || ch == 's')
			{
				if (nowLine < lineCount + 2 && nowCusorPos.Y - 1 <= lineCount)
				{
					nowCusorPos.Y++;
					if (nowCusorPos.Y + 2 > bInfo.srWindow.Bottom)
					{
						nowCusorPos.Y = bInfo.srWindow.Bottom - 2;
						nowLine++;
					}
				}

			}
			else if (ch == 'e' || ch == 'Q')  //向下滚屏
			{

				nowLine += bInfo.srWindow.Bottom;
				if (nowLine + bInfo.srWindow.Bottom > lineCount)
				{
					nowLine = lineCount - bInfo.srWindow.Bottom;
				}
				//ShowBuffer(nowLine);
			}
			else if (ch == 'q' || ch == 'I') //向上滚屏
			{
				nowLine -= bInfo.srWindow.Bottom;
				if (nowLine < 0)
				{
					nowLine = 0;
				}

			}
			else if (ch == 27)
			{
				Editmode = 2;
			}
		}
		else if (Editmode == 1)
		{
			if (ch == 27)
			{
				Editmode = 0;
			}
			else if (ch == 224)
			{
				char nowchar = _getch();
				if (nowchar == 75) //向左
				{
					if (nowCusorPos.X > 0)
					{
						nowCusorPos.X--;
					}
				}
				else if (nowchar == 72) //向上
				{
					if (nowCusorPos.Y > 0)
					{
						nowCusorPos.Y--;
					}

				}
				else if (nowchar == 77) //向右
				{
					if (nowCusorPos.X < OneLineCharSum)
					{
						nowCusorPos.X++;
					}
				}
				else if (nowchar == 80) //向下
				{
					if (nowCusorPos.Y < lineCount)
					{
						nowCusorPos.Y++;
					}
				}
			}

			else if (ch == '\b')
			{
				if (nowCusorPos.X > 0)
				{
					nowCusorPos.X--;
					ChangeLineBuffer(nowCusorPos.X, nowLine + nowCusorPos.Y - 1, ' ');
					nowCusorPos.X--;
				}
				else if (nowCusorPos.X == 0)
				{
					nowCusorPos.X--;
					ChangeLineBuffer(nowCusorPos.X, nowLine + nowCusorPos.Y - 1, ' ');
					nowCusorPos.Y--;
					nowCusorPos.X = strlen(lineBuffer[nowLine + nowCusorPos.Y - 1]);
				}

			}
			else if (ch == '\r')
			{
				ChangeLineBuffer(nowCusorPos.X, nowLine + nowCusorPos.Y - 1, '\n');
				ChangeLineBuffer(nowCusorPos.X + 1, nowLine + nowCusorPos.Y - 1, '\0');
				nowCusorPos.X = 0;
				nowCusorPos.Y++;
				lineCount++;
			}
			else {
				ChangeLineBuffer(nowCusorPos.X, nowLine + nowCusorPos.Y - 1, ch);
			}
		}
		else if (Editmode == 2)
		{
			if (ch == '\b')
			{
				command[strlen(command) - 1] = '\0';
			}
			else if (ch == '\r')
			{

				if (strcmp(command, ":wq") == 0)
				{
					ShowBuffer(nowLine, filename);
					LineBufferToBuffer();
					Save(path);
					return;
				}
				else if (strcmp(command, ":q") == 0)
				{
					return;
				}
				else if (strcmp(command, ":") == 0)
				{
					Editmode = 0;
				}
			}
			else
			{
				command[strlen(command)] = ch;
			}


		}
		SetPos(nowCusorPos);
		ShowBuffer(nowLine, filename);
	}

}



bool VIM(char* filename, inode* NowPath)
{
	inode* BmpInode = getInodeByPathName(filename, NowPath, 1);
	if (BmpInode != NULL)
	{
		int BmpIndex = BmpInode->DataBlockIndex0[0];
		File* BmpFile = OpenFile(disk, BmpInode);

		ofstream out;
		out.open("tempVIM.txt", ios::out | ios::binary);

		char* buffer = (char*)malloc(sizeof(CHAR) * BmpFile->dataSize);
		::memcpy(buffer, BmpFile->data, sizeof(CHAR) * BmpFile->dataSize);
		out.write(buffer, sizeof(CHAR) * BmpFile->dataSize);
		out.close();
		free(buffer);

		//fopen();

		VIM_("tempVIM.txt", BmpInode->Name);
		system("cls");
		free(buffer);

		return true;
	}
	else return false;
}
