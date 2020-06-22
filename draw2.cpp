// draw.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "draw2.h"
#include <string>
#include <vector>
#include <queue>
#include <cstdio>
#include <cmath>

#define MAX_LOADSTRING 100
#define TMR_1 1
#define TMR_2 2
#define TMR_3 3
#define TMR_4 4

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// buttons
HWND hwndButton;
HWND hwndGrabLose;
HWND hwndEventLog;
HWND hwndYellowWeight;
HWND hwndGreenWeight;
HWND hwndBlueWeight;
HWND hwndRedWeight;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Buttons(HWND, UINT, WPARAM, LPARAM);

//sizes & constants
const int wnd_width = 1600;
const int wnd_height = 900;
const int btn_width = 200;
const int btn_height = 50;
const int spl_btn_width = 75;
const int ground = wnd_height - btn_height;

const double pi = 3.1415926535;
const double step = 0.0087266462; //po³owa stopnia
const double initPos = - pi / 2;

const int acceleration = 1;
const int grab_dist = 50;

//draw areas
RECT drawArea = { btn_width, 2 * btn_height, wnd_width, wnd_height };
RECT actionArea = { btn_width, 2 * btn_height, wnd_width, wnd_height - btn_height};

//robot info
double speed = 1;
const int arm_length = 350;
bool grabbed = 0;
double alpha = 0, beta = 0;
const int fix_x = btn_width + 100 + 8 * spl_btn_width;
const int fix_y = ground - btn_height;
int joint_x;
int joint_y;
int hand_x;
int hand_y;

//box info
struct box
{
	std::string name;
	bool grabbed = false;
	int grip_x = 0;
	int grip_y = 0;
	int weight = 0;
	int velocity = 0;
};

box yellow_box, green_box, blue_box, red_box;

//vectors,arrays,queues
std::vector<box> boxes;
double distances[4];
std::queue<int> auto_queue;
std::queue<int> record_queue;

//other
int control = 0; //0 - stop, 1 - alpha_right, 2 - alpha_left, 3 - beta_right, 4 - beta_left, 5 - grabbing

int auto_state = 0; //0 - start, 9 - default position 
					//1 - pick yellow, 2 - pick green, 3 - pick blue, 4 - pick red
					//5 - drop as first, 6 - drop as second, 7 - drop as third, 8 - drop as fourth

int program_state = 0; //0 - waiting, 1 - manual, 2 - auto, 3 - recording, 4 - playing

void changeSpeed(HWND hWnd, int new_speed)
{
	speed = new_speed;
	if (control != 0)
	{
		KillTimer(hWnd, TMR_1);
		SetTimer(hWnd, TMR_1, speed, 0);
	}
}

void showText(HWND hwnd ,std::string text)
{
	SetWindowTextA(hwnd, text.c_str());
}

void defineBoxes()
{
	yellow_box.name = "yellow";
	yellow_box.weight = 9;

	green_box.name = "green";
	green_box.weight = 6;
	
	blue_box.name = "blue";
	blue_box.weight = 15;
	
	red_box.name = "red";
	red_box.weight = 14;
}

void resetBoxesPosition()
{
	yellow_box.grabbed = false;
	yellow_box.grip_x = btn_width + spl_btn_width;
	yellow_box.grip_y = ground - btn_height;

	green_box.grabbed = false;
	green_box.grip_x = btn_width + 3 * spl_btn_width;
	green_box.grip_y = ground - btn_height;

	blue_box.grabbed = false;
	blue_box.grip_x = btn_width + 5 * spl_btn_width;
	blue_box.grip_y = ground - btn_height;

	red_box.grabbed = false;
	red_box.grip_x = btn_width + 7 * spl_btn_width;
	red_box.grip_y = ground - btn_height;

	SetWindowText(hwndGrabLose, L"CHWYÆ");
}

void createVector()
{
	boxes.clear();
	boxes.push_back(yellow_box);
	boxes.push_back(green_box);
	boxes.push_back(blue_box);
	boxes.push_back(red_box);
}

void sortVector()
{
	for (int i = boxes.size() - 1; i > 0; i--)
	{
		for (int j = 0; j < i; j++)
		{
			if (boxes[j].weight > boxes[j + 1].weight)
			{
				box temp = boxes[j];
				boxes[j] = boxes[j + 1];
				boxes[j + 1] = temp;
			}
		}
	}
}

