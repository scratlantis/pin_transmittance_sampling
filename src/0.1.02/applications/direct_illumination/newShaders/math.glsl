void assureNotZero(inout vec3 v)
{
	if(abs(v.x) < EPSILON)
	{
		v.x = EPSILON;
	}
	if(abs(v.y) < EPSILON)
	{
		v.y = EPSILON;
	}
	if(abs(v.z) < EPSILON)
	{
		v.z = EPSILON;
	}
}
