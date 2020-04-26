uniform sampler2D texture;
uniform mat3 placeMat;
varying vec4 texc;

#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.785398163397448309616

void sphericalToCartesian(in float theta, in float phi, out vec3 cartesian) {
    cartesian.y = sin(phi);
    cartesian.x = cos(phi) * sin(theta);
    cartesian.z = cos(phi) * cos(theta);
}

float cartesianTospherical(in vec3 cartesian, out float theta, out float phi) {
    float result = 1.0f;
	phi = asin(cartesian.y);
	float cosphi = pow(1.0f - (cartesian.y*cartesian.y), 0.5f);
	if (cosphi == 0.0f)
	{
		theta = 0.0f;
		return 0.0f;
	}
	else
	{
		theta = atan(cartesian.x / cosphi, cartesian.z / cosphi);
		return 1.0f;
	}
}

// return float instead of bool because bool / int do not work on MacBookPro 10.6
float XYnToThetaPhi(in float x_n, in float y_n, out float theta,  out float phi)
{
    float result = 1.0f;
    theta = (2.0f * x_n - 1.0f) * M_PI;
    phi   = M_PI * y_n - M_PI_2;

    return result;
}

// return float instead of bool because bool / int do not work on MacBookPro 10.6
float ThetaPhiToXYn(in float theta,  in float phi, out float x_n, out float y_n)
{
    float result = 1.0f;
	x_n = (theta / M_PI + 1) / 2;
	y_n = (phi + M_PI_2) / M_PI;
    return result;
}

float XYnToDstXYn(in vec2 xyn, out vec2 dstXyn) {
    float result = 1.0;

    float theta, phi;
    vec3 cartesian;

    result *= XYnToThetaPhi(xyn.x, xyn.y, theta, phi);
    sphericalToCartesian(theta, phi,cartesian);
	vec3 U = placeMat * cartesian;
	cartesianTospherical(U, theta, phi);
	float x, y;
	ThetaPhiToXYn(theta, phi, x, y);
	dstXyn.x = x;
	dstXyn.y = y;
    return result;
}

void main(void)
{
	vec2 dstUV;
	vec2 vTexUV = vec2(texc.x, texc.y);
	XYnToDstXYn(vTexUV, dstUV);
	gl_FragColor = texture2D(texture, dstUV);
}