#pragma once
#include <ScratchWin.h>

#include <iostream>
#include <sstream>

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CLOSE:
			PostQuitMessage(420);
			break;
		case WM_KEYDOWN:
			if (wParam == 'F')
				SetWindowText(hWnd, L"Pay Respects");
			break;
			//WM_CHAR is used for text inpu
		case WM_CHAR:
		{
			static std::wstring title;
			title.push_back((wchar_t)wParam);
			SetWindowText(hWnd, title.c_str());
		}
		break;
		case WM_LBUTTONDOWN:
		{
			const POINTS pt = MAKEPOINTS(lParam);
			std::wstringstream oss;
			oss << "(" << pt.x << ", " << pt.y << ")";
			SetWindowText(hWnd, oss.str().c_str());
		}
		break;
		}


		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	int CALLBACK WinMain(
		HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine,
		int nCmdShow)
	{
		const auto pClassName = L"ScratchHW3D";
		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof(wc);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = nullptr;
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = pClassName;
		wc.hIconSm = nullptr;

		//register windows class
		RegisterClassEx(&wc);

		HWND hWnd = CreateWindowEx(
			0,
			pClassName,
			L"Scratch",
			WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
			200, 200, 640, 480,
			nullptr, nullptr,
			hInstance,
			nullptr
		);


		ShowWindow(hWnd, SW_SHOW);

		//message pump
		MSG msg;
		BOOL gResult;
		while (gResult = GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (gResult == -1)
		{
			return -1;
		}
		else
		{
			return msg.wParam;
		}

		return 0;
	}