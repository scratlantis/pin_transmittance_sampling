

float eval_gaussian(float x, float mean, float stdDeviation)
{
	float c = 0.3989422804014337; // 1/sqrt(2*pi)
	float a = (x - mean) / stdDeviation;
	return max((1.0/stdDeviation)*c*exp(-0.5 * a * a), 0.0);
}

float gauss_cdf_approx_Eidous_Ananbeh(float z)
{
	float c1 = 1.5957764;
	float c2 = 0.0726161;
	float c3 = 0.00003318;
	float c4 = 0.00021785;
	float c5 = 0.00006293;
	float c6 = 0.00000519;
	float z3 = z * z * z;
	float z6 = z3*z3;
	float z7 = z6*z;
	float z8 = z7*z;
	float z9 = z8*z;
	return clamp(1.0 - 1.0 / (1.0 + exp(c1*z + c2*z3+c3*z6+c4*z7+c5*z8+c6*z9)), 0.0, 1.0);
}

float evalTransmittanceGaussianSegment(vec3 origin, vec3 destination, Gaussian g)
{
	 float stdDeviation = sqrt(g.variance);
	 vec3 direction = normalize(destination - origin);
	 float t = dot(direction, g.mean - origin);
	 vec3 p = origin + t * direction;
	 float d1 = length(p - g.mean);
	 float f1 = eval_gaussian(d1, 0.0, stdDeviation);
	 
	 float invStdDeviation = 1.0/stdDeviation;
	 float dOrigin = -dot(direction, g.mean - origin) * invStdDeviation;
	 float dDst = -dot(direction, g.mean - destination) * invStdDeviation;

	 float c = 0.3989422804014337; // 1/sqrt(2*pi)
	 float f2 = c * invStdDeviation * (gauss_cdf_approx_Eidous_Ananbeh(dDst) - gauss_cdf_approx_Eidous_Ananbeh(dOrigin));
	 return f1 * f2;
}

float evalTransmittanceGaussianSegmentPhi(vec3 origin, vec3 destination, Gaussian g, inout float phiOrigin, inout float phiDst)
{
	 float stdDeviation = sqrt(g.variance);
	 vec3 direction = normalize(destination - origin);
	 float t = dot(direction, g.mean - origin);
	 vec3 p = origin + t * direction;
	 float d1 = length(p - g.mean);
	 float f1 = eval_gaussian(d1, 0.0, stdDeviation);
	 
	 float invStdDeviation = 1.0/stdDeviation;
	 float dOrigin = -dot(direction, g.mean - origin) * invStdDeviation;
	 float dDst = -dot(direction, g.mean - destination) * invStdDeviation;

	 float c = 0.3989422804014337; // 1/sqrt(2*pi)

	 phiOrigin = gauss_cdf_approx_Eidous_Ananbeh(dOrigin);
	 phiDst = gauss_cdf_approx_Eidous_Ananbeh(dDst);
	 float f2 = c * invStdDeviation * (phiDst - phiOrigin);
	 return f1 * f2;
}

float gauss_inv_cdf_approx_Schmeiser(float pIn)
{
	float coef = 5.06329113924; // 1.0 / 0.1975
	if(pIn>=0.5)
	{
		float p = pIn;
		return (pow(p, 0.135) - pow(1.0 - p, 0.135)) * coef;
	}
	else
	{
		float p = 1.0-pIn;
		return - (pow(p, 0.135) - pow(1.0 - p, 0.135)) * coef;
	}
}


bool intersect2(vec3 origin, vec3 destination, Gaussian g, vec2 rng, float weight, inout float t)
{
	float phiOrigin = 0;
	float phiDst = 0;
	float density = weight*evalTransmittanceGaussianSegmentPhi(origin, destination, g, phiOrigin, phiDst);


	float transmittance = exp(-density);

	//vec2 r = vec2(0.01);
	if(rng.x < transmittance)
	{
		return false;
	}
	float p = rng.y * (phiDst - phiOrigin) + phiOrigin;
	float z = gauss_inv_cdf_approx_Schmeiser(p);

	z*=sqrt(g.variance);
	vec3 dir = normalize(destination - origin);
	z+=dot(dir, g.mean - origin);
	t = min(t,z);
	return true;
}



bool intersect(vec3 origin, vec3 destination, Gaussian g, vec2 rng, float weight, inout float t)
{
	float phiOrigin = 0;
	float phiDst = 0;
	float transmittance = weight*evalTransmittanceGaussianSegmentPhi(origin, destination, g, phiOrigin, phiDst);

	//vec2 r = vec2(0.01);
	if(rng.x > transmittance)
	{
		return false;
	}
	float p = rng.y * (phiDst - phiOrigin) + phiOrigin;
	float z = gauss_inv_cdf_approx_Schmeiser(p);

	z*=sqrt(g.variance);
	vec3 dir = normalize(destination - origin);
	z+=dot(dir, g.mean - origin);
	t = min(t,z);
	return true;
}