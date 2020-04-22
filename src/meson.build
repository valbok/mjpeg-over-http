thread_dep = dependency('threads')
v4l2_lib = shared_library('Capture_v4l2', ['v4l2.cpp'], include_directories : inc, install : true)
socket_lib = shared_library('Capture_socket', ['socket.cpp', 'socket_thread.cpp'], include_directories : inc, install : true, dependencies : thread_dep)

