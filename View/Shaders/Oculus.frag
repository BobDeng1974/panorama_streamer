uniform sampler2D texture;

varying vec2 texc;

void main(void)
{
   vec2 uv = vec2(texc.x, 1.0f - texc.y);
   gl_FragColor = texture2D(texture, texc); //vec4(1.0f, 0.0f, 0.0f, 1.0f); //texture2D(texture, texc);
}