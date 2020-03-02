project('Capture v4l2', 'cpp',
        version : '0.0.0',
        license : 'MIT')

inc = include_directories('include')

subdir('include')
subdir('src')
subdir('tests')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : lib,
                 version : '1.0',
                 name : 'Capture_v4l2',
                 filebase : 'Capture_v4l2',
                 description : 'Capture v4l2.')

