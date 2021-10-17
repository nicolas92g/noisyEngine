#include "utils.h"

#include <fstream>
#include <iostream>
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

YAML::Node ns::conf = ns::loadAndCheckFile(CONFIG_FILE);

std::string ns::to_string(const glm::vec2& vec)
{
	return "( " + std::to_string(vec.x) + ", " + std::to_string(vec.y) + " )";
}

std::string ns::to_string(const glm::vec3& vec)
{
	return "( " + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + " )";
}

std::string ns::to_string(const glm::vec4& vec)
{
	return "( " + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ", " + std::to_string(vec.w) + " )";
}

std::string ns::to_string(const glm::ivec2& vec)
{
	return "( " + std::to_string(vec.x) + ", " + std::to_string(vec.y) + " )";
}

std::string ns::to_string(const glm::ivec3& vec)
{
	return "( " + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + " )";
}

std::string ns::to_string(const glm::ivec4& vec)
{
	return "( " + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ", " + std::to_string(vec.w) + " )";
}

std::string ns::to_string(const glm::mat4& mat)
{
	return ns::to_string(mat[0]) + ns::to_string(mat[1]) + ns::to_string(mat[2]) + ns::to_string(mat[3]);
}

glm::vec3 ns::to_vec3(const aiVector3D& vec)
{
	return { vec.x, vec.y, vec.z };
}

glm::vec3 ns::to_vec3(const aiColor3D& vec)
{
	return { vec.r, vec.g, vec.b };
}

//glm::vec3 ns::to_vec3(const ofbx::Color& vec)
//{
//	return {vec.r, vec.g, vec.b};
//}
//
//glm::vec3 ns::to_vec3(const ofbx::Vec3& vec)
//{
//	return { vec.x, vec.y, vec.z };
//}
//
//glm::vec4 ns::to_vec4(const ofbx::Vec4& vec)
//{
//	return { vec.x, vec.y, vec.z, vec.w };
//}

void ns::to_mat4(glm::mat4& output, const aiMatrix4x4* mat)
{
	for (char i = 0; i < 4; i++) for (char j = 0; j < 4; j++) output[i][j] = (*mat)[i][j];
}

void ns::clearConfigFile()
{
	std::ofstream file(CONFIG_FILE);
	file << "";
	file.close();
}

glm::vec4 ns::getClearColor()
{
	GLfloat bk[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, bk);
	return *reinterpret_cast<glm::vec4*>(&bk);
}

void ns::SetupImGuiStyle(bool bStyleDark_, float alpha_)
{
    ImGuiStyle& style = ImGui::GetStyle();

    // light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
    style.Alpha = 1.0f;
    style.FrameRounding = 3.0f;
    style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
    //style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    //style.Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    //style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    //style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    //style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    //style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
    //style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
    //style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    //style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

    if (bStyleDark_)
    {
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
            ImVec4& col = style.Colors[i];
            float H, S, V;
            ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

            if (S < 0.1f)
            {
                V = 1.0f - V;
            }
            ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
            if (col.w < 1.00f)
            {
                col.w *= alpha_;
            }
        }
    }
    else
    {
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
            ImVec4& col = style.Colors[i];
            if (col.w < 1.00f)
            {
                col.x *= alpha_;
                col.y *= alpha_;
                col.z *= alpha_;
                col.w *= alpha_;
            }
        }
    }
}

void ns::saveConfiguration()
{
    std::ofstream file(CONFIG_FILE);
    file << conf;
}

bool ns::isFileExist(const std::string& filepath)
{
    std::ifstream file(filepath);
    return file.is_open();
}

YAML::Node ns::loadAndCheckFile(const std::string& filename)
{
    if (isFileExist(filename)) {
        try {
            return YAML::LoadFile(filename);
        }
        catch (...) {
            dout << "failed to read the configuration file !\n";
            return YAML::Node();
        }
    }
    dout << "failed to load the configuration file at : " << CONFIG_FILE << newl;
    return YAML::Node();
}

constexpr float grad(int32_t hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

constexpr int32_t fastfloor(float n) {
    const int32_t i = static_cast<int32_t>(n);
    return (n < i) ? (i - 1) : (i);
}

constexpr uint8_t hash(int32_t i) {
    constexpr uint8_t perm[256] = {
        151, 160, 137, 91, 90, 15,
        131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
        190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
        88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
        77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
        102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
        135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
        5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
        223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
        129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
        251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
        49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
        138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
    };
    return perm[static_cast<uint8_t>(i)];
}

float ns::noise(const glm::vec3& input)
{
    float n0, n1, n2, n3;

    constexpr float F3 = .333333333f;
    constexpr float G3 = .166666666f;

    const float s = (input.x + input.y + input.z) * F3; 
    const int i = fastfloor(input.x + s);
    const int j = fastfloor(input.y + s);
    const int k = fastfloor(input.z + s);
    const float t = (i + j + k) * G3;
    const float X0 = i - t; 
    const float Y0 = j - t;
    const float Z0 = k - t;
    const float x0 = input.x - X0;
    const float y0 = input.y - Y0;
    const float z0 = input.z - Z0;

    int i1, j1, k1; 
    int i2, j2, k2;

    if (x0 >= y0) {
        if (y0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
        }
        else if (x0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1;
        }
        else {
            i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1;
        }
    }
    else {
        if (y0 < z0) {
            i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1;
        }
        else if (x0 < z0) {
            i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1;
        }
        else {
            i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
        }
    }

    const float x1 = x0 - i1 + G3;
    const float y1 = y0 - j1 + G3;
    const float z1 = z0 - k1 + G3;
    const float x2 = x0 - i2 + 2.f * G3;
    const float y2 = y0 - j2 + 2.f * G3;
    const float z2 = z0 - k2 + 2.f * G3;
    const float x3 = x0 - 1.f + 3.f * G3; 
    const float y3 = y0 - 1.f + 3.f * G3;
    const float z3 = z0 - 1.f + 3.f * G3;

    const int gi0 = hash(i + hash(j + hash(k)));
    const int gi1 = hash(i + i1 + hash(j + j1 + hash(k + k1)));
    const int gi2 = hash(i + i2 + hash(j + j2 + hash(k + k2)));
    const int gi3 = hash(i + 1 + hash(j + 1 + hash(k + 1)));

    float t0 = .6f - x0 * x0 - y0 * y0 - z0 * z0;
    if (t0 < 0) {
        n0 = 0.f;
    }
    else {
        t0 *= t0;
        n0 = t0 * t0 * grad(gi0, x0, y0, z0);
    }

    float t1 = .6f - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 < 0) {
        n1 = 0.f;
    }
    else {
        t1 *= t1;
        n1 = t1 * t1 * grad(gi1, x1, y1, z1);
    }

    float t2 = .6f - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 < 0) {
        n2 = 0.f;
    }
    else {
        t2 *= t2;
        n2 = t2 * t2 * grad(gi2, x2, y2, z2);
    }

    float t3 = .6f - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 < 0) {
        n3 = 0.f;
    }
    else {
        t3 *= t3;
        n3 = t3 * t3 * grad(gi3, x3, y3, z3);
    }

    return 32.f * (n0 + n1 + n2 + n3);
}

glm::vec3 ns::genNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    static glm::vec3 normal;
    const glm::vec3 U = b - a;
    const glm::vec3 V = c - a;

    normal.x = (U.y * V.z) - (U.z * V.y);
    normal.y = (U.z * V.x) - (U.x * V.z);
    normal.z = (U.x * V.y) - (U.y * V.x);
    return normal;
}
