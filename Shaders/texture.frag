varying vec4 texc;
uniform sampler2D texture;

void main(void)
{
	vec2 uv = vec2(texc.x, texc.y);	
	gl_FragColor = texture2D(texture, uv);
}