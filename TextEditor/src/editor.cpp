#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_opengl3.h>

#include "tools.h"
#include "editor.h"
#include "data_structures.h"
#include "widgets.h"

#define MIN_FONT_SIZE 14
#define MAX_FONT_SIZE 40
#define DEFAULT_FONT_SIZE 20

#define MAX_TABS_COUNT 128

ImFont* g_fontRegular;
// const char* g_fontRegularPath = ".\\res\\fonts\\Roboto-Regular.ttf";
const char* g_fontRegularPath = "C:/Windows/Fonts/msyh.ttc";

u32 g_lastDockNodeId;
bool isSettingsOpen;

enum text_node_type : u8 {
	Node_Original,
	Node_Added
};

struct text_node {
	u64 start, length;
	text_node_type type;
};

enum encoding_type {
	Encoding_None = 0,
	Encoding_UTF8
};

struct text_tab {
	memory_arena arena;
	
	s64 cursorIndex;
	u32 id;
	encoding_type encoding;
	
	array_dynamic<char> added;
	array_dynamic<text_node> nodes;
};

// TODO: заглушки, реализовать

u64 GetTextLength_utf8(text_tab* textTab) {
	return 0;
}

u64 GetText_utf8(text_tab* textTab, char* buffer) {
	return 0;
}

void TextInsertChar(text_tab* textTab, code_point utf8CodePoint, s64 pos) {
	s32 length = GetLength(utf8CodePoint);
	for (s32 i = 0; i < length; i++)
	{
		Push<char>(&textTab->added, &textTab->arena, utf8CodePoint.bytes[i]);
	}
}

void TextInsertTest(text_tab* textTab) {
	code_point c = {0};
	const unsigned char s1[] = "汉";
	MemCopy((void*)c.bytes, (void*)s1, StrLen(s1));
	TextInsertChar(textTab, c, 0);
}

struct editor_state {
	text_tab tabs[MAX_TABS_COUNT];
	u32 tabsCount;
	u32 tabIDCounter;
	
	u32 fontSize;
	s32 currentTextTab;
};

text_tab* GetCurrentTab(editor_state* editor) {
	if (editor->currentTextTab < 0 || editor->currentTextTab >= editor->tabsCount)
		return NULL;
	return &editor->tabs[editor->currentTextTab];
}

void AddTextTab(editor_state* editor) {
	text_tab* newTab = &editor->tabs[editor->tabsCount];
	ZeroStruct(*newTab);
	
	newTab->arena = ArenaAlloc(Megabytes(1));
	
	newTab->encoding = Encoding_UTF8;
	newTab->id = editor->tabIDCounter++; 
	
	newTab->added = Array<char>(&newTab->arena);
	
	editor->currentTextTab = editor->tabsCount;
	editor->tabsCount++;
}

void TestCode() { }

