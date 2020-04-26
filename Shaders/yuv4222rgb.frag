uniform sampler2D texture;

uniform int bytesPerLine;
uniform int width;

varying vec4 texc;

void main(void)
{
	vec2 tex = vec2(texc.x*width*2 / bytesPerLine, texc.y);
	vec4 yuyv = texture2D(texture, tex);
	float u = yuyv.g;
	float v = yuyv.a;

	float y;
	if(mod(texc.x * width, 2.0f) == 0.0f)
	{
		y = yuyv.r;
	}
	else
	{
		y = yuyv.b;
	}

	y=1.1643*(y-0.0625);
	u=u-0.5;
	v=v-0.5;
	
	float r=y+1.5958*v;
	float g=y-0.39173*u-0.81290*v;
	float b=y+2.017*u;
    
	gl_FragColor = vec4(r, g, b, 1.0f);
}