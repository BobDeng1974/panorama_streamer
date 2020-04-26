uniform sampler2D textures[8];
varying vec4 texc;
uniform int viewCnt;

void main(void)
{
	vec2 uv = vec2(texc.x, texc.y);
	float ws = 0.0f;
	vec3 csum = vec3(0.0f, 0.0f, 0.0f);
	for( int i = 0; i < viewCnt; i++ )
	{
		vec4 src = texture2D(textures[i], uv);
		vec3 csrc = src.xyz * src.w;
		csum = csum + csrc;
		ws = ws + src.w;
	}
	
    if(ws == 0.0f)
       csum = vec3(0.0f, 0.0f, 0.0f);
    else
       csum = csum / ws;
    
    gl_FragColor = vec4(csum, 1.0f);
}