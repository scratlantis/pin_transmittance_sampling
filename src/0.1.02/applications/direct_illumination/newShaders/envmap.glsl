#define ENVMAP_PDF_BINS 64

vec2 sampleSphericalMap(vec3 v)
{
	const vec2 invAtan = vec2(0.1591, 0.3183);
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec3 uvToDirection(vec2 uv)
{
   uv+=0.5;
   float theta = uv.y * 3.14159265359;
   float phi = uv.x * 3.14159265359 * 2.0;
   return vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
}


//vec3 uvToDirection(vec2 uv)
//{
//    vec2 invAtan = vec2(0.1591, 0.3183);
//	uv -= 0.5;
//	uv /= invAtan;
//
//    float zOverX = tan(uv.x);
//	float y = sin(uv.y);
//
//
//
//	float z = cos(uv.y);
//	float r = sqrt(1.0 - z*z);
//	float phi = 2.0 * 3.14159265359 * uv.x;
//	float x = r * cos(phi);
//
//	return vec3(x, y, z);
//}


