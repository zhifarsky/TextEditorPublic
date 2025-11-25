#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_opengl3.h>

#include "tools.h"
#include "editor.h"
#include "data_structures.h"

#define DEFAULT_FONT_SIZE 20
#define MAX_TABS_COUNT 128

ImFont* g_fontRegular;
const char* g_fontRegularPath = ".\\res\\fonts\\Roboto-Regular.ttf";

bool g_rerenderFonts;

u32 g_lastDockNodeId;

enum encoding_type {
    Encoding_None = 0,
    Encoding_UTF8
};

struct text_tab {
    s64 cursorIndex;
    u32 id;
    encoding_type encoding;
};

void TextInsertChar(text_tab* textTab, u32 utf8CodePoint, s64 pos) {
    
}

struct editor_state {
    text_tab tabs[MAX_TABS_COUNT];
    u32 tabsCount;
    u32 tabIDCounter;
    
    u32 fontSize;
    s32 currentTextTab;
};

void AddTextTab(editor_state* editor) {
    text_tab* newTab = &editor->tabs[editor->tabsCount];
    ZeroStruct(*newTab);
    
    newTab->encoding = Encoding_UTF8;
    newTab->id = editor->tabIDCounter++; 
    
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
        Init(&permStorage->arena,
            ((u8*)permStorage->base)    + sizeof(*editorState), 
            permStorage->capacity       - sizeof(*editorState)
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
        
        ImGuiIO& io = ImGui::GetIO();
        editorState->fontSize = DEFAULT_FONT_SIZE;
        g_fontRegular = io.Fonts->AddFontFromFileTTF(
            g_fontRegularPath, editorState->fontSize, NULL, io.Fonts->GetGlyphRangesCyrillic());
        
        g_rerenderFonts = false;
        
        memory->isInitialized = true;
    }
    
    //
    // Rerender fonts if needed 
    //
    
    if (g_rerenderFonts) {
        ImGuiIO &io = ImGui::GetIO();
        
        ImGui_ImplOpenGL3_DestroyFontsTexture();
        io.Fonts->Clear();

        g_fontRegular = io.Fonts->AddFontFromFileTTF(
        g_fontRegularPath, editorState->fontSize, NULL, io.Fonts->GetGlyphRangesCyrillic());

        io.Fonts->Build();
        ImGui_ImplOpenGL3_CreateFontsTexture();
    }
    
    //
    // Process Event Queue
    //

    if (editorState->currentTextTab >= 0)
    {
        text_tab* textTab = &editorState->tabs[editorState->currentTextTab];
        
        u8* i = (u8*)eventQueue->base;
        
        while (i < (u8*)eventQueue->base + eventQueue->size) {
            event_type eventType = *(event_type*)i;
        
            switch (eventType)
            {
                case Event_Char: {
                    char_event event = *(char_event*)i;
                    platform_Print("Processing char event\n");
                    TextInsertChar(textTab, event.utf8CodePoint, textTab->cursorIndex);
                    // ...
                    i += sizeof(event);
                } break;

                case Event_KeyDown: {
                    key_event event = *(key_event*)i;
                    // ...
                    i += sizeof(event);
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

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiStyle& style = ImGui::GetStyle();

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
            if (ImGui::Button("Settings")) { /*isSettingsOpen = true;*/ }
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

    if (ImGui::Begin("Test window")) {
        ImGui::Text("Tabs count: %d", editorState->tabsCount);
        ImGui::Text("Current tab: %d", editorState->currentTextTab);
        if (ImGui::Button("Add text tab")) {
            AddTextTab(editorState);
        }
        
        // draw text tabs
        for (size_t i = 0; i < editorState->tabsCount; i++)
        {
            text_tab* tab = &editorState->tabs[i];
            
            // create label
            char label[256] = "New tab##";
            char id[256];
            string labelStr = String(label, sizeof(label), StrLen(label));
            string idStr = String(id, sizeof(id));
            IntToString(&idStr, tab->id);
            Concat(&labelStr, &idStr);
            labelStr.base[labelStr.size] = 0;
            
            if (ImGui::Begin(labelStr.base, NULL, ImGuiWindowFlags_NoSavedSettings))
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
                        ImGui::DockBuilderDockWindow(labelStr.base, g_lastDockNodeId);
                    }
                }
            }    
                        
            ImGui::End();
        }
        
    }
    ImGui::End();
    
    platform_EndFrame();
}