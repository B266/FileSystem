#include"showPic.h"
#include"FileSystem.h"



#define _CRT_SECURE_NO_WARNINGS
#include<fstream>
#include<windows.h>
#include<iostream>
#include<tchar.h>
using namespace std;


unsigned char* pBmpBuf;//读入图像数据的指针
int bmpWidth;//图像的宽
int bmpHeight;//图像的高
RGBQUAD* pColorTable;//颜色表指针
int biBitCount;//图像类型，每像素位数



//显示位图文件头信息   
void showBmpHead(BITMAPFILEHEADER pBmpHead) {
	cout << "\n位图文件头:" << endl;
	cout << "文件大小:" << pBmpHead.bfSize << endl;
	cout << "保留字_1:" << pBmpHead.bfReserved1 << endl;
	cout << "保留字_2:" << pBmpHead.bfReserved2 << endl;
	cout << "实际位图数据的偏移字节数:" << pBmpHead.bfOffBits << endl << endl;
}
//显示位图信息头信息  
void showBmpInforHead(BITMAPINFOHEADER pBmpInforHead) {
	cout << "\n位图信息头:" << endl;
	cout << "结构体的长度:" << pBmpInforHead.biSize << endl;
	cout << "位图宽:" << pBmpInforHead.biWidth << endl;
	cout << "位图高:" << pBmpInforHead.biHeight << endl;
	cout << "biPlanes平面数:" << pBmpInforHead.biPlanes << endl;
	cout << "biBitCount采用颜色位数:" << pBmpInforHead.biBitCount << endl;
	cout << "压缩方式:" << pBmpInforHead.biCompression << endl;
	cout << "biSizeImage实际位图数据占用的字节数:" << pBmpInforHead.biSizeImage << endl;
	cout << "X方向分辨率:" << pBmpInforHead.biXPelsPerMeter << endl;
	cout << "Y方向分辨率:" << pBmpInforHead.biYPelsPerMeter << endl;
	cout << "使用的颜色数:" << pBmpInforHead.biClrUsed << endl;
	cout << "重要颜色数:" << pBmpInforHead.biClrImportant << endl;
}

//-----------------------------------------------------------------------------------------
//给定一个图像位图数据、宽、高、颜色表指针及每像素所占的位数等信息,将其写到指定文件中
bool readBmp(const char* bmpName)
{
	FILE* fp ;//二进制读方式打开指定的图像文件
	fopen_s(&fp, bmpName, "rb");
	if (fp == 0)
		return 0;

	//跳过位图文件头结构BITMAPFILEHEADER
	fseek(fp, sizeof(BITMAPFILEHEADER), 0);
	/*
	BITMAPFILEHEADER filehead;
	fread(&filehead, 1, sizeof(BITMAPFILEHEADER), fp);
	showBmpHead(filehead);//显示文件头
*/

//定义位图信息头结构变量，读取位图信息头进内存，存放在变量head中
	BITMAPINFOHEADER infohead;
	fread(&infohead, sizeof(BITMAPINFOHEADER), 1, fp); //获取图像宽、高、每像素所占位数等信息
	bmpWidth = infohead.biWidth;
	bmpHeight = infohead.biHeight;
	biBitCount = infohead.biBitCount;//定义变量，计算图像每行像素所占的字节数（必须是4的倍数）
	//showBmpInforHead(infohead);//显示信息头 


	int lineByte = (bmpWidth * biBitCount / 8 + 3) / 4 * 4;//灰度图像有颜色表，且颜色表表项为256
	if (biBitCount == 8)
	{
		//申请颜色表所需要的空间，读颜色表进内存
		pColorTable = new RGBQUAD[256];
		fread(pColorTable, sizeof(RGBQUAD), 256, fp);
	}

	//申请位图数据所需要的空间，读位图数据进内存
	pBmpBuf = new unsigned char[lineByte * bmpHeight];
	fread(pBmpBuf, 1, lineByte * bmpHeight, fp);
	fclose(fp);//关闭文件
	return 1;//读取文件成功
}





//用于显示窗体的全局变量
//----------------------------------

const int bits = 24;
extern BYTE* buffer;




class Color
{
public:
	Color(int R, int G, int B) :r(R), g(G), b(B) {};
	Color() :r(255), g(255), b(255) {};
	int r, g, b;
};


void CleanScreen();

void DrawPoint(int x, int y, Color color = Color(255, 255, 255));

void PutBufferToScreen();

void DrawPoint(int x, int y, const Color color)
{
	if (x <= 0 || x >= bmpWidth)return;
	if (y <= 0 || y >= bmpHeight)return;

	buffer[int(y) * bmpWidth * 3 + (int(x) + 1) * 3 - 1] = color.r;
	buffer[int(y) * bmpWidth * 3 + (int(x) + 1) * 3 - 2] = color.g;
	buffer[int(y) * bmpWidth * 3 + (int(x) + 1) * 3 - 3] = color.b;
}


