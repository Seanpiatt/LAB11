#version 120

uniform sampler1D transforms;
uniform int numTransforms;

attribute vec2 position;
attribute vec2 texCoords;
attribute float spriteNumber;

varying vec2 texCoordsOut;

// Given a world position (for this lab, bottom-left is (0, 0), top-right of window is (1280, 720)),
// convert it to be in [-1, 1]^2, where y = -1 is the bottom and x = -1 is the left side.
vec2 cameraPos(vec2 worldPos) {

    return vec2(2*worldPos.x/1280.0 -1,2*worldPos.y/720.0 -1 ); 
}

void main() {
    // output to gl_Position and to texCoordsOut.
    // In this case, using window.draw, SFML provides tex coords in gl_MultiTexCoord0.xy, and
    //  vertex positions in gl_Vertex.xy.
    float where = (spriteNumber + 0.5) / float(numTransforms);
    vec4 t = texture1D(transforms, where);
    vec2 worldPos = position * t.xy + t.zw;
    gl_Position = vec4(cameraPos(worldPos), 0.0, 1.0);
    texCoordsOut = vec2(texCoords.x, 1.0 - texCoords.y);
}

