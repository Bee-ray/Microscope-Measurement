#define UNICODE
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>

using namespace Gdiplus;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
Bitmap *open_img(HWND hwnd);
void load_cal(double *cal);

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdline, int cmdshow)
{
	MSG msg;
	WNDCLASS wc = {0};

	GdiplusStartupInput gpsi;
	ULONG_PTR gpt;

	GdiplusStartup(&gpt, &gpsi, NULL);

	wc.lpszClassName = TEXT("measure graphene");
	wc.hInstance = instance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc = WndProc;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	RegisterClass(&wc);

	CreateWindow(wc.lpszClassName, TEXT("measure graphene"), WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 968, 720, 0, 0, instance, 0);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	GdiplusShutdown(gpt);
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hwnd_contr, hwnd_magni, hwnd_track, hwnd_dist;

	static WCHAR contr_txt[64] = TEXT("R 0.000000 G 0.000000 B 0.000000");
	static WCHAR magni_txt[32] = TEXT("obj. mag.: 5X");
	static WCHAR dist_txt[32] = TEXT("0.000000 * 10^-6 m");

	static int slider = 0;
	static int obj_mag;

	static int mode = 0; //contrast - 0, distance - 1
	static double r_contr, g_contr, b_contr;
	static double dist;

	static Graphics *gfx;
	static Bitmap *img;
	static double w = 0.0, h = 0.0, ratio = 1.0; //ratio = image pixel / screen pixel.

	static POINTS contr_point[4];
	static POINTS dist_point[2];
	static POINTS temp_point;
	static int state = 0; //next point to draw;

	static Pen *black = new Pen(Color(255, 0, 0, 0), 1);
	static double calibration[5]; //= microns / image pixel

	switch (msg) {
	case WM_CREATE:
		load_cal(calibration);
		InitCommonControls();

		CreateWindow(TEXT("button"), TEXT("open"), WS_VISIBLE | WS_CHILD, 4, 7, 64, 32, hwnd, (HMENU) 1, NULL, NULL);
		CreateWindow(TEXT("STATIC"), TEXT("measure:"), WS_CHILD | WS_VISIBLE | SS_LEFT, 80, 14, 64, 25, hwnd, (HMENU) 2, NULL, NULL);
		CreateWindow(TEXT("button"), TEXT("contrast:"), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 150, 7, 80, 32, hwnd, (HMENU) 3, NULL, NULL);
		CreateWindow(TEXT("button"), TEXT("distance:"), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 480, 7, 80, 32, hwnd, (HMENU) 4, NULL, NULL);
		hwnd_contr = CreateWindow(TEXT("STATIC"), contr_txt, WS_CHILD | WS_VISIBLE, 232, 14, 248, 25, hwnd, (HMENU) 5, NULL, NULL);
		hwnd_magni = CreateWindow(TEXT("STATIC"), magni_txt, WS_CHILD | WS_VISIBLE, 564, 14, 106, 25, hwnd, (HMENU) 6, NULL, NULL);
		hwnd_track = CreateWindowEx(0, TRACKBAR_CLASS, TEXT("Trackbar Control"), WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE, 664, 7, 100, 32, hwnd, (HMENU) 7, NULL, NULL);
		SendMessage(hwnd_track, TBM_SETRANGE, TRUE, MAKELONG(0, 4));
		hwnd_dist = CreateWindow(TEXT("STATIC"), dist_txt, WS_CHILD | WS_VISIBLE, 766, 14, 300, 25, hwnd, (HMENU) 8, NULL, NULL);
		CheckRadioButton(hwnd, 3, 4, 3);

		gfx = new Graphics(hwnd);
		img = NULL;
		break;
	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
			case 1:
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
				if (img) delete img;
				img = NULL;
				img = open_img(hwnd);
				if (img == NULL) break;
				w = img->GetWidth() / 960.0;
				h = img->GetHeight() / 640.0;
				ratio = w>h?w:h;
				if (ratio < 1) ratio = 1;
				w *= 960.0 / ratio;
				h *= 640.0 / ratio;
				gfx->DrawImage(img, 0.0, 46.0, (REAL) w, (REAL) h);
				break;
			case 3:
				mode = 0;
				state = 0;
				if (img) gfx->DrawImage(img, 0.0, 46.0, (REAL) w, (REAL) h);
				break;
			case 4:
				mode = 1;
				state = 0;
				if (img) gfx->DrawImage(img, 0.0, 46.0, (REAL) w, (REAL) h);
				break;
			default:
				break;
			}
		}
		SetFocus(hwnd);
		break;
	case WM_HSCROLL:
		slider = (int)SendMessage(hwnd_track, TBM_GETPOS, 0, 0);
		switch (slider) {
		case 0:
			obj_mag = 5;
			break;
		case 1:
			obj_mag = 10;
			break;
		case 2:
			obj_mag = 20;
			break;
		case 3:
			obj_mag = 50;
			break;
		case 4:
			obj_mag = 100;
			break;
		}
		swprintf(magni_txt, TEXT("obj. mag.: %dX"), obj_mag);
		SetWindowText(hwnd_magni, magni_txt);
		swprintf(dist_txt, TEXT("%f * 10^-6 m"), calibration[slider] * ratio * sqrt(pow(dist_point[1].x - dist_point[0].x, 2.0) + pow(dist_point[1].y - dist_point[0].y, 2.0)));
		SetWindowText(hwnd_dist, dist_txt);
		SetFocus(hwnd);
		break;
	case WM_PAINT:
		if (img) gfx->DrawImage(img, 0.0, 46.0, (REAL) w, (REAL) h);
		if (mode == 0) {
			if (contr_point[1].y != 0) gfx->DrawLine(black, contr_point[0].x, contr_point[0].y, contr_point[1].x, contr_point[1].y);
			if (contr_point[3].y != 0) gfx->DrawLine(black, contr_point[2].x, contr_point[2].y, contr_point[3].x, contr_point[3].y);
		} else { //mode = 1
			if (dist_point[1].y != 0) gfx->DrawLine(black, dist_point[0].x, dist_point[0].y, dist_point[1].x, dist_point[1].y);
		}
		break;
	case WM_LBUTTONDOWN:
		temp_point = MAKEPOINTS(lParam);
		if (temp_point.y < 46 || temp_point.y >= h + 46 || temp_point.x >= w) break;

		if (img) gfx->DrawImage(img, 0.0, 46.0, (REAL) w, (REAL) h);

		if (mode == 0) {
			contr_point[state] = temp_point;
			switch (state) {
			case 3: {
				double i_s[3] = {0.0, 0.0, 0.0}, i_g[3] = {0.0, 0.0, 0.0};
				int x_i, y_i;
				Color color;
				for (int i = 0; i <= 24; i++) {
					x_i = ratio * ((24 - i) * contr_point[0].x + i * contr_point[1].x) / 24;
					y_i = ratio * ((24 - i) * (contr_point[0].y - 46) + i * (contr_point[1].y - 46)) / 24;
					img->GetPixel(x_i, y_i, &color);
					i_g[0] += color.GetR();
					i_g[1] += color.GetG();
					i_g[2] += color.GetB();
					x_i = ratio * ((24 - i) * contr_point[2].x + i * contr_point[3].x) / 24;
					y_i = ratio * ((24 - i) * (contr_point[2].y - 46) + i * (contr_point[3].y - 46)) / 24;
					img->GetPixel(x_i, y_i, &color);
					i_s[0] += color.GetR();
					i_s[1] += color.GetG();
					i_s[2] += color.GetB();
				}
				swprintf(contr_txt, TEXT("R %f G %f B %f"), (i_g[0] - i_s[0]) / i_s[0], (i_g[1] - i_s[1]) / i_s[1], (i_g[2] - i_s[2]) / i_s[2]);
				SetWindowText(hwnd_contr, contr_txt);
			}
			case 1:
				if (contr_point[1].y != 0) gfx->DrawLine(black, contr_point[0].x, contr_point[0].y, contr_point[1].x, contr_point[1].y);
				if (contr_point[3].y != 0) gfx->DrawLine(black, contr_point[2].x, contr_point[2].y, contr_point[3].x, contr_point[3].y);
				break;
			case 0:
				contr_point[3].x = 0;
				contr_point[3].y = 0;
			}
			state = (state + 1) % 4;
		} else { //mode = 1
			dist_point[state] = temp_point;
			if (state && dist_point[1].y != 0) {
				gfx->DrawLine(black, dist_point[0].x, dist_point[0].y, dist_point[1].x, dist_point[1].y);
				swprintf(dist_txt, TEXT("%f * 10^-6 m"), calibration[slider] * ratio * sqrt(pow(dist_point[1].x - dist_point[0].x, 2.0) + pow(dist_point[1].y - dist_point[0].y, 2.0)));
				SetWindowText(hwnd_dist, dist_txt);
			}
			state = !state;
		}
		break;
	/*case WM_MOUSEMOVE:
		if (state % 2 == 0) break;
		temp_point = MAKEPOINTS(lParam);
		if (temp_point.y < 46 || temp_point.y >= h + 46 || temp_point.x >= w) break;

		if (img) gfx->DrawImage(img, 0.0, 46.0, (REAL) w, (REAL) h);

		if (mode == 0) {
			contr_point[state] = temp_point;
			if (contr_point[1].y != 0) gfx->DrawLine(black, contr_point[0].x, contr_point[0].y, contr_point[1].x, contr_point[1].y);
			if (contr_point[3].y != 0) gfx->DrawLine(black, contr_point[2].x, contr_point[2].y, contr_point[3].x, contr_point[3].y);
		} else { //mode = 1
			dist_point[state] = temp_point;
			if (dist_point[1].y != 0) gfx->DrawLine(black, dist_point[0].x, dist_point[0].y, dist_point[1].x, dist_point[1].y);
			swprintf(dist_txt, TEXT("%f * 10^-6 m"), calibration[slider] * ratio * sqrt(pow(dist_point[1].x - dist_point[0].x, 2.0) + pow(dist_point[1].y - dist_point[0].y, 2.0)));
			SetWindowText(hwnd_dist, dist_txt);
		}
		break;*/
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

Bitmap *open_img(HWND hwnd)
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.hwndOwner = hwnd;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = TEXT("All files(*.*)\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn))
		return new Bitmap(ofn.lpstrFile);
	return NULL;
}

void load_cal(double *cal)
{
	FILE *file;
	char str[64];
	int i = 0;
	file = fopen("calibration.txt", "r");
	if (file == NULL)
		exit(1);
	while (fgets(str, 64, file) != NULL && i < 5)
		cal[i++] = atof(str);
	while (i < 5)
		cal[i++] = 0;
	fclose(file);
}