#include "rendering.h"
#include <Windows.h>
#include <GL/gl.h>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include "tools.h"
#include "pieceTable.h"
#include "structs.h"
#include "encodings.h"

ImFont* fontRegular;
const char* fontRegularPath = ".\\res\\fonts\\Roboto-Regular.ttf";

void structTestWindow();
void debugWindow();

void commandsWindow() {
  static char txtCommand[1024] = {0};
  
  ImGuiIO& io = ImGui::GetIO();
  ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, 200), ImGuiCond_Always, ImVec2(0.5f,0.5f));
  if (ImGui::Begin("Commands list", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
    // устанавливаем фокус на InputText
    if (ImGui::IsWindowAppearing())
        ImGui::SetKeyboardFocusHere(0);
    ImGui::InputText("Command", txtCommand, 1024);
    ImGui::SameLine();
    if (ImGui::Button("X")) {
      g_editor.showCommandsWindow = false;
    }
  } ImGui::End();
}

char* goToCharStart(char* text) {
  int i = 0;
  while (text[i] & 0b11 != 0b10) {
    i--;
  }
  return &text[i];
}

void rerenderFonts(float fontSize) {
    ImGuiIO &io = ImGui::GetIO();
    
    ImGui_ImplOpenGL3_DestroyFontsTexture();
    io.Fonts->Clear();

    fontRegular = io.Fonts->AddFontFromFileTTF(
      fontRegularPath, fontSize, NULL, io.Fonts->GetGlyphRangesCyrillic());

    io.Fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture();
}

void getWindowRegion(ImVec2* min, ImVec2* max) {
  *min = ImGui::GetWindowContentRegionMin();
  *max = ImGui::GetWindowContentRegionMax();

  min->x += ImGui::GetWindowPos().x;
  min->y += ImGui::GetWindowPos().y;
  max->x += ImGui::GetWindowPos().x;
  max->y += ImGui::GetWindowPos().y;
}

void startFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void endFrame() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SwapBuffers(g_editor.renderInfo.deviceContext);
}

// void drawCursor(Editor* editor, int charWidth = 7, int charHeight = 13, ImU32 color = IM_COL32_WHITE) {
void drawCursor(int cursorIndex, char* text, ImU32 color = IM_COL32_WHITE) {
  int lineBreakCount = 0; // количество строк перед курсором
  int lineStart = 0; // индекс, с которого начинается строка
  int lineLen = 0; // длина строки
  
  // находим количество строк перед курсором, начало последней строки и длину этой строки
  for (size_t i = 0; i < cursorIndex; i++)
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
  getWindowRegion(&vMin, &vMax);

  ImVec2 cursorPos1(lineSize.x + vMin.x, lineBreakCount * lineSize.y + vMin.y);
  ImVec2 cursorPos2(cursorPos1);
  cursorPos2.y += lineSize.y;

  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddLine(cursorPos1, cursorPos2, color, 1.0f);
}

/*
// text editor debug info
{
  ImGui::Text("Text len: %d, buffer capacity: %d", countCharsU8(buffer.mem), buffer.capacity);

  PieceTableV2& pt = g_editor.text.pt;
  // ImGui::SeparatorText("Buffers:");
  // ImGui::Text("orig: \n%s \nadded: \n%s", pt.original, pt.added.mem);
  
  ImGui::SeparatorText("Nodes:");
  for (size_t i = 0; i < pt.nodes.size; i++)
  {
    char buf[64 + 1];
    int len = min(64, pt.nodes[i].length);
    if (pt.nodes[i].type == PTNodeType_Original) {
      te_memcpy(buf, pt.original + pt.nodes[i].start, len);
    }
    else {
      te_memcpy(buf, pt.added.mem + pt.nodes[i].start, len);
    }
    buf[len] = '\0';
    
    ImGui::PushID(i);
    ImGui::Text("%d. \"%s\" (start: %d, length: %d)", i, buf, pt.nodes[i].start, pt.nodes[i].length);
    // ImGui::Text("%d. (start: %d, length: %d)", i, pt.nodes[i].start, pt.nodes[i].length);
    ImGui::PopID();
  }
}
*/

// C:/folder/abc.txt -> abc.txt
char* pathToFilename(char* path) {
  int i = 0;
  int fnameIndex = 0;
  while (path[i++] != '\0') {
    char c = path[i];
    if (c == '\\' || c == '/')
      fnameIndex = i;
  }
  
  if (i > 0)
    return path + fnameIndex + 1;
  else
    return path;
}

