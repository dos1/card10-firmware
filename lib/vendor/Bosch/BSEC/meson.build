# libalgobsec.a
cc = meson.get_compiler('c')
libalgobsec = cc.find_library('libalgobsec', dirs: meson.current_source_dir() + '/algo/normal_version/bin/gcc/Cortex_M4')

includes = include_directories(
  'examples/',
  'algo/normal_version/inc/',
  'config/'
)

sources = files(
  'examples/bsec_integration.c',
#  'config/generic_18v_3s_28d/bsec_serialized_configurations_iaq.c'
)

lib = static_library(
  'bsec',
  sources,
  dependencies: [bme680, libalgobsec],
  include_directories: includes,
)

bsec = declare_dependency(
  include_directories: includes,
  link_with: lib,
  dependencies: libalgobsec,
)
