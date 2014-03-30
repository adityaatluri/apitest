#include "pch.h"

#include "naive.h"
#include "framework/gfx_gl.h"

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
TexturedQuadsGLNaive::TexturedQuadsGLNaive()
: mIndexBuffer()
, mVertexBuffer()
, mVertexArray()
, mProgram()
{ }

// --------------------------------------------------------------------------------------------------------------------
bool TexturedQuadsGLNaive::Init(const std::vector<TexturedQuadsProblem::Vertex>& _vertices,
                                const std::vector<TexturedQuadsProblem::Index>& _indices,
                                const std::vector<TextureDetails*>& _textures,
                                size_t _objectCount)
{
    if (!TexturedQuadsSolution::Init(_vertices, _indices, _textures, _objectCount)) {
        return false;
    }

    // Program
    const char* kUniformNames[] = { "ViewProjection", "World", "gTex", nullptr };

    mProgram = CreateProgramT("textures_gl_naive_vs.glsl",
                              "textures_gl_naive_fs.glsl",
                              kUniformNames, &mUniformLocation);

    if (mProgram == 0) {
        console::warn("Unable to initialize solution '%s', shader compilation/linking failed.", GetName().c_str());
        return false;
    }

    // Textures
    for (auto it = _textures.begin(); it != _textures.end(); ++it) {
        GLuint tex = NewTex2DFromDetails(*(*it));
        if (!tex) {
            console::warn("Unable to initialize solution '%s', texture creation failed.", GetName().c_str());
            return false;
        }

        // Needs to be freed later.
        mTextures.push_back(tex);
    }

    // Buffers
    glGenVertexArrays(1, &mVertexArray);
    glBindVertexArray(mVertexArray);

    mVertexBuffer = NewBufferFromVector(GL_ARRAY_BUFFER, _vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedQuadsProblem::Vertex), (void*)offsetof(TexturedQuadsProblem::Vertex, pos));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedQuadsProblem::Vertex), (void*)offsetof(TexturedQuadsProblem::Vertex, tex));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    mIndexBuffer = NewBufferFromVector(GL_ELEMENT_ARRAY_BUFFER, _indices, GL_STATIC_DRAW);

    return glGetError() == GL_NO_ERROR;
}

// --------------------------------------------------------------------------------------------------------------------
void TexturedQuadsGLNaive::Render(const std::vector<Matrix>& _transforms)
{
    // Program
    Vec3 dir = { 0, 0, 1 };
    Vec3 at = { 0, 0, 0 };
    Vec3 up = { 0, 1, 0 };
    dir = normalize(dir);
    Vec3 eye = at - 250 * dir;
    Matrix view = matrix_look_at(eye, at, up);
    Matrix view_proj = mProj * view;

    glUseProgram(mProgram);
    glUniformMatrix4fv(mUniformLocation.ViewProjection, 1, GL_TRUE, &view_proj.x.x);
    // We will bind the texture we care about to unit 0, so let the program know now.
    glUniform1i(mUniformLocation.gTex, 0);

    // Rasterizer State
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    glDisable(GL_SCISSOR_TEST);

    // Blend State
    glDisable(GL_BLEND);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Depth Stencil State
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    size_t xformCount = _transforms.size();
    assert(xformCount <= mObjectCount);

    // Code below assumes at least 1 texture.
    assert(mTextures.size() > 0);
    auto texIt = mTextures.begin();

    for (size_t u = 0; u < xformCount; ++u) {
        if (texIt == mTextures.end()) {
            texIt = mTextures.begin();
        }

        GLuint activeTex = *texIt;
        ++texIt;

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, activeTex);
        Matrix world = transpose(_transforms[u]);
        glUniformMatrix4fv(mUniformLocation.World, 1, GL_TRUE, &world.x.x);

        glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, nullptr);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void TexturedQuadsGLNaive::Shutdown()
{
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    for (auto it = mTextures.begin(); it != mTextures.end(); ++it) {
        glDeleteTextures(1, &*it);
    }

    glDeleteBuffers(1, &mIndexBuffer);
    glDeleteBuffers(1, &mVertexBuffer);
    glDeleteVertexArrays(1, &mVertexArray);
    glDeleteProgram(mProgram);

    mTextures.clear();
}

