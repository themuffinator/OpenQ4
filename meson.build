project(
  'OpenQ4',
  ['c', 'cpp'],
  version: '0.0.1',
  meson_version: '>=1.2.0',
  default_options: ['c_std=c89', 'cpp_std=vc++latest'],
)

host_system = host_machine.system()
host_cpu_family = host_machine.cpu_family()

if host_system != 'windows'
  error(
    'Meson host bring-up is staged. '
    + 'Current actively validated host is Windows x64. '
    + 'See doc/platform-support.md for Linux/macOS roadmap.'
  )
endif

if host_cpu_family != 'x86_64'
  warning(
    'OpenQ4 currently targets x64 as the active compatibility baseline. '
    + 'Non-x64 builds are considered experimental during platform bring-up.'
  )
endif

platform_backend = get_option('platform_backend')
use_sdl3_backend = platform_backend == 'sdl3'
if not use_sdl3_backend
  warning(
    'legacy_win32 backend is transitional. '
    + 'Use -Dplatform_backend=sdl3 for portability-focused development.'
  )
endif

python_mod = import('python')
py = python_mod.find_installation('python3', required: true)

source_discovery = run_command(
  py,
  meson.project_source_root() / 'tools' / 'build' / 'meson_sources.py',
  '--host-system',
  host_system,
  '--platform-backend',
  platform_backend,
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

msvc_2026_min = '19.46.0'
if cpp.version().version_compare('<' + msvc_2026_min)
  msvc_2026_msg = 'MSVC 19.46+ (Visual Studio 2026 toolset baseline) is recommended. Detected: ' + cpp.version() + '.'
  if get_option('enforce_msvc_2026')
    error(msvc_2026_msg)
  else
    warning(msvc_2026_msg + ' Configure with -Denforce_msvc_2026=true to enforce this baseline.')
  endif
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

sdl3_dep = disabler()
if use_sdl3_backend
  sdl3_dep = dependency(
    'sdl3',
    version: '>=3.4.0',
    required: true,
    fallback: ['sdl3', 'sdl3_dep'],
    default_options: fallback_opt,
  )
endif

deps = [
  glew_dep,
  openal_dep,
  stb_vorbis_dep,
  cc.find_library('opengl32', required: true),
  cc.find_library('Dbghelp', required: true),
  cc.find_library('Iphlpapi', required: true),
  cc.find_library('Winmm', required: true),
  cc.find_library('ws2_32', required: true),
]

if use_sdl3_backend
  deps += [sdl3_dep]
else
  deps += [
    cc.find_library('dinput8', required: true),
    cc.find_library('dxguid', required: true),
  ]
endif

cpp_args = ['-DUSE_OPENAL', '-D_WINDOWS', '-DWIN32']
c_args = ['-D_WINDOWS', '-DWIN32']
link_args = ['/STACK:16777216,16777216']

if use_sdl3_backend
  cpp_args += ['-DUSE_SDL3=1']
  c_args += ['-DUSE_SDL3=1']
endif

if cpp.get_argument_syntax() == 'msvc'
  cpp_args += ['/J', '/FS', '/source-charset:windows-1252', '/execution-charset:windows-1252', '/Zc:strictStrings-']
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

dedicated_cpp_args = cpp_args + ['-DID_DEDICATED']
dedicated_c_args = c_args + ['-DID_DEDICATED']

executable(
  'OpenQ4-ded',
  openq4_sources,
  include_directories: include_directories('src'),
  dependencies: deps,
  cpp_args: dedicated_cpp_args,
  c_args: dedicated_c_args,
  cpp_pch: pch_headers,
  win_subsystem: 'windows',
  link_args: link_args,
  install: false,
)

summary(
  {
    'Host system': host_system,
    'Host cpu family': host_cpu_family,
    'MSVC toolchain': cc.get_id() == 'msvc' and cpp.get_id() == 'msvc',
    'MSVC version': cpp.version(),
    'MSVC 2026 baseline enforced': get_option('enforce_msvc_2026'),
    'C++ standard': get_option('cpp_std'),
    'Platform backend': platform_backend,
    'Resource compiler': resource_compiler.found(),
    'SDL3': use_sdl3_backend and sdl3_dep.found(),
    'GLEW': glew_dep.found(),
    'OpenAL': openal_dep.found(),
    'stb_vorbis': stb_vorbis_dep.found(),
    'Precompiled headers': get_option('use_pch'),
    'Dedicated server target': true,
  },
  section: 'Build Features',
  bool_yn: true,
)