// TEXT WINDOW
void textWindow(Text& text, int id, bool* isOpen) {
  char windowsLabel[128];
  if (text.filename == NULL)
    wsprintfA(windowsLabel, "new %d###TextWindow%d", text.id, text.id);
  else {
    wsprintfA(windowsLabel, "%s###TextWindow%d", pathToFilename(text.filename), text.id);
  }
    
  if (ImGui::Begin(windowsLabel, isOpen, ImGuiWindowFlags_NoSavedSettings)) {
    if (ImGui::IsWindowFocused()) {
      g_editor.currentTextTab = id;
    }
    // записываем id, по которому можно будет присоеденить окно
    if (ImGui::IsWindowDocked()) {
      if (ImGui::IsWindowFocused()) {
        g_editor.renderInfo.lastDockNodeId = ImGui::GetWindowDockID();
      }
    }
    // проверяем есть ли окно, к которому можно присоединить новое
    else if (g_editor.renderInfo.lastDockNodeId != 0) {
      // проверяем перетаскивается ли окно. если нет - автоматически присоединяем к существующему окну
      if (!(ImGui::IsWindowHovered() && 
        ImGui::IsMouseDragging(ImGuiMouseButton_Left))) {
          ImGui::DockBuilderDockWindow(windowsLabel, g_editor.renderInfo.lastDockNodeId);
        }
    }

    Array<char>& buffer = text.textBuffer;
    static bool firstRun = true;
    if (firstRun) {
      firstRun = false;
    }

    int textSize = text.toText();
    buffer.size = textSize;

    if (text.filename != NULL) {
      ImGui::Text("%s", text.filename);
    }

    if (ImGui::BeginChild("TextChild", ImVec2(0, 0), 1)) {
      {
        ImGui::TextUnformatted(buffer.mem);
        
        drawCursor(text.cursorIndex2, buffer.mem, IM_COL32(255, 255, 255, 180));
        drawCursor(text.cursorIndex, buffer.mem);
      }
    } ImGui::EndChild();
  }
  ImGui::End();
}

static int g_fontSize;
static bool g_renderFont;

void initFonts(float fontSize) {
  ImGuiIO& io = ImGui::GetIO();
  g_fontSize = fontSize;
  fontRegular = io.Fonts->AddFontFromFileTTF(
    fontRegularPath, fontSize, NULL, io.Fonts->GetGlyphRangesCyrillic());
}

void initUI() {
  initFonts();
  g_renderFont = false;
}

static bool isSettingsOpen = false;

void renderUI() {
  if (g_renderFont) {
    rerenderFonts(g_fontSize);
  }
  
  startFrame();
  ImGui::PushFont(fontRegular);
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGuiStyle& style = ImGui::GetStyle();

  // верхняя панель
  float toolbarHeight = ImGui::GetTextLineHeight() +
  style.FramePadding.y * 2.0f +
  style.WindowPadding.y * 2.0f;
  {
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, toolbarHeight)); // Высота панели инструментов
    ImGui::SetNextWindowViewport(viewport->ID);

    // Флаги для неизменяемой панели инструментов
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
      if (ImGui::Button("New")) { g_editor.AddTextTab(); }
      ImGui::SameLine();
      if (ImGui::Button("Load file")) { 
        if (g_editor.textTabs.size > 0) {
          g_editor.getCurrentTextTab().openFile(); // TODO: проверка, найден ли currentTextTab?
        }
        else {
          g_editor.AddTextTab();
          g_editor.textTabs[0].openFile();
        }
      }
      ImGui::SameLine();
      if (ImGui::Button("Save file")) {
        if (g_editor.textTabs.size > 0) {
          g_editor.getCurrentTextTab().saveFile(); // TODO: проверка, найден ли currentTextTab?
        }
      }
    }
    ImGui::End();
  }

  // ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);

  // Теперь создаем dock space ниже панели инструментов
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
    if (g_editor.renderInfo.lastDockNodeId == 0)
      g_editor.renderInfo.lastDockNodeId = dockspace_id;


    ImGui::End();
  } 

  if (isSettingsOpen) {
    if (ImGui::Begin("Settings", &isSettingsOpen)) {
      if (ImGui::SliderInt("Font size", &g_fontSize, 10, 64)) {
        g_renderFont = true;
      }
    } ImGui::End();
  }

  // Text windows
  for (size_t i = 0; i < g_editor.textTabs.size; ) {
    if (g_editor.isTextOpen[i]) {
      textWindow(g_editor.textTabs[i], i, &g_editor.isTextOpen[i]);
      i++;
    }
    else {
      // TODO: закрыть окно (должно появиться предупреждение)
      g_editor.CloseTextTab(i);
    }
  }

