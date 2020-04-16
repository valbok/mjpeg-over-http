project('Capture/it', 'cpp', version : '0.0.0', license : 'MIT')

inc = include_directories('include')

subdir('include')
subdir('src')
subdir('tests')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : v4l2_lib,
                 version : '1.0',
                 name : 'Capture_v4l2',
                 filebase : 'Capture_v4l2',
                 description : 'Capture v4l2.')

pkg_mod.generate(libraries : socket_lib,
                 version : '1.0',
                 name : 'Capture_socket',
                 filebase : 'Capture_socket',
                 description : 'Capture socket.')

