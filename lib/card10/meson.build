includes = include_directories(
  './',
)

sources = files(
  'bosch.c',
  'card10.c',
  'leds.c',
  'pmic.c',
  'portexpander.c',
  'pb.c',
  'display.c'
)

deps = [
    bhy1,
    bma400,
    bme680,
    board_card10,
    libgfx,
    max77650,
    max86150,
    periphdriver,
]

lib = static_library(
  'card10',
  sources,
  include_directories: includes,
  dependencies: deps,
)

libcard10 = declare_dependency(
  include_directories: includes,
  link_with: lib,
  dependencies: deps,
)

