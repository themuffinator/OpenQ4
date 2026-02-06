project(
  'OpenQ4',
  ['c', 'cpp'],
  version: '0.0.1',
  meson_version: '>=1.2.0',
  default_options: ['c_std=c89'],
)

if host_machine.system() != 'windows'
  error('The Meson build is currently configured for Windows targets only.')
endif

python_mod = import('python')
py = python_mod.find_installation('python3', required: true)

source_discovery = run_command(
  py,
  meson.project_source_root() / 'tools' / 'build' / 'meson_sources.py',
  check: true,
)

source_paths = source_discovery.stdout().strip().split('\n')
if source_paths.length() == 0
  error('tools/build/meson_sources.py returned no source files.')
endif

openq4_sources = files(source_paths)

resource_compiler = find_program('rc', 'windres', required: false)
if resource_compiler.found()
  windows_mod = import('windows')
  openq4_sources += windows_mod.compile_resources('src/sys/win32/rc/OpenQ4Version.rc')
endif

cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')

if cc.get_id() != 'msvc' or cpp.get_id() != 'msvc'
  error(
    'The Meson build currently requires Microsoft cl.exe. '
    + 'Use a VS Developer shell or tools/build/meson_setup.ps1.'
  )
endif

fallback_opt = ['default_library=static']

glew_dep = dependency(
  'glew',
  required: true,
  fallback: ['glew', 'glew_dep'],
  default_options: fallback_opt,
)

stb_vorbis_dep = dependency(
  'stb_vorbis',
  required: true,
  fallback: ['stb_vorbis', 'stb_vorbis_dep'],
  default_options: fallback_opt,
)

openal_dep = dependency(
  'openal',
  required: true,
  fallback: ['openal-soft-prebuilt', 'openal_dep'],
  default_options: fallback_opt,
)

deps = [
  glew_dep,
  openal_dep,
  stb_vorbis_dep,
  cc.find_library('opengl32', required: true),
  cc.find_library('Dbghelp', required: true),
  cc.find_library('Iphlpapi', required: true),
  cc.find_library('dinput8', required: true),
  cc.find_library('Winmm', required: true),
  cc.find_library('dxguid', required: true),
  cc.find_library('ws2_32', required: true),
]

cpp_args = ['-DUSE_OPENAL', '-D_WINDOWS', '-DWIN32']
c_args = ['-D_WINDOWS', '-DWIN32']
link_args = ['/STACK:16777216,16777216']

if cpp.get_argument_syntax() == 'msvc'
  cpp_args += ['/J', '/FS', '/source-charset:windows-1252', '/execution-charset:windows-1252']
  c_args += ['/FS', '/source-charset:windows-1252', '/execution-charset:windows-1252']
endif

pch_headers = []
if get_option('use_pch')
  pch_headers = ['src/idlib/precompiled.h']
endif

executable(
  'OpenQ4',
  openq4_sources,
  include_directories: include_directories('src'),
  dependencies: deps,
  cpp_args: cpp_args,
  c_args: c_args,
  cpp_pch: pch_headers,
  win_subsystem: 'windows',
  link_args: link_args,
  install: false,
)

summary(
  {
    'MSVC toolchain': cc.get_id() == 'msvc' and cpp.get_id() == 'msvc',
    'Resource compiler': resource_compiler.found(),
    'GLEW': glew_dep.found(),
    'OpenAL': openal_dep.found(),
    'stb_vorbis': stb_vorbis_dep.found(),
    'Precompiled headers': get_option('use_pch'),
  },
  section: 'Build Features',
  bool_yn: true,
)