void createQueue()
{
	std::queue<int> empty;
	std::swap(auto_queue, empty);
	sortVector();
	
	for (int i = 0; i < boxes.size(); i++)
	{
		if (boxes[i].name == "yellow")
		{
			auto_queue.push(1);
		}
		else if (boxes[i].name == "green")
		{
			auto_queue.push(2);
		}
		else if (boxes[i].name == "blue")
		{
			auto_queue.push(3);
		}
		else if (boxes[i].name == "red")
		{
			auto_queue.push(4);
		}
		auto_queue.push(5 + i);
	}
	auto_queue.push(9);
}

double distance(box box)
{
	double distance;
	distance = sqrt(pow(box.grip_x - hand_x,2)+pow(box.grip_y - hand_y,2));
	return distance;
}

void getDistances()
{
	distances[0] = distance(yellow_box);
	distances[1] = distance(green_box);
	distances[2] = distance(blue_box);
	distances[3] = distance(red_box);
}

int findClosest()
{
	getDistances();
	int n = 0;

	for (int i = 1; i <= 3; i++)
	{
		if (distances[n] > distances[i])
		{
			n = i;
		}
	}
	return n;
}

void moveBox()
{
	if (yellow_box.grabbed)
	{
		yellow_box.grip_x = hand_x;
		yellow_box.grip_y = hand_y;
	}
	else if (green_box.grabbed)
	{
		green_box.grip_x = hand_x;
		green_box.grip_y = hand_y;
	}
	else if (blue_box.grabbed)
	{
		blue_box.grip_x = hand_x;
		blue_box.grip_y = hand_y;
	}
	else if (red_box.grabbed)
	{
		red_box.grip_x = hand_x;
		red_box.grip_y = hand_y;
	}
}

void falling(HWND hWnd)
{
	if (yellow_box.grip_y + btn_height <= ground && !yellow_box.grabbed)
	{
		if (yellow_box.grip_y + yellow_box.velocity + btn_height > ground)
		{
			yellow_box.grip_y = ground - btn_height;
			yellow_box.velocity = 0;
		}
		else
		{
			yellow_box.grip_y = yellow_box.grip_y + yellow_box.velocity;
			yellow_box.velocity += acceleration;
		}
	}
	if (green_box.grip_y + btn_height <= ground && !green_box.grabbed)
	{
		if (green_box.grip_y + green_box.velocity + btn_height > ground)
		{
			green_box.grip_y = ground - btn_height;
			green_box.velocity = 0;
		}
		else
		{
			green_box.grip_y = green_box.grip_y + green_box.velocity;
			green_box.velocity += acceleration;
		}
	}
	if (blue_box.grip_y + btn_height <= ground && !blue_box.grabbed)
	{
		if (blue_box.grip_y + blue_box.velocity + btn_height > ground)
		{
			blue_box.grip_y = ground - btn_height;
			blue_box.velocity = 0;
		}
		else
		{
			blue_box.grip_y = blue_box.grip_y + blue_box.velocity;
			blue_box.velocity += acceleration;
		}
	}
	if (red_box.grip_y + btn_height <= ground && !red_box.grabbed)
	{
		if (red_box.grip_y + red_box.velocity + btn_height > ground)
		{
			red_box.grip_y = ground - btn_height;
			red_box.velocity = 0;
		}
		else
		{
			red_box.grip_y = red_box.grip_y + red_box.velocity;
			red_box.velocity += acceleration;
		}
	}
	if (yellow_box.grip_y + btn_height == ground && green_box.grip_y + btn_height == ground && blue_box.grip_y + btn_height == ground && red_box.grip_y + btn_height == ground)
	{
		KillTimer(hWnd, TMR_2);
	}
}

void attach_detachBox(HWND hWnd)
{
	if (yellow_box.grip_y != ground - btn_height || green_box.grip_y != ground - btn_height || blue_box.grip_y != ground - btn_height || red_box.grip_y != ground - btn_height)
	{
		SetTimer(hWnd, TMR_2, 20, 0);
	}

	if (program_state == 3)
	{
		record_queue.push(5);
	}

	switch (findClosest())
	{
	case 0:
		yellow_box.grabbed = !yellow_box.grabbed;
		break;
	case 1:
		green_box.grabbed = !green_box.grabbed;
		break;
	case 2:
		blue_box.grabbed = !blue_box.grabbed;
		break;
	case 3:
		red_box.grabbed = !red_box.grabbed;
		break;
	}
	grabbed = !grabbed;
}

