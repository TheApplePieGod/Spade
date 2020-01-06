
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
		engine::IsRunning = false;
	}
	case WM_CLOSE:
	{
		engine::IsRunning = false;
	}
	case WM_SETCURSOR:
	{
		HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
		SetCursor(hCursor);
		break;
	}
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		u32 KeyCode = (u32)wParam;
		if (KeyCode < 256)
			UserInputs->KeysDown[KeyCode].Pressed = false;
		break;
	}
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		u32 KeyCode = (u32)wParam;

		// temp
		if (KeyCode == VK_ESCAPE)
			engine::IsRunning = false;

		if (KeyCode < 256)
		{
			if (PlayerState->EnableInput && !io.WantCaptureKeyboard)
				UserInputs->KeysDown[KeyCode].Pressed = true;
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

int WINAPI wWinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PWSTR CommandLine, int ShowFlag) // entrypoint
{
	// TODO: Need to allow the user to choose their resolution.
	u32 InitialResX = 1920;
	u32 InitialResY = 1080;
	UserScreenSizeX = InitialResX;
	UserScreenSizeY = InitialResY;
	GlobalScale = { 18.f * ((f32)UserScreenSizeX / 1920), 18.f * ((f32)UserScreenSizeY / 1080) };
	GlobalScale.z = sqrt(pow(GlobalScale.x, 2) + pow(GlobalScale.y, 2));

	// Calculate the required size of the window rectangle based on the desired client-rectangle size
	RECT rc = { 0, 0, InitialResX, InitialResY };
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

#if SPADE_DEBUG
	assetLoader::ScanAssets(false);
#endif

	OleInitialize(NULL);

	if (RegisterClassA(&WindowClass))
	{
		Window =
			CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"Engine2D",
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

		//
		// Initialization
		//

#if SPADE_DEBUG
		assetLoader::InitializeAssetsInDirectory("assets\\", &AssetLoadCallbacks);
#else
		assetLoader::InitializeAssetsFromPac(&AssetLoadCallbacks);
#endif

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// Setup Platform/Renderer bindings
		ImGui_ImplWin32_Init(Window);
		//ImGui_ImplDX11_Init(renderer::Device, renderer::DeviceContext);

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

		QueryPerformanceFrequency(&DebugData.frequency);
		while (engine::IsRunning) // game loop
		{
			QueryPerformanceCounter(&DebugData.t1);
		}

		//RevokeDragDrop(Window);
	}

	OleUninitialize();

	return 0;
}