thread_dep = dependency('threads')

executable('v4l2', 'v4l2.cpp',
    include_directories : inc,
    link_with : v4l2_lib)

executable('v4l2_cache', 'v4l2_cache.cpp',
    include_directories : inc,
    link_with : v4l2_lib)

executable('socket', 'socket.cpp',
    include_directories : inc,
    link_with : socket_lib,
    dependencies : thread_dep)
