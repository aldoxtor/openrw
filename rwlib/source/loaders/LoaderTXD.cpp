#include <gl/TextureData.hpp>
#include <loaders/LoaderTXD.hpp>

#include <algorithm>
#include <iostream>
#include <vector>

GLuint gErrorTextureData[] = {0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF};
GLuint gDebugTextureData[] = {0xFF0000FF, 0xFF00FF00};
GLuint gTextureRed[] = {0xFF0000FF};
GLuint gTextureGreen[] = {0xFF00FF00};
GLuint gTextureBlue[] = {0xFFFF0000};

TextureData::Handle getErrorTexture() {
    static GLuint errTexName = 0;
    static TextureData::Handle tex;
    if (errTexName == 0) {
        glGenTextures(1, &errTexName);
        glBindTexture(GL_TEXTURE_2D, errTexName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, gErrorTextureData);
        glGenerateMipmap(GL_TEXTURE_2D);

        tex = TextureData::create(errTexName, {2, 2}, false);
    }
    return tex;
}

const size_t paletteSize = 1024;
void processPalette(uint32_t* fullColor, RW::BinaryStreamSection& rootSection) {
    uint8_t* dataBase = reinterpret_cast<uint8_t*>(
        rootSection.raw() + sizeof(RW::BSSectionHeader) +
        sizeof(RW::BSTextureNative) - 4);

    uint8_t* coldata = (dataBase + paletteSize + sizeof(uint32_t));
    uint32_t raster_size = *reinterpret_cast<uint32_t*>(dataBase + paletteSize);
    uint32_t* palette = reinterpret_cast<uint32_t*>(dataBase);

    for (size_t j = 0; j < raster_size; ++j) {
        *(fullColor++) = palette[coldata[j]];
    }
}

