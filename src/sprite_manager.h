#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include "basic_transform.h"
#include "texture_atlas.h"
#include "vector_util.h"
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#define GL_SILENCE_DEPRECATION  // silences macOS OpenGL deprecation warnings

#define glCheck(expr)                                                                              \
    expr;                                                                                          \
    if (int e = glGetError()) {                                                                    \
        std::cerr << "OpenGL error: " << e << " in file " __FILE__ << ", line number " << __LINE__ \
                  << '\n';                                                                         \
        abort();                                                                                   \
    }

using SpriteID = size_t;

class SpriteManager {
public:
    SpriteManager(const sf::Vector2u& atlasSize, unsigned padding)
        : mVertices(sf::PrimitiveType::Triangles),
          mNumSprites{0},
          mAtlas(atlasSize, padding),
          mShader(std::filesystem::path{"main.vert"}, std::filesystem::path{"main.frag"}) {

        mPositionAttribLocation = glGetAttribLocation(mShader.getNativeHandle(),"position");
        mTexCoordsAttribLocation = glGetAttribLocation(mShader.getNativeHandle(),"texCoords");
        mSpriteNumberAttribLocation = glGetAttribLocation(mShader.getNativeHandle(),"spriteNumber");
        mTextureUniformLocation = glGetUniformLocation(mShader.getNativeHandle(),"texture");
        mTransformsUniformLocation = glGetUniformLocation(mShader.getNativeHandle(),"transforms");
        mNumTransformsUniformLocation = glGetUniformLocation(mShader.getNativeHandle(),"numTransforms");

        glGenBuffers(1, &mVerticesVBO);
        glGenBuffers(1, &mSpriteNumberVBO);
        glGenTextures(1, &mTransformsTexture);
    }
    ~SpriteManager() {         
        glDeleteBuffers(1, &mVerticesVBO);
        glDeleteBuffers(1, &mSpriteNumberVBO);
        glDeleteTextures(1,&mTransformsTexture);
    }

    // TODO
    TextureID registerTexture(const std::string& filepath) {
        TextureID id = mAtlas.registerTexture(filepath);
        return id;
    };

    // TODO
    SpriteID createSprite(TextureID textureID) { 
        SpriteID id = mNumSprites; //idk what current count is
        sf::Vector2f size = sf::Vector2f(mAtlas.getSizePixels(textureID));
        sf::FloatRect uv = mAtlas.getBoundsUV(textureID);

        //triangle 1 bl-br-tr
        mVertices.append(sf::Vertex{{0, 0}, sf::Color::White, {uv.position.x, uv.position.y}});
        mSpriteNumbers.push_back(float(mNumSprites));

        mVertices.append(sf::Vertex{{size.x, 0}, sf::Color::White, {uv.position.x + uv.size.x, uv.position.y}});
        mSpriteNumbers.push_back(float(mNumSprites));

        mVertices.append(sf::Vertex{{size.x, size.y}, sf::Color::White, {uv.position.x + uv.size.x, uv.position.y + uv.size.y}});
        mSpriteNumbers.push_back(float(mNumSprites));
        //triangle 2 tl -bl -tr
        mVertices.append(sf::Vertex{{0, size.y}, sf::Color::White, {uv.position.x, uv.position.y + uv.size.y}});
        mSpriteNumbers.push_back(float(mNumSprites));

        mVertices.append(sf::Vertex{{0, 0}, sf::Color::White, {uv.position.x, uv.position.y}});
        mSpriteNumbers.push_back(float(mNumSprites));

        mVertices.append(sf::Vertex{{size.x, size.y}, sf::Color::White, {uv.position.x + uv.size.x, uv.position.y + uv.size.y}});
        mSpriteNumbers.push_back(float(mNumSprites));

        mTransforms.push_back(BasicTransform());
        mSpriteNumbers.push_back(float(mNumSprites));
        ++mNumSprites;

        return id;

     };

