#version 460
#extension GL_GOOGLE_include_directive : enable
//  pin end
// v2----v3
// | .	  |
// |  .	  |
// |   .  |
// |    . |
// v0----v1
//  pin start


layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;
layout (location = 0) in float inFlux[];
layout (location = 0) out float outU;
//layout(location = 0) out vec3 color;
layout(push_constant) uniform PC
{
	vec2 extent;
	float thickness;
} pc;

void main()
{
    vec4 p1 = gl_in[0].gl_Position;
    vec4 p2 = gl_in[1].gl_Position;
    float flux = inFlux[0];
    if((pc.thickness * flux) < 1.0)
    {
		return;
    }
    //flux = 1.0;
    //https://stackoverflow.com/questions/54686818/glsl-geometry-shader-to-replace-gllinewidth
    vec2 dir    = normalize((p2.xy/p2.w - p1.xy/p1.w) * pc.extent);
    vec2 offset = vec2(-dir.y, dir.x) * (pc.thickness * flux) / pc.extent;

    //color = vec3(1.0, 0.0, 0.0);
    outU = 1.0;
    gl_Position = p1 + vec4(offset.xy * p1.w, 0.0, 0.0);
    EmitVertex();
    //color = vec3(1.0, 0.0, 0.0);
    outU = 0.0;
    gl_Position = p1 - vec4(offset.xy * p1.w, 0.0, 0.0);
    EmitVertex();
    //color = vec3(1.0, 0.0, 0.0);
    outU = 1.0;
    gl_Position = p2 + vec4(offset.xy * p2.w, 0.0, 0.0);
    EmitVertex();
    //color = vec3(1.0, 0.0, 0.0);
    outU = 0.0;
    gl_Position = p2 - vec4(offset.xy * p2.w, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}