#include "Shaders.hpp"

#include <IMGUI/imgui.h>
#include <IMGUI/imgui_impl_glfw.h>
#include <IMGUI/imgui_impl_opengl3.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

// FreeType includes
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#include FT_MODULE_H
#include FT_ADVANCES_H
#include FT_GLYPH_H
#include FT_DRIVER_H

#define ASSERT(x) if (!(x)) { fprintf(stderr, "Assertion failed: %s\n", #x); exit(1); }
#define HEXTOFLOAT(hex) \
        (float)((hex >> 16) & 0xFF) / 255.0f, \
        (float)((hex >> 8) & 0xFF) / 255.0f, \
        (float)(hex & 0xFF) / 255.0f, \
        1.0f \

std::string inputText("Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.");
bool textChanged = true;

/*
    - width: the width (in pixels) of the bitmap accessed via face->glyph->bitmap.width.
    - height: the height (in pixels) of the bitmap accessed via face->glyph->bitmap.rows.
    - bearingX: the horizontal bearing e.g. the horizontal position (in pixels) of the bitmap relative to the origin accessed via face->glyph->bitmap_left.
    - bearingY: the vertical bearing e.g. the vertical position (in pixels) of the bitmap relative to the baseline accessed via face->glyph->bitmap_top.
    - advance: the horizontal advance e.g. the horizontal distance (in 1/64th pixels) from the origin to the origin of the next glyph. Accessed via face->glyph->advance.x.
*/
typedef struct 
{
    int x;          // Glyph offset X (bearing X)
    int y;          // Glyph offset Y (bearing Y)
    int w;          // Glyph width
    int h;          // Glyph height

    float s0;       // Texture coordinates (left)
    float s1;       // Texture coordinates (right)
    float t0;       // Texture coordinates (top)
    float t1;       // Texture coordinates (bottom)

    float advance;
    char  charCode;
}GlyphData;

#pragma pack(push, 1)
typedef struct 
{
    float x, y;     // Position
    float s, t;     // Texture coordinates
}Vertex;
#pragma pack(pop)

struct gc_t
{
    GLFWwindow* window;

    int width;
    int height;

    int mouseLastX;
    int mouseX;
    int mouseLastY;
    int mouseY;
    bool firstMouse = true;

    bool debug;
    bool reload;

    float deltaTime;
    float currentTime;
    float lastFrame;
};
gc_t gc;

struct Ui
{
    float rotation      = 0.0f;
    float bgcol[3]      = { 0.157f, 0.165f, 0.212f };
    float fontcol[3]    = { 0.973f, 0.973f, 0.867f };
    float clipBounds[3] = {300.0f, 300.0f, 0.867f };
    float textPos[3]    = {0.0f, 0.0f, 0.0f };

    bool clipText       = false;
    bool showBounds     = false;

    char  str0[128];

    Ui(GLFWwindow *window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO &io = ImGui::GetIO(); (void) io;

        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        //ImGui::StyleColorsDark();
        DarkTheme();
    }

    void DarkTheme()
    {
        ImVec4 *colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
        colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
        colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
        colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        // colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        // colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

        ImGuiStyle &style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(8.00f, 8.00f);
        style.FramePadding = ImVec2(5.00f, 2.00f);
        style.CellPadding = ImVec2(6.00f, 6.00f);
        style.ItemSpacing = ImVec2(6.00f, 6.00f);
        style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
        style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
        style.IndentSpacing = 25;
        style.ScrollbarSize = 15;
        style.GrabMinSize = 10;
        style.WindowBorderSize = 1;
        style.ChildBorderSize = 1;
        style.PopupBorderSize = 1;
        style.FrameBorderSize = 1;
        style.TabBorderSize = 1;
        style.WindowRounding = 7;
        style.ChildRounding = 4;
        style.FrameRounding = 3;
        style.PopupRounding = 4;
        style.ScrollbarRounding = 9;
        style.GrabRounding = 3;
        style.LogSliderDeadzone = 4;
        style.TabRounding = 4;
    }

