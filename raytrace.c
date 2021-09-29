#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glext.h"

#define SEED (0x0213de4208353f2eUL) // my lucky number

/*
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
*/

const char CLASS_NAME[] = "raytracer";

const float SCREEN_VERTICES[] = {
    -1.f, -1.f, 0.f,
    -1.f,  1.f, 0.f,
     1.f, -1.f, 0.f,
    -1.f,  1.f, 0.f,
     1.f,  1.f, 0.f,
     1.f, -1.f, 0.f
};

#include "shader.h"


static WORD winwidth = 1024;
static WORD winheight = 1024;


static GLuint shader_program;

static GLuint ichannel;
static GLuint ichannelloc;
static GLuint dimloc;
static int iframe = 0;
static GLuint iframeloc;


static unsigned long sps = SEED;
static unsigned int xos[4];

static inline unsigned int rotl(const unsigned int x, int k) {
	return (x << k) | (x >> (32 - k));
}

// adapted from http://prng.di.unimi.it/splitmix64.c
static unsigned long splitmix(void) {
    unsigned long  z = (sps += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

static void initxoshiro128p(void) {
    xos[0] = (unsigned int) splitmix();
    xos[1] = (unsigned int) splitmix();
    xos[2] = (unsigned int) splitmix();
    xos[3] = (unsigned int) splitmix();
}

static unsigned int xoshiro128p(void) {
    const unsigned int r = xos[0] + xos[3];
    const unsigned int t = xos[1] << 9;
    xos[2] ^= xos[0];
    xos[3] ^= xos[1];
    xos[1] ^= xos[2];
    xos[0] ^= xos[3];
    xos[2] ^= t;
    xos[3] = rotl(xos[3], 11);
    return r;
}

static float ranf(void) {
    union {
        unsigned int i;
        float f;
    } pun = { ( 0x3f800000U | (xoshiro128p() >> 9) ) };
    return pun.f - 1.f;
}


void *getGLproc(const char *name) {
  void *p = (void *)wglGetProcAddress(name);
  if(p == 0 ||
    (p == (void *)0x1) || (p == (void *)0x2) || (p == (void *)0x3) ||
    (p == (void *)-1) ) {
    HMODULE ogl = LoadLibraryA("opengl32.dll");
    p = (void *)GetProcAddress(ogl, name);
  }
  return p;
}


static void display(void) {
    if (!(iframe&7) && iframe<1024) {
        void *imgdata;
        register int towrite = winwidth * winheight;

        glBindTexture(GL_TEXTURE_2D, ichannel);
        imgdata = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
            16 * towrite);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, imgdata);

        float *b = (float *)imgdata;
        do {
            b += 3;
            *b++ = ranf();
        } while (--towrite);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
            winwidth, winheight, 0, GL_RGBA, GL_FLOAT, imgdata);

        HeapFree(GetProcessHeap(), 0, imgdata);
    }

    ((PFNGLUNIFORM1IPROC) getGLproc("glUniform1i"))
        (iframeloc, ++iframe);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glFlush();
}

static void resize(void) {
    void *imgdata;
    register int towrite = winwidth * winheight;

    glViewport(0, 0, winwidth, winheight);
    ((PFNGLUNIFORM2FPROC) getGLproc("glUniform2f"))
        (dimloc, (float) winwidth, (float) winheight);

    imgdata = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
        16 * towrite);

    float *b = (float *)imgdata;
    do {
        b += 3;
        *b++ = ranf();
    } while (--towrite);

    glBindTexture(GL_TEXTURE_2D, ichannel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
        winwidth, winheight, 0, GL_RGBA, GL_FLOAT, imgdata);

    HeapFree(GetProcessHeap(), 0, imgdata);

    ((PFNGLBINDIMAGETEXTUREPROC) getGLproc("glBindImageTexture"))
        (0, ichannel, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    iframe = 0;
    ((PFNGLUNIFORM1IPROC) getGLproc("glUniform1i"))
        (iframeloc, iframe);
}


