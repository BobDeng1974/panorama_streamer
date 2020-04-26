varying vec4 texc;
uniform sampler2D texture;
uniform int width, height;

void main(void)
{
	vec4 src0 = texture2D(texture, vec2(texc.x, texc.y));
	vec4 src1 = texture2D(texture, vec2(texc.x + 1.0f / width, texc.y));
	vec4 src2 = texture2D(texture, vec2(texc.x - 1.0f / width, texc.y));
	vec4 src3 = texture2D(texture, vec2(texc.x, texc.y - 1.0f / height));
	vec4 src4 = texture2D(texture, vec2(texc.x, texc.y + 1.0f / height));

	if((src0[0] == src1[0])&&(src0[0] == src2[0])&&(src0[0] == src3[0])&&(src0[0] == src4[0])) 
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	else
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}