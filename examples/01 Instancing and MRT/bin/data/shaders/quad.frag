#version 330
#extension GL_ARB_shader_precision          : enable
#extension GL_ARB_conservative_depth        : enable
#extension GL_ARB_texture_cube_map_array    : enable
#extension GL_ARB_shading_language_420pack  : enable
#extension GL_ARB_shading_language_packing  : enable
#extension GL_ARB_explicit_uniform_location : enable
layout(depth_unchanged) out float gl_FragDepth;

vec4 when_eq  (vec4 x, vec4 y) { return 1.0 - abs(sign(x - y)); }
vec4 when_neq (vec4 x, vec4 y) { return abs(sign(x - y)); }
vec4 when_gt  (vec4 x, vec4 y) { return max(sign(x - y), 0); }
vec4 when_lt  (vec4 x, vec4 y) { return max(sign(y - x), 0); }
vec4 when_ge  (vec4 x, vec4 y) { return 1.0 - when_lt(x, y); }
vec4 when_le  (vec4 x, vec4 y) { return 1.0 - when_gt(x, y); }

vec3 ScreenSpaceDither(vec2 vScreenPos)
{
  vec3 vDither = vec3(dot(vec2(171.0, 231.0), vScreenPos.xy));
	vDither.rgb = fract(vDither.rgb / vec3(103.0, 71.0, 97.0)) - vec3(0.5, 0.5, 0.5);
  return vDither.rgb / 255.0;
}

int colorToInt(vec3 c)
{
  int r = int(c.r * 255.0);
  int g = int(c.g * 255.0);
  int b = int(c.b * 255.0);
  return b + (g << 8) + (r << 16);
}

layout(location = 0) uniform vec3 cam_pos;

layout(binding = 10) uniform sampler2DArray in_fbo_depth;
layout(binding = 11) uniform sampler2DArray in_fbo_mesh_id;
layout(binding = 12) uniform sampler2DArray in_fbo_diffuse;
layout(binding = 13) uniform sampler2DArray in_fbo_reflect;
layout(binding = 14) uniform sampler2DArray in_fbo_normal;
layout(binding = 15) uniform sampler2DArray in_fbo_position;

smooth in vec2 vs_uv;
out vec4 color;

void main()
{
  float depth    = texture(in_fbo_depth,    vec3(vs_uv, 0)).r;
  vec3  mesh_id  = texture(in_fbo_mesh_id,  vec3(vs_uv, 0)).rgb;
  vec4  diffuse  = texture(in_fbo_diffuse,  vec3(vs_uv, 0));
  vec4  reflect  = texture(in_fbo_reflect,  vec3(vs_uv, 0));
  vec3  normal   = texture(in_fbo_normal,   vec3(vs_uv, 0)).rgb;
  vec3  position = texture(in_fbo_position, vec3(vs_uv, 0)).rgb;
  
  vec4 id = vec4(colorToInt(mesh_id) - 1);
  
  vec4 colors = vec4(0);
  colors += vec4(1.0, 0.0, 0.0, 1.0) * when_eq(id, vec4(0));
  colors += vec4(0.0, 1.0, 0.0, 1.0) * when_eq(id, vec4(1));
  colors += vec4(0.0, 0.0, 1.0, 1.0) * when_eq(id, vec4(2));
  
  color = mix((diffuse * 0.4) + (colors * 0.6), reflect, dot(normal, normalize(position - cam_pos)) * 0.5 + 0.5);
  
  color += vec4(ScreenSpaceDither(gl_FragCoord.xy), 0);
}
