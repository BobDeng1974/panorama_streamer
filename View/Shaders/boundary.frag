uniform sampler2D textures[8];
uniform int viewCnt;
uniform int currentIdx;

varying vec4 texc;

void main(void)
{
	vec2 uv = vec2(texc.x, texc.y);
	vec4 src0 = texture2D(textures[currentIdx], uv);
	
	if (src0[3] == 0.0f)
	{
		gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	else
	{
		bool isPrime = true;
		for (int i = 0; i < viewCnt; i++)
		{
			if (i != currentIdx)
			{
				vec4 src1 = texture2D(textures[i], uv);
				if(src1[3] != 0.0f)
				{
					if (src1[3] > src0[3])
					{
						isPrime = false;
						break;
					}
				}
			}
		}
		if(isPrime)
		{
			gl_FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else
		{
			gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}	
}