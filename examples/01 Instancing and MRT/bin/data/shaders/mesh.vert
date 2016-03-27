#version 330
#extension GL_ARB_shader_precision          : enable
#extension GL_ARB_separate_shader_objects   : enable
#extension GL_ARB_shading_language_420pack  : enable
#extension GL_ARB_explicit_uniform_location : enable
out gl_PerVertex { vec4 gl_Position; };

vec4 qconj(vec4 q)         { return vec4(-q.xyz, q.w); }
vec3 qrot(vec3 v, vec4 q)  { return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + v * q.w); }
vec4 proj(vec3 mv, vec4 p) { return vec4(mv.xy * p.xy, mv.z * p.z + p.w, -mv.z); }

layout(binding = 0) uniform isamplerBuffer s_mesh_id;
layout(binding = 1) uniform isamplerBuffer s_attr_first;
layout(binding = 2) uniform isamplerBuffer s_attr_id;

layout(binding = 3) uniform  samplerBuffer s_pos;
layout(binding = 4) uniform  samplerBuffer s_uv;
layout(binding = 5) uniform  samplerBuffer s_nor;

layout(binding = 6) uniform isamplerBuffer s_ins_first;
layout(binding = 7) uniform  samplerBuffer s_ins_pos;

layout(location = 0) uniform vec3 cam_pos;
layout(location = 1) uniform vec4 cam_rot;
layout(location = 2) uniform vec4 cam_prj;

layout(location = 0) flat   out int  id;
layout(location = 1) smooth out vec2 uv;
layout(location = 2) smooth out vec3 nor;
layout(location = 3) smooth out vec3 pos;

void main()
{
  id = texelFetch(s_mesh_id, gl_VertexID).x;
  
  ivec3 attr_first = texelFetch(s_attr_first, id).xyz;
  ivec3 attr_id    = texelFetch(s_attr_id, gl_VertexID).xyz;
  
  pos = texelFetch(s_pos, attr_first.x + attr_id.x).xyz;
  uv  = texelFetch(s_uv,  attr_first.y + attr_id.y).xy;
  nor = texelFetch(s_nor, attr_first.z + attr_id.z).xyz;
  
  int  ins_first = texelFetch(s_ins_first, id).x;
  vec3 ins_pos   = texelFetch(s_ins_pos, ins_first + gl_InstanceID).xyz;
  
  pos += ins_pos;
  
  vec3 mv = pos;
  mv -= cam_pos;
  mv  = qrot(mv, qconj(cam_rot));
  gl_Position = proj(mv, cam_prj);
}
