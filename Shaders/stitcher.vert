attribute vec4 vertex;
attribute vec4 texCoord;
varying vec4 texc;
uniform mat4 matrix;

void main(void)
{
    gl_Position = matrix * vertex;
    texc = texCoord;
}
