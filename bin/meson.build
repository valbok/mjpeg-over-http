thread_dep = dependency('threads')

executable('mjpg-over-http', 'mjpg-over-http.cpp',
    include_directories : inc,
    link_with : [v4l2_lib, socket_lib],
    dependencies : thread_dep,
    install : true)
