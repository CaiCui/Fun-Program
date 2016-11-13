#include<Windows.h>//菜单： 工具->代码段管理器 就可以选择语言来自动补全代码了 工具”-“选项”-文本编辑器-行号，可以调出行号
#include<string>
#define ID_TIMER 1001
#define STRMAXLEN 30 //每列字符串最长30
#define STRMINLEN 12 
//链表节点
typedef struct tarCharChain{
	struct tarCharChain *prev;
	TCHAR ch;
	struct tarCharChain * next;
}CharChain, *pCharChain;

//链表,保存一列显示的数据，双向链表
typedef struct tagCharColumn{
	CharChain *head, *current, *point;//point保存前一个节点
	int x, y, iStrlen;
	int iStopTimes, iMustStopTimes;
}CharColumn, *pCharColumn;

TCHAR randomChar()
{
	return (TCHAR)(rand() % (126 - 33) + 33);//33~126是可见字符 (126-33)+33
}
//初始化一列字符串。
//参数：cc	链表指针 
void init(CharColumn *cc, int cyScreen, int x)
{
	int j;
	cc->iStrlen = rand() % (STRMAXLEN - STRMINLEN) + STRMINLEN;//产生随机字符串长度
	cc->x = x + 3; //左上角(0,0) 横的是x,竖的是y
	cc->y = rand() % 3 ? rand() % cyScreen : 0;
	cc->iMustStopTimes = rand() % 6;
	cc->iStopTimes = 0;
	//动态申请内存，保存整列每个元素字符,就是申请了一个iStrlen长度的CharChain类型内存空间
	cc->head = cc->current = (pCharChain)calloc(cc->iStrlen, sizeof(CharChain));
	//将动态申请的节点全部初始化为链表
	for (j = 0; j<cc->iStrlen-1; j++)
	{
		cc->current->prev = cc->point;//初始为null,当前节点的前向指针指向上一个节点
		cc->current->ch = 0;  //全部初始化为0字符
		cc->current->next = cc->current + 1;//当前节点后向指针指向一个节点
		cc->point = cc->current++;//当前节点指针后移,point保存下一个节点的前一个节点，就是当前节点
	}
	//最后一个指针处理
	cc->current->prev = cc->point;
	cc->current->ch = 0;
	cc->current->next = cc->head;
	cc->head->prev = cc->current;

	cc->current = cc->point = cc->head;
	cc->head->ch = randomChar();
	return;

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)//回调函数
{
	HDC hdc;//获取DC用于绘图
	INT i, j, temp, ctn;
	static HDC hdcMem;
	HFONT hFont;
	static HBITMAP hBitmap;
	static int cxScreen, cyScreen;
	static int iFontWidth = 10, iFontHeight = 15, iColumnCount;
	static CharColumn *ccChain;//动态分配一个行链表数组，申请内存后每个元素都是CharColumn类型和int *p相似 
	static TCHAR *szlogo = TEXT("3333");
	switch (message)
	{
	case WM_CREATE:
		cxScreen = GetSystemMetrics(SM_CXSCREEN);
		cyScreen = GetSystemMetrics(SM_CYSCREEN);
		//实现一个定时器，用于下雨效果
		SetTimer(hWnd, ID_TIMER, 10, NULL);
		//创建DC
		hdc = GetDC(hWnd);
		hdcMem = CreateCompatibleDC(hdc);
		hBitmap = CreateCompatibleBitmap(hdc, cxScreen, cyScreen);
		SelectObject(hdcMem, hBitmap);
		ReleaseDC(hWnd, hdc);
		//创建字体
		hFont = CreateFont(iFontHeight, iFontWidth - 4, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, FIXED_PITCH | FF_SWISS, TEXT("ZS FIXED"));
		SelectObject(hdcMem, hFont);//字体选入内存DC
		DeleteObject(hFont);
		SetBkMode(hdcMem, TRANSPARENT); //设置背景透明

		iColumnCount = cxScreen / (iFontWidth * 3 / 2); //按屏幕宽度计算应该要出现多少列字符

		ccChain = (pCharColumn)calloc(iColumnCount, sizeof(CharColumn));//分配所有列的链表头指针,calloc是连续空间内存分配，在动态分配完内存后，自动初始化该内存空间为零，而malloc不初始化，里边数据是随机的垃圾数据。
		//循环初始化所有列链表
		for (i = 0; i<iColumnCount; i++)
		{
			init(ccChain + i, cyScreen, (iFontWidth * 3 / 2)*i);
		}
		break;
	case WM_TIMER:
		//动态显示
		hdc = GetDC(hWnd);
		PatBlt(hdcMem, 0, 0, cxScreen, cyScreen, BLACKNESS);//绘制黑色背景
		//循环绘制屏幕字符列
		for (i = 0; i<iColumnCount; i++)
		{
			ctn = (ccChain + i)->iStopTimes++ >(ccChain + i)->iMustStopTimes;
			(ccChain + i)->point = (ccChain + i)->head; //通过point遍历全链表

			SetTextColor(hdcMem, RGB(255, 255, 255));//设置文件颜色为白色
			TextOut(hdcMem, (ccChain + i)->x, (ccChain + i)->y, &((ccChain + i)->point->ch), 1);//绘制链表当前第一个字符，呈现白色
			j = (ccChain + i)->y;
			(ccChain + i)->point = (ccChain + i)->point->next;
			//显示后面所有字符，需要显示成绿色，颜色渐变
			temp = 0;
			while ((ccChain + i)->point != (ccChain + i)->head && (ccChain + i)->point->ch)
			{
				SetTextColor(hdcMem, RGB(0, 255 - (255 * (temp++) / (ccChain + i)->iStrlen), 0));
				TextOut(hdcMem, (ccChain + i)->x, j -= iFontHeight, &((ccChain + i)->point->ch), 1);
				(ccChain + i)->point = (ccChain + i)->point->next;
			}
			if (ctn)
				(ccChain + i)->iStopTimes = 0;
			else
				continue;
			//更新下次开始显示的y的位置
			(ccChain + i)->y += iFontHeight;
			//如果字符串已经超过屏幕下边界，链表内存释放，并重新申请新链表
			if ((ccChain + i)->y - (ccChain + i)->iStrlen * iFontHeight > cyScreen)
			{
				free((ccChain + i)->current);
				init(ccChain + i, cyScreen, (iFontWidth * 3 / 2)*i);
			}
			(ccChain + i)->head = (ccChain + i)->head->prev;
			(ccChain + i)->head->ch = randomChar();

		}
		SetTextColor(hdcMem, RGB(255, 0, 0));//LOGO红色
		TextOut(hdcMem, cxScreen/2, cyScreen/2, szlogo, 4);//绘制链表当前第一个字符，呈现白色
		//已经绘制完所有字符列到内存DC
		//需要将DC绘制到屏幕
		BitBlt(hdc, 0, 0, cxScreen, cyScreen, hdcMem, 0, 0, SRCCOPY);
		ReleaseDC(hWnd, hdc);
		break;
	case WM_RBUTTONDOWN://右键按下，暂停画面
		KillTimer(hWnd, ID_TIMER);
		break;
	case WM_RBUTTONUP://右键松开，重设定时器
		SetTimer(hWnd, ID_TIMER, 10, NULL);
		break;
		//当有鼠标点击或按键发生，直接退出程序
	case WM_KEYUP:
	case WM_LBUTTONUP:
	case WM_DESTROY:
		KillTimer(hWnd, ID_TIMER);//关闭定时器
		DeleteObject(hBitmap);
		DeleteDC(hdcMem);
		for (i = 0; i<iColumnCount; i++)
		{
			free((ccChain + i)->current);
		}
		free(ccChain);
		PostQuitMessage(0);
		break;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	static TCHAR *szAppName = TEXT("IS Edu");
	HWND hWnd;
	MSG msg;
	WNDCLASS wndClass;

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = szAppName;

	if (!RegisterClass(&wndClass))
	{
		MessageBox(NULL, TEXT("窗口注册失败"), szAppName, MB_OK | MB_ICONERROR);
		return 0;
	}
	//创建一个大小为屏幕的窗口
	hWnd = CreateWindow(szAppName, NULL, WS_DLGFRAME | WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, SW_SHOWMAXIMIZED); //最大化显示窗口
	UpdateWindow(hWnd);
	ShowCursor(FALSE);//隐藏鼠标

	//创建随机数
	srand((int)GetCurrentTime());
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	ShowCursor(TRUE);//隐藏鼠标
	return 0;
}