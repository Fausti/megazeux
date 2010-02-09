/* MegaZeux
 *
 * Copyright (C) 2008 Joel Bouchard Lamontagne <logicow@gmail.com>
 * Copyright (C) 2006-2007 Gilead Kutnick <exophase@adelphia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "graphics.h"
#include "render.h"
#include "render_sdl.h"
#include "render_gl.h"
#include "renderers.h"
#include "util.h"
#include "data.h"

typedef struct
{
  int syms_loaded;
  void (APIENTRY *glAlphaFunc)(GLenum func, GLclampf ref);
  void (APIENTRY *glBegin)(GLenum mode);
  void (APIENTRY *glBindTexture)(GLenum target, GLuint texture);
  void (APIENTRY *glBlendFunc)(GLenum sfactor, GLenum dfactor);
  void (APIENTRY *glClear)(GLbitfield mask);
  void (APIENTRY *glColor3ubv)(const GLubyte *v);
  void (APIENTRY *glColor4f)(GLfloat red, GLfloat green, GLfloat blue,
   GLfloat alpha);
  void (APIENTRY *glColor4ub)(GLubyte red, GLubyte green, GLubyte blue,
   GLubyte alpha);
  void (APIENTRY *glCopyTexImage2D)(GLenum target, GLint level,
   GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height,
   GLint border);
  void (APIENTRY *glDisable)(GLenum cap);
  void (APIENTRY *glEnable)(GLenum cap);
  void (APIENTRY *glEnd)(void);
  void (APIENTRY *glGenTextures)(GLsizei n, GLuint *textures);
  const GLubyte* (APIENTRY *glGetString)(GLenum name);
  void (APIENTRY *glTexCoord2f)(GLfloat s, GLfloat t);
  void (APIENTRY *glTexImage2D)(GLenum target, GLint level,
   GLint internalformat, GLsizei width, GLsizei height, GLint border,
   GLenum format, GLenum type, const GLvoid *pixels);
  void (APIENTRY *glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
  void (APIENTRY *glTexParameteri)(GLenum target, GLenum pname, GLint param);
  void (APIENTRY *glTexSubImage2D)(GLenum target, GLint level, GLint xoffset,
   GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type,
   const GLvoid *pixels);
  void (APIENTRY *glVertex2f)(GLfloat x, GLfloat y);
  void (APIENTRY *glVertex2i)(GLint x, GLint y);
  void (APIENTRY *glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);

  GLenum (APIENTRY *glCreateProgramObjectARB)(void);
  GLenum (APIENTRY *glCreateShaderObjectARB)(GLenum shaderType);
  void (APIENTRY *glShaderSourceARB)(GLhandleARB shader, GLuint number_strings,
   const GLcharARB** strings, GLint * length);
  void (APIENTRY *glCompileShaderARB)(GLhandleARB shader);
  void (APIENTRY *glAttachObjectARB)(GLhandleARB program, GLhandleARB shader);
  void (APIENTRY *glUseProgramObjectARB)(GLhandleARB program);
  void (APIENTRY *glLinkProgramARB)(GLhandleARB program);
} glsl_syms;

typedef struct
{
  Uint32 *pixels;
  Uint32 charset_texture[CHAR_H * CHARSET_SIZE * CHAR_W * 2];
  Uint32 background_texture[SCREEN_W * SCREEN_H];
  GLuint texture_number[3];
  GLubyte palette[3 * SMZX_PAL_SIZE];
  Uint8 remap_texture;
  Uint8 remap_char[CHARSET_SIZE * 2];
  Uint8 ignore_linear;
  glsl_syms gl;
  char gl_tilemap_fragment_shader_path[MAX_PATH];
  char gl_tilemap_vertex_shader_path[MAX_PATH];
  char gl_scaling_fragment_shader_path[MAX_PATH];
  char gl_scaling_vertex_shader_path[MAX_PATH];
  Uint32 gl_tilemap;
  Uint32 gl_scaling;
} glsl_render_data;

static int glsl_load_syms(glsl_syms *gl)
{
  if(gl->syms_loaded)
    return true;

  // Since 1.1
  GL_LOAD_SYM(gl, glAlphaFunc)
  // Since 1.0
  GL_LOAD_SYM(gl, glBegin)
  // Since 1.1
  GL_LOAD_SYM(gl, glBindTexture)
  // Since 1.0
  GL_LOAD_SYM(gl, glBlendFunc)
  // Since 1.0
  GL_LOAD_SYM(gl, glClear)
  // Since 1.0
  GL_LOAD_SYM(gl, glColor3ubv)
  // Since 1.0
  GL_LOAD_SYM(gl, glColor4f)
  // Since 1.0
  GL_LOAD_SYM(gl, glColor4ub)
  // Since 1.1
  GL_LOAD_SYM(gl, glCopyTexImage2D)
  // Since 1.0 (parameters may require more recent version)
  GL_LOAD_SYM(gl, glDisable)
  // Since 1.0 (parameters may require more recent version)
  GL_LOAD_SYM(gl, glEnable)
  // Since 1.0
  GL_LOAD_SYM(gl, glEnd)
  // Since 1.1
  GL_LOAD_SYM(gl, glGenTextures)
  // Since 1.0
  GL_LOAD_SYM(gl, glGetString)
  // Since 1.0
  GL_LOAD_SYM(gl, glTexCoord2f)
  // Since 1.0 (parameters may require more recent version)
  GL_LOAD_SYM(gl, glTexImage2D)
  // Since 1.0
  GL_LOAD_SYM(gl, glTexParameterf)
  // Since 1.0
  GL_LOAD_SYM(gl, glTexParameteri)
  // Since 1.1
  GL_LOAD_SYM(gl, glTexSubImage2D)
  // Since 1.0
  GL_LOAD_SYM(gl, glVertex2f)
  // Since 1.0
  GL_LOAD_SYM(gl, glVertex2i)
  // Since 1.0
  GL_LOAD_SYM(gl, glViewport)

  // Added since 1.4/1.5: shader programming
  GL_LOAD_SYM_EXT(gl, glCreateProgramObjectARB)
  GL_LOAD_SYM_EXT(gl, glCreateShaderObjectARB)
  GL_LOAD_SYM_EXT(gl, glShaderSourceARB)
  GL_LOAD_SYM_EXT(gl, glCompileShaderARB);
  GL_LOAD_SYM_EXT(gl, glAttachObjectARB);
  GL_LOAD_SYM_EXT(gl, glUseProgramObjectARB);
  GL_LOAD_SYM_EXT(gl, glLinkProgramARB);

  gl->syms_loaded = true;
  return true;
}

static char *glsl_loadstring(char *filename)
{
  char *buffer = NULL;
  unsigned long size;
  FILE *f;

  f = fopen(filename, "rb");
  if(!f)
    goto err_out;

  size = ftell_and_rewind(f);
  if(!size)
    goto err_close;

  buffer = malloc(size + 1);
  buffer[size] = '\0';

  if(fread(buffer, size, 1, f) != 1)
  {
    free(buffer);
    buffer = NULL;
  }

err_close:
  fclose(f);
err_out:
  return buffer;
}

static void glsl_load_shaders(graphics_data *graphics)
{
  int len;
  GLenum shader_vert_scaling;
  GLenum shader_frag_scaling;
  GLenum shader_vert_tilemap;
  GLenum shader_frag_tilemap;
  GLenum scaling_program;
  GLenum tilemap_program;
  char *shader_vert_source_scaling;
  char *shader_frag_source_scaling;
  char *shader_vert_source_tilemap;
  char *shader_frag_source_tilemap;
  glsl_syms *gl;
  glsl_render_data *render_data = graphics->render_data;
  gl = &render_data->gl;

  shader_vert_source_tilemap =
    glsl_loadstring(render_data->gl_tilemap_vertex_shader_path);
  shader_frag_source_tilemap =
    glsl_loadstring(render_data->gl_tilemap_fragment_shader_path);

  if(shader_vert_source_tilemap != NULL && shader_frag_source_tilemap != NULL)
  {
    tilemap_program = gl->glCreateProgramObjectARB();
    shader_vert_tilemap = gl->glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    shader_frag_tilemap = gl->glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

    len = (int)strlen(shader_vert_source_tilemap);
    gl->glShaderSourceARB(shader_vert_tilemap, 1,
      (const GLcharARB**)&shader_vert_source_tilemap, &len);
    len = (int)strlen(shader_frag_source_tilemap);
    gl->glShaderSourceARB(shader_frag_tilemap, 1,
      (const GLcharARB**)&shader_frag_source_tilemap, &len);

    gl->glCompileShaderARB(shader_vert_tilemap);
    gl->glCompileShaderARB(shader_frag_tilemap);
    gl->glAttachObjectARB(tilemap_program, shader_vert_tilemap);
    gl->glAttachObjectARB(tilemap_program, shader_frag_tilemap);
    gl->glLinkProgramARB(tilemap_program);

    free(shader_vert_source_tilemap);
    free(shader_frag_source_tilemap);
  }
  else
  {
    if(shader_vert_source_tilemap != NULL)
       free(shader_vert_source_tilemap);
    if(shader_frag_source_tilemap != NULL)
       free(shader_frag_source_tilemap);
    tilemap_program = 0;
  }

  shader_vert_source_scaling =
    glsl_loadstring(render_data->gl_scaling_vertex_shader_path);
  shader_frag_source_scaling =
    glsl_loadstring(render_data->gl_scaling_fragment_shader_path);

  if(shader_vert_source_scaling != NULL && shader_frag_source_scaling != NULL)
  {
    scaling_program = gl->glCreateProgramObjectARB();
    shader_vert_scaling = gl->glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    shader_frag_scaling = gl->glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

    len = (int)strlen(shader_vert_source_scaling);
    gl->glShaderSourceARB(shader_vert_scaling, 1,
      (const GLcharARB**)&shader_vert_source_scaling, &len);
    len = (int)strlen(shader_frag_source_scaling);
    gl->glShaderSourceARB(shader_frag_scaling, 1,
      (const GLcharARB**)&shader_frag_source_scaling, &len);

    gl->glCompileShaderARB(shader_vert_scaling);
    gl->glCompileShaderARB(shader_frag_scaling);
    gl->glAttachObjectARB(scaling_program, shader_vert_scaling);
    gl->glAttachObjectARB(scaling_program, shader_frag_scaling);
    gl->glLinkProgramARB(scaling_program);

    free(shader_vert_source_scaling);
    free(shader_frag_source_scaling);
  }
  else
  {
    if(shader_vert_source_scaling != NULL)
       free(shader_vert_source_scaling);
    if(shader_frag_source_scaling != NULL)
       free(shader_frag_source_scaling);
    scaling_program = 0;
  }

  render_data->gl_scaling = scaling_program;
  render_data->gl_tilemap = tilemap_program;
}

static int glsl_init_video(graphics_data *graphics, config_info *conf)
{
  glsl_render_data *render_data = malloc(sizeof(glsl_render_data));
  glsl_syms *gl = &render_data->gl;
  const char *version, *extensions;

  if(!render_data)
    return false;

  if(!GL_CAN_USE)
    goto err_free;

  graphics->render_data = render_data;
  gl->syms_loaded = false;

  graphics->gl_vsync = conf->gl_vsync;
  graphics->allow_resize = conf->allow_resize;
  graphics->gl_filter_method = conf->gl_filter_method;
  sprintf(render_data->gl_tilemap_fragment_shader_path, "%s/%s", current_dir,
    conf->gl_tilemap_fragment_shader);
  sprintf(render_data->gl_tilemap_vertex_shader_path, "%s/%s", current_dir,
    conf->gl_tilemap_vertex_shader);
  sprintf(render_data->gl_scaling_fragment_shader_path, "%s/%s", current_dir,
    conf->gl_scaling_fragment_shader);
  sprintf(render_data->gl_scaling_vertex_shader_path, "%s/%s", current_dir,
    conf->gl_scaling_vertex_shader);
  graphics->bits_per_pixel = 32;

  // OpenGL only supports 16/32bit colour
  if(conf->force_bpp == 16 || conf->force_bpp == 32)
    graphics->bits_per_pixel = conf->force_bpp;

  render_data->pixels = malloc(sizeof(Uint32) * GL_POWER_2_WIDTH *
   GL_POWER_2_HEIGHT);
  if(!render_data->pixels)
    goto err_free;

  if(!set_video_mode())
    goto err_free;

  // NOTE: These must come AFTER set_video_mode()!
  version = (const char *)gl->glGetString(GL_VERSION);
  extensions = (const char *)gl->glGetString(GL_EXTENSIONS);

  // we need a specific "version" of OpenGL compatibility
  if(version && atof(version) < 1.1)
  {
    fprintf(stderr, "OpenGL implementation is too old (need v1.1).\n");
    goto err_free;
  }

  // we also need to be able to utilise shader extensions
  if(!(extensions && strstr(extensions, "GL_ARB_shading_language_100")))
  {
    fprintf(stderr, "OpenGL missing GL_ARB_shading_language_100 extension.");
    goto err_free;
  }

  return true;

err_free:
  free(render_data);
  return false;
}

static void glsl_remap_charsets(graphics_data *graphics)
{
  glsl_render_data *render_data = graphics->render_data;
  render_data->remap_texture = true;
}

// FIXME: Many magic numbers
static void glsl_resize_screen(graphics_data *graphics, int viewport_width,
 int viewport_height)
{
  glsl_render_data *render_data = graphics->render_data;
  glsl_syms *gl = &render_data->gl;

  render_data->ignore_linear = false;

  gl->glViewport(0, 0, viewport_width, viewport_height);

  gl->glGenTextures(3, render_data->texture_number);

  gl->glBindTexture(GL_TEXTURE_2D, render_data->texture_number[0]);

  gl_set_filter_method(CONFIG_GL_FILTER_LINEAR, gl->glTexParameteri);

  gl->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  gl->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  gl->glEnable(GL_TEXTURE_2D);
  gl->glAlphaFunc(GL_GREATER, 0.565f);

  gl->glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

  memset(render_data->pixels, 255,
   sizeof(Uint32) * GL_POWER_2_WIDTH * GL_POWER_2_HEIGHT);

  gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GL_POWER_2_WIDTH,
   GL_POWER_2_HEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE,
   render_data->pixels);

  gl->glBindTexture(GL_TEXTURE_2D, render_data->texture_number[1]);

  gl_set_filter_method(CONFIG_GL_FILTER_NEAREST, gl->glTexParameteri);

  gl->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  gl->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA,
   GL_UNSIGNED_BYTE, render_data->pixels);

  glsl_remap_charsets(graphics);

  gl->glBindTexture(GL_TEXTURE_2D, render_data->texture_number[2]);

  gl_set_filter_method(CONFIG_GL_FILTER_NEAREST, gl->glTexParameteri);

  gl->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  gl->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 32, 0, GL_RGBA,
   GL_UNSIGNED_BYTE, render_data->pixels);

  glsl_load_shaders(graphics);
}

static int glsl_set_video_mode(graphics_data *graphics, int width, int height,
 int depth, int fullscreen, int resize)
{
  glsl_render_data *render_data = graphics->render_data;
  glsl_syms *gl = &render_data->gl;

  gl_set_attributes(graphics);

  if(!SDL_SetVideoMode(width, height, depth,
   GL_STRIP_FLAGS(sdl_flags(depth, fullscreen, resize))))
    return false;

  if(!glsl_load_syms(gl))
    return false;

  glsl_resize_screen(graphics, width, height);
  return true;
}

static void glsl_remap_char(graphics_data *graphics, Uint16 chr)
{
  glsl_render_data *render_data = graphics->render_data;
  render_data->remap_char[chr] = true;
}

static void glsl_remap_charbyte(graphics_data *graphics, Uint16 chr, Uint8 byte)
{
  glsl_remap_char(graphics, chr);
}

static int *glsl_char_bitmask_to_texture(signed char *c, int *p)
{
  // This tests the 7th bit, if 0, result is 0.
  // If 1, result is 255 (because of sign extended bit shift!).
  // Note the use of char constants to force 8bit calculation.
  *(p++) = *c << 24 >> 31;
  *(p++) = *c << 25 >> 31;
  *(p++) = *c << 26 >> 31;
  *(p++) = *c << 27 >> 31;
  *(p++) = *c << 28 >> 31;
  *(p++) = *c << 29 >> 31;
  *(p++) = *c << 30 >> 31;
  *(p++) = *c << 31 >> 31;
  return p;
}

static inline void glsl_do_remap_charsets(graphics_data *graphics)
{
  glsl_render_data *render_data = graphics->render_data;
  glsl_syms *gl = &render_data->gl;
  signed char *c = (signed char *)graphics->charset;
  int *p = (int *)render_data->charset_texture;
  unsigned int i, j, k;

  for(i = 0; i < 16; i++, c += -14 + 32 * 14)
    for(j = 0; j < 14; j++, c += -32 * 14 + 1)
      for(k = 0; k < 32; k++, c += 14)
        p = glsl_char_bitmask_to_texture(c, p);

  gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 32 * 8, 16 * 14, GL_RGBA,
    GL_UNSIGNED_BYTE, render_data->charset_texture);
}

static inline void glsl_do_remap_char(graphics_data *graphics, Uint16 chr)
{
  glsl_render_data *render_data = graphics->render_data;
  glsl_syms *gl = &render_data->gl;
  signed char *c = (signed char *)graphics->charset;
  int *p = (int *)render_data->charset_texture;
  unsigned int i;

  c += chr * 14;

  for(i = 0; i < 14; i++, c++)
    p = glsl_char_bitmask_to_texture(c, p);
  gl->glTexSubImage2D(GL_TEXTURE_2D, 0, chr % 32 * 8, chr / 32 * 14, 8, 14,
   GL_RGBA, GL_UNSIGNED_BYTE, render_data->charset_texture);
}

static void glsl_update_colors(graphics_data *graphics, rgb_color *palette,
 Uint32 count)
{
  glsl_render_data *render_data = graphics->render_data;
  Uint32 i;
  for(i = 0; i < count; i++)
  {
#if PLATFORM_BYTE_ORDER == PLATFORM_BIG_ENDIAN
    graphics->flat_intensity_palette[i] = (palette[i].r << 24) |
     (palette[i].g << 16) | (palette[i].b << 8);
#else
    graphics->flat_intensity_palette[i] = (palette[i].b << 16) |
     (palette[i].g << 8) | palette[i].r;
#endif
    render_data->palette[i*3  ] = (GLubyte)palette[i].r;
    render_data->palette[i*3+1] = (GLubyte)palette[i].g;
    render_data->palette[i*3+2] = (GLubyte)palette[i].b;
  }
}

static void glsl_render_graph(graphics_data *graphics)
{
  glsl_render_data *render_data = graphics->render_data;
  glsl_syms *gl = &render_data->gl;
  Uint32 i;
  Uint32 *dest;
  Uint32 *colorptr;
  char_element *src = graphics->text_video;

  gl->glUseProgramObjectARB(render_data->gl_tilemap);

  gl->glBindTexture(GL_TEXTURE_2D, render_data->texture_number[1]);
  if(render_data->remap_texture)
  {
    glsl_do_remap_charsets(graphics);
    render_data->remap_texture = false;
    memset(render_data->remap_char, false, sizeof(Uint8) * CHARSET_SIZE * 2);
  }
  else
  {
    for(i = 0; i < CHARSET_SIZE * 2; i++)
    {
      if(render_data->remap_char[i])
      {
        glsl_do_remap_char(graphics, i);
        render_data->remap_char[i] = false;
      }
    }
  }

  if(graphics->window_width < 640 || graphics->window_height < 350)
  {
    gl->glViewport(0, 0,
      (graphics->window_width<1024)?graphics->window_width:1024,
      (graphics->window_height<512)?graphics->window_height:512);
  }
  else
  {
    gl->glViewport(0, 0, 640, 350);
  }

  dest = render_data->background_texture;

  for(i = 0; i < SCREEN_W * SCREEN_H; i++, dest++, src++)
    *dest = (src->char_value<<16) + (src->bg_color<<8) + src->fg_color;

  gl->glBindTexture(GL_TEXTURE_2D, render_data->texture_number[1]);

  gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 225, SCREEN_W, SCREEN_H, GL_RGBA,
    GL_UNSIGNED_BYTE, render_data->background_texture);

  colorptr = graphics->flat_intensity_palette;
  dest = render_data->background_texture;

  for(i = 0; i < 512; i++, dest++, colorptr++)
    *dest = *colorptr;

  gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 224, 256, 1, GL_RGBA,
    GL_UNSIGNED_BYTE, render_data->background_texture);

  gl->glColor4f(1.0, 1.0, 1.0, 1.0);

  gl->glBegin(GL_QUADS);
    gl->glTexCoord2f(0, 1);
    gl->glVertex2i(-1, -1);
    gl->glTexCoord2f(0, 0);
    gl->glVertex2i(-1, 1);
    gl->glTexCoord2f(1, 0);
    gl->glVertex2i(1, 1);
    gl->glTexCoord2f(1, 1);
    gl->glVertex2i(1, -1);
  gl->glEnd();

}

static void glsl_render_cursor(graphics_data *graphics, Uint32 x, Uint32 y,
 Uint8 color, Uint8 lines, Uint8 offset)
{
  glsl_render_data *render_data = graphics->render_data;
  glsl_syms *gl = &render_data->gl;

  gl->glDisable(GL_TEXTURE_2D);
  gl->glUseProgramObjectARB(0);

  gl->glBegin(GL_QUADS);
    gl->glColor3ubv(&render_data->palette[color * 3]);
    gl->glVertex2f((x * 8)*2.0/640.0-1.0,
                (y * 14 + lines + offset)*-2.0/350.0+1.0);
    gl->glVertex2f((x * 8)*2.0/640.0-1.0,
                (y * 14 + offset)*-2.0/350.0+1.0);
    gl->glVertex2f((x * 8 + 8)*2.0/640.0-1.0,
                (y * 14 + offset)*-2.0/350.0+1.0);
    gl->glVertex2f((x * 8 + 8)*2.0/640.0-1.0,
                (y * 14 + lines + offset)*-2.0/350.0+1.0);
  gl->glEnd();

  gl->glEnable(GL_TEXTURE_2D);
}

static void glsl_render_mouse(graphics_data *graphics, Uint32 x, Uint32 y,
 Uint8 w, Uint8 h)
{
  glsl_render_data *render_data = graphics->render_data;
  glsl_syms *gl = &render_data->gl;

  gl->glDisable(GL_TEXTURE_2D);
  gl->glEnable(GL_BLEND);
  gl->glUseProgramObjectARB(0);

  gl->glBegin(GL_QUADS);
    gl->glColor4ub(255, 255, 255, 255);
    gl->glVertex2f( x*2.0/640.0-1.0,         (y + h)*-2.0/350.0+1.0);
    gl->glVertex2f( x*2.0/640.0-1.0,          y*-2.0/350.0+1.0);
    gl->glVertex2f((x + w)*2.0/640.0-1.0,     y*-2.0/350.0+1.0);
    gl->glVertex2f((x + w)*2.0/640.0-1.0,    (y + h)*-2.0/350.0+1.0);
  gl->glEnd();

  gl->glEnable(GL_TEXTURE_2D);
  gl->glDisable(GL_BLEND);
}

static void glsl_sync_screen(graphics_data *graphics)
{
  glsl_render_data *render_data = graphics->render_data;
  glsl_syms *gl = &render_data->gl;

  gl->glUseProgramObjectARB(render_data->gl_scaling);

  gl->glBindTexture(GL_TEXTURE_2D, render_data->texture_number[0]);
  gl->glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 1024, 512, 0);

  if(!graphics->fullscreen)
    gl->glViewport(0, 0, graphics->window_width, graphics->window_height);
  else
    gl->glViewport(0, 0, graphics->resolution_width,
      graphics->resolution_height);

  gl->glColor4f(1.0, 1.0, 1.0, 1.0);

  if(graphics->window_width < 640 || graphics->window_height < 350)
  {
    gl->glBegin(GL_QUADS);
      gl->glTexCoord2f(0,
        ((graphics->window_height<512)?graphics->window_height:512)/512.0);
      gl->glVertex2i(-1, 1);
      gl->glTexCoord2f(0, 0);
      gl->glVertex2i(-1, -1);
      gl->glTexCoord2f(
        ((graphics->window_width<1024)?graphics->window_width:1024)/1024.0, 0);
      gl->glVertex2i(1, -1);
      gl->glTexCoord2f(
        ((graphics->window_width<1024)?graphics->window_width:1024)/1024.0,
        ((graphics->window_height<512)?graphics->window_height:512)/512.0);
      gl->glVertex2i(1, 1);
    gl->glEnd();
  }
  else
  {
    gl->glBegin(GL_QUADS);
      gl->glTexCoord2f(0, 0.68359375);
      gl->glVertex2i(-1, 1);
      gl->glTexCoord2f(0, 0);
      gl->glVertex2i(-1, -1);
      gl->glTexCoord2f(0.625, 0);
      gl->glVertex2i(1, -1);
      gl->glTexCoord2f(0.625, 0.68359375);
      gl->glVertex2i(1, 1);
    gl->glEnd();
  }

  gl->glBindTexture(GL_TEXTURE_2D, render_data->texture_number[1]);

  SDL_GL_SwapBuffers();

  gl->glClear(GL_COLOR_BUFFER_BIT);
}

void render_glsl_register(graphics_data *graphics)
{
  graphics->init_video = glsl_init_video;
  graphics->check_video_mode = gl_check_video_mode;
  graphics->set_video_mode = glsl_set_video_mode;
  graphics->update_colors = glsl_update_colors;
  graphics->resize_screen = resize_screen_standard;
  graphics->remap_charsets = glsl_remap_charsets;
  graphics->remap_char = glsl_remap_char;
  graphics->remap_charbyte = glsl_remap_charbyte;
  graphics->get_screen_coords = get_screen_coords_scaled;
  graphics->set_screen_coords = set_screen_coords_scaled;
  graphics->render_graph = glsl_render_graph;
  graphics->render_cursor = glsl_render_cursor;
  graphics->render_mouse = glsl_render_mouse;
  graphics->sync_screen = glsl_sync_screen;
  graphics->focus_pixel = NULL;
}
