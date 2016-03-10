// Public domain single-file C implementation by Constantine Tarasenkov
//
// LICENSE
//
// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#ifndef BOB_H_
#define BOB_H_

// BOB [BINARY OBJECT] MESH LOADER /////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

// TYPES ///////////////////////////////////////////////////////////////////////

typedef struct
{
  // For gpu_cmd_t
  gpu_buffer_t first;
  gpu_buffer_t count;
  // For attributes
  gpu_buffer_t mesh_id;
  gpu_buffer_t attr_first;
  gpu_buffer_t attr_id;
  gpu_buffer_t pos;
  gpu_buffer_t uv;
  gpu_buffer_t normal;
} bob_t;

// PROTOTYPES //////////////////////////////////////////////////////////////////

void gfBobReadHeader(const char * path, u32 * bytes_vp, u32 * bytes_vt, u32 * bytes_vn, u32 * bytes_id);
void gfBobRead(const char * path, u32 bytes_vp, void ** alloc_vp, u32 bytes_vt, void ** alloc_vt, u32 bytes_vn, void ** alloc_vn, u32 bytes_id, void ** alloc_id);
bob_t gfBobCreate(u32 bob_count, const char ** bob_paths);

#ifdef __cplusplus
}
#endif
#endif // BOB_H_

// IMPLEMENTATION //////////////////////////////////////////////////////////////

#ifdef BOB_IMPLEMENTATION

void gfBobReadHeader(
  const char * path,
  u32 * bytes_vp,
  u32 * bytes_vt,
  u32 * bytes_vn,
  u32 * bytes_id)
{
  SDL_RWops * fd = SDL_RWFromFile(path, "rb");
  if(fd == NULL)
    gfError("Error: File Not Found", path);
  
  SDL_RWread(fd, bytes_vp, sizeof(u32), 1);
  SDL_RWread(fd, bytes_vt, sizeof(u32), 1);
  SDL_RWread(fd, bytes_vn, sizeof(u32), 1);
  SDL_RWread(fd, bytes_id, sizeof(u32), 1);

  SDL_RWclose(fd);
}

void gfBobRead(
  const char * path,
  u32 bytes_vp, void ** alloc_vp,
  u32 bytes_vt, void ** alloc_vt,
  u32 bytes_vn, void ** alloc_vn,
  u32 bytes_id, void ** alloc_id)
{
  SDL_RWops * fd = SDL_RWFromFile(path, "rb");
  if(fd == NULL)
    gfError("Error: File Not Found", path);
  
  SDL_RWseek(fd, 4 * sizeof(u32), RW_SEEK_CUR);
  SDL_RWread(fd, *alloc_vp, bytes_vp, 1);
  SDL_RWread(fd, *alloc_vt, bytes_vt, 1);
  SDL_RWread(fd, *alloc_vn, bytes_vn, 1);
  SDL_RWread(fd, *alloc_id, bytes_id, 1);

  SDL_RWclose(fd);
}