#ifdef _DEBUG
  debugWindow();
  structTestWindow();
#endif

  if (g_editor.showCommandsWindow) {
    commandsWindow();
  }

  ImGui::PopFont();

  // Очистка экрана
  // double time = GetTimeElapsedSeconds(programStartTime, GetTime());
  endFrame();
}

#ifdef _DEBUG
void structTestWindow() {
  static PieceTableV2 pt;
  static char textBuffer[1024] = {0};
  static char addBuffer[1024] = "test";
  
  if (ImGui::Begin("Piece table test")) {
    static int cursorPos = 0;
    static int removeCount = 0;
    static Arena arena;
    if (arena.capacity == 0)
      arena.init(4 * 1024);

    ImGui::InputText("Text to insert", addBuffer, 1024);
    ImGui::InputInt("Cursor", &cursorPos);
    ImGui::InputInt("Remove count", &removeCount);
    
    if (ImGui::Button("pt init"))     pt.init(&arena, "Lorem ipsum", 128, 128);
    ImGui::SameLine();
    if (ImGui::Button("Insert text")) pt.insertText(&arena, addBuffer, te_strlen(addBuffer), cursorPos);
    ImGui::SameLine();
    if (ImGui::Button("Remove text")) pt.remove(&arena, cursorPos, removeCount);

    if (ImGui::Button("PieceTable test data")) {
      pt.init(&arena, "ipsum sit amet", 128, 128);
      char* add = "Lorem deletedtext dolor";
      int addLen = te_strlen(add);
      memcpy(pt.added.mem, add, addLen);
      pt.added.size = addLen;
      PTNode node;
      node.type = PTNodeType_Added; node.start = 0; node.length = 6;
      pt.nodes[0] = node;
      node.type = PTNodeType_Original; node.start = 0; node.length = 5;
      pt.nodes[1] = node;
      node.type = PTNodeType_Added; node.start = 17; node.length = 6;
      pt.nodes[2] = node;
      node.type = PTNodeType_Original; node.start = 5; node.length = 9;
      pt.nodes[3] = node;
      pt.nodes.size = 4;
    }

    ImGui::SeparatorText("Text:");
    pt.toText(textBuffer);
    ImGui::Text(textBuffer);
    
    ImGui::SeparatorText("Buffers:");
    ImGui::Text("orig: \n%s \nadded: \n%s", pt.original, pt.added.mem);
    
    ImGui::SeparatorText("Nodes:");
    for (size_t i = 0; i < pt.nodes.size; i++)
    {
      char buf[128];
      if (pt.nodes[i].type == PTNodeType_Original) {
        te_memcpy(buf, pt.original + pt.nodes[i].start, pt.nodes[i].length);
      }
      else {
        te_memcpy(buf, pt.added.mem + pt.nodes[i].start, pt.nodes[i].length);
      }
      buf[pt.nodes[i].length] = '\0';
      
      ImGui::PushID(i);
      ImGui::Text("%d. %s (start: %d, length: %d)", i, buf, pt.nodes[i].start, pt.nodes[i].length);
      ImGui::PopID();
    }
    

    ImGui::Separator();

    static char textBuffer2[1024];
    static int byteNumber = 0;
    ImGui::Text("Inspect utf8");
    ImGui::InputText("Text", textBuffer2, 1024);
    ImGui::InputInt("Byte number", &byteNumber);

    // int currentByte = ((int*)textBuffer2)[byteNumber];
    char *charStart = goToCharStart(&textBuffer2[byteNumber]);
    ImGui::Text("Char start: %d", charStart - &textBuffer2[byteNumber]);
  } ImGui::End();
}

void ArenaDebug(Arena* arena, int* size) {
  int i = 1;
    ImGui::InputInt("Size", size);
  if (ImGui::Button("init"))    arena->init(*size);
  ImGui::SameLine();
  if (ImGui::Button("alloc"))   arena->alloc(*size);
  ImGui::SameLine();
  if (ImGui::Button("clear"))   arena->clear();
  ImGui::SameLine();
  if (ImGui::Button("release")) arena->release();
  ImGui::SameLine();
  if (ImGui::Button("memest")) {
    Arena* node = arena;
    while (node)
    {
      for (size_t i = 0; i < node->size; i++) {
        node->mem[i] = 0;
      }
      node = node->next;
    }
  }

  ImGui::Text("Nodes:");
  Arena* node = arena;
  while (node) {
    ImGui::Text("%d) size: %d, capacity: %d next: %p", i, node->size, node->capacity, node->next);
    node = node->next;
    i++;
  }
}

