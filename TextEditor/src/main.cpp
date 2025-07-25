// удаление CRT
// https://hero.handmade.network/forums/code-discussion/t/94-guide_-_how_to_avoid_c_c_runtime_on_windows
#include <math.h>
#include <Windows.h>
#include <wingdi.h>
#include <GL/gl.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include "GL/wglext.h"
#include "tools.h"
#include "structs.h"
#include "editor.h"
#include "rendering.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern "C" int _fltused = 0x9875; // чтобы работал float
void __security_check_cookie(uintptr_t cookie) {
	// Пустая реализация (небезопасно!)
}

typedef HGLRC (*pwglCreateContextAttribsARB)(HDC hDC, HGLRC hShareContext, const int *attribList);

static LARGE_INTEGER timerFrequency;
void GnitTimer() {
	QueryPerformanceFrequency(&timerFrequency);
}

LARGE_INTEGER GetTime() {
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time); 
	return time;
}

double GetTimeElapsedSeconds(LARGE_INTEGER start, LARGE_INTEGER end) {
	long long timeElapsed = end.QuadPart - start.QuadPart;
	return (double)timeElapsed / (double)timerFrequency.QuadPart;
}

#define INITIAL_FONT_SIZE 20
int windowWidth = 1280, windowHeight = 720;
LARGE_INTEGER programStartTime;
bool programRunning = true;
Editor g_editor = {0};

// window callback
LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam))
			return true;
		// DefWindowProcWFunc pDefWindowProcW = (DefWindowProcWFunc)GetProcAddress(hUser32, "DefWindowProcW");
		LRESULT result = 0;

    switch (msg) {
    case WM_DESTROY:
    case WM_CLOSE: 
        programRunning = false;
				return 0;
    break;
    // case WM_ACTIVATEAPP: break;

    // case WM_SYSKEYDOWN:
    // case WM_SYSKEYUP:
    // case WM_KEYUP:
    // break;
    case WM_KEYDOWN: {
			g_editor.ProcessKeydown(wParam);
			return 0;
		} break;

		case WM_CHAR: {
			g_editor.ProcessChar(wParam);
			return 0;
		} break;

		// case WM_HOTKEY: {
		// 	return 0;
		// } break;

    case WM_SIZE: {
        // if (wParam != SIZE_MINIMIZED) {
        //     ImGuiIO& io = ImGui::GetIO();
        //     io.DisplaySize = ImVec2((float)LOWORD(lParam), (float)HIWORD(lParam));
        // }
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



int MainLoop(HWND window) {
		MSG message;
	while (programRunning) {
			// while (programRunning && PeekMessageA(&message, 0, 0, 0, PM_REMOVE)) {
			if (PeekMessageA(&message, window, 0, 0, PM_REMOVE)) {
					switch (message.message) {
					}
				TranslateMessage(&message);
				DispatchMessageA(&message);
			}
			else {
				g_editor.ProcessHotkey(window);
				renderUI();
			}
	}

	// Очистка
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(g_editor.renderInfo.oglContext);
	ReleaseDC(window, g_editor.renderInfo.deviceContext);
	DestroyWindow(window);
	return 0;
}


int WinMain(
  HINSTANCE Instance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nShowCmd
) 
{
	GnitTimer();
	programStartTime = GetTime();
	RenderInfo& renderInfo = g_editor.renderInfo;

	// HINSTANCE Instance = GetModuleHandle(0);

	// create window class
	WNDCLASSW wc = { };
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WindowProc;
	wc.hInstance = Instance;
	wc.lpszClassName = L"TextEditorWindowClass";

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
			// init opengl
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
				ErrorHandle();
			wglMakeCurrent(renderInfo.deviceContext, oglTempContext);
			PFNWGLCREATECONTEXTATTRIBSARBPROC func = 
				(PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
			if (!func)
				ErrorHandle();
			
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

			// imgui
			const char* glsl_version = "#version 330";
			IMGUI_CHECKVERSION();
			auto imguiContext = ImGui::CreateContext();
			// ImGui::SetCurrentContext(imguiContext);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // imgui docking 
			ImGui::StyleColorsDark();
			ImGui_ImplWin32_Init(renderInfo.window);
			ImGui_ImplOpenGL3_Init(glsl_version);
			
			initFonts(INITIAL_FONT_SIZE);
			initUI();
			g_editor.init();
		  OpenClipboard(renderInfo.window);
			MainLoop(renderInfo.window);
		}
	} 


	return 1;
}

int __stdcall WinMainCRTStartup() {
	int result = WinMain(GetModuleHandle(0), 0, 0, 0);
	ExitProcess(result);
}