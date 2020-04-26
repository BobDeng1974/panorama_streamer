uniform sampler2D texture;
varying vec4 texc;

// compositing parameters
uniform float blendingFalloff;
uniform float fisheyeLensRadiusRatio1;
uniform float fisheyeLensRadiusRatio2;
uniform float xrad1;
uniform float xrad2;
uniform float yrad1;
uniform float yrad2;
uniform float blendCurveStart;

#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.785398163397448309616

void sphericalToCartesian(in float theta, in float phi, out vec3 cartesian) {
    cartesian.y = sin(phi);
    cartesian.x = cos(phi) * sin(theta);
    cartesian.z = cos(phi) * cos(theta);
}

// return float instead of bool because bool / int do not work on MacBookPro 10.6
float XYnToThetaPhi(in float x_n, in float y_n, out float theta,  out float phi)
{
    float result = 1.0f;
    theta = (2.0f * x_n - 1.0f) * M_PI;
    phi   = M_PI * y_n - M_PI_2;

    return result;
}

/////////////////////////////////
// camera parameters
#define   LensModel_standard      0
#define   LensModel_fisheye       1
uniform   int   lens;

uniform float cx;
uniform float cy;
uniform float offset_x;
uniform float offset_y;
uniform float k1;
uniform float k2;
uniform float k3;
uniform float FoV;

uniform float globalYaw;
uniform float globalPitch;
uniform float globalRoll;
uniform float yaw;
uniform float pitch;
uniform float roll;

uniform float imageWidth;
uniform float imageHeight;

uniform mat3 cP;

mat3 getCP(float fYaw, float fPitch, float fRoll)
{
   float A = fPitch;
   float B = fYaw;
   float C = fRoll;
   
   A = A * M_PI / 180.0f;
   B = B * M_PI / 180.0f;
   C = C * M_PI / 180.0f;

   mat3 pitchMat;
   mat3 yawMat;
   mat3 rollMat;
   
   rollMat[0] = vec3( cos(C), sin(C), 0);
   rollMat[1] = vec3( -sin(C), cos(C), 0.0);
   rollMat[2] = vec3( 0.0, 0.0, 1.0);
   
   
   yawMat[0] = vec3( cos(B), 0.0, sin(B) );
   yawMat[1] = vec3( 0, 1.0, 0.0 );
   yawMat[2] = vec3( -sin(B), 0, cos(B) );
   

   pitchMat[0] = vec3( 1, 0, 0  );
   pitchMat[1] = vec3( 0, cos(A), sin(A) );
   pitchMat[2] = vec3( 0, -sin(A), cos(A) );


   return rollMat*pitchMat*yawMat;
}

// return float instead of bool because bool / int do not work on MacBookPro 10.6
float toLocal(in vec3 cartesian, in float scale, out vec2 camera) {

    float alpha = 1.0;
    
    vec3 U = cP * cartesian;
//	vec3 U = getCP(yaw, pitch, roll) * getCP(globalYaw, globalPitch, globalRoll) * cartesian;

    float x_c = 0;
    float y_c = 0;

	float r = 0;
	float fisheye_radius = imageHeight;
    if(lens==LensModel_fisheye)
    {
    	float theta = acos( U[2] / sqrt(U[0]*U[0] + U[1]*U[1] + U[2]*U[2]) );
        //float phi = atan(-U[1], U[0]);
        
		// for equidistant
        float f = imageWidth / 2 / (FoV/180*M_PI / 2);
        r = theta * f;
        
		// fisheye equisolid
        //float f = imageWidth / 4 / sin(FoV/180*M_PI/4);
        //float r = 2 * f * sin(theta/2);
		
		// fisheye stereographic
		//float f = imageWidth / 4 / tan(FoV/180*M_PI/4);
		//float r = 2 * f * tan(theta/2);
		
		// orthogonal
		//float f = imageWidth/2 / sin(FoV/180*M_PI / 2);
		//float r = f * sin(theta);
    }
	else if(lens == LensModel_standard)
	{
		// Standard
		float theta = acos( U[2] / sqrt(U[0]*U[0] + U[1]*U[1] + U[2]*U[2]) );
		
        float f = imageWidth / 2 / tan(FoV / 180 * M_PI / 2);
		
		r = f * tan(theta);
	}	
	
	float r0 = min(imageWidth, imageHeight) / 2.0f;
	float asp = max(imageWidth, imageHeight) / min(imageWidth, imageHeight);
	float rt = r / r0;
	if (lens == LensModel_fisheye)
	{
		rt = clamp(rt, 0, asp);
	}
	else if (lens == LensModel_standard)
	{
		if (rt < 0) rt = 0;
	}
	float rc = r*( ((k1*rt+k2)*rt+k3)*rt + (1-k1-k2-k3));
	
	float xoff = offset_x;
	float yoff = -offset_y;
	
	float r2 = sqrt(U[0]*U[0] + U[1]*U[1]);
	float dx = U[0];
	float dy = U[1];
	if(r2 != 0)
	{
		dx /= r2;
		dy /= r2;
	}
	
	//float Vx = rc * cos(phi);
	//float Vy = -rc * sin(phi);
	float Vx = rc * dx; 
	float Vy = rc * dy;
	
	x_c = Vx + xoff;
	y_c = Vy + yoff;
	
	x_c += cx; // OK
	y_c += cy;
	
    camera = vec2(x_c*scale, y_c*scale);
	if(rt == 0.0)
		alpha = 0.0;

    if( (x_c<0.0) || (x_c>=imageWidth) || (y_c<0.0) || (y_c>=imageHeight) )
        alpha = 0.0;

    return alpha;
}