void CleanScreen()
{
	for (int y = 0; y < bmpHeight; y++)
	{
		for (int x = 0; x < bmpWidth; x++)
		{

			buffer[int(y) * bmpWidth * 3 + (int(x) + 1) * 3 - 1] = 0;
			buffer[int(y) * bmpWidth * 3 + (int(x) + 1) * 3 - 2] = 0;
			buffer[int(y) * bmpWidth * 3 + (int(x) + 1) * 3 - 3] = 0;
		}
	}
}


void GameLoop();

bool quitWindowsFlag = false;

BYTE* buffer;

HDC screen_hdc;
HDC hCompatibleDC;
HBITMAP hCompatibleBitmap;
HBITMAP hOldBitmap;
BITMAPINFO binfo;

HINSTANCE hInstance;
WNDCLASS Draw;
HWND hwnd;
MSG msg;


LRESULT CALLBACK WindowProc(
	_In_	HWND hwnd,
	_In_	UINT uMsg,
	_In_	WPARAM wParam,
	_In_	LPARAM lParam
)
{
	switch (uMsg)
	{
	case WM_DESTROY:
	{
		quitWindowsFlag = true;
		PostQuitMessage(0);
		return 0;
	}
	case WM_KEYDOWN:
	{
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void PutBufferToScreen()
{
	SetDIBits(screen_hdc, hCompatibleBitmap, 0, bmpHeight, buffer, (BITMAPINFO*)&binfo, DIB_RGB_COLORS);
	BitBlt(screen_hdc, -1, -1, bmpWidth, bmpHeight, hCompatibleDC, 0, 0, SRCCOPY);
}


bool showPic(const char*readPath, const char* filename)
{

	if (readBmp(readPath))
	{
		cout << "readOK!" << endl;
		cout << "\nwidth=" << bmpWidth << "\nheight=" << bmpHeight << endl;

		buffer = (BYTE*)malloc(sizeof(BYTE) * bmpWidth * bmpHeight * bits / 8);
		hInstance = GetModuleHandle(NULL);

		WCHAR* title = (WCHAR*)malloc(sizeof(WCHAR) * 100);
		wsprintf(title, _T("ShowPic"));

		Draw.cbClsExtra = 0;
		Draw.cbWndExtra = 0;
		Draw.hCursor = LoadCursor(hInstance, IDC_ARROW);
		Draw.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
		Draw.lpszMenuName = NULL;
		Draw.style = WS_MINIMIZEBOX | CS_HREDRAW | CS_VREDRAW;
		Draw.hbrBackground = (HBRUSH)COLOR_WINDOW;
		Draw.lpfnWndProc = WindowProc;
		Draw.lpszClassName = title;
		Draw.hInstance = hInstance;

		RegisterClass(&Draw);

		hwnd = CreateWindow(
			title,
			title,
			WS_OVERLAPPEDWINDOW,
			38,
			20,
			bmpWidth + 15,
			bmpHeight + 38,
			NULL,
			NULL,
			hInstance,
			NULL
		);

		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);

		//init bitbuffer
		ZeroMemory(&binfo, sizeof(BITMAPINFO));
		binfo.bmiHeader.biBitCount = bits;
		binfo.bmiHeader.biCompression = BI_RGB;
		binfo.bmiHeader.biHeight = -bmpHeight;
		binfo.bmiHeader.biPlanes = 1;
		binfo.bmiHeader.biSizeImage = 0;
		binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		binfo.bmiHeader.biWidth = bmpWidth;

		screen_hdc = GetDC(hwnd);
		hCompatibleDC = CreateCompatibleDC(screen_hdc);
		hCompatibleBitmap = CreateCompatibleBitmap(screen_hdc, bmpWidth, bmpHeight);
		hOldBitmap = (HBITMAP)SelectObject(hCompatibleDC, hCompatibleBitmap);

		while (1)
		{
			if (quitWindowsFlag)
			{
				return true;
			}
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			GameLoop();
		}
		return true;
	}
	else
	{
		cout << "file no found!" << endl;
		return false;
	}
}



void GameLoop()
{
	CleanScreen();
	for (int y = 0; y < bmpHeight; y++)
		for (int x = 0; x < bmpWidth; x++)
		{
			Color color = Color(pBmpBuf[y * 3 * bmpWidth + x * 3 + 2], pBmpBuf[y * 3 * bmpWidth + x * 3 + 1], pBmpBuf[y * 3 * bmpWidth + x * 3]);
			DrawPoint(x, bmpHeight - y, color);
		}

	PutBufferToScreen();
}

bool showPic(char* filename, inode* NowPath)
{
	inode* BmpInode = getInodeByPathName(filename, NowPath, 1);
	quitWindowsFlag = false;
	if (BmpInode != NULL)
	{

		int BmpIndex = BmpInode->DataBlockIndex0[0];
		File* BmpFile = OpenFile(disk, BmpInode);

		ofstream out;
		out.open("temp.bmp", ios::out | ios::binary);

		char* buffer = (char*)malloc(sizeof(CHAR)* BmpFile->dataSize);
		::memcpy(buffer, BmpFile->data, sizeof(CHAR) * BmpFile->dataSize);
		out.write(buffer, sizeof(CHAR) * BmpFile->dataSize);
		out.close();
		free(buffer);

		//fopen();

		//展示图片
		showPic("temp.bmp", BmpInode->Name);

		return true;
	}
	else return false;
}