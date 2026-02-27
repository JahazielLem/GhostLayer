inc_dirs = include_directories('.')

sources = files(
    'main.c',
)

subdir('backend')
subdir('ui')
subdir('core')
subdir('dissectors')

executable(
    'ghostlayer',
    sources,
    dependencies: [gtkdep, sparkclidep],
    install: true
)