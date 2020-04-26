varying vec4 texc;
uniform sampler2D textures[8];
uniform int viewCnt, viewIdx;

void main(void)
{
	vec2 uv = vec2(texc.x, texc.y);
	float r = 0.0;
	gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);

	if(viewIdx == 0) {
		for(int i = 0; i < viewCnt; i++){		
			r = 125 * (i + 1);
			vec4 src0 = texture2D(textures[i], uv);

			if((src0[3] != 0.0)) 
				gl_FragColor = vec4(r/1000.0, 0.0, 0.0, 1.0);
		}
	}
	else {
		vec4 src0 = texture2D(textures[viewIdx-1], uv);

		if((src0[3] != 0.0)) 
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
}