#include "../engine/Engine.h"
engine* Engine;
#include <chrono>
HWND Window;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(Window, Message, wParam, lParam))
		return true;

	if (ImGui::GetCurrentContext() == NULL)
		return DefWindowProcA(Window, Message, wParam, lParam);

	LRESULT Result = NULL;
	ImGuiIO& io = ImGui::GetIO();

	switch (Message)
	{
	case WM_DESTROY:
	{
		Engine->IsRunning = false;
	}
	case WM_CLOSE:
	{
		Engine->IsRunning = false;
	}
	case WM_SETCURSOR:
	{
		//HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
		//SetCursor(hCursor);
		break;
	}
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		u32 KeyCode = (u32)wParam;
		if (KeyCode < 256)
		{
			Engine->UserInputs.KeysDown[KeyCode].JustPressed = false;
			Engine->UserInputs.KeysDown[KeyCode].Pressed = false;
		}
		break;
	}
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		u32 KeyCode = (u32)wParam;

		// temp
#if SPADE_DEBUG
		if (KeyCode == VK_ESCAPE)
			Engine->IsRunning = false;
#endif

		if (KeyCode < 256)
		{
			if (Engine->UserInputs.KeysDown[KeyCode].Pressed)
				Engine->UserInputs.KeysDown[KeyCode].JustPressed = false;
			else
				Engine->UserInputs.KeysDown[KeyCode].JustPressed = true;
			Engine->UserInputs.KeysDown[KeyCode].Pressed = true;
		}
		break;
	}
	default:
	{
		Result = DefWindowProcA(Window, Message, wParam, lParam);
		break;
	}
	}

	return Result;
}

void ProcessPendingMessages()
{
	Engine->UserInputs.MouseMovement = false;
	int MouseDeltaBufferX = 0;
	int MouseDeltaBufferY = 0;
	MSG Message;
	ImGuiIO& io = ImGui::GetIO();

	while (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
	{
		switch (Message.message)
		{
		case WM_INPUT:
		{
			if (true)//(!io.WantCaptureMouse)//(PlayerState->EnableInput && !io.WantCaptureMouse && !DebugData.Editor_ContentBrowser_DraggingFiles)
			{
				UINT dwSize;

				GetRawInputData((HRAWINPUT)Message.lParam, RID_INPUT, NULL, &dwSize,
					sizeof(RAWINPUTHEADER));
				LPBYTE lpb = new BYTE[dwSize];
				if (lpb == NULL)
				{
					break;
				}

				if (GetRawInputData((HRAWINPUT)Message.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
					Assert(1 == 2);//OutputDebugMessage("GetRawInputData does not return correct size !");

				RAWINPUT* raw = (RAWINPUT*)lpb;

				if (raw->header.dwType == RIM_TYPEMOUSE)
				{
					int xPosRelative = raw->data.mouse.lLastX;
					int yPosRelative = raw->data.mouse.lLastY;

					MouseDeltaBufferX += xPosRelative;
					MouseDeltaBufferY += yPosRelative;

					//OutputDebugFloat((f32)xPosRelative);

					// Need to re-generalize this so that we do not need to repeat this
					// for each button type. See Win32ProcessKeyboardMessage above.

					int LButton = raw->data.mouse.usButtonFlags;

					// left mouse
					if (LButton == RI_MOUSE_LEFT_BUTTON_DOWN)
					{
						Engine->UserInputs.LMB.Pressed = true;
					}
					if (LButton == RI_MOUSE_LEFT_BUTTON_UP)
					{
						if (GetFocus() != (HWND)Engine->Renderer.Window) // reset window focus if clicked & out of focus
						{
							SetFocus((HWND)Engine->Renderer.Window);
						}
						Engine->UserInputs.LMB.Pressed = false;
					}

					// right mouse
					if (LButton == RI_MOUSE_RIGHT_BUTTON_DOWN)
					{
						Engine->UserInputs.RMB.Pressed = true;
					}
					if (LButton == RI_MOUSE_RIGHT_BUTTON_UP)
					{
						if (GetFocus() != (HWND)Engine->Renderer.Window) // reset window focus if clicked & out of focus
						{
							SetFocus((HWND)Engine->Renderer.Window);
						}
						Engine->UserInputs.RMB.Pressed = false;
					}
					Engine->UserInputs.MouseMovement = true;
				}
			}
			else
			{
				Engine->UserInputs.LMB.Pressed = false;
				Engine->UserInputs.RMB.Pressed = false;
			}
			break;
		}
		default:
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
			break;
		}
		}
	}
	//Engine->UserInputs.MousePosX += MouseDeltaBufferX;
	Engine->UserInputs.MouseDeltaX = (f32)MouseDeltaBufferX;
	//Engine->UserInputs.MousePosY += MouseDeltaBufferY;
	Engine->UserInputs.MouseDeltaY = (f32)MouseDeltaBufferY;

	POINT p;
	if (GetCursorPos(&p))
		if (ScreenToClient(Window, &p))
		{
			Engine->UserInputs.MousePosX = (f32)p.x;
			Engine->UserInputs.MousePosY = (f32)p.y;
		}
}

int WINAPI wWinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PWSTR CommandLine, int ShowFlag) // entrypoint
{
	// TODO: Need to allow the user to choose their resolution.
	u32 InitialResX = 1920;
	u32 InitialResY = 1080;
	//UserScreenSizeX = InitialResX;
	//UserScreenSizeY = InitialResY;

	// Calculate the required size of the window rectangle based on the desired client-rectangle size
	RECT rc = { 0, 0, (LONG)InitialResX, (LONG)InitialResY };
	AdjustWindowRect(&rc, (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) | WS_VISIBLE, FALSE);
	u32 WindowHeight = rc.bottom - rc.top;
	u32 WindowWidth = rc.right - rc.left;

	WNDCLASSA WindowClass = { 0 };
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = WindowProc;
	WindowClass.hInstance = Instance;
	WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
	WindowClass.lpszClassName = "EngineWindowClass";
	//WindowClass.hbrBackground = CreateSolidBrush(RGB(30, 30, 30)); // a red window class background

	OleInitialize(NULL);

	if (RegisterClassA(&WindowClass))
	{
		Window =
			CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"Spade",
				(WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) | WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				WindowWidth,  // TODO: Need to factor in title bar offset
				WindowHeight,
				0,
				0,
				Instance,
				0);

		if (Window == NULL)
		{
			return 1;
		}

		Engine = new engine();
		Engine->Initialize(Window, InitialResX, InitialResY);

		//
		// Enable High-Definition Mouse Movement
		//
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

		RAWINPUTDEVICE Rid[1];
		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		Rid[0].dwFlags = RIDEV_INPUTSINK;
		Rid[0].hwndTarget = Window;
		if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
			Assert(1 == 2);

		//DropManager dm;
		//RegisterDragDrop(Window, &dm);

		//QueryPerformanceFrequency(&DebugData.frequency);
		while (Engine->IsRunning) // game loop
		{
			auto start = std::chrono::high_resolution_clock::now();
			ProcessPendingMessages();
			Engine->Tick();
			auto stop = std::chrono::high_resolution_clock::now();
			Engine->UserInputs.DeltaTime = max((f32)(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)).count(), 0.5f);
		}

		Engine->Cleanup();
		//RevokeDragDrop(Window);
	}

	OleUninitialize();

	return 0;
}