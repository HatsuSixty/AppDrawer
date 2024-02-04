project('AppDrawer', 'cpp',
        default_options : ['cpp_std=c++20',
                           'warning_level=2'])

incdir = include_directories('Include')

raylib_proj = subproject('raylib')
raylib = raylib_proj.get_variable('raylib')

subdir('AppDrawer')
subdir('LibDraw')
subdir('TestClient')
subdir('Example')
