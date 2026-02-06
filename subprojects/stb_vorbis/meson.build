project(
  'stb_vorbis',
  'c',
  default_options: ['default_library=static'],
)

stb_vorbis_inc = include_directories('.')

stb_vorbis_lib = static_library(
  'stb_vorbis',
  files('stb_vorbis.c'),
  include_directories: stb_vorbis_inc,
)

stb_vorbis_dep = declare_dependency(
  include_directories: stb_vorbis_inc,
  link_with: stb_vorbis_lib,
)
