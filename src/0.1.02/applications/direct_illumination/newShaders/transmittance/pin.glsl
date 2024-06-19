
void getRay(Pin pin, inout vec3 origin, inout vec3 direction)
{
	vec2 x = sin(pin.phi) * cos(pin.theta);
    vec2 y = sin(pin.phi) * sin(pin.theta);
    vec2 z = cos(pin.phi);
	origin = vec3(x.x, y.x, z.x);
	origin*=0.866025403784; // sqrt(3)/2
	origin += vec3(0.5);
	direction = normalize(vec3(x.y - x.x, y.y - y.x, z.y - z.x));
}
