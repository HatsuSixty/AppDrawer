project('raylib', 'c', default_options : ['c_std=c99', 'warning_level=0'])

c_args = [
  '-D' + get_option('platform'),
  '-D_POSIX_C_SOURCE=199309L',
  '-D_GNU_SOURCE',
]

deps = []
if get_option('platform') == 'PLATFORM_DRM'
  deps += dependency('libdrm')
  deps += dependency('gl')
  deps += dependency('egl')
  deps += dependency('gbm')

  c_args += '-DGRAPHICS_API_OPENGL_ES2'
  c_args += '-fno-strict-aliasing'
  c_args += '-DEGL_NO_X11'
endif

raylib_lib = static_library('raylib', [
  'src/raudio.c',
  'src/rcore.c',
  'src/rglfw.c',
  'src/rmodels.c',
  'src/rshapes.c',
  'src/rtext.c',
  'src/rtextures.c',
  'src/utils.c',
], c_args : c_args, include_directories : [
  include_directories('src/external/glfw/include'),
], dependencies : deps)

raylib_inc = include_directories('src')
raylib = declare_dependency(link_with : raylib_lib, include_directories : raylib_inc)
