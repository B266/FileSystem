#include "simple_vim.h"
using namespace std;

void gotoxy(HANDLE hOut, int x, int y)	//�ƶ���굽ָ��λ��
{
	COORD pos;
	pos.X = x;             //������
	pos.Y = y;            //������
	SetConsoleCursorPosition(hOut, pos);
}

void vim(inode* nowPath, char* fileName) {
	// ��ȡĿ���ļ�Inode
	inode* fileInode = getInodeByPathName(fileName, nowPath);
	inode* fatherFolderInode = getInodeByPathName(fileName, nowPath, 2);
	File* openFile;
	// �ж��ļ��Ƿ����
	if (fatherFolderInode == NULL) {
		cout << "vim: �޷���\"" << fileName << "\": û���Ǹ��ļ���Ŀ¼" << endl;
		return;
	}
	// �½��ļ�
	if (fileInode == NULL && fatherFolderInode != NULL) {
		int indexInode = GetAInode();
		char FileName[NameLen] = { 0 };
		char ExtensionName[NameLen] = { 0 };
		GetFileNameAndExtensionName(fileName, FileName, ExtensionName);
		//��дinode
		strcpy_s(Inode[indexInode].Name, FileName);
		strcpy_s(Inode[indexInode].ExtensionName, ExtensionName);
		strcpy_s(Inode[indexInode].username, NowUser);
		strcpy_s(Inode[indexInode].usergroupname, NowGroupName);
		Inode[indexInode].size = 0;
		Inode[indexInode].permissions = 666;

		//�޸��ϼ�Ŀ¼
		AddItemInFolder(fatherFolderInode, FileName, indexInode);

		fileInode = &Inode[indexInode];
	}
	
	openFile = OpenFile(disk, fileInode);

	//cout << openFile->data << endl;
	int maxlen = 0; // �ﵽ������󳤶�
	
	char buf[99999];	//���100K

	//��ʼ��vi
	int cnt = 0;
	system("cls");	//����

	int winx, winy, curx, cury;

	HANDLE handle_out;                              //����һ�����  
	CONSOLE_SCREEN_BUFFER_INFO screen_info;         //���崰�ڻ�������Ϣ�ṹ��  
	COORD pos = { 0, 0 };                             //����һ������ṹ��

	//���ļ����ݶ�ȡ��������ʾ�ڣ�������
	int i = 0;
	int sumlen = fileInode->size;	//�ļ�����

	cnt = strlen(openFile->data);
	strcpy_s(buf, openFile->data);
	maxlen = sumlen;

	//������֮��Ĺ��λ��
	handle_out = GetStdHandle(STD_OUTPUT_HANDLE);   //��ñ�׼����豸���  
	GetConsoleScreenBufferInfo(handle_out, &screen_info);   //��ȡ������Ϣ  
	winx = screen_info.srWindow.Right - screen_info.srWindow.Left + 1;
	winy = screen_info.srWindow.Bottom - screen_info.srWindow.Top + 1;
	curx = screen_info.dwCursorPosition.X;
	cury = screen_info.dwCursorPosition.Y;

	//����vi
	//����vi��ȡ�ļ�����

	int mode = 0;	//viģʽ��һ��ʼ������ģʽ
	unsigned char c;
	while (1) {
		if (mode == 0) {	//������ģʽ
			c = _getch();

			if (c == 'i' || c == 'a') {	//����ģʽ
				if (c == 'a') {
					curx++;
					if (curx == winx) {
						curx = 0;
						cury++;

						/*
						if(cury>winy-2 || cury%(winy-1)==winy-2){
							//������һ�������·�ҳ
							if(cury%(winy-1)==winy-2)
								printf("\n");
							SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
							int i;
							for(i=0;i<winx-1;i++)
								printf(" ");
							gotoxy(handle_out,0,cury+1);
							printf(" - ����ģʽ - ");
							gotoxy(handle_out,0,cury);
						}
						*/
					}
				}

				if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
					//������һ�������·�ҳ
					if (cury % (winy - 1) == winy - 2)
						printf("\n");
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, cury + 1);
					printf(" - ����ģʽ - ");
					gotoxy(handle_out, 0, cury);
				}
				else {
					//��ʾ "����ģʽ"
					gotoxy(handle_out, 0, winy - 1);
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, winy - 1);
					printf(" - ����ģʽ - ");
					gotoxy(handle_out, curx, cury);
				}

				gotoxy(handle_out, curx, cury);
				mode = 1;


			}
			else if (c == ':') {
				//system("color 09");//�����ı�Ϊ��ɫ
				if (cury - winy + 2 > 0)
					gotoxy(handle_out, 0, cury + 1);
				else
					gotoxy(handle_out, 0, winy - 1);
				_COORD pos;
				if (cury - winy + 2 > 0)
					pos.X = 0, pos.Y = cury + 1;
				else
					pos.X = 0, pos.Y = winy - 1;
				SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
				int i;
				for (i = 0; i < winx - 1; i++)
					printf(" ");

				if (cury - winy + 2 > 0)
					gotoxy(handle_out, 0, cury + 1);
				else
					gotoxy(handle_out, 0, winy - 1);

				WORD att = BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_INTENSITY; // �ı�����
				//FillConsoleOutputAttribute(handle_out, att, winx, pos, NULL);	//����̨������ɫ 
				//SetConsoleTextAttribute(handle_out, FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_GREEN);	//�����ı���ɫ
				printf(":");

				char pc;
				int tcnt = 1;	//������ģʽ������ַ�����
				while (c = _getch()) {
					if (c == '\r') {	//�س�
						break;
					}
					else if (c == '\b') {	//�˸񣬴�������ɾ��һ���ַ� 
						//SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
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
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					system("cls");
					break;	//vi >>>>>>>>>>>>>> �˳�����
				}
				else {
					if (cury - winy + 2 > 0)
						gotoxy(handle_out, 0, cury + 1);
					else
						gotoxy(handle_out, 0, winy - 1);
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");

					if (cury - winy + 2 > 0)
						gotoxy(handle_out, 0, cury + 1);
					else
						gotoxy(handle_out, 0, winy - 1);
					//SetConsoleTextAttribute(handle_out, FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_GREEN);	//�����ı���ɫ
					//FillConsoleOutputAttribute(handle_out, att, winx, pos, NULL);	//����̨������ɫ
					printf(" ��������");
					//getch();
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					gotoxy(handle_out, curx, cury);
				}
			}
			else if (c == 27) {	//ESC��������ģʽ����״̬��
				gotoxy(handle_out, 0, winy - 1);
				SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
				int i;
				for (i = 0; i < winx - 1; i++)
					printf(" ");
				gotoxy(handle_out, curx, cury);

			}

		}
		else if (mode == 1) {	//����ģʽ

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
			printf("[��:%d,��:%d]", curx == -1 ? 0 : curx, cury);
			gotoxy(handle_out, curx, cury);

			c = _getch();
			if (c == 27) {	// ESC����������ģʽ
				mode = 0;
				//��״̬��
				gotoxy(handle_out, 0, winy - 1);
				SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
				int i;
				for (i = 0; i < winx - 1; i++)
					printf(" ");
				continue;
			}
			else if (c == '\b') {	//�˸�ɾ��һ���ַ�
				if (cnt == 0)	//�Ѿ��˵��ʼ
					continue;
				printf("\b");
				printf(" ");
				printf("\b");
				curx--;
				cnt--;	//ɾ���ַ�
				if (buf[cnt] == '\n') {
					//Ҫɾ��������ַ��ǻس������ص���һ��
					if (cury != 0)
						cury--;
					int k;
					curx = 0;
					for (k = cnt - 1; buf[k] != '\n' && k >= 0; k--)
						curx++;
					gotoxy(handle_out, curx, cury);
					printf(" ");
					gotoxy(handle_out, curx, cury);
					if (cury - winy + 2 >= 0) {	//��ҳʱ
						gotoxy(handle_out, curx, 0);
						gotoxy(handle_out, curx, cury + 1);
						SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
						int i;
						for (i = 0; i < winx - 1; i++)
							printf(" ");
						gotoxy(handle_out, 0, cury + 1);
						printf(" - ����ģʽ - ");

					}
					gotoxy(handle_out, curx, cury);

				}
				else
					buf[cnt] = ' ';
				continue;
			}
			else if (c == 224) {	//�ж��Ƿ��Ǽ�ͷ
				c = _getch();
				if (c == 75) {	//���ͷ
					if (cnt != 0) {
						cnt--;
						curx--;
						if (buf[cnt] == '\n') {
							//��һ���ַ��ǻس�
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
				else if (c == 77) {	//�Ҽ�ͷ
					cnt++;
					if (cnt > maxlen)
						maxlen = cnt;
					curx++;
					if (curx == winx) {
						curx = 0;
						cury++;

						if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
							//������һ�������·�ҳ
							if (cury % (winy - 1) == winy - 2)
								printf("\n");
							SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
							int i;
							for (i = 0; i < winx - 1; i++)
								printf(" ");
							gotoxy(handle_out, 0, cury + 1);
							printf(" - ����ģʽ - ");
							gotoxy(handle_out, 0, cury);
						}

					}
					gotoxy(handle_out, curx, cury);
				}
				continue;
			}
			if (c == '\r') {	//�����س�
				printf("\n");
				curx = 0;
				cury++;

				if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
					//������һ�������·�ҳ
					if (cury % (winy - 1) == winy - 2)
						printf("\n");
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, cury + 1);
					printf(" - ����ģʽ - ");
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
			//�ƶ����
			curx++;
			if (curx == winx) {
				curx = 0;
				cury++;

				if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
					//������һ�������·�ҳ
					if (cury % (winy - 1) == winy - 2)
						printf("\n");
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, cury + 1);
					printf(" - ����ģʽ - ");
					gotoxy(handle_out, 0, cury);
				}

				buf[cnt++] = '\n';
				if (cnt > maxlen)
					maxlen = cnt;
				if (cury == winy) {
					printf("\n");
				}
			}
			//��¼�ַ� 
			buf[cnt++] = c;
			if (cnt > maxlen)
				maxlen = cnt;
		}
		else {	//����ģʽ
		}
	}

	// ��buf����д���ļ��Ĵ��̿�
	SaveFileData(disk, fileInode, buf, strlen(buf));
}
