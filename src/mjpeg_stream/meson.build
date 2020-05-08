mjpeg_stream_lib = shared_library('Capture_mjpeg_stream', ['mjpeg_stream.cpp'], include_directories : inc, install : true)

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : mjpeg_stream_lib,
                 version : '1.0',
                 name : 'Capture_mjpeg_stream',
                 filebase : 'Capture_mjpeg_stream',
                 description : 'Capture mjpeg stream.')
