

void assureNotZero(inout vec3 v)
{
	if(abs(v.x) < 0.0001f)
	{
		v.x = 0.0001f;
	}
	if(abs(v.y) < 0.0001f)
	{
		v.y = 0.0001f;
	}
	if(abs(v.z) < 0.0001f)
	{
		v.z = 0.0001f;
	}
}