#pragma region automat

void initialPosition(HWND hWnd)
{
	if ((alpha >= initPos - step && alpha <= initPos + step && beta >= initPos - step && beta <= initPos + step) && auto_queue.empty())
	{
		showText(hwndEventLog, "Zakoñczono pracê.");
		program_state = 0;
		KillTimer(hWnd, TMR_3);
		return;
	}

	if (alpha > initPos)
	{
		alpha -= step;
	}
	else if (alpha < initPos)
	{
		alpha += step;
	}

	if (beta > initPos)
	{
		beta -= step;
	}
	else if (beta < initPos)
	{
		beta += step;
	}
	moveBox();
}

bool ifPicked(box box, HWND hWnd)
{
	if (distance(box) <= grab_dist)
	{
		attach_detachBox(hWnd);
		if (auto_queue.size() != 0)
		{
			auto_state = auto_queue.front();
			auto_queue.pop();
		}
		else
		{
			auto_state = 9;
		}
		return true;
	}
	else
	{
		return false;
	}
}

void pickYellow(HWND hWnd)
{
	if ( ifPicked(yellow_box, hWnd) )
	{
		return;
	}

	if ( joint_x > yellow_box.grip_x + ( fix_x - yellow_box.grip_x ) / 2 )
	{
		alpha -= step;
	}

	if ( hand_y < ground - btn_height )
	{
		beta -= step;
	}
}

void pickGreen(HWND hWnd)
{
	if (ifPicked(green_box, hWnd))
	{
		return;
	}

	if (joint_x > green_box.grip_x + (fix_x - green_box.grip_x) / 2)
	{
		alpha -= step;
	}

	if (hand_y < ground - btn_height)
	{
		beta -= step;
	}
}

void pickBlue(HWND hWnd)
{
	if (ifPicked(blue_box, hWnd))
	{
		return;
	}

	if ( joint_x > blue_box.grip_x + ( fix_x - blue_box.grip_x ) / 2 )
	{
		alpha -= step;
	}

	if ( hand_y < ground - btn_height )
	{
		beta -= step;
	}
}

void pickRed(HWND hWnd)
{
	if (ifPicked(red_box, hWnd))
	{
		return;
	}

	if ( joint_x > red_box.grip_x + ( fix_x - red_box.grip_x ) / 2 )
	{
		alpha -= step;
	}

	if ( hand_y < ground - btn_height )
	{
		beta -= step;
	}
}

bool ifDropable(HWND hWnd)
{
	if (arm_length * sin(step) + hand_y >= -btn_height + ground)
	{
		attach_detachBox(hWnd);
		if (auto_queue.size() != 0)
		{
			auto_state = auto_queue.front();
			auto_queue.pop();
		}
		else
		{
			auto_state = 9;
		}
		return true;
	}
	else
	{
		return false;
	}
}

void moveToDrop(int position)
{
	if (joint_x < fix_x + (position - fix_x) / 2)
	{
		alpha += step;
	}

	if (hand_y < ground - btn_height)
	{
		beta += step;
	}

	moveBox();
}

void dropAsFirst(HWND hWnd)
{
	int position = fix_x + btn_width / 2 + spl_btn_width;

	if ( ifDropable(hWnd) )
	{
		return;
	}

	moveToDrop(position);
}

void dropAsSecond(HWND hWnd)
{
	int position = fix_x + btn_width / 2 + 3 * spl_btn_width;

	if (ifDropable(hWnd))
	{
		return;
	}

	moveToDrop(position);
}

void dropAsThird(HWND hWnd)
{
	int position = fix_x + btn_width / 2 + 5 * spl_btn_width;

	if (ifDropable(hWnd))
	{
		return;
	}

	moveToDrop(position);
}

void dropAsFourth(HWND hWnd)
{
	int position = fix_x + btn_width / 2 + 7 * spl_btn_width;

	if (ifDropable(hWnd))
	{
		return;
	}

	moveToDrop(position);
}

#pragma endregion automat

void playing(HWND hWnd)
{
	switch (record_queue.front())
	{
	case 1://alpha_left
		alpha += step;
		beta += step;
		break;
	case 2://alpha_right
		alpha -= step;
		beta -= step;
		break;
	case 3://beta_left
		beta += step;
		break;
	case 4://beta_right
		beta -= step;
		break;
	case 5://grab
		attach_detachBox(hWnd);
		break;
	}
	record_queue.pop();
}

