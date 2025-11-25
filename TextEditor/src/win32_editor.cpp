#include <windows.h>
#include <GL/gl.h>
#include <GL/wglext.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_opengl3.h>

#include "editor.h"
#include "data_structures.h"

#include "tools.cpp"
#include "data_structures.cpp"
#include "editor.cpp"

extern "C" int _fltused = 0x9875; // чтобы работал float

void __security_check_cookie(uintptr_t cookie) { } // TODO: выяснить зачем это нужно

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

// 
// Global state
//

static bool g_ProgramRunning;
static event_queue g_EventQueue;

static HANDLE g_Console;
static HDC g_DeviceContext;
static HGLRC g_OglContext;
static HWND g_Window;

void* debug_malloc(u64 size) {
	return VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

//
// Platform API
//

void platform_Print(const char* msg) {
	if (!g_Console)
		g_Console = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleA(g_Console, msg, strlen(msg), NULL, 0);
}

void platform_StartFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void platform_EndFrame() {
	ImGuiIO& io = ImGui::GetIO();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
	SwapBuffers(g_DeviceContext);
}

void ErrorHandle(const char* msg = NULL) {
	DWORD errorCode = GetLastError();
	platform_Print("\nERROR");
	if (msg)
		platform_Print(msg);
	ExitProcess(EXIT_FAILURE);
}

//
// Main
//


int WinMain(
    HINSTANCE Instance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
)
{
	WNDCLASSW wc = { };
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WindowProc;
	wc.hInstance = Instance;
	wc.lpszClassName = L"TextEditorWindowClass";

	s32 windowWidth = 900;
	s32 windowHeight = 600;

	// register class and create window
	if (RegisterClassW(&wc)) {
		g_Window = CreateWindowExW(
			0,
			wc.lpszClassName,
			L"Text Editor",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			windowWidth,
			windowHeight,
			0,
			0,
			Instance,
			0
		);

		if (g_Window) {
			//
			// Init OpenGL
			//
			
			g_DeviceContext = GetDC(g_Window);
			PIXELFORMATDESCRIPTOR pfd = {
					sizeof(PIXELFORMATDESCRIPTOR),
					1,
					PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
					PFD_TYPE_RGBA,
					32, // Цветовой буфер
					0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0,
					24, // Глубина буфера
					8,  // Буфер трафарета
					0,
					PFD_MAIN_PLANE,
					0,
					0, 0, 0
			};
			int pixelFormat = ChoosePixelFormat(g_DeviceContext, &pfd);
			SetPixelFormat(g_DeviceContext, pixelFormat, &pfd);

			HGLRC oglTempContext = wglCreateContext(g_DeviceContext);
			if (!oglTempContext)
				ErrorHandle("wglCreateContext()\n");
			
			wglMakeCurrent(g_DeviceContext, oglTempContext);
			
			PFNWGLCREATECONTEXTATTRIBSARBPROC func = 
				(PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
			if (!func)
				ErrorHandle("wglGetProcAddress()");

			// ogl v3.3
			int oglAttributes[] = {
				WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
				WGL_CONTEXT_MINOR_VERSION_ARB, 3,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
				0
			};
			g_OglContext = func(g_DeviceContext, 0, oglAttributes);

			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(oglTempContext);
			wglMakeCurrent(g_DeviceContext, g_OglContext);

			// 
			// Init Imgui
			//

			const char* glsl_version = "#version 330";
			IMGUI_CHECKVERSION();
			auto imguiContext = ImGui::CreateContext();
			// ImGui::SetCurrentContext(imguiContext);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // imgui docking
			// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			// io.ConfigWindowsMoveFromTitleBarOnly = true;
			ImGui::StyleColorsDark();
			ImGui_ImplWin32_Init(g_Window);
			ImGui_ImplOpenGL3_Init(glsl_version);

			//
			// Init state
			//
			
			u64 permStorageCapaicty = Megabytes(5);
			u64 eventQueueCapacity = Megabytes(5);
			
			program_memory memory = {0};
			memory.permStorage.base = debug_malloc(permStorageCapaicty);
			memory.permStorage.capacity = permStorageCapaicty;
			
			Init(&g_EventQueue, debug_malloc(eventQueueCapacity), eventQueueCapacity);
			g_ProgramRunning = true;

			program_input input = {0};

			//
			// Main loop
			//

			while (g_ProgramRunning) {
				//
				// Write events to event queue
				//
				
				Clear(&g_EventQueue); // подгатавливаем очередь к WindowProc

				MSG message;
				// NOTE: в hwnd нужен NULL, иначе обработка сообщений начинает работать неправильно
				// (не работал ALT + SHIFT, когда окно было в фокусе)
				while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
					TranslateMessage(&message);
					DispatchMessageW(&message);
				}



				//
				// Editor Code
				//

				EditorUpdateAndRender(&memory, &g_EventQueue, &input);
			}

			//
			// Shutdown Imgui
			//
			
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();

			//
			// Cleanup
			//

			// wglMakeCurrent(NULL, NULL);
			// wglDeleteContext(g_OglContext);
			// ReleaseDC(g_Window, g_DeviceContext);
			// DestroyWindow(g_Window);
		}
	}



	return EXIT_SUCCESS;
}

// Debug компилируем с флагом /subsystem:console
// Release с /subsystem:windows

#if _DEBUG
s32 mainCRTStartup() {
#else
s32 WinMainCRTStartup() {
#endif
	s32 result = WinMain(GetModuleHandle(0), 0, 0, 0);
    ExitProcess(result);
}
	s32 result = WinMain(GetModuleHandle(0), 0, 0, 0);
    ExitProcess(result);
}


//
// Window callback
//

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam))
		return true;

    switch (msg) {
    
	case WM_DESTROY:
    case WM_CLOSE: 
        g_ProgramRunning = false;
	return 0;

	case WM_INPUTLANGCHANGE:
		platform_Print("Input language changed\n");
	return 1;

	case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
	break;

	case WM_SYSKEYUP:
	case WM_KEYUP:
	break;

	case WM_CHAR: {
		wchar_t wideChar = wParam;
		// char utf8Buffer[5] = {0};
		u32 utf8CodePoint = 0;
		u8* debugCodePoint = (u8*)&utf8CodePoint;
		s32 bytesWritten = 
			WideCharToMultiByte(CP_UTF8, 0, &wideChar, 1, (char*)&utf8CodePoint, sizeof(utf8CodePoint), NULL, NULL);
		
		// u16 repeatCount = lParam & 0xFFFF;
		
		char_event event = CharEvent(utf8CodePoint);
		PUSH_EVENT(g_EventQueue, event);
	} return 0;

    case WM_SIZE:
		return 0;

    case WM_PAINT: {
		PAINTSTRUCT paint;
		BeginPaint(window, &paint);
		EndPaint(window, &paint);
	} return 0;
    }

    return DefWindowProcW(window, msg, wParam, lParam);;
}
