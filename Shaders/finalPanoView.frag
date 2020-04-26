uniform sampler2D textures[2];
uniform bool isStereo;
varying vec4 texc;

#define   LeftView 		 0
#define   RightView      1

void main(void)
{
	vec2 uv = vec2(texc.x, texc.y);
	
	bool isRight = false;
	if(isStereo)
	{
		if(uv.y < 0.5f)
		{
			uv.y *= 2;
		}
		else
		{
			uv.y = (uv.y - 0.5f) * 2;
			isRight = true;
		}
	}
	
	vec4 src;
	if(isRight)
	{
		src = texture2D(textures[RightView], uv);
	}
	else
	{
		src = texture2D(textures[LeftView], uv);
	}
	src = vec4(src.xyz * src.w, 1.0f);
    gl_FragColor = src;
}