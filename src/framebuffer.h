#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "util.h"
#include "sm.h"
#include "shader.h"

#include <glad/gl.h>

class Framebuffer {
   private:
    struct QVertex {
        QVertex(vec2 p, vec2 tc) : pos(p), uv(tc) {}
        vec2 pos;
        vec2 uv;
    };
    std::vector<QVertex> vertexData = {
        {{-1, 1}, {0, 1}},
        {{-1, -1}, {0, 0}},
        {{1, 1}, {1, 1}},
        {{1, -1}, {1, 0}},
    };

   public:
    struct TextureBuffer {
        // create an empty TextureBuffer object
        TextureBuffer() {}
        // create a TextureBuffer object for a pre-existing texture object
        TextureBuffer(unsigned texObj, std::string texName, GLenum texType) : tex(texObj), name(texName), format(texType) {}
        // create a TextureBuffer object for this framebuffer
        TextureBuffer(unsigned fboTarget,
                      std::string texName,
                      GLenum texFormat,
                      GLenum texComponent,
                      GLenum texAttachment,
                      int w,
                      int h) : name(texName),
                               format(texFormat),
                               component(texComponent),
                               attachment(texAttachment) {
            glCreateTextures(format, 1, &tex);
            glTextureStorage2D(tex, 1, component, w, h);
            glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glNamedFramebufferTexture(fboTarget, attachment, tex, 0);
        }
        std::string name = "DefaultTextureBuffer";
        unsigned tex = 0;
        GLenum component;
        GLenum attachment;
        GLenum format;
    };

    Framebuffer() { name = "NewDefaultFrameBuffer"; }
    Framebuffer(std::string nm, std::string vertexShaderPath, std::string fragmentShaderPath, int fboWidth, int fboHeight) {
        name = nm;
        quadShader = new Shader(name + "_shader", vertexShaderPath, fragmentShaderPath);
        width = fboWidth;
        height = fboHeight;
        populateBuffers();
    }
    ~Framebuffer() {}

    void populateBuffers() {
        glCreateFramebuffers(1, &FBO);
        glCreateVertexArrays(1, &quadVAO);
        glBindVertexArray(quadVAO);

        glCreateBuffers(1, &quadVBO);
        glNamedBufferStorage(quadVBO, sizeof(vertexData[0]) * vertexData.size(), &vertexData[0], GL_MAP_READ_BIT);
        glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, sizeof(QVertex));

        glEnableVertexArrayAttrib(quadVAO, 0);
        glVertexArrayAttribFormat(quadVAO, 0, 2, GL_FLOAT, GL_FALSE, offsetof(QVertex, pos));
        glVertexArrayAttribBinding(quadVAO, 0, 0);

        glEnableVertexArrayAttrib(quadVAO, 1);
        glVertexArrayAttribFormat(quadVAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(QVertex, uv));
        glVertexArrayAttribBinding(quadVAO, 1, 0);

        glBindVertexArray(0);
    }

    void addDrawBuffers(std::vector<GLenum> bufs) {
        glNamedFramebufferDrawBuffers(FBO, bufs.size(), bufs.data());
    }

    // add a texture buffer to be rendered to a fullscreen quad
    void addTexture(std::string texName, GLenum texFormat, GLenum texComponent, GLenum texAttachment) {
        TextureBuffer* tb = new TextureBuffer(FBO, texName, texFormat, texComponent, texAttachment, width, height);
        textures.push_back(tb);
    }

    // add a pre-existing texture object
    void addTexture(std::string texName, unsigned texObj, GLenum texType) {
        TextureBuffer* tb = new TextureBuffer(texObj, texName, texType);
        textures.push_back(tb);
    }

    void addRenderbuffer(std::string texName, GLenum texComponent, GLenum texAttachment) {
        glCreateRenderbuffers(1, &RBO);
        glNamedRenderbufferStorage(RBO, texComponent, width, height);
        glNamedFramebufferRenderbuffer(FBO, texAttachment, GL_RENDERBUFFER, RBO);
    }

    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    }

    // unbind the current framebuffer, which will bind to the default framebuffer
    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // draw to the currently bound framebuffer
    void draw() {
        draw(false, 0);
    }

    // draw the framebuffer given the attachments in `bufs`
    void draw(bool blit, GLenum returnMask, bool useOwnShader = true) {
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (useOwnShader) quadShader->use();
        glBindVertexArray(quadVAO);
        glDisable(GL_DEPTH_TEST);
        // bind all textures
        for (int i = 0; i < textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(textures[i]->format, textures[i]->tex);
            glBindTextureUnit(i, textures[i]->tex);
        }

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // unbind all textures
        for (int i = 0; i < textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(textures[i]->format, 0);
        }
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);

        // copy framebuffer's buffers to the default framebuffer
        if (blit) glBlitNamedFramebuffer(FBO, 0, 0, 0, width, height, 0, 0, SM::width, SM::height, returnMask, GL_NEAREST);
    }

    unsigned FBO = 0;
    unsigned RBO = 0;
    unsigned quadVAO = 0;
    unsigned quadVBO = 0;
    std::vector<TextureBuffer*> textures;
    Shader* quadShader;
    std::string name;
    int width;
    int height;
};

#endif /* FRAMEBUFFER_H */
