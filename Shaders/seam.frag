varying vec4 texc;
uniform sampler2D texture, mask;

void main(void)
{
	vec2 uv = vec2(texc.x, texc.y);	
	vec4 src = texture2D(texture, uv);
	vec4 maskSrc = texture2D(mask, uv);
	if(maskSrc[0] == 0.0)
		gl_FragColor = src;
	else
		gl_FragColor = maskSrc;
}