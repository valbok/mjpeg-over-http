thread_dep = dependency('threads')
cc = meson.get_compiler('cpp')
jpeg_dep = cc.find_library('jpeg', required : true)

v4l2_lib = shared_library('Capture_v4l2', ['v4l2.cpp','jpeg_utils.cpp'], include_directories : inc, install : true, dependencies : jpeg_dep)