    // TODO
    void setTexture(SpriteID sprite, TextureID newTexture) {
        sf::FloatRect uv = mAtlas.getBoundsUV(newTexture);
        sf::Vector2f size = sf::Vector2f(mAtlas.getSizePixels(newTexture));
        mVertices[6*sprite + 0] = sf::Vertex{{0, 0}, sf::Color::White, {uv.position.x, uv.position.y}};
        mVertices[6*sprite + 1] = sf::Vertex{{size.x, 0}, sf::Color::White, {uv.position.x + uv.size.x, uv.position.y}};
        mVertices[6*sprite + 2] = sf::Vertex{{size.x, size.y}, sf::Color::White, {uv.position.x + uv.size.x, uv.position.y + uv.size.y}};

        mVertices[6*sprite + 3] = sf::Vertex{{0, size.y}, sf::Color::White, {uv.position.x, uv.position.y + uv.size.y}};
        mVertices[6*sprite + 4] = sf::Vertex{{0, 0}, sf::Color::White, {uv.position.x, uv.position.y}};
        mVertices[6*sprite + 5] = sf::Vertex{{size.x, size.y}, sf::Color::White, {uv.position.x + uv.size.x, uv.position.y + uv.size.y}};
    };

    // TODO
    void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        //found online, not really sure if it is needed. 
        sf::VertexArray temp = mVertices;

        for (std::size_t i = 0; i < mVertices.getVertexCount(); ++i) {
            std::size_t spriteID = i/6;
            const BasicTransform& t = mTransforms[spriteID];
            sf::Vector2f original = mVertices[i].position;
            temp[i].position = t.applyPoint(original);
        }
        states.texture = &mAtlas.getTexture();
        states.shader = &mShader;
        mShader.setUniform("texture", sf::Shader::CurrentTexture);
        target.draw(temp, states);

    };

    // TODO
    void drawGL() {
        glUseProgram(mShader.getNativeHandle());

        glBindBuffer(GL_ARRAY_BUFFER, mVerticesVBO);
        glBufferData(GL_ARRAY_BUFFER, mVertices.getVertexCount() * sizeof(sf::Vertex),  &mVertices[0], GL_DYNAMIC_DRAW);

        glVertexAttribPointer(mPositionAttribLocation, 2, GL_FLOAT, GL_FALSE,  sizeof(sf::Vertex), (void*)offsetof(sf::Vertex, position));
        glEnableVertexAttribArray(mPositionAttribLocation);

        glVertexAttribPointer(mTexCoordsAttribLocation, 2, GL_FLOAT, GL_FALSE, sizeof(sf::Vertex), (void*)offsetof(sf::Vertex, texCoords));
        glEnableVertexAttribArray(mTexCoordsAttribLocation);

        glBindBuffer(GL_ARRAY_BUFFER, mSpriteNumberVBO);
        glBufferData(GL_ARRAY_BUFFER, mSpriteNumbers.size() * sizeof(float), mSpriteNumbers.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(mSpriteNumberAttribLocation, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(mSpriteNumberAttribLocation);

        glActiveTexture(GL_TEXTURE1);                       
        glBindTexture(GL_TEXTURE_1D, mTransformsTexture);   
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F_ARB, (GLsizei)mNumSprites, 0, GL_RGBA, GL_FLOAT, mTransforms.data());

        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glUniform1i(mTransformsUniformLocation, 1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mAtlas.getTexture().getNativeHandle());
        glUniform1i(mTextureUniformLocation, 0);
        glUniform1i(mNumTransformsUniformLocation, (GLint)mNumSprites);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)mVertices.getVertexCount());

    };

    // TODO
    BasicTransform& getTransform(SpriteID id) {
        return mTransforms[id];
    }

    // TODO
    const TextureAtlas& getAtlas() const {
        return mAtlas;
    };

private:
    sf::VertexArray mVertices;
    std::vector<BasicTransform> mTransforms;
    size_t mNumSprites;
    TextureAtlas mAtlas;
    mutable sf::Shader mShader;

    // Use the below only in Batched rendering (part 2):
    std::vector<float> mSpriteNumbers;

    GLint mTextureUniformLocation;
    GLint mTransformsUniformLocation;
    GLint mNumTransformsUniformLocation;

    GLint mPositionAttribLocation;
    GLint mTexCoordsAttribLocation;
    GLint mSpriteNumberAttribLocation;

    GLuint mVerticesVBO;
    GLuint mSpriteNumberVBO;
    GLuint mTransformsTexture;
};

#endif  // SPRITE_MANAGER_H
