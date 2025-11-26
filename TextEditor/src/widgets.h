#pragma once
#include <imgui/imgui.h>
#include "data_structures.h"

namespace ImGui {
    void GetWindowRegion(ImVec2* min, ImVec2* max) {
        *min = ImGui::GetWindowContentRegionMin();
        *max = ImGui::GetWindowContentRegionMax();

        min->x += ImGui::GetWindowPos().x;
        min->y += ImGui::GetWindowPos().y;
        max->x += ImGui::GetWindowPos().x;
        max->y += ImGui::GetWindowPos().y;
    }
    
    void Text(string str) {
        ImGui::TextUnformatted(StrFirst(str), StrLast(str) + 1);
    }
    
    void DrawCursor(int cursorIndex, string text, ImU32 color = IM_COL32_WHITE) {
        int lineBreakCount = 0; // количество строк перед курсором
        int lineStart = 0; // индекс, с которого начинается строка
        int lineLen = 0; // длина строки
        
        // находим количество строк перед курсором, начало последней строки и длину этой строки
        for (u64 i = 0; i < cursorIndex && i < text.size; i++)
        {
            lineLen++;
            if (text[i] == '\n') {
                lineBreakCount++;
                lineLen = 0;
            }
            if (i > 0 && text[i-1] == '\n') {
                lineStart = i;
            }
        }
        
        // находим размер строки до курсора в пикселях, чтобы отрисовать курсор в нужном месте
        ImVec2 lineSize = ImGui::CalcTextSize(&text[lineStart], &text[lineStart] + lineLen);

        ImVec2 vMin, vMax;
        GetWindowRegion(&vMin, &vMax);

        ImVec2 cursorPos1(lineSize.x + vMin.x, lineBreakCount * lineSize.y + vMin.y);
        ImVec2 cursorPos2(cursorPos1);
        cursorPos2.y += lineSize.y;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddLine(cursorPos1, cursorPos2, color, 1.0f);
    }
}