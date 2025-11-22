#include <windows.h>

#include "editor.h"
#include "editor.cpp"

extern "C" int _fltused = 0x9875; // чтобы работал float

void __security_check_cookie(uintptr_t cookie) { } // TODO: выяснить зачем это нужно

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

// 
// Global state
//

static bool g_ProgramRunning;
static HANDLE console;

struct {
	HDC deviceContext;
	HGLRC oglContext;
	HWND window;
} renderInfo; // TODO: временно, убрать


void win32_Print(const char* msg) {
	if (!console)
		console = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleA(console, msg, strlen(msg), NULL, 0);
}

void win32_ErrorHandle(const char* msg = NULL) {
	DWORD errorCode = GetLastError();
	win32_Print("\nERROR");
	if (msg)
		win32_Print(msg);
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
		renderInfo.window = CreateWindowExW(
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

		if (renderInfo.window) {
			//
			// Init OpenGL
			//
			
			renderInfo.deviceContext = GetDC(renderInfo.window);
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
			int pixelFormat = ChoosePixelFormat(renderInfo.deviceContext, &pfd);
			SetPixelFormat(renderInfo.deviceContext, pixelFormat, &pfd);

			HGLRC oglTempContext = wglCreateContext(renderInfo.deviceContext);
			if (!oglTempContext)
				win32_ErrorHandle("wglCreateContext()\n");
			
			wglMakeCurrent(renderInfo.deviceContext, oglTempContext);
			
			PFNWGLCREATECONTEXTATTRIBSARBPROC func = 
				(PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
			if (!func)
				win32_ErrorHandle("wglGetProcAddress()");

			// ogl v3.3
			int oglAttributes[] = {
				WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
				WGL_CONTEXT_MINOR_VERSION_ARB, 3,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
				0
			};
			renderInfo.oglContext = func(renderInfo.deviceContext, 0, oglAttributes);

			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(oglTempContext);
			wglMakeCurrent(renderInfo.deviceContext, renderInfo.oglContext);

			// 
			// Init Imgui
			//

			const char* glsl_version = "#version 330";
			IMGUI_CHECKVERSION();
			auto imguiContext = ImGui::CreateContext();
			// ImGui::SetCurrentContext(imguiContext);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // imgui docking
			ImGui::StyleColorsDark();
			ImGui_ImplWin32_Init(renderInfo.window);
			ImGui_ImplOpenGL3_Init(glsl_version);

			//
			// Main loop
			//

			program_memory memory;
			program_input input;
			g_ProgramRunning = true;

			while (g_ProgramRunning) {
				MSG message;
				if (PeekMessageA(&message, renderInfo.window, 0, 0, PM_REMOVE)) {
					switch (message.message) {}					TranslateMessage(&message);
					DispatchMessageA(&message);
				}
				else {
					// g_editor.ProcessHotkey(window);

					//
					// Start Frame
					//

					ImGui_ImplOpenGL3_NewFrame();
					ImGui_ImplWin32_NewFrame();
					ImGui::NewFrame();

					//
					// Editor Code
					//

					UpdateAndRender(&memory, &input);

					//
					// End Frame
					//

					glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT);
					ImGui::Render();
					ImGuiIO& io = ImGui::GetIO();
					glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
					ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
					SwapBuffers(renderInfo.deviceContext);
				}
			}

		}

	}

	return EXIT_SUCCESS;
}

s32 mainCRTStartup() {
	s32 result = WinMain(GetModuleHandle(0), 0, 0, 0);
    ExitProcess(result);
}

//
// Window callback
//

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam))
		return true;

	LRESULT result = 0;

    switch (msg) {
    case WM_DESTROY:
    case WM_CLOSE: 
        g_ProgramRunning = false;
		return 0;
    break;

    case WM_KEYDOWN: {
		// g_editor.ProcessKeydown(wParam);
		return 0;
	} break;

	case WM_CHAR: {
		// g_editor.ProcessChar(wParam);
		return 0;
	} break;

    case WM_SIZE: {
		return 0;
	}

    case WM_PAINT: {
		PAINTSTRUCT paint;
		BeginPaint(window, &paint);
		EndPaint(window, &paint);
		return 0;
	} break;
    default:
        result = DefWindowProcW(window, msg, wParam, lParam);
		break;
    }

    return result;
}