LRESULT CALLBACK wind_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            winwidth = LOWORD(lParam);
            winheight = HIWORD(lParam);
            resize();
            PostMessage(hwnd, WM_PAINT, 0, 0);
            return 0;
        case WM_PAINT:
            PAINTSTRUCT ps;
            display();
            HDC hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
    LPSTR lpCmdLine, int nCmdShow) {

    HDC hdc;
    HGLRC hrc;
    HWND hwnd;
    MSG msg;

    {
        WNDCLASSEX wc;
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = wind_proc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInst;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = CLASS_NAME;
        wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

        RegisterClassEx(&wc);

        hwnd = CreateWindowEx(
            0,
            CLASS_NAME,
            "RAYTRACER",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, winwidth, winheight,
            NULL,
            NULL,
            hInst,
            NULL
        );
        if (!hwnd) return -1;
    } // create window

    hdc = GetDC(hwnd);

    {
        int pf;
        PIXELFORMATDESCRIPTOR pfd;

        register int i = sizeof(PIXELFORMATDESCRIPTOR);
        register char *p = (char *)&pfd;
        do {
            *p++ = 0;
        } while (--i);
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;

        pf = ChoosePixelFormat(hdc, &pfd);
        if (!pf) return -2;
        if (!SetPixelFormat(hdc, pf, &pfd)) return -3;
        DescribePixelFormat(hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    } // initialise pixel format

    hrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hrc);

    ShowWindow(hwnd, nCmdShow);

    initxoshiro128p();

    {
        GLuint vertex_shader;
        GLuint frag_shader;
        GLuint _vao;
        GLuint _vbo;
        void *imgdata;

        ((PFNGLGENVERTEXARRAYSPROC) getGLproc("glGenVertexArrays"))(1, &_vao);
        ((PFNGLBINDVERTEXARRAYPROC) getGLproc("glBindVertexArray"))(_vao);

        ((PFNGLGENBUFFERSPROC) getGLproc("glGenBuffers"))(1, &_vbo);
        ((PFNGLBINDBUFFERPROC) getGLproc("glBindBuffer"))
            (GL_ARRAY_BUFFER, _vbo);
        ((PFNGLBUFFERDATAPROC) getGLproc("glBufferData"))
            (GL_ARRAY_BUFFER, sizeof(SCREEN_VERTICES),
                SCREEN_VERTICES, GL_STATIC_DRAW);

        ((PFNGLVERTEXATTRIBPOINTERPROC) getGLproc("glVertexAttribPointer"))
            (0, 3, GL_FLOAT, GL_FALSE,
                3 * sizeof(float), (void *)0);
        ((PFNGLENABLEVERTEXATTRIBARRAYPROC)
            getGLproc("glEnableVertexAttribArray"))(0);

        vertex_shader = ((PFNGLCREATESHADERPROC)
            getGLproc("glCreateShader"))(GL_VERTEX_SHADER);
        ((PFNGLSHADERSOURCEPROC) getGLproc("glShaderSource"))
            (vertex_shader, 1, &vertex_source, NULL);
        ((PFNGLCOMPILESHADERPROC) getGLproc("glCompileShader"))(vertex_shader);

        frag_shader = ((PFNGLCREATESHADERPROC)
            getGLproc("glCreateShader"))(GL_FRAGMENT_SHADER);
        ((PFNGLSHADERSOURCEPROC) getGLproc("glShaderSource"))
            (frag_shader, 1, &frag_source, NULL);
        ((PFNGLCOMPILESHADERPROC) getGLproc("glCompileShader"))(frag_shader);

        shader_program =  ((PFNGLCREATEPROGRAMPROC)
            getGLproc("glCreateProgram"))();
        ((PFNGLATTACHSHADERPROC) getGLproc("glAttachShader"))
            (shader_program, vertex_shader);
        ((PFNGLATTACHSHADERPROC) getGLproc("glAttachShader"))
            (shader_program, frag_shader);
        ((PFNGLLINKPROGRAMPROC) getGLproc("glLinkProgram"))(shader_program);

// -- debug
    /*{

        PFNGLGETSHADERIVPROC glGetProgramiv = (PFNGLGETPROGRAMIVPROC) wglGetProcAddress("glGetProgramiv");

		PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetProgramInfoLog");
		//PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");

		char str[512];
		glGetProgramInfoLog(shader_program, sizeof(str), NULL, str);
		//glGetShaderInfoLog(shader, sizeof(str), NULL, str);
		OutputDebugString("\n\n");
		OutputDebugString(str);
		OutputDebugString("\n\n\n");

		GLint compiled;
		glGetProgramiv(shader_program, GL_LINK_STATUS, &compiled);
		if (!compiled)
		{
			MessageBox(hwnd, str, "Error", MB_OK);
			return -1;
		}
    }*/
// --

        ((PFNGLDELETESHADERPROC) getGLproc("glDeleteShader"))(vertex_shader);
        ((PFNGLDELETESHADERPROC) getGLproc("glDeleteShader"))(frag_shader);

        ((PFNGLUSEPROGRAMPROC) getGLproc("glUseProgram"))(shader_program);
        ((PFNGLBINDVERTEXARRAYPROC) getGLproc("glBindVertexArray"))(_vao);

        dimloc = ((PFNGLGETUNIFORMLOCATIONPROC)
            getGLproc("glGetUniformLocation"))(shader_program, "u_res");
        ((PFNGLUNIFORM2FPROC) getGLproc("glUniform2f"))
            (dimloc, (float)winwidth, (float)winheight);
        iframeloc = ((PFNGLGETUNIFORMLOCATIONPROC)
            getGLproc("glGetUniformLocation"))(shader_program, "iframe");
        ichannelloc = ((PFNGLGETUNIFORMLOCATIONPROC)
            getGLproc("glGetUniformLocation"))(shader_program, "ichannel");
        ((PFNGLUNIFORM1IPROC) getGLproc("glUniform1i"))
            (ichannelloc, 0);

        glGenTextures(1, &ichannel);
        resize();

    } // shader loading

    UpdateWindow(hwnd);

    do {
        display();
        while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
            if (GetMessage(&msg, NULL, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } else goto cleanup;
        }
        Sleep(7);
    } while (1);
cleanup:

    wglMakeCurrent(NULL, NULL);
    ReleaseDC(hwnd, hdc);
    wglDeleteContext(hrc);
    DestroyWindow(hwnd);

    return msg.wParam;
}

void __stdcall WinMainCRTStartup() {
    //__security_init_cookie();
    int result = WinMain(GetModuleHandle(NULL), 0, 0, SW_SHOWDEFAULT);
    ExitProcess(result);
}
