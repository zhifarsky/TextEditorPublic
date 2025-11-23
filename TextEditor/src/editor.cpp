#include "editor.h"

u32 g_fontSize;
ImFont* g_fontRegular;
const char* g_fontRegularPath = ".\\res\\fonts\\Roboto-Regular.ttf";

bool g_rerenderFonts;

void EditorInit(u32 fontSize) {
    g_rerenderFonts = false;
    
    ImGuiIO& io = ImGui::GetIO();
    g_fontSize = fontSize;
    g_fontRegular = io.Fonts->AddFontFromFileTTF(
        g_fontRegularPath, fontSize, NULL, io.Fonts->GetGlyphRangesCyrillic());
}

void EditorUpdateAndRender(program_memory* memory, event_queue* eventQueue, program_input* input) {
    //
    // Rerender fonts if needed 
    //
    
    if (g_rerenderFonts) {
        ImGuiIO &io = ImGui::GetIO();
        
        ImGui_ImplOpenGL3_DestroyFontsTexture();
        io.Fonts->Clear();

        g_fontRegular = io.Fonts->AddFontFromFileTTF(
        g_fontRegularPath, g_fontSize, NULL, io.Fonts->GetGlyphRangesCyrillic());

        io.Fonts->Build();
        ImGui_ImplOpenGL3_CreateFontsTexture();
    }
    
    //
    // Process Event Queue
    //

    {
        u8* i = (u8*)eventQueue->data;
        
        while (i < (u8*)eventQueue->data + eventQueue->size) {
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

    if (ImGui::Begin("Test window")) {
        ImGui::Text("TEEEEEEEEEEEXT");
    }
    ImGui::End();
}
