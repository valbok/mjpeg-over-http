thread_dep = dependency('threads')
cc = meson.get_compiler('cpp')
jpeg_dep = cc.find_library('jpeg', required : true)

v4l2_lib = shared_library('Capture_v4l2', ['v4l2.cpp','jpeg_utils.cpp'], include_directories : inc, install : true, dependencies : jpeg_dep)

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : v4l2_lib,
                 version : '1.0',
                 name : 'Capture_v4l2',
                 filebase : 'Capture_v4l2',
                 description : 'Capture v4l2.')