void resetEverything(HWND hWnd)
{
	program_state = 0;
	KillTimer(hWnd, TMR_1);
	KillTimer(hWnd, TMR_2);
	KillTimer(hWnd, TMR_3);
	KillTimer(hWnd, TMR_4);
	control = 0;
	speed = 1;
	alpha = initPos;
	beta = initPos;
	resetBoxesPosition();
	createVector();
	createQueue();
	getDistances();
}

void MyOnPaint(HDC hdc)
{
	Graphics graphics(hdc);

	Color black(255, 0, 0, 0);
	Color yellow(255, 255, 255, 0);
	Color green(255, 0, 255, 0);
	Color blue(255, 0, 0, 255);
	Color red(255, 255, 0, 0);

	Pen markerYELLOW(yellow, spl_btn_width);
	Pen markerGREEN(green, spl_btn_width);
	Pen markerBLUE(blue, spl_btn_width);
	Pen markerRED(red, spl_btn_width);

	Pen penBLACK(black, 9);
	Pen penYELLOW(yellow, 9);
	Pen penGREEN(green, 9);
	Pen penBLUE(blue, 9);
	Pen penRED(red, 9);

	Pen pencilBLACK(black, 1);
	Pen pencilYELLOW(yellow, 1);
	Pen pencilGREEN(green, 1);
	Pen pencilBLUE(blue, 1);
	Pen pencilRED(red, 1);

	//dynamic elements

	joint_x = fix_x + arm_length * cos(alpha);
	joint_y = fix_y + arm_length * sin(alpha);
	hand_x = joint_x + arm_length * cos(beta);
	hand_y = joint_y + arm_length * sin(beta);

	graphics.DrawLine(&penBLUE,
		fix_x,
		fix_y,
		joint_x,
		joint_y);
	graphics.DrawLine(&penRED,
		joint_x,
		joint_y,
		hand_x,
		hand_y);

	Rect joint_circle = { joint_x - 4, joint_y - 4, 8, 8 };
	graphics.DrawEllipse(&penGREEN, joint_circle);

	graphics.DrawLine(&markerYELLOW, yellow_box.grip_x, yellow_box.grip_y, yellow_box.grip_x, yellow_box.grip_y+btn_height);
	graphics.DrawLine(&markerGREEN, green_box.grip_x, green_box.grip_y, green_box.grip_x, green_box.grip_y + btn_height);
	graphics.DrawLine(&markerBLUE, blue_box.grip_x, blue_box.grip_y, blue_box.grip_x, blue_box.grip_y + btn_height);
	graphics.DrawLine(&markerRED, red_box.grip_x, red_box.grip_y, red_box.grip_x, red_box.grip_y + btn_height);

	//static elements
	graphics.DrawLine(&penBLACK, fix_x, fix_y, fix_x, ground - 1);

	Rect fix_circle = { fix_x - 4, fix_y - 4, 8, 8 };
	graphics.DrawEllipse(&penGREEN, fix_circle);

	graphics.DrawRectangle(&pencilYELLOW, btn_width, ground, 2 * spl_btn_width - 1, btn_height - 1);
	graphics.DrawRectangle(&pencilGREEN, btn_width + 2 * spl_btn_width, ground, 2 * spl_btn_width - 1, btn_height - 1);
	graphics.DrawRectangle(&pencilBLUE, btn_width + 4 * spl_btn_width, ground, 2 * spl_btn_width - 1, btn_height - 1);
	graphics.DrawRectangle(&pencilRED, btn_width + 6 * spl_btn_width, ground, 2 * spl_btn_width - 1, btn_height - 1);

	graphics.DrawRectangle(&pencilBLACK, btn_width + 8 * spl_btn_width, ground, btn_width - 1, btn_height - 1);

	graphics.DrawRectangle(&pencilBLACK, 2 * btn_width + 8 * spl_btn_width, ground, 2 * spl_btn_width - 1, btn_height - 1);
	graphics.DrawRectangle(&pencilBLACK, 2 * btn_width + 10 * spl_btn_width, ground, 2 * spl_btn_width - 1, btn_height - 1);
	graphics.DrawRectangle(&pencilBLACK, 2 * btn_width + 12 * spl_btn_width, ground, 2 * spl_btn_width - 1, btn_height - 1);
	graphics.DrawRectangle(&pencilBLACK, 2 * btn_width + 14 * spl_btn_width, ground, 2 * spl_btn_width - 1, btn_height - 1);
}

