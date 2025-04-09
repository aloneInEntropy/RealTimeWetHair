#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <vector>
#include <iostream>
#include <glad/gl.h>

#include "util.h"

class Texture {
   private:
    // Load an atlas texture `tex` with width `tileSize` and individual textures `tiles`
    bool _loadAtlas(std::string tex, int tileSize, int tiles);

   public:
    // create from memory buffer
    Texture(GLenum texType = GL_TEXTURE_2D) {
        textureEnum = texType;
        isAtlas = texType == GL_TEXTURE_2D_ARRAY;
    }

    // create from file
    Texture(const std::string fname, GLenum texType = GL_TEXTURE_2D) {
        textureEnum = texType;
        file_name = fname;
        isAtlas = texType == GL_TEXTURE_2D_ARRAY;
    }

    // create from files
    Texture(const std::vector<std::string> fnames, GLenum texType = GL_TEXTURE_CUBE_MAP) {
        textureEnum = texType;
        file_names = fnames;
    }

    // load the file given in the constructor
    bool load(bool flip = true);

    // load the file `tex`
    bool load(std::string tex, bool flip = true);

    // load from memory buffer (embedded textures)
    bool load(unsigned int, void*);

    // load an atlas of textures.
    // `tiles` is the number of actual, non-empty tiles in the atlas, and `tileSize` is the width (and height) of a single tile.
    bool loadAtlas(std::string tex, int tileSize, int tiles);

    // load the atlas of textures given in the constructor.
    // `tiles` is the number of actual, non-empty tiles in the atlas, and `tileSize` is the width (and height) of a single tile.
    bool loadAtlas(int tileSize, int tiles);

    // load from files (cubemap)
    bool loadCubemap(std::vector<std::string> faces);

    // bind a single texture
    void bind(GLenum textureSlot);

    // unbind a single texture
    void unbind(GLenum textureSlot);

    std::string file_name;
    std::vector<std::string> file_names;
    GLenum textureEnum;
    unsigned int texture = 0;
    int width_ = 0, height_ = 0, nrChannels_ = 0;
    bool isAtlas = false;
};

#endif /* TEXTURE_H */
