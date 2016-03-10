#version 330
#extension GL_ARB_shader_precision          : enable
#extension GL_ARB_conservative_depth        : enable
#extension GL_ARB_texture_cube_map_array    : enable
#extension GL_ARB_shading_language_420pack  : enable
#extension GL_ARB_shading_language_packing  : enable
#extension GL_ARB_explicit_uniform_location : enable
layout(depth_unchanged) out float gl_FragDepth;

layout(binding = 8) uniform sampler2DArray   in_textures;
layout(binding = 9) uniform samplerCubeArray in_cubemaps;

layout(location = 0) uniform vec3 cam_pos;

layout(location = 0) out vec4 fbo_mesh_id;
layout(location = 1) out vec4 fbo_diffuse;
layout(location = 2) out vec4 fbo_reflect;
layout(location = 3) out vec4 fbo_normal;
layout(location = 4) out vec4 fbo_position;

flat   in int  vs_mesh_id;
smooth in vec2 vs_uv;
smooth in vec3 vs_normal;
smooth in vec3 vs_position;

vec3 intToColor(int i)
{
  vec3 color;
  color.r = float(i >> 16 & 0xFF) / 255.0;
  color.g = float(i >> 8  & 0xFF) / 255.0;
  color.b = float(i       & 0xFF) / 255.0;
  return color;
}

void main()
{
  fbo_mesh_id  = vec4(intToColor(vs_mesh_id + 1), 1);
  fbo_diffuse  = texture(in_textures, vec3(vs_uv, vs_mesh_id));
  fbo_reflect  = texture(in_cubemaps, vec4(reflect((vs_position - cam_pos), vs_normal), 0));
  fbo_normal   = vec4(vs_normal, 1);
  fbo_position = vec4(vs_position, 1);
}
