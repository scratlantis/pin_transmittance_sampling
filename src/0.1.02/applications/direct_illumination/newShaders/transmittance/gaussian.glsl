

float eval_gaussian(float x, float mean, float stdDeviation)
{
	float c = 0.3989422804014337; // 1/sqrt(2*pi)
	float a = (x - mean) / stdDeviation;
	return (1.0/stdDeviation)*c*exp(-0.5 * a * a);
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
	return 1.0 / (1.0 + exp(c1*z + c2*z3+c3*z6+c4*z7+c5*z8+c6*z9));
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
	 return f1 * abs(f2);
}