float XYnToLocal(in vec2 xyn, in float scale, out vec2 camera) {
    float result = 1.0;

    float theta, phi;
    vec3 cartesian;

    result *= XYnToThetaPhi(xyn.x, xyn.y, theta, phi);
    sphericalToCartesian(theta, phi,cartesian);

    // cartesian to camera
    result *= toLocal(cartesian, scale, camera);

    return result;
}

float curve(float s)
{
	return s;
}

float getRadialDistance2(vec2 imageUV)
{
	float dist2;
    if(lens == 1)
	{
		float xdist = (imageWidth/2-imageUV.x) / (imageWidth/2);
		if(imageUV.x < imageWidth/2)
			xdist = xdist / xrad1;
		else
			xdist = xdist / xrad2;
		xdist = xdist * xdist;
		float ydist = (imageHeight/2-imageUV.y) / (imageHeight/2);
		if (imageUV.y > imageHeight/2)
			ydist = ydist / yrad1;
		else
			ydist = ydist / yrad2;
		ydist = ydist * ydist;
   
	   dist2 = xdist + ydist;
	   dist2 = pow(dist2, 1.0f / 2 * blendingFalloff);
	}
	else if(lens == 0)
	{
		float xdist = (imageWidth/2-imageUV.x) / (imageWidth/2 * fisheyeLensRadiusRatio1);
		float ydist = (imageHeight/2-imageUV.y) / (imageHeight/2 * fisheyeLensRadiusRatio2);
		if(xdist < 0) xdist = -xdist;
		if(ydist < 0) ydist = -ydist;
		
		dist2 = 1;
		if(xdist >= ydist)
		{
			dist2 = curve(xdist);
		}
		else
		{
			dist2 = curve(ydist);
		}
		
		dist2 = pow(dist2, 1.0f / 2 * blendingFalloff);
	}
	
	// convert [0, start, end=1.0f] to [0, 0, 1]
	float start = blendCurveStart;
	float end = 1.0f;
	dist2 = clamp((dist2-start) * (1.0f/(end-start)), 0.0f, 1.0f);
   
   return dist2;
}

void main()
{
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);

	vec2 camera;
    vec2 vTexUV = vec2(texc.x, 1.0f - texc.y);
    float alpha = XYnToLocal(vTexUV, 1.0f, camera);

	float radialDist2 = 1.0f - getRadialDistance2(camera);

	if(alpha != 0.0)
	{
		vec4 cSrc = texture2D(texture, vec2(camera.x/imageWidth, 1.0f - camera.y/imageHeight));
		cSrc.a *= alpha;
		cSrc = clamp(cSrc, 0.0, 1.0);
		cSrc.a = radialDist2;
		gl_FragColor = cSrc;
	}
	else
	{
		//discard;
		gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}