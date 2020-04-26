uniform sampler2D colorMap[8];
uniform sampler2D blendMap[8];
varying vec4 texc;
uniform float levelScale;
uniform int viewCnt;

void main(void)
{
	vec2 uv = vec2(texc.x, texc.y);
	float ws = 0.0f;
	vec3 csum = vec3(0.0f, 0.0f, 0.0f);
	for( int i = 0; i < viewCnt; i++ )
	{
		vec3 src = texture2D(colorMap[i], uv).xyz;
		float weight = texture2D(blendMap[i], uv * levelScale).w;
		vec3 csrc = src * weight;
		csum = csum + csrc;
		ws = ws + weight;
	}
    if(ws == 0.0f)
       csum = vec3(0.0f, 0.0f, 0.0f);
    else
       csum = csum / ws;
    
    gl_FragColor = vec4(csum, 1.0f);
}