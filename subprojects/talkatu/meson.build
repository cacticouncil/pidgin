###############################################################################
# Project Info
###############################################################################
project('talkatu', 'c', version : '0.1.0-dev',
  license : 'GPL-2.0-or-later',
  meson_version : '>=0.56.0',
  default_options : ['c_std=c99'])

parts = meson.project_version().split('-')
if parts.length() > 1
  extra = parts[1]
else
  extra = ''
endif

parts = parts[0].split('.')
TALKATU_MAJOR_VERSION = parts[0]

version_conf = configuration_data()
version_conf.set('TALKATU_MAJOR_VERSION', TALKATU_MAJOR_VERSION)
version_conf.set('TALKATU_MINOR_VERSION', parts[1])
version_conf.set('TALKATU_MICRO_VERSION', parts[2])
version_conf.set('TALKATU_EXTRA_VERSION', extra)
version_conf.set('TALKATU_VERSION', meson.project_version())

LOCALE_DIR = get_option('prefix') / get_option('localedir')
add_project_arguments('-DLOCALEDIR="@0@"'.format(LOCALE_DIR), language : 'c')

###############################################################################
# Dependencies
###############################################################################
gnome = import('gnome')
pkgconfig = import('pkgconfig')

GLIB = dependency('glib-2.0', version : '>=2.52.0')
GOBJECT = dependency('gobject-2.0')

GTK3 = dependency('gtk+-3.0', version : '>=3.24.0')
GSPELL = dependency('gspell-1', version : '>=1.2')

GUMBO = dependency('gumbo', version : '>=0.10')

CMARK = dependency('libcmark', version : '>=0.28.0')

if get_option('help2man')
	HELP2MAN = find_program('help2man')
endif

###############################################################################
# NLS
###############################################################################
GETTEXT_PACKAGE = 'talkatu'

add_project_arguments('-DGETTEXT_PACKAGE="@0@"'.format(GETTEXT_PACKAGE),
	language : 'c')

if get_option('nls')
	i18n = import('i18n')
endif

###############################################################################
# Build Info
###############################################################################
compiler = meson.get_compiler('c')

top_srcdir = include_directories('.')

add_project_arguments(
	'-DPREFIX="@0@"'.format(get_option('prefix')),
	'-DLIBDIR="@0@"'.format(get_option('libdir')),
	language : 'c'
)

if compiler.has_argument('-Wformat')
	add_project_arguments('-Wformat', language : 'c')
	if compiler.has_multi_arguments(['-Wformat', '-Werror=format-security'])
		add_project_arguments('-Werror=format-security', language : 'c')
	endif
endif

# check if we're using gcc
if compiler.get_id() == 'gcc' or host_machine.system() == 'darwin'
	add_project_arguments(
		'-DTALKATU_UNUSED=__attribute__((unused))',
		'-ggdb',
		language : 'c'
	)
else
	add_project_arguments(
		'-DTALKATU_UNUSED=',
		language : 'c'
	)
endif

toplevel_inc = include_directories('.')

###############################################################################
# Subdirectories
###############################################################################
subdir('talkatu')
subdir('po')
subdir('demo')
subdir('packaging')
subdir('vapi')

###############################################################################
# Install stuff
###############################################################################
# documentation
install_data('ChangeLog', 'INSTALL', 'README.md', 'HACKING', 'logo.png',
	install_dir : get_option('datadir') / 'doc/talkatu',
)
