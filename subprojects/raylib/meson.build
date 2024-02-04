project('raylib', 'c', default_options : ['c_std=c11', 'warning_level=0'])

raylib_lib = static_library('raylib', [
  'src/raudio.c',
  'src/rcore.c',
  'src/rglfw.c',
  'src/rmodels.c',
  'src/rshapes.c',
  'src/rtext.c',
  'src/rtextures.c',
  'src/utils.c',
], c_args : [
  '-D' + get_option('platform'),
  '-D_POSIX_C_SOURCE=199309L',
  '-D_GNU_SOURCE',
], include_directories : [
  include_directories('src/external/glfw/include'),
])

raylib_inc = include_directories('src')
raylib = declare_dependency(link_with : raylib_lib, include_directories : raylib_inc)
