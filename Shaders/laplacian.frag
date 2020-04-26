uniform sampler2D original;
uniform sampler2D gaussian;
varying vec4 texc;

void main(void)
{
	vec4 org = texture2D(original, texc.xy);
	vec4 gauss = texture2D(gaussian, texc.xy);
	if (org.w == 0.0f)
		gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	else
		gl_FragColor = vec4(org.xyz - gauss.xyz + vec3(0.5f, 0.5f, 0.5f), 1.0f);
}