void repaintWindow(HWND hWnd, HDC &hdc, PAINTSTRUCT &ps, RECT *drawArea)
{
	if (drawArea==NULL)
		InvalidateRect(hWnd, NULL, TRUE); // repaint all
	else
		InvalidateRect(hWnd, drawArea, TRUE); //repaint drawArea
	hdc = BeginPaint(hWnd, &ps);
	MyOnPaint(hdc);
	EndPaint(hWnd, &ps);
}

int OnCreate(HWND window)
{
	return 0;
}

// main function (exe hInstance)
int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DRAW, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);



	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DRAW));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	GdiplusShutdown(gdiplusToken);

	return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DRAW));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_DRAW);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;


	hInst = hInstance; // Store instance handle (of exe) in our global variable

	// main window
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		50, 50, wnd_width + 15, wnd_height+60, NULL, NULL, hInstance, NULL);

	// buttons                                                      									

	hwndButton = CreateWindow(TEXT("button"),            // The class name required is button          
		TEXT("1/200"),								 // the caption of the button
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,			 // the styles
		0, 0,											 // the left and top co-ordinates
		btn_width - 1, btn_height - 1,					 // width and height
		hWnd,											 // parent window handle
		(HMENU)ID_FIRSTSPEED,                  			 // the ID of your button
		hInstance,										 // the instance of your application
		NULL);											 // extra bits you dont really need

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("1/100"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0, btn_height,
		btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_SECONDSPEED,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("1/50"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0, 2 * btn_height,
		btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_THIRDSPEED,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("1/25"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0, 3 * btn_height,
		btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_FOURTHSPEED,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("1/1"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0, 4 * btn_height,
		btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_FIFTHSPEED,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("NAGRYWAJ"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0, 5 * btn_height,
		btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_RECORD,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("ODTWÓRZ"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0, 6 * btn_height,
		btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_PLAY,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("AUTOMAT"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0, 7 * btn_height,
		btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_AUTO,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("RESET"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0, 8 * btn_height,
		btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_RESET,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("<---"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		btn_width, 0,
		btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_BETA_PLUS,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("<--"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		btn_width, btn_height,
		btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_ALPHA_PLUS,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("STOP"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		2 * btn_width, 0,
		btn_width - 1, 2 * btn_height - 1,
		hWnd,
		(HMENU)ID_STOP,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("--->"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		3 * btn_width, 0,
		btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_BETA_MINUS,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("-->"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		3 * btn_width, btn_height,
		btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_ALPHA_MINUS,
		hInstance,
		NULL);

	hwndGrabLose = CreateWindow(TEXT("button"),
		TEXT("CHWYÆ"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		4 * btn_width, 0,
		btn_width - 1, 2 * btn_height - 1,
		hWnd,
		(HMENU)ID_GRAB_LOSE,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("+"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		5 * btn_width, 0,
		spl_btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_YELLOW_PLUS,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("--"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		5 * btn_width + spl_btn_width, 0,
		spl_btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_YELLOW_MINUS,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("+"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		5 * btn_width + 2 * spl_btn_width, 0,
		spl_btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_GREEN_PLUS,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("--"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		5 * btn_width + 3 * spl_btn_width, 0,
		spl_btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_GREEN_MINUS,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("+"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		5 * btn_width + 4 * spl_btn_width, 0,
		spl_btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_BLUE_PLUS,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("--"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		5 * btn_width + 5 * spl_btn_width, 0,
		spl_btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_BLUE_MINUS,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("+"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		5 * btn_width + 6 * spl_btn_width, 0,
		spl_btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_RED_PLUS,
		hInstance,
		NULL);

	hwndButton = CreateWindow(TEXT("button"),
		TEXT("--"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		5 * btn_width + 7 * spl_btn_width, 0,
		spl_btn_width - 1, btn_height - 1,
		hWnd,
		(HMENU)ID_RED_MINUS,
		hInstance,
		NULL);

	// textboxes

	hwndEventLog = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"),
		NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
		0, 9 * btn_height, 
		btn_width - 1, wnd_height - 9 * btn_height - 1,
		hWnd,
		NULL,
		hInstance,
		NULL);

	hwndYellowWeight = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"),
		NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
		5 * btn_width, btn_height,
		2 * spl_btn_width - 1, btn_height - 1,
		hWnd,
		NULL,
		hInstance,
		NULL);

	hwndGreenWeight = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"),
		NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
		5 * btn_width + 2 * spl_btn_width, btn_height,
		2 * spl_btn_width - 1, btn_height - 1,
		hWnd,
		NULL,
		hInstance,
		NULL);

	hwndBlueWeight = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"),
		NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
		5 * btn_width + 4 * spl_btn_width, btn_height,
		2 * spl_btn_width - 1, btn_height - 1,
		hWnd,
		NULL,
		hInstance,
		NULL);

	hwndRedWeight = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"),
		NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER  | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
		5 * btn_width + 6 * spl_btn_width, btn_height,
		2 * spl_btn_width - 1, btn_height - 1,
		hWnd,
		NULL,
		hInstance,
		NULL);

	OnCreate(hWnd);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window (low priority)
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	
	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		// MENU & BUTTON messages
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_FIRSTSPEED:
			if (program_state == 0 || program_state == 3)
			{
				changeSpeed(hWnd, 200);
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniac prêdkoœci w czasie odtwarzania ruchu lub pracy automatycznej.");
			}
			break;
		case ID_SECONDSPEED:
			if (program_state == 0 || program_state == 3)
			{
				changeSpeed(hWnd, 100);
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniac prêdkoœci w czasie odtwarzania ruchu lub pracy automatycznej.");
			}
			break;
		case ID_THIRDSPEED:
			if (program_state == 0 || program_state == 3)
			{
				changeSpeed(hWnd, 50);
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniac prêdkoœci w czasie odtwarzania ruchu lub pracy automatycznej.");
			}
			break;
		case ID_FOURTHSPEED:
			if (program_state == 0 || program_state == 3)
			{
				changeSpeed(hWnd, 25);
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniac prêdkoœci w czasie odtwarzania ruchu lub pracy automatycznej.");
			}
			break;
		case ID_FIFTHSPEED:
			if (program_state == 0 || program_state == 3)
			{
				changeSpeed(hWnd, 1);
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniac prêdkoœci w czasie odtwarzania ruchu lub pracy automatycznej.");
			}
			break;
		case ID_RECORD:
			if (program_state == 0)
			{
				program_state = 3;
				alpha = initPos;
				beta = initPos;
				resetBoxesPosition();
				repaintWindow(hWnd, hdc, ps, &drawArea);
				showText(hwndEventLog, "Rozpoczêto nagrywanie");
			}
			else if (program_state == 3)
			{
				program_state = 0;
				showText(hwndEventLog, "Zakoñczono nagrywanie");
			}
			else if (program_state == 2)
			{
				showText(hwndEventLog, "Robot jest w trybie automatycznym. Zaczekaj a¿ skoñczy pracê lub wciœnij RESET.");
			}
			else if (program_state == 4)
			{
				showText(hwndEventLog, "Robot odtwarza wczeœniej nagrany ruch. Zaczekaj a¿ skoñczy pracê lub wciœnij RESET.");
			}
			break;
		case ID_PLAY:
			if (program_state == 0 || program_state == 3)
			{
				if (!record_queue.empty())
				{
					resetEverything(hWnd);
					resetBoxesPosition();
					repaintWindow(hWnd, hdc, ps, &drawArea);
					program_state = 4;
					SetTimer(hWnd, TMR_4, speed, 0);
					showText(hwndEventLog, "Rozpoczêto odtwarzanie.");
				}
				else
				{
					showText(hwndEventLog, "Nie ma nic do odtworzenia.");
				}
			}
			else if (program_state == 4)
			{
				showText(hwndEventLog, "Robot ju¿ odtwarza nagrany ruch.");
			}
			else if (program_state == 2)
			{
				showText(hwndEventLog, "Robot jest w trybie automatycznym. Zaczekaj a¿ skoñczy pracê lub wciœnij RESET.");
			}
			break;
		case ID_AUTO:
			if (program_state == 0)
			{
				auto_state = 0;
				resetEverything(hWnd);
				resetBoxesPosition();
				repaintWindow(hWnd, hdc, ps, &drawArea);
				program_state = 2;
				SetTimer(hWnd, TMR_3, speed, 0);
				showText(hwndEventLog, "Rozpoczêto sortowanie.");
			}
			else if (program_state == 2)
			{
				showText(hwndEventLog, "Robot jest ju¿ w trybie automatycznym.");
			}
			else if (program_state == 4)
			{
				showText(hwndEventLog, "Robot odtwarza nagrany ruch. Zaczekaj a¿ skoñczy pracê lub wciœnij RESET.");
			}
			break;
		case ID_RESET:
			resetEverything(hWnd);
			defineBoxes();
			showText(hwndYellowWeight, std::to_string(yellow_box.weight));
			showText(hwndGreenWeight, std::to_string(green_box.weight));
			showText(hwndBlueWeight, std::to_string(blue_box.weight));
			showText(hwndRedWeight, std::to_string(red_box.weight));
			repaintWindow(hWnd, hdc, ps, &drawArea);
			break;
		case ID_BETA_MINUS:
			if (program_state == 0 || program_state == 3)
			{
				if (control != 3)
				{
					KillTimer(hWnd, TMR_1);
				}
				control = 3;
				SetTimer(hWnd, TMR_1, speed, 0);
			}
			else
			{
				showText(hwndEventLog, "Zaczekaj a¿ robot skoñczy wykonywan¹ pracê lub wciœnij RESET.");
			}
			break;
		case ID_ALPHA_MINUS:
			if (program_state == 0 || program_state == 3)
			{
				if (control != 1)
				{
					KillTimer(hWnd, TMR_1);
				}
				control = 1;
				SetTimer(hWnd, TMR_1, speed, 0);
			}
			else
			{
				showText(hwndEventLog, "Zaczekaj a¿ robot skoñczy wykonywan¹ pracê lub wciœnij RESET.");
			}
			break;
		case ID_STOP:
			if (program_state == 0 || program_state == 3)
			{
				KillTimer(hWnd, TMR_1);
				control = 0;
			}
			else
			{
				showText(hwndEventLog, "Zaczekaj a¿ robot skoñczy wykonywan¹ pracê lub wciœnij RESET.");
			}
			break;
		case ID_BETA_PLUS:
			if (program_state == 0 || program_state == 3)
			{
				if (control != 4)
				{
					KillTimer(hWnd, TMR_1);
				}
				control = 4;
				SetTimer(hWnd, TMR_1, speed, 0);
			}
			else
			{
				showText(hwndEventLog, "Zaczekaj a¿ robot skoñczy wykonywan¹ pracê lub wciœnij RESET.");
			}
			break;
		case ID_ALPHA_PLUS:
			if (program_state == 0 || program_state == 3)
			{
				if (control != 2)
				{
					KillTimer(hWnd, TMR_1);
				}
				control = 2;
				SetTimer(hWnd, TMR_1, speed, 0);
			}
			else
			{
				showText(hwndEventLog, "Zaczekaj a¿ robot skoñczy wykonywan¹ pracê lub wciœnij RESET.");
			}
			break;
		case ID_GRAB_LOSE:
			if (program_state == 0 || program_state == 3)
			{
				if (grabbed)
				{
					attach_detachBox(hWnd);
					SetWindowText(hwndGrabLose, L"CHWYÆ");
				}
				else
				{
					if (distances[findClosest()] < grab_dist)
					{
						SetWindowText(hwndGrabLose, L"PUŒÆ");
						attach_detachBox(hWnd);
						moveBox();
						repaintWindow(hWnd, hdc, ps, &actionArea);
					}
				}
			}
			break;
		case ID_YELLOW_PLUS:
			if (program_state != 2)
			{
				yellow_box.weight++;
				createVector();
				createQueue();
				showText(hwndYellowWeight, std::to_string(yellow_box.weight));
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniaæ wag w trakcie sortowania.");
			}
			break;
		case ID_YELLOW_MINUS:
			if (program_state != 2)
			{
				yellow_box.weight--;
				createVector();
				createQueue();
				showText(hwndYellowWeight, std::to_string(yellow_box.weight));
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniaæ wag w trakcie sortowania.");
			}
			break;
		case ID_GREEN_PLUS:
			if (program_state != 2)
			{
				green_box.weight++;
				createVector();
				createQueue();
				showText(hwndGreenWeight, std::to_string(green_box.weight));
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniaæ wag w trakcie sortowania.");
			}
			break;
		case ID_GREEN_MINUS:
			if (program_state != 2)
			{
				green_box.weight--;
				createVector();
				createQueue();
				showText(hwndGreenWeight, std::to_string(green_box.weight));
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniaæ wag w trakcie sortowania.");
			}
			break;
		case ID_BLUE_PLUS:
			if (program_state != 2)
			{
				blue_box.weight++;
				createVector();
				createQueue();
				showText(hwndBlueWeight, std::to_string(blue_box.weight));
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniaæ wag w trakcie sortowania.");
			}
			break;
		case ID_BLUE_MINUS:
			if (program_state != 2)
			{
				blue_box.weight--;
				createVector();
				createQueue();
				showText(hwndBlueWeight, std::to_string(blue_box.weight));
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniaæ wag w trakcie sortowania.");
			}
			break;
		case ID_RED_PLUS:
			if (program_state != 2)
			{
				red_box.weight++;
				createVector();
				createQueue();
				showText(hwndRedWeight, std::to_string(red_box.weight));
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniaæ wag w trakcie sortowania.");
			}
			break;
		case ID_RED_MINUS:
			if (program_state != 2)
			{
				red_box.weight--;
				createVector();
				createQueue();
				showText(hwndRedWeight, std::to_string(red_box.weight));
			}
			else
			{
				showText(hwndEventLog, "Nie mo¿na zmieniaæ wag w trakcie sortowania.");
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here (not depend on timer, buttons)
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		switch (wParam)
		{
		case TMR_1:
			switch (control)
			{
			case 1:
				if ((alpha + step > 0) || ((hand_x > fix_x) && (arm_length * sin(step) + hand_y > fix_y)))
				{
					KillTimer(hWnd, TMR_1);
				}
				else
				{
					alpha += step;
					beta += step;
					if (program_state == 3)
					{
						record_queue.push(1);
					}
				}
				break;
			case 2:
				if ((alpha - step < -pi) || ((hand_x < fix_x) && (arm_length * sin(step) + hand_y > fix_y)))
				{
					KillTimer(hWnd, TMR_1);
				}
				else
				{
					alpha -= step;
					beta -= step;
					if (program_state == 3)
					{
						record_queue.push(2);
					}
				}
				break;
			case 3:
				if ((beta + step > pi + alpha) || ((hand_x > fix_x) && (arm_length * sin(step) + hand_y > fix_y)))
				{
					KillTimer(hWnd, TMR_1);
				}
				else
				{
					beta += step;
					if (program_state == 3)
					{
						record_queue.push(3);
					}
				}
				break;
			case 4:
				if ((beta - step < alpha - pi) || ((hand_x < fix_x) && (arm_length * sin(step) + hand_y > fix_y)))
				{
					KillTimer(hWnd, TMR_1);
				}
				else
				{
					beta -= step;
					if (program_state == 3)
					{
						record_queue.push(4);
					}
				}
				break;
			}
			moveBox();
			repaintWindow(hWnd, hdc, ps, &actionArea);
			break;
		case TMR_2:
			falling(hWnd);
			repaintWindow(hWnd, hdc, ps, &actionArea);
			break;
		case TMR_3:
			switch (auto_state)
			{
			case 0:
				auto_state = auto_queue.front();
				auto_queue.pop();
				break;
			case 1:
				pickYellow(hWnd);
				break;
			case 2:
				pickGreen(hWnd);
				break;
			case 3:
				pickBlue(hWnd);
				break;
			case 4:
				pickRed(hWnd);
				break;
			case 5:
				dropAsFirst(hWnd);
				break;
			case 6:
				dropAsSecond(hWnd);
				break;
			case 7:
				dropAsThird(hWnd);
				break;
			case 8:
				dropAsFourth(hWnd);
				break;
			case 9:
				initialPosition(hWnd);
				break;
			}
			repaintWindow(hWnd, hdc, ps, &actionArea);
			break;
		case TMR_4:
			if (!record_queue.empty())
			{
				playing(hWnd);
				moveBox();
				repaintWindow(hWnd, hdc, ps, &actionArea);
			}
			else
			{
				KillTimer(hWnd, TMR_4);
				program_state = 0;
				showText(hwndEventLog, "Zakoñczono odtwarzanie.");
			}
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