void debugWindow() {
  static Arena arena;

  if (ImGui::Begin("Debug")) {
    if (arena.capacity == 0) {
      arena.init(4 * 1024);
    }  

    if (ImGui::BeginTabBar("tb")) {
      if (ImGui::BeginTabItem("tools")) {
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Arena")) {
        static Arena arena;
        static int size = 64;
        ArenaDebug(&arena, &size);

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("array")) {
        static Array<int> ar;
        static int arInitSize = 8;
        static int value = 5;
        static int pos = 0;
        static int fillSize = 0;

        ImGui::InputInt("init size", &arInitSize);
        ImGui::InputInt("value", &value);
        ImGui::InputInt("pos", &pos);
        ImGui::InputInt("fill size", &fillSize);
        
        if (ImGui::Button("init"))    ar.init(&arena, arInitSize);
        ImGui::SameLine();
        if (ImGui::Button("append"))  ar.append(value);
        ImGui::SameLine();
        if (ImGui::Button("insert"))  ar.insert(value, pos);
        ImGui::SameLine();
        if (ImGui::Button("remove"))  ar.remove(pos);
        if (ImGui::Button("fill")) {
          for (size_t i = 0; i < fillSize; i++)
            ar.mem[i] = i;
          ar.size = fillSize;
        }

        ImGui::Text("capacity: %d \nsize: %d", ar.capacity, ar.size);

        for (size_t i = 0; i < ar.size; i++)
        {
          ImGui::Text("%d", ar.mem[i]);
          ImGui::SameLine();
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("dynamic array")) {
        static DynamicArray<int> ar;
        static int arInitSize = 8;
        static int value = 5;
        static int pos = 0;
        static int fillSize = 0;

        ImGui::InputInt("init size", &arInitSize);
        ImGui::InputInt("value", &value);
        ImGui::InputInt("pos", &pos);
        ImGui::InputInt("fill size", &fillSize);
        
        if (ImGui::Button("init"))    ar.init(&arena, arInitSize);
        ImGui::SameLine();
        if (ImGui::Button("append"))  ar.append(&arena, value);
        ImGui::SameLine();
        if (ImGui::Button("insert"))  ar.insert(&arena, value, pos);
        ImGui::SameLine();
        if (ImGui::Button("remove"))  ar.remove(pos);
        ImGui::SameLine();
        if (ImGui::Button("replace")) ar[pos] = value;
        if (ImGui::Button("fill")) {
          for (size_t i = 0; i < fillSize; i++)
            ar.mem[i] = i;
          ar.size = fillSize;
        }

        ImGui::Text("capacity: %d \nsize: %d", ar.capacity, ar.size);

        for (size_t i = 0; i < ar.size; i++)
        {
          ImGui::Text("%d", ar.mem[i]);
          ImGui::SameLine();
        }
        ImGui::EndTabItem();
      } 
      if (ImGui::BeginTabItem("utf8")) {
        static char buf[128] = {0};
        ImGui::InputText("Input", buf, 128);
        
        const char* text = "Тест";
        int textLen = te_strlen(text) + 1;
        ImGui::Text("%s, len: %d", text, textLen);
        for (size_t i = 0; i < textLen; i++)
        {
          CharTypeU8 type = getCharTypeU8(text[i]);
          if (!(unsigned char)text[i] == (char)text[i]) {
            DebugBreak();
          }
          const char* charType = "NONE";
          switch (type)
          {
          case Char1Byte:
            charType = "1byte";
            break;
          case Char2Byte:
            charType = "2byte";
            break;
          case Char3Byte:
            charType = "3byte";
            break;
          case Char4Byte:
            charType = "4byte";
            break;
          case CharMiddle:
            charType = "middle";
            break;
          default:
            break;
          }

          ImGui::PushID(i);
          ImGui::Text("%d. Type: %s", i, charType);
          ImGui::PopID();
        }
        
        ImGui::EndTabItem();
      } 
    } ImGui::EndTabBar();
  } ImGui::End();
}
#endif