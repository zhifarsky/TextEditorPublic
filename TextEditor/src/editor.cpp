#include <GL/gl.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>

#include "editor.h"
#include "data_structures.h"
#include "tools.h"

#define DEFAULT_FONT_SIZE 20

ImFont* g_fontRegular;
const char* g_fontRegularPath = ".\\res\\fonts\\Roboto-Regular.ttf";

bool g_rerenderFonts;

struct editor_state {
    u32 fontSize;
    s32 currentTextTab;
};

void EditorUpdateAndRender(program_memory* memory, event_queue* eventQueue, program_input* input) {
    editor_state* editorState = (editor_state*)memory->permStorage.base;
    
    if (!memory->isInitialized) {
        permanent_storage* permStorage = &memory->permStorage;
        Init(&permStorage->arena,
            ((u8*)permStorage->base)    + sizeof(*editorState), 
            permStorage->capacity       - sizeof(*editorState)
        ); 
        
        memory->isInitialized = true;
        
        //
        //  Init Editor
        //
        
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

    {
        u8* i = (u8*)eventQueue->base;
        
        while (i < (u8*)eventQueue->base + eventQueue->size) {
            event_type eventType = *(event_type*)i;
        
            switch (eventType)
            {
                case Event_Char: {
                    char_event event = *(char_event*)i;
                    platform_Print("Processing char event\n");
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

    if (ImGui::Begin("Test window")) {
        ImGui::Text("TEEEEEEEEEEEXT");
    }
    ImGui::End();
    
    platform_EndFrame();
}
