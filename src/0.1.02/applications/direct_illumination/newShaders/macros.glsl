#define MAT4_POSITION(mat) (mat).m[3].xyz
#define VEC3_TO_VEC4(vec) vec4(vec, 1.0f)
#define TRANSFORM(mat, vec) (mat * vec4(vec, 1.0f)).xyz
#define TRANSFORM_DIR(mat, vec) (mat * vec4(vec, 0.0f)).xyz