thread_dep = dependency('threads')
socket_lib = shared_library('Capture_socket', ['socket.cpp', 'socket_thread.cpp'], include_directories : inc, install : true, dependencies : thread_dep)

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : socket_lib,
                 version : '1.0',
                 name : 'Capture_socket',
                 filebase : 'Capture_socket',
                 description : 'Capture socket.')
