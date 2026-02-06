project(
  'glew',
  'c',
  default_options: ['default_library=static'],
)

glew_inc = include_directories('include')

glew_lib = static_library(
  'glew',
  files('src/glew.c'),
  include_directories: glew_inc,
)

glew_dep = declare_dependency(
  include_directories: glew_inc,
  link_with: glew_lib,
)