TextureData::Handle createTexture(RW::BSTextureNative& texNative,
                                  RW::BinaryStreamSection& rootSection) {
    // TODO: Exception handling.
    if (texNative.platform != 8) {
        std::cerr << "Unsupported texture platform " << std::dec
                  << texNative.platform << std::endl;
        return getErrorTexture();
    }

    bool isPal8 =
        (texNative.rasterformat & RW::BSTextureNative::FORMAT_EXT_PAL8) ==
        RW::BSTextureNative::FORMAT_EXT_PAL8;
    bool isFulc = texNative.rasterformat == RW::BSTextureNative::FORMAT_1555 ||
                  texNative.rasterformat == RW::BSTextureNative::FORMAT_8888 ||
                  texNative.rasterformat == RW::BSTextureNative::FORMAT_888;
    // Export this value
    bool transparent =
        !((texNative.rasterformat & RW::BSTextureNative::FORMAT_888) ==
          RW::BSTextureNative::FORMAT_888);

    GLuint textureName = 0;

    if (isPal8) {
        std::vector<uint32_t> fullColor(texNative.width * texNative.height);

        processPalette(fullColor.data(), rootSection);

        glGenTextures(1, &textureName);
        glBindTexture(GL_TEXTURE_2D, textureName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texNative.width,
                     texNative.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     fullColor.data());
    } else if (isFulc) {
        auto coldata = rootSection.raw() + sizeof(RW::BSTextureNative);
        coldata += sizeof(uint32_t);

        GLenum type = GL_UNSIGNED_BYTE, format = GL_RGBA;
        switch (texNative.rasterformat) {
            case RW::BSTextureNative::FORMAT_1555:
                format = GL_RGBA;
                type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
                break;
            case RW::BSTextureNative::FORMAT_8888:
                format = GL_BGRA;
                // type = GL_UNSIGNED_INT_8_8_8_8_REV;
                coldata += 8;
                type = GL_UNSIGNED_BYTE;
                break;
            case RW::BSTextureNative::FORMAT_888:
                format = GL_BGRA;
                type = GL_UNSIGNED_BYTE;
                break;
            default:
                break;
        }

        glGenTextures(1, &textureName);
        glBindTexture(GL_TEXTURE_2D, textureName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texNative.width,
                     texNative.height, 0, format, type, coldata);
    } else {
        std::cerr << "Unsuported raster format " << std::dec
                  << texNative.rasterformat << std::endl;
        return getErrorTexture();
    }

    GLenum magFilter, minFilter;
    switch (texNative.filterflags & 0xFF) {
        default:
        case RW::BSTextureNative::FILTER_MYSTERY_OPTION:
        case RW::BSTextureNative::FILTER_LINEAR:
            magFilter = GL_LINEAR;
            minFilter = GL_LINEAR;
            break;
        case RW::BSTextureNative::FILTER_NEAREST:
            magFilter = GL_NEAREST;
            minFilter = GL_NEAREST;
            break;
        case RW::BSTextureNative::FILTER_MIP_NEAREST:
            magFilter = GL_NEAREST;
            minFilter = GL_NEAREST_MIPMAP_NEAREST;
            break;
        case RW::BSTextureNative::FILTER_MIP_LINEAR:
            magFilter = GL_NEAREST;
            minFilter = GL_NEAREST_MIPMAP_LINEAR;
            break;
        case RW::BSTextureNative::FILTER_LINEAR_MIP_NEAREST:
            magFilter = GL_LINEAR;
            minFilter = GL_LINEAR_MIPMAP_NEAREST;
            break;
        case RW::BSTextureNative::FILTER_LINEAR_MIP_LINEAR:
            magFilter = GL_LINEAR;
            minFilter = GL_LINEAR_MIPMAP_LINEAR;
            break;
        case RW::BSTextureNative::FILTER_NONE:
            // Don't know what it is
            magFilter = GL_LINEAR;
            minFilter = GL_LINEAR;
            break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    GLenum texwrap;
    switch (texNative.wrapU) {
        default:
        case RW::BSTextureNative::WRAP_NONE:
        case RW::BSTextureNative::WRAP_WRAP:
            texwrap = GL_REPEAT;
            break;
        case RW::BSTextureNative::WRAP_CLAMP:
            texwrap = GL_CLAMP_TO_EDGE;
            break;
        case RW::BSTextureNative::WRAP_MIRROR:
            texwrap = GL_MIRRORED_REPEAT;
            break;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texwrap);

    switch (texNative.wrapV) {
        default:
        case RW::BSTextureNative::WRAP_NONE:
        case RW::BSTextureNative::WRAP_WRAP:
            texwrap = GL_REPEAT;
            break;
        case RW::BSTextureNative::WRAP_CLAMP:
            texwrap = GL_CLAMP_TO_EDGE;
            break;
        case RW::BSTextureNative::WRAP_MIRROR:
            texwrap = GL_MIRRORED_REPEAT;
            break;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texwrap);

    glGenerateMipmap(GL_TEXTURE_2D);

    return TextureData::create(textureName, {texNative.width, texNative.height},
                               transparent);
}

bool TextureLoader::loadFromMemory(FileHandle file,
                                   TextureArchive& inTextures) {
    auto data = file->data;
    RW::BinaryStreamSection root(data);
    /*auto texDict =*/root.readStructure<RW::BSTextureDictionary>();

    size_t rootI = 0;
    while (root.hasMoreData(rootI)) {
        auto rootSection = root.getNextChildSection(rootI);

        if (rootSection.header.id != RW::SID_TextureNative) continue;

        RW::BSTextureNative texNative =
            rootSection.readStructure<RW::BSTextureNative>();
        std::string name = std::string(texNative.diffuseName);
        std::string alpha = std::string(texNative.alphaName);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::transform(alpha.begin(), alpha.end(), alpha.begin(), ::tolower);

        auto texture = createTexture(texNative, rootSection);

        inTextures[name] = texture;
    }

    return true;
}
