v4l2_lib = shared_library('Capture_v4l2', ['v4l2.cpp', 'v4l2_cache.cpp'], include_directories : inc, install : true)
thread_dep = dependency('threads')
socket_lib = shared_library('Capture_socket', ['socket.cpp', 'socket_thread.cpp'], include_directories : inc, install : true, dependencies : thread_dep)

