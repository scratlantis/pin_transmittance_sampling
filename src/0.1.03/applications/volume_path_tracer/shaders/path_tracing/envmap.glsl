
vec2 dirToUv(vec3 v)
{
	const vec2 invAtan = vec2(0.1591, 0.3183);
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec3 uvToDir(vec2 uv)
{
   uv+=0.5;
   float theta = uv.y * 3.14159265359;
   float phi = uv.x * 3.14159265359 * 2.0;
   return vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
}


#ifndef ENVMAP_PDF_BINS_X
#define ENVMAP_PDF_BINS_X 64
#endif

#ifndef ENVMAP_PDF_BINS_Y
#define ENVMAP_PDF_BINS_Y 64
#endif

#ifndef ENVMAP_PDF_ARRAY
float envmapPdf[ENVMAP_PDF_BINS_Y + ENVMAP_PDF_BINS_X * ENVMAP_PDF_BINS_Y];
#define ENVMAP_PDF_ARRAY envmapPdf
#endif


vec2 sampleEnvMapPdf(inout float pdf, vec4 rng)
{
	int x = 0;
	int y = 0;
	float cdf = 0;
	float pdfRow;

	for(; y < ENVMAP_PDF_BINS_Y; y++)
	{
		pdfRow = ENVMAP_PDF_ARRAY[y];
		cdf += pdfRow;
		if(rng.y < cdf)
		{
			break;
		}
	}

	cdf = 0;
	float pdfColumn;
	for(; x < ENVMAP_PDF_BINS_X; x++)
	{
		pdfColumn = ENVMAP_PDF_ARRAY[ENVMAP_PDF_BINS_Y + y * ENVMAP_PDF_BINS_X + x];
		cdf += pdfColumn;
		if(rng.x < cdf)
		{
			break;
		}
	}
	
	float selectionPDF = pdfRow * pdfColumn;
	pdf = selectionPDF * float(ENVMAP_PDF_BINS_X * ENVMAP_PDF_BINS_Y);


	vec2 uv = vec2((float(x)+rng.z)/ENVMAP_PDF_BINS_X, (float(y)+rng.w)/ENVMAP_PDF_BINS_Y);

	return uv;
}
