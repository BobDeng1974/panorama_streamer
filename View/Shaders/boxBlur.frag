uniform sampler2D texture;
uniform bool isVertical;
uniform int blurRadius;
uniform int width;
uniform int height;
uniform bool isPartial;
varying vec4 texc;

void main(void)
{
	if (isPartial)
	{
		vec4 src = texture2D(texture, texc.xy);
		if (src.w == 0.0f)
		{
			gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
			return;
		}
	}
	
	vec4 dst4 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	int pixelCnt = 0;
	if(isVertical)
	{
		for (int i = -blurRadius; i <= blurRadius; i++)
		{
			vec2 vTexUV = vec2(texc.x, texc.y + 1.0f * i / height);
			if (vTexUV.y < 0) continue;
			if (vTexUV.y > 1) continue;
			vec4 pix = texture2D(texture, vTexUV);
			if (!isPartial)
			{
				dst4 += pix;
				pixelCnt++;
			}
			else if (pix.w != 0.0f)
			{
				dst4 += vec4(pix.xyz, 1.0f);
				pixelCnt++;
			}
		}
	}
	else
	{
		for (int i = -blurRadius; i <= blurRadius; i++)
		{
			vec2 vTexUV = vec2(texc.x + 1.0f * i / width, texc.y);
			if (vTexUV.x < 0) continue;
			if (vTexUV.x > 1) continue;
			vec4 pix = texture2D(texture, vTexUV);
			if (!isPartial)
			{
				dst4 += pix;
				pixelCnt++;
			}
			else if (pix.w != 0.0f)
			{
				dst4 += vec4(pix.xyz, 1.0f);
				pixelCnt++;
			}
		}
	}	
	if (pixelCnt == 0)
	{
		gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	else
	{
		gl_FragColor = dst4 / pixelCnt;
	}
}