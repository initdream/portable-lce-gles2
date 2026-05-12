#pragma once

// windows hack: Windows SDK OpenGL headers include WINGDIAPI in their declarations,
// which can only be found in the Windows API
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include <glad/glad.h>
#include <SDL_video.h>

#include <cstdio>

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88B4
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif
#ifndef GL_PROJECTION
#define GL_PROJECTION 0x1701
#endif
#ifndef GL_PROJECTION_MATRIX
#define GL_PROJECTION_MATRIX 0x0BA7
#endif
#ifndef GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX 0x0BA6
#endif
#ifndef GL_EXP
#define GL_EXP 0x0800
#endif

static inline bool gl3_load() {
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "[gl_loader] ERROR: GLAD initialization failed\n");
        return false;
    }
    #ifdef GLES
    if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "[gl_loader] ERROR: GLAD GLES2 loader failed\n");
    }
    #endif
    if (!GLAD_GL_VERSION_3_3 && !GLAD_GL_ES_VERSION_2_0) {
        fprintf(stderr, "[gl_loader] ERROR: Need GL 3.3 or GLES 2.0, not supported.\n");
        return false;
    }

    fprintf(stderr, "[gl_loader] GL %s loaded successfully.\n",
            (const char*)glGetString(GL_VERSION));
    return true;
}