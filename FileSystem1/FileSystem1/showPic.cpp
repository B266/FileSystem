#include"showPic.h"
#include"FileSystem.h"



#define _CRT_SECURE_NO_WARNINGS
#include<fstream>
#include<windows.h>
#include<iostream>
#include<tchar.h>
using namespace std;


unsigned char* pBmpBuf;//����ͼ�����ݵ�ָ��
int bmpWidth;//ͼ��Ŀ�
int bmpHeight;//ͼ��ĸ�
RGBQUAD* pColorTable;//��ɫ��ָ��
int biBitCount;//ͼ�����ͣ�ÿ����λ��



//��ʾλͼ�ļ�ͷ��Ϣ   
void showBmpHead(BITMAPFILEHEADER pBmpHead) {
	cout << "\nλͼ�ļ�ͷ:" << endl;
	cout << "�ļ���С:" << pBmpHead.bfSize << endl;
	cout << "������_1:" << pBmpHead.bfReserved1 << endl;
	cout << "������_2:" << pBmpHead.bfReserved2 << endl;
	cout << "ʵ��λͼ���ݵ�ƫ���ֽ���:" << pBmpHead.bfOffBits << endl << endl;
}
//��ʾλͼ��Ϣͷ��Ϣ  
void showBmpInforHead(BITMAPINFOHEADER pBmpInforHead) {
	cout << "\nλͼ��Ϣͷ:" << endl;
	cout << "�ṹ��ĳ���:" << pBmpInforHead.biSize << endl;
	cout << "λͼ��:" << pBmpInforHead.biWidth << endl;
	cout << "λͼ��:" << pBmpInforHead.biHeight << endl;
	cout << "biPlanesƽ����:" << pBmpInforHead.biPlanes << endl;
	cout << "biBitCount������ɫλ��:" << pBmpInforHead.biBitCount << endl;
	cout << "ѹ����ʽ:" << pBmpInforHead.biCompression << endl;
	cout << "biSizeImageʵ��λͼ����ռ�õ��ֽ���:" << pBmpInforHead.biSizeImage << endl;
	cout << "X����ֱ���:" << pBmpInforHead.biXPelsPerMeter << endl;
	cout << "Y����ֱ���:" << pBmpInforHead.biYPelsPerMeter << endl;
	cout << "ʹ�õ���ɫ��:" << pBmpInforHead.biClrUsed << endl;
	cout << "��Ҫ��ɫ��:" << pBmpInforHead.biClrImportant << endl;
}

//-----------------------------------------------------------------------------------------
//����һ��ͼ��λͼ���ݡ������ߡ���ɫ��ָ�뼰ÿ������ռ��λ������Ϣ,����д��ָ���ļ���
bool readBmp(const char* bmpName)
{
	FILE* fp ;//�����ƶ���ʽ��ָ����ͼ���ļ�
	fopen_s(&fp, bmpName, "rb");
	if (fp == 0)
		return 0;

	//����λͼ�ļ�ͷ�ṹBITMAPFILEHEADER
	fseek(fp, sizeof(BITMAPFILEHEADER), 0);
	/*
	BITMAPFILEHEADER filehead;
	fread(&filehead, 1, sizeof(BITMAPFILEHEADER), fp);
	showBmpHead(filehead);//��ʾ�ļ�ͷ
*/

//����λͼ��Ϣͷ�ṹ��������ȡλͼ��Ϣͷ���ڴ棬����ڱ���head��
	BITMAPINFOHEADER infohead;
	fread(&infohead, sizeof(BITMAPINFOHEADER), 1, fp); //��ȡͼ������ߡ�ÿ������ռλ������Ϣ
	bmpWidth = infohead.biWidth;
	bmpHeight = infohead.biHeight;
	biBitCount = infohead.biBitCount;//�������������ͼ��ÿ��������ռ���ֽ�����������4�ı�����
	//showBmpInforHead(infohead);//��ʾ��Ϣͷ 


	int lineByte = (bmpWidth * biBitCount / 8 + 3) / 4 * 4;//�Ҷ�ͼ������ɫ��������ɫ������Ϊ256
	if (biBitCount == 8)
	{
		//������ɫ������Ҫ�Ŀռ䣬����ɫ�����ڴ�
		pColorTable = new RGBQUAD[256];
		fread(pColorTable, sizeof(RGBQUAD), 256, fp);
	}

	//����λͼ��������Ҫ�Ŀռ䣬��λͼ���ݽ��ڴ�
	pBmpBuf = new unsigned char[lineByte * bmpHeight];
	fread(pBmpBuf, 1, lineByte * bmpHeight, fp);
	fclose(fp);//�ر��ļ�
	return 1;//��ȡ�ļ��ɹ�
}





//������ʾ�����ȫ�ֱ���
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

		//չʾͼƬ
		showPic("temp.bmp", BmpInode->Name);

		return true;
	}
	else return false;
}