uniform sampler2D textures[8];

uniform int viewCnt;

varying vec4 texc;

// simple tile viewer (n*2)
// this can be expanded to general tile viewer (2(w) * 2(h) ~ 4 * 2 ~ 3*n tile viewer)
// or we can make frame on qt, and just shows one view for each widget.
void main(void)
{
	vec2 uv = vec2(texc.x, texc.y);
	
	vec4 src;
	float xn;
	int m, n;
	m = 2;
	n = viewCnt / m;
	
	int i = uv.y * m;
	int j = uv.x * n;
	src = texture2D(textures[i*n+j], vec2(uv.x * n - j, uv.y * m - i));
    gl_FragColor = src;
}