bob_t gfBobCreate(
  u32 bob_count,
  const char ** bob_paths)
{
  bob_t bobs = {};

  bobs.first = gfBufferCreate((gpu_buffer_t){.format = x_u32, .count = bob_count});
  bobs.count = gfBufferCreate((gpu_buffer_t){.format = x_u32, .count = bob_count});
  
  u32 mesh_id_format = x_u32;
  
  if(bob_count <= 65535+1) mesh_id_format = x_u16;
  if(bob_count <= 255+1)   mesh_id_format = x_u8;
  
  bobs.mesh_id   .format = mesh_id_format;
  bobs.attr_first.format = xyz_u32;
  bobs.attr_id   .format = xyz_u32;
  bobs.pos       .format = xyz_f32;
  bobs.uv        .format =  xy_f32;
  bobs.normal    .format = xyz_f32;
  
  u32 bob_bytes_vp[bob_count];
  u32 bob_bytes_vt[bob_count];
  u32 bob_bytes_vn[bob_count];
  u32 bob_bytes_id[bob_count];
  
  for(u32 i = 0; i < bob_count; ++i)
  {
    gfBobReadHeader(
       bob_paths[i],
      &bob_bytes_vp[i],
      &bob_bytes_vt[i],
      &bob_bytes_vn[i],
      &bob_bytes_id[i]
    );
    bobs.mesh_id   .bytes += bob_bytes_id[i] / 3;
    bobs.attr_first.bytes += sizeof(u32) * 3;
    bobs.attr_id   .bytes += bob_bytes_id[i];
    bobs.pos       .bytes += bob_bytes_vp[i];
    bobs.uv        .bytes += bob_bytes_vt[i];
    bobs.normal    .bytes += bob_bytes_vn[i];
  }
  
  for(u32 i = 0; i < bob_count; ++i)
  {
    bobs.count.as_u32[i] = bob_bytes_id[i] / sizeof(u32) / 3;
  }
  
  for(u32 i = 0, total_idx_count = 0; i < bob_count; ++i)
  {
    bobs.first.as_u32[i] = total_idx_count;
    total_idx_count += bobs.count.as_u32[i];
  }
  
  bobs.mesh_id    = gfBufferCreate(bobs.mesh_id);
  bobs.attr_first = gfBufferCreate(bobs.attr_first);
  bobs.attr_id    = gfBufferCreate(bobs.attr_id);
  bobs.pos        = gfBufferCreate(bobs.pos);
  bobs.uv         = gfBufferCreate(bobs.uv);
  bobs.normal     = gfBufferCreate(bobs.normal);
  
  for(u32 i = 0,
          j = 0,
   vp_first = 0,
   vt_first = 0,
   vn_first = 0; i < bob_count; ++i)
  {
    bobs.attr_first.as_u32[j++] = vp_first;
    bobs.attr_first.as_u32[j++] = vt_first;
    bobs.attr_first.as_u32[j++] = vn_first;
    
    vp_first += bob_bytes_vp[i] / sizeof(u32) / 3;
    vt_first += bob_bytes_vt[i] / sizeof(u32) / 2;
    vn_first += bob_bytes_vn[i] / sizeof(u32) / 3;
  }
  
  if(mesh_id_format == x_u8)
  {
    for(u8 i = 0; i < bob_count; ++i)
    {
      const u32 first = bobs.first.as_u32[i];
      const u32 count = bobs.count.as_u32[i];
      SDL_memset(bobs.mesh_id.ptr + first, i, count);
    }
  }
  else
  if(mesh_id_format == x_u16)
  {
    for(u16 i = 0; i < bob_count; ++i)
    {
      const u32 first = bobs.first.as_u32[i];
      const u32 count = bobs.count.as_u32[i] + first;
      for(u32 j = first; j < count; ++j)
        bobs.mesh_id.as_u16[j] = i;
    }
  }
  else
  {
    for(u32 i = 0; i < bob_count; ++i)
    {
      const u32 first = bobs.first.as_u32[i];
      const u32 count = bobs.count.as_u32[i] + first;
      for(u32 j = first; j < count; ++j)
        bobs.mesh_id.as_u32[j] = i;
    }
  }
  
  for(u32 i = 0,
   vp_first = 0,
   vt_first = 0,
   vn_first = 0,
   id_first = 0; i < bob_count; ++i)
  {
    u8 * bob_ptr_vp = &bobs.pos    .ptr[vp_first];
    u8 * bob_ptr_vt = &bobs.uv     .ptr[vt_first];
    u8 * bob_ptr_vn = &bobs.normal .ptr[vn_first];
    u8 * bob_ptr_id = &bobs.attr_id.ptr[id_first];
    
    gfBobRead(
      bob_paths[i],
      bob_bytes_vp[i], &bob_ptr_vp,
      bob_bytes_vt[i], &bob_ptr_vt,
      bob_bytes_vn[i], &bob_ptr_vn,
      bob_bytes_id[i], &bob_ptr_id
    );
    
    vp_first += bob_bytes_vp[i];
    vt_first += bob_bytes_vt[i];
    vn_first += bob_bytes_vn[i];
    id_first += bob_bytes_id[i];
  }
  
  return bobs;
}

#endif // BOB_IMPLEMENTATION
