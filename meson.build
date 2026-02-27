project('GhostLayer', 'c',
    license: 'GPL-3.0-or-later',
    version : run_command('git', '-C', meson.project_source_root(), 'describe', '--tags', '--always', check: false).stdout().strip(),
)
gtkdep = dependency('gtk+-3.0', required: true)
sparkclidep = dependency('sparkcli', required: true)
subdir('src')