    void beginFrame() 
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void debugWindow()
    {
        ImGui::Begin("Debug");
            if (ImGui::TreeNode("General")){
                ImGui::ColorEdit3("Background Color", bgcol);
                ImGui::ColorEdit3("Font Color", fontcol);
                ImGui::SliderFloat2("TextPos", textPos, 0.0f, (float)gc.width);
                ImGui::SliderFloat("rotation", &rotation, 0, 3.14f); 
                ImGui::SliderFloat2("Bounds", clipBounds, 0.0f, (float)gc.width);

                ImGui::Checkbox("clipText", &clipText);
                ImGui::Checkbox("showBounds", &showBounds);

                if(ImGui::Button("Reload Text Shader")){
                    gc.reload = true;
                }

                const char* items[] = {"7", "13", "20", "32", "50", "65", "100", "160", "200", "325", "600", "3250"};
                static int item_selected_idx = 6;

                const char* combo_preview_value = items[item_selected_idx];

                if (ImGui::BeginCombo("Sizes", combo_preview_value, 0))
                {
                    for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                    {
                        const bool is_selected = (item_selected_idx == n);
                        if (ImGui::Selectable(items[n], is_selected)){
                            item_selected_idx = n;
                        }
                        if (is_selected){
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::Checkbox("Debug", &gc.debug);
                ImGui::TreePop();
            }
            sprintf_s(str0, "Time: %f ms/frame", gc.deltaTime*1000.0f);
            ImGui::Text(str0);

        ImGui::End();
    }

    void demoWindow()
    {
        ImGui::ShowDemoWindow();
    }

    void render() 
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
};
Ui *ui;

struct Text
{
    FT_Library              ftLibrary;
    FT_Face                 ftFace;

    // the way I understand it that some characters that 
    // exceed the cap line either above or below
    int                     ascender;   
    int                     descender;

    // Atlas dimensions
    int                     ATLAS_WIDTH;
    int                     ATLAS_HEIGHT;
    GLuint                  atlasTexture;

    std::vector<GlyphData>  glyphs;

    // number of renderable glyphs that the text contains
    int                     glyphCount;

    unsigned char           ascii_low  = 32;
    unsigned char           ascii_high = 126;

    int                     lineHeight;

    GLuint                  vao;
    GLuint                  vbo;
    GLuint                  ibo;

    GLuint                  shaderProgram;

    glm::vec2               position            = {0.0f, 0.0f};
    glm::vec2               scale               = {1.0f, 1.0f};                    // Scale 
    float                   rotation            = 0.0f;      
    glm::vec4               bounds              = {0.0f, 0.0f, 300.0f, 300.0f};    // x, y, width, height
    bool                    clipText            = true;                            // Whether to clip text to bounds

    bool                    showBounds          = false;
    GLuint                  boundingBoxVAO      = 0;
    GLuint                  boundingBoxVBO      = 0;
    glm::vec3               boundingBoxColor    = {1.0f, 0.0f, 0.0f}; 

    bool                    enableWordWrap = true;

    Text(float startX, float startY, const char *fontPath, int size)
    {
        position = {startX, startY};

        if (FT_Init_FreeType(&ftLibrary)) {
            fprintf(stderr, "Failed to initialize FreeType\n");
        }
        
        // subpixel rendering settings
        FT_UInt interpVer = TT_INTERPRETER_VERSION_40;
        FT_Property_Set(ftLibrary, "truetype", "interpreter-version", &interpVer);
        FT_Library_SetLcdFilter(ftLibrary, FT_LCD_FILTER_DEFAULT);
        
        if (FT_New_Face(ftLibrary, fontPath, 0, &ftFace)) {
            fprintf(stderr, "Failed to load font: %s\n", fontPath);
            exit(1);
        }
        
        // Set font size (0 means dynamically calculate width based on height)
        FT_Set_Pixel_Sizes(ftFace, 0, size);
        
        // Get font metrics
        ascender  = ftFace->size->metrics.ascender  >> 6;
        descender = ftFace->size->metrics.descender >> 6; 
        
        printf("Font metrics: ascender=%d, descender=%d\n", ascender, descender);

        initShaders();
        initBoundingBox();
    }

    Text(const char *fontPath, int size) : Text(0.0f, 0.0f, fontPath, size){}

    void initShaders(void)
    {
        std::string vertexShaderSource   = readShaderSource("..\\shaders\\vs.glsl"); 
        std::string fragmentShaderSource = readShaderSource("..\\shaders\\fs.glsl"); 

        shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    }

    void updateShaders(void)
    {
        glDeleteProgram(shaderProgram);
        initShaders();
    }

    void initBoundingBox()
    {
        float vertices[] = 
        {
            0.0f, 0.0f,     // Bottom-left
            1.0f, 0.0f,     // Bottom-right
            1.0f, 1.0f,     // Top-right
            0.0f, 1.0f,     // Top-left
            
            0.0f, 0.0f,     // Repeat first vertex to close the loop
        };
        
        glGenVertexArrays(1, &boundingBoxVAO);
        glGenBuffers(1, &boundingBoxVBO);
        
        glBindVertexArray(boundingBoxVAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, boundingBoxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    int nextPowerOfTwo(int n) 
    {
        int power = 1;
        while (power < n) {
            power *= 2;
        }
        return power;
    }

    void createASCIIAtlas()
    {
        // The dumbest way I could think of to create the atlas
        int maxWidth    = 0;
        int maxHeight   = 0;
        int padding     = 1;

        for (unsigned char c = ascii_low; c < ascii_high; c++) 
        {
            if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) {
                continue;  
            }
            maxWidth  = std::max(maxWidth,  (int)ftFace->glyph->bitmap.width);
            maxHeight = std::max(maxHeight, (int)ftFace->glyph->bitmap.rows);
        }

        int maxGlyphWidth  = maxWidth  + padding * 2;
        int maxGlyphHeight = maxHeight + padding * 2;

        lineHeight = maxGlyphHeight;

        int numChars = ascii_high - ascii_low;

        int numCols = ceil(sqrt(numChars));
        int numRows = ceil(numChars / (float)numCols);

        ATLAS_WIDTH  = maxGlyphWidth * numCols;
        ATLAS_HEIGHT = maxGlyphHeight * numRows;
        
        glGenTextures(1, &atlasTexture);
        glBindTexture(GL_TEXTURE_2D, atlasTexture);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ATLAS_WIDTH, ATLAS_HEIGHT,
                     0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        std::cout << "Atlas dimensions: " << ATLAS_WIDTH << "x" << ATLAS_HEIGHT << std::endl;
        std::cout << "Glyphs: " << numRows << " rows, " << numCols << " columns" << std::endl;
        
        glyphs.resize(numChars);
        
        int x = 0; // Current x position in atlas
        int y = 0; // Current y position in atlas
        
        for (int i = 0; i < numChars; ++i) 
        {
            char currentChar = ascii_low + i;
            
            FT_UInt glyphIndex = FT_Get_Char_Index(ftFace, currentChar);
            
            // Load and render glyph with subpixel rendering (LCD mode)
            FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_TARGET_LCD);
            FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_LCD);
            
            int glyphWidth  = (ftFace->glyph->bitmap.width / 3); // divide by three because LCD triples width
            int glyphHeight = ftFace->glyph->bitmap.rows;
            
            // Handle space character 
            if (currentChar == ' ') 
            {
                x += (ftFace->glyph->advance.x >> 6) + padding*2;  // Add padding
                continue;
            }
            
            // Check if we need to move to next row
            if (x + glyphWidth + padding*2 > ATLAS_WIDTH) 
            {
                x = 0;
                y += maxGlyphHeight + padding*2; // Move to next row with padding
                
                if (y > ATLAS_HEIGHT) {
                    fprintf(stderr, "Error: Atlas height exceeded. Increase ATLAS_HEIGHT.\n");
                    return;
                }
            }

            glyphs[i].charCode = currentChar;
            
            // Store glyph metrics
            glyphs[i].x = ftFace->glyph->bitmap_left;
            glyphs[i].y = ftFace->glyph->bitmap_top;
            glyphs[i].w = glyphWidth;
            glyphs[i].h = glyphHeight;
            
            glyphs[i].advance = ftFace->glyph->advance.x >> 6;
            
            // Calculate texture coordinates
            glyphs[i].s0 = (float)x / (float)ATLAS_WIDTH;
            glyphs[i].t0 = (float)y / (float)ATLAS_HEIGHT;
            glyphs[i].s1 = (float)(x + glyphWidth) / (float)ATLAS_WIDTH;
            glyphs[i].t1 = (float)(y + glyphHeight) / (float)ATLAS_HEIGHT;
            
            
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, glyphWidth, glyphHeight, 
                           GL_RGB, GL_UNSIGNED_BYTE, ftFace->glyph->bitmap.buffer);
            
            x += glyphWidth + padding*2;
        }
        
        printf("ASCII Atlas created successfully with %d glyphs\n", numChars);
    }

    const GlyphData* findGlyph(char c) 
    {
        int numChars = ascii_high - ascii_low;
        int index = c - 32;  // Convert from ASCII to array index
        if (index >= 0 && index < numChars) {
            return &glyphs[index];
        }
        return nullptr;  // Character not found
    }

    void wrapText(std::string& text) 
    {
        if (text.empty()) {
            return;
        }

        float availableWidth = bounds.z;  // Width of the bounding box
        
        std::vector<std::string> words;
        std::string currentWord;
        
        for (size_t i = 0; i < text.length(); ++i) 
        {
            char c = text[i];
            
            if (c == ' ' || c == '\n') 
            {
                if (!currentWord.empty()) {
                    words.push_back(currentWord);
                    currentWord.clear();
                }
                
                if (c == '\n') {
                    words.push_back("\n");
                } else {
                    words.push_back(" ");
                }
            } 
            else 
            {
                currentWord += c;
            }
        }
        
        if (!currentWord.empty()) {
            words.push_back(currentWord);
        }
        
        std::string wrappedText;
        std::string currentLine;
        float lineWidth = 0.0f;
        
        for (size_t i = 0; i < words.size(); ++i) 
        {
            const std::string& word = words[i];
            
            if (word == "\n") {
                wrappedText += currentLine + "\n";
                currentLine = "";
                lineWidth = 0.0f;
                continue;
            }
            
            float wordWidth = 0.0f;
            for (char c : word) 
            {
                if (c == ' ') {
                    wordWidth += (ftFace->glyph->advance.x >> 6);
                    continue;
                }
                
                const GlyphData* glyph = findGlyph(c);
                if (glyph) {
                    wordWidth += glyph->advance;
                }
            }
            
            if (lineWidth + wordWidth <= availableWidth || currentLine.empty()) 
            {
                currentLine += word;
                lineWidth += wordWidth;
            } 
            else 
            {
                wrappedText += currentLine + "\n";
                currentLine = word;
                lineWidth = wordWidth;
            }
        }
        // Add the last line if there is one
        if (!currentLine.empty()) {
            wrappedText += currentLine;
        }
        
        text = wrappedText;
    }

    void renderText(std::string &text) 
    {
        if (text.empty()){
            return; 
        }

        std::string processedText = text;

        if (enableWordWrap && clipText && bounds.z > 0) {
            wrapText(processedText);
        }

        renderTextInternal(processedText);
    }

    void renderTextInternal(std::string &text)
    {
        if (text.empty()){
            return; 
        }

        glBindTexture(GL_TEXTURE_2D, atlasTexture);
        
        glBlendFuncSeparate(GL_SRC1_COLOR, GL_ONE_MINUS_SRC1_COLOR, 
                            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        float currentX = 0;
        float currentY = 0;
        
        // first pass to see how many renderable glyphs 
        glyphCount = 0;
        for (char c : text) {
            if (c != '\n' && c != ' ') {
                glyphCount++;
            }
        }
        if (glyphCount == 0){
            return;
        } 
        
        std::vector<Vertex> vertices(4 * glyphCount);
        std::vector<GLuint> indices(6 * glyphCount);
        
        int charIndex = 0;
        
        for (char c : text) 
        {
            if (c == '\n') {
                currentX = 0;
                currentY += lineHeight;
                continue; 
            }
            
            int glyphIndex = c - 32;
            
            int numChars = ascii_high - ascii_low;

            if (glyphIndex < 0 || glyphIndex >= numChars)
            {
                continue;
            } 
            
            // just advance the cursor
            if (c == ' ') {
                currentX += (ftFace->glyph->advance.x >> 6);
                continue;
            }
            
            float x0 = currentX + glyphs[glyphIndex].x;
            float y0 = currentY + ascender - glyphs[glyphIndex].y - descender;
            float x1 = x0 + glyphs[glyphIndex].w;
            float y1 = y0 + glyphs[glyphIndex].h;

            // Optimization, dont render it if 
            if (clipText) 
            {
                float boundsLeft    = position.x;
                float boundsTop     = position.y;
                float boundsRight   = position.x + bounds.z;
                float boundsBottom  = position.y + bounds.w;
                
                if (x0 > boundsRight || 
                    y0 > boundsBottom) 
                {
                    currentX += glyphs[glyphIndex].advance;
                    continue;
                }
            }
            
            // Top-left vertex
            vertices[4*charIndex + 0].x = x0;
            vertices[4*charIndex + 0].y = y0;
            vertices[4*charIndex + 0].s = glyphs[glyphIndex].s0;
            vertices[4*charIndex + 0].t = glyphs[glyphIndex].t0;
            
            // Bottom-left vertex
            vertices[4*charIndex + 1].x = x0;
            vertices[4*charIndex + 1].y = y1;
            vertices[4*charIndex + 1].s = glyphs[glyphIndex].s0;
            vertices[4*charIndex + 1].t = glyphs[glyphIndex].t1;
            
            // Bottom-right vertex
            vertices[4*charIndex + 2].x = x1;
            vertices[4*charIndex + 2].y = y1;
            vertices[4*charIndex + 2].s = glyphs[glyphIndex].s1;
            vertices[4*charIndex + 2].t = glyphs[glyphIndex].t1;
            
            // Top-right vertex
            vertices[4*charIndex + 3].x = x1;
            vertices[4*charIndex + 3].y = y0;
            vertices[4*charIndex + 3].s = glyphs[glyphIndex].s1;
            vertices[4*charIndex + 3].t = glyphs[glyphIndex].t0;
            
            indices[6*charIndex + 0] = 4*charIndex + 0; // Top-left
            indices[6*charIndex + 1] = 4*charIndex + 1; // Bottom-left
            indices[6*charIndex + 2] = 4*charIndex + 3; // Top-right
            indices[6*charIndex + 3] = 4*charIndex + 3; // Top-right
            indices[6*charIndex + 4] = 4*charIndex + 1; // Bottom-left
            indices[6*charIndex + 5] = 4*charIndex + 2; // Bottom-right
            
            currentX += glyphs[glyphIndex].advance;
            charIndex++;
        }
        
        if (charIndex == 0)
        {
            return;
        } 

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);

        glBindVertexArray(vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), 
                     vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                     indices.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void draw()
    {
        if(gc.reload){
            updateShaders();
            gc.reload = false;
        }

        glUseProgram(shaderProgram);
        
        glm::mat4 projection = glm::ortho(0.0f, (float)gc.width, (float)gc.height,
                                          0.0f, -1.0f, 1.0f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f));
        model = glm::translate(model, glm::vec3(bounds.z * 0.5f, bounds.w * 0.5f, 0.0f));
        model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(-bounds.z * 0.5f, -bounds.w * 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(scale.x, scale.y, 1.0f));
        glm::mat4 mvp = projection * model;

        setMat4(shaderProgram, "uProjection", mvp);
        
        setVec3(shaderProgram, "uTextColor", glm::vec3(ui->fontcol[0], ui->fontcol[1], ui->fontcol[2]));

        float currentTime = glfwGetTime(); 
        setFloat(shaderProgram, "uTime", currentTime);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, atlasTexture);

        setInt(shaderProgram, "uAtlasTexture", 0);

        if (clipText) 
        {
            glEnable(GL_SCISSOR_TEST);
            
            int scissorX      = position.x;
            int scissorY      = gc.height - (position.y + bounds.w); // OpenGL has bottom-left origin
            int scissorWidth  = bounds.z;
            int scissorHeight = bounds.w;
            
            glScissor(scissorX, scissorY, scissorWidth, scissorHeight);
        }
        
        glBlendFuncSeparate(GL_SRC1_COLOR, GL_ONE_MINUS_SRC1_COLOR, 
                            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glBindVertexArray(vao);
        
        glDrawElements(GL_TRIANGLES, 6 * glyphCount, GL_UNSIGNED_INT, 0);

        if (clipText) {
            glDisable(GL_SCISSOR_TEST);
        }

        if (showBounds) 
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

            GLuint lineShader = getLineShader(); 
            glUseProgram(lineShader);
            
            glm::mat4 boxModel = glm::mat4(1.0f);
            boxModel = glm::translate(boxModel, glm::vec3(position.x, position.y, 0.0f));
            
            boxModel = glm::translate(boxModel, glm::vec3(bounds.z * 0.5f, bounds.w * 0.5f, 0.0f));
            boxModel = glm::rotate(boxModel, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
            boxModel = glm::translate(boxModel, glm::vec3(-bounds.z * 0.5f, -bounds.w * 0.5f, 0.0f));
            
            boxModel = glm::scale(boxModel, glm::vec3(bounds.z, bounds.w, 1.0f));
            
            glm::mat4 boxMVP = projection * boxModel;
            
            setMat4(lineShader, "uMVP", boxMVP);
            
            setVec3(lineShader, "uColor", boundingBoxColor);
            
            glBindVertexArray(boundingBoxVAO);
            glLineWidth(2.0f); 
            
            glDrawArrays(GL_LINE_STRIP, 0, 5); // 5 vertices to close the box
            
            glLineWidth(1.0f); // Reset line width
            glBlendFuncSeparate(GL_SRC1_COLOR, GL_ONE_MINUS_SRC1_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
        }
    }

    GLuint getLineShader()
    {
        static GLuint lineShader = 0;
        
        if (lineShader == 0) 
        {
            std::string lineVS = readShaderSource("..\\shaders\\line_vs.glsl"); 
            std::string lineFS = readShaderSource("..\\shaders\\line_fs.glsl"); 
            lineShader = createShaderProgram(lineVS, lineFS);
        }
        
        return lineShader;
    }
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;
    gc.width = width;
    gc.height = height;
    glViewport(0, 0, gc.width, gc.height);
}

void errorCallback(int error, const char* description) 
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void window_refresh_callback(GLFWwindow* window)
{
    glfwSwapBuffers(window);
}

void getMouseDelta(float *xoffset, float *yoffset)
{
    if (gc.firstMouse)
    {
        gc.mouseLastX = (float)gc.mouseX;
        gc.mouseLastY = (float)gc.mouseY;
        gc.firstMouse = false;
    }

    *xoffset = (float)gc.mouseX - gc.mouseLastX;
    *yoffset = gc.mouseLastY - (float)gc.mouseY; // reversed since y-coordinates range from bottom to top

    gc.mouseLastX = (float)gc.mouseX;
    gc.mouseLastY = (float)gc.mouseY;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    (void)window;

    gc.mouseX = (float)xpos;
    gc.mouseY = (float)ypos;

    float xoffset, yoffset;
    getMouseDelta(&xoffset, &yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) 
{
    (void)window;
    (void)xoffset;
    (void)yoffset;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT) 
    {
        if (key == GLFW_KEY_BACKSPACE && !inputText.empty()) {
            inputText.pop_back();
            textChanged = true;
        }
        else if (key == GLFW_KEY_ENTER) {
            inputText += '\n';
            textChanged = true;
        }
        else if(key == GLFW_KEY_TAB)
        {
            for(int i = 0; i < 4; i++){
                inputText += ' ';
            }
            textChanged = true;
        }
    }
}

void charCallback(GLFWwindow* window, unsigned int codepoint) 
{
    if (codepoint >= 32 && codepoint <= 126) {
        inputText += static_cast<char>(codepoint);
        textChanged = true;
    }
}

bool init_GL(void)
{
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 8); // Enable multisampling for smoother edges
    
    gc.window = glfwCreateWindow(gc.width, gc.height,
                                 "FreeType Text Demo", 
                                  NULL, NULL);
    if (!gc.window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(gc.window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW initialization error: %s\n", glewGetErrorString(err));
        return 1;
    }

    glViewport(0, 0, gc.width, gc.height);
    
    glfwSetKeyCallback(gc.window, keyCallback);
    glfwSetCharCallback(gc.window, charCallback);
    glfwSetErrorCallback(errorCallback);
    glfwSetFramebufferSizeCallback(gc.window, framebuffer_size_callback);
    glfwSetWindowRefreshCallback(gc.window, window_refresh_callback);
    glfwSetCursorPosCallback(gc.window, mouse_callback);
    glfwSetScrollCallback(gc.window, scroll_callback);
    
    glEnable(GL_BLEND);

    glfwSwapInterval(1);
    
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

void cleanup_GL(void)
{
    glfwDestroyWindow(gc.window);
    glfwTerminate();
}

void clearBackground(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int main(void) 
{
    gc.width  = 800;
    gc.height = 600;

    init_GL();
    ui = new Ui(gc.window);

    Text text("..\\assets\\CaskaydiaCoveNerdFont-Regular.ttf", 24);
    text.createASCIIAtlas();
    
    while (!glfwWindowShouldClose(gc.window)) 
    {
        gc.currentTime = (float)glfwGetTime();
        gc.deltaTime = gc.currentTime - gc.lastFrame;
        gc.lastFrame = gc.currentTime;
    
        // clearBackground(HEXTOFLOAT(0x282a36));
        clearBackground(ui->bgcol[0],ui->bgcol[1],ui->bgcol[2],1.0f);

        ui->beginFrame();

        // ui->demoWindow();
        ui->debugWindow();

        text.bounds.z   = ui->clipBounds[0];
        text.bounds.w   = ui->clipBounds[1];
        text.position.x = ui->textPos[0];
        text.position.y = ui->textPos[1];
        text.rotation   = ui->rotation;
        text.showBounds = ui->showBounds;
        text.clipText   = ui->clipText;

        // if(textChanged)
        // {
            text.renderText(inputText);
            textChanged = false;
        // }

        text.draw();

        ui->render();
        
        glfwSwapBuffers(gc.window);
        glfwPollEvents();
    }
    
    // // Delete OpenGL objects
    // glDeleteVertexArrays(1, &vao);
    // glDeleteBuffers(1, &vbo);
    // glDeleteBuffers(1, &ibo);
    // glDeleteTextures(1, &gc.atlasTexture);
    // glDeleteProgram(shaderProgram);
    
    // // Clean up FreeType resources
    // FT_Done_Face(gc.ftFace);
    // FT_Done_FreeType(ftLibrary);
    
    cleanup_GL();
    
    return 0;
}
