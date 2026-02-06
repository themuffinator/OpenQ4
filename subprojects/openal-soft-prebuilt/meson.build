project('openal-soft-prebuilt', 'c')

if host_machine.system() != 'windows'
  openal_dep = dependency('openal', required: true)
  subdir_done()
endif

if host_machine.cpu_family() != 'x86_64'
  error('Bundled OpenAL import library is only provided for x86_64 Windows.')
endif

cc = meson.get_compiler('c')
openal_include = include_directories('include')
openal_lib_dir = meson.current_source_dir() / 'lib' / 'win64'

openal_lib = cc.find_library(
  'OpenAL32',
  dirs: [openal_lib_dir],
  required: true,
)

openal_dep = declare_dependency(
  include_directories: openal_include,
  dependencies: [openal_lib],
)
