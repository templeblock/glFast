#define GLFAST_IMPLEMENTATION
#include "../../glfast.h"

#define countof(x) (sizeof(x) / sizeof((x)[0]))

typedef struct
{
  union { f32 x; f32 r; };
  union { f32 y; f32 g; };
  union { f32 z; f32 b; };
  union { f32 w; f32 a; };
} vec4;

i32 main()
{
  SDL_Window  * sdl_window;
  SDL_GLContext sdl_glcontext;
  
  gfWindow(&sdl_window, &sdl_glcontext, 0, 0, "GPGPU", 1280, 720, 4);
  
  u32 dim_x = 4096;
  u32 dim_y = 4096;
  
  gpu_storage_t mat_1 = gfStorageCreate(.format = x_f32, .count = dim_x * dim_y);
  gpu_storage_t mat_2 = gfStorageCreate(.format = x_f32, .count = dim_x * dim_y);
  
  for(u32 i = 0; i < mat_1.count; ++i) mat_1.as_f32[i] = (f32)i + 1.f;
  for(u32 i = 0; i < mat_2.count; ++i) mat_2.as_f32[i] = (f32)i + 1.f;
  
  const char * vs_str = GF_VERT_HEAD GF_TO_STRING
  (
    const vec2 pos[] = vec2[]
    (
      vec2( -1.0, -1.0 ),
      vec2(  1.0, -1.0 ),
      vec2( -1.0,  1.0 ),
      vec2( -1.0,  1.0 ),
      vec2(  1.0, -1.0 ),
      vec2(  1.0,  1.0 )
    );
    
    void main()
    {
      gl_Position = vec4(pos[gl_VertexID], 0, 1);
    }
  );
  
  const char * fs_str = GF_FRAG_HEAD GF_TO_STRING
  (
    layout(binding = 0) uniform samplerBuffer mat_1;
    layout(binding = 1) uniform samplerBuffer mat_2;
    
    layout(location = 0) out vec4 mat_out;
    
    void main()
    {
      int row = int(floor(gl_FragCoord.x));
      int col = int(floor(gl_FragCoord.y));
      
      float mat_1_elem = texelFetch(mat_1, col * 4096 + row).x;
      float mat_2_elem = texelFetch(mat_2, col * 4096 + row).x;
      
      float mat_sum = mat_1_elem + mat_2_elem;
      
      mat_out = vec4(mat_sum, 0, 0, 0);
    }
  );
  
  u32 vs = gfProgramCreateFromString(GL_VERTEX_SHADER, vs_str);
  u32 fs = gfProgramCreateFromString(GL_FRAGMENT_SHADER, fs_str);
  u32 pp = gfProgramPipelineCreate(vs, fs);
  
  gpu_texture_t mat_out = gfTextureCreate(.w = dim_x, dim_y, .format = rgba_f32);
  
  u32 fbo_colors[] =
  {
    [0] = mat_out.id,
  };
  
  u32 fbo = gfFboCreate(0, 0, countof(fbo_colors), fbo_colors, 0);
  
  u32 state_textures[16] =
  {
    [0] = mat_1.id,
    [1] = mat_2.id,
    [2] = mat_out.id
  };
  
  glBindTextures(0, 16, state_textures);
  
  gfClear();
  SDL_GL_SwapWindow(sdl_window);
  
  glViewport(0, 0, dim_x, dim_y);
  gfFboBind(fbo);
    gfFire(pp, 6);
  gfFboBind(0);
  glViewport(0, 0, 1280, 720);
  
  u32 pixels_bytes = dim_x * dim_y * sizeof(vec4);
  vec4 * pixels = SDL_malloc(pixels_bytes);
  
  gfTextureGetPixels(mat_out.id, 0, 0, 0, dim_x, dim_y, GL_RGBA, GL_FLOAT, pixels_bytes, pixels);
  
  char print_str[10000] = {0};
  SDL_snprintf(print_str, 10000, "pixels[%d].rgba: %f %f %f %f\n", dim_x * dim_y - 1,
    pixels[dim_x * dim_y - 1].r,
    pixels[dim_x * dim_y - 1].g,
    pixels[dim_x * dim_y - 1].b,
    pixels[dim_x * dim_y - 1].a
  );
  
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Completed", print_str, NULL);
  
  SDL_free(pixels);
  
  return 0;
}