void EditorUpdateAndRender(program_memory* memory, event_queue* eventQueue, program_input* input) {
	editor_state* editorState = (editor_state*)memory->permStorage.base;
	
	//
	// Init State
	//
	
	if (!memory->isInitialized) {
		TestCode();
		
		permanent_storage* permStorage = &memory->permStorage;
		permStorage->arena = Arena( 
			((u8*)permStorage->base)    + sizeof(*editorState), 
			permStorage->capacity       - sizeof(*editorState)
		); 
		
		transient_storage* tranStorage = &memory->tranStorage;
		tranStorage->frameArena = Arena( 
			tranStorage->base, 
			tranStorage->capacity
		);
		
		memory->isInitialized = true;
				
		//
		//  Init Editor
		//
		
		ZeroStruct(*editorState);
		editorState->currentTextTab = -1;
		
		//
		// Init Fonts
		//

		editorState->fontSize = DEFAULT_FONT_SIZE; // TODO: больше не нужен?
		g_fontRegular = ImGui::GetIO().Fonts->AddFontFromFileTTF(g_fontRegularPath);
	}
	
	memory_arena* frameArena = &memory->tranStorage.frameArena;
	ArenaClear(frameArena);
	
	//
	// Process Event Queue
	//

	text_tab* textTab = GetCurrentTab(editorState);
	
	if (textTab) {
		u8* i = (u8*)eventQueue->base;
		
		while (i < (u8*)eventQueue->base + eventQueue->size) {
			event_type eventType = *(event_type*)i;
		
			switch (eventType)
			{
				case Event_Char: {
					char_event* event = (char_event*)i;
					platform_Print("Processing char event\n");
					TextInsertChar(textTab, event->utf8CodePoint, textTab->cursorIndex);
					// ...
					i += sizeof(*event);
				} break;

				case Event_Key: {
					key_event* event = (key_event*)i;
					if (event->key == Key_ArrowRight) {
						textTab->cursorIndex++;
					}
					else if (event->key == Key_ArrowLeft) {
						textTab->cursorIndex--;
					}
					i += sizeof(*event);
				} break;

				default:
					platform_Print("Unknown event");
				break;
			}
		}
	}

	//
	// Draw UI
	//

	platform_StartFrame();
	
	ImGui::PushFont(g_fontRegular, editorState->fontSize);
	
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGuiStyle& style = ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO();
	
	//
	// Toolbar
	//

	float toolbarHeight = ImGui::GetTextLineHeight() +
		style.FramePadding.y * 2.0f +
		style.WindowPadding.y * 2.0f;
	{
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, toolbarHeight)); // Высота панели инструментов
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags toolbarFlags = 
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoSavedSettings;

		if (ImGui::Begin("Toolbar", nullptr, toolbarFlags))
		{
			if (ImGui::Button("Settings")) { isSettingsOpen = true; }
			ImGui::SameLine();
			
			if (ImGui::Button("New")) { AddTextTab(editorState); }
			ImGui::SameLine();
			
			if (ImGui::Button("Load file")) { 
				// if (g_editor.textTabs.size > 0) {
				//     g_editor.getCurrentTextTab().openFile(); // TODO: проверка, найден ли currentTextTab?
				// }
				// else {
				//     g_editor.AddTextTab();
				//     g_editor.textTabs[0].openFile();
				// }
			}
			ImGui::SameLine();
			
			if (ImGui::Button("Save file")) {
				// if (g_editor.textTabs.size > 0) {
				// g_editor.getCurrentTextTab().saveFile(); // TODO: проверка, найден ли currentTextTab?
				// }
			}
		}
		ImGui::End();
	}
	
	//
	// Dock panel
	//
	
	{
		ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + toolbarHeight));
		ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - toolbarHeight));
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags dockspaceFlags = 
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", nullptr, dockspaceFlags);
		ImGui::PopStyleVar();

		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
		if (g_lastDockNodeId == 0)
			g_lastDockNodeId = dockspace_id;

		ImGui::End();
	}
	
	// TEST
	ImGui::Begin("TEST");
	string_builder testBuilder = {0};
	StrAppend(frameArena, &testBuilder, "Hello, ");
	StrAppend(frameArena, &testBuilder, "World");
	StrAppend(frameArena, &testBuilder, "!!!!!");
	string testString = testBuilder.buffer;
	ImGui::Text(testString);
	ImGui::End();
	
	// 
	// Text tabs
	// 
	
	for (size_t i = 0; i < editorState->tabsCount; i++) {
		text_tab* tab = &editorState->tabs[i];
		
		string_builder builder = {0};
		StrAppend(frameArena, &builder, "New ");
		IntAppend(frameArena, &builder, tab->id);
		char label[256];
		ToCString(label, 256, &builder);
		
		if (ImGui::Begin(label, NULL, ImGuiWindowFlags_NoSavedSettings))
		{
			if (ImGui::IsWindowFocused()) {
				editorState->currentTextTab = tab->id;
			}
			// записываем id, по которому можно будет присоеденить окно
			if (ImGui::IsWindowDocked()) {
				if (ImGui::IsWindowFocused()) {
					g_lastDockNodeId = ImGui::GetWindowDockID();
				}
			}
			// проверяем есть ли окно, к которому можно присоединить новое
			else if (g_lastDockNodeId != 0) {
				// проверяем перетаскивается ли окно. если нет - автоматически присоединяем к существующему окну
				if (!(ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))) {
					ImGui::DockBuilderDockWindow(label, g_lastDockNodeId);
				}
			}
			
			string filenameStub = String("filename_stub.txt");
			ImGui::Text(filenameStub);
			
			if (ImGui::Button("ut8 test")) {
				TextInsertTest(tab);
			}
			
			// string textBufferStub = String("Text Stub");
			string text = String(tab->added.items, tab->added.count);
			// g_rangesBuilder.AddText(StrFirst(textBuffer), StrLast(textBuffer) + 1); // TEST
			
			if (ImGui::BeginChild("TextChild", ImVec2(0, 0), 1)) {
				ImGui::Text(text);
				
				// drawCursor(text.cursorIndex2, buffer.mem, IM_COL32(255, 255, 255, 180));
				ImGui::DrawCursor(tab->cursorIndex, text);
			} ImGui::EndChild();
		} ImGui::End();
	}
	
	//
	// Settings
	//
	if (isSettingsOpen) {
		if (ImGui::Begin("Settings", &isSettingsOpen)) {
			ImGui::SliderInt("Font size", (int*)&editorState->fontSize, MIN_FONT_SIZE, MAX_FONT_SIZE);
		} ImGui::End();
	}
	
	ImGui::PopFont();
	
	platform_EndFrame();
}