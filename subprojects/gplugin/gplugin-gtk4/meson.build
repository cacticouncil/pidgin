if get_option('gtk4')

###############################################################################
# Build Info
###############################################################################
GTK4 = dependency('gtk4', version : '>=4.0.0')

GPLUGIN_GTK4_LIBRARY_VERSION = '0.1.0'

GPLUGIN_GTK4_SOURCES = [
]

GPLUGIN_GTK4_HEADERS = [
]

GPLUGIN_GTK4_PRIVATE_SOURCES = [
]

GPLUGIN_GTK4_PRIVATE_HEADERS = [
]

GPLUGIN_GTK4_PUBLIC_BUILT_SOURCES = [
]

GPLUGIN_GTK4_PUBLIC_BUILT_HEADERS = [
]

GPLUGIN_GTK4_GENERATED_TARGETS = [
]

###############################################################################
# Configure Files
###############################################################################
GPLUGIN_GTK4_H_INCLUDES = ''
foreach header : GPLUGIN_GTK4_HEADERS
	GPLUGIN_GTK4_H_INCLUDES = '@0@\n#include <@1@>'.format(
		GPLUGIN_GTK4_H_INCLUDES,
		header)
endforeach

conf = configuration_data()
conf.set('GPLUGIN_GTK_H_INCLUDES', GPLUGIN_GTK4_H_INCLUDES)
gplugin_gtk4_h = configure_file(
	input : 'gplugin-gtk.h.in',
	output : 'gplugin-gtk.h',
	configuration : conf,
	install : true,
	install_dir : get_option('includedir') / 'gplugin-gtk4-1.0')

###############################################################################
# Resource Target
###############################################################################
gplugin_gtk4_resources = gnome.compile_resources(
	'gplugin-gtk4-resources',
	'data/gplugin-gtk.gresource.xml',
	c_name: 'gplugin_gtk',
	source_dir: 'data')

###############################################################################
# Library Target
###############################################################################
gplugin_gtk4 = shared_library('gplugin-gtk4',
	GPLUGIN_GTK4_SOURCES,
	GPLUGIN_GTK4_HEADERS,
	GPLUGIN_GTK4_PRIVATE_SOURCES,
	GPLUGIN_GTK4_PRIVATE_HEADERS,
	GPLUGIN_GTK4_PUBLIC_BUILT_SOURCES,
	GPLUGIN_GTK4_PUBLIC_BUILT_HEADERS,
	gplugin_gtk4_resources,
	c_args : ['-DGPLUGIN_GTK_COMPILATION', '-DG_LOG_DOMAIN="GPluginGtk"'],
	include_directories : toplevel_inc,
	dependencies : [gplugin_dep, GTK4],
	version : GPLUGIN_GTK4_LIBRARY_VERSION,
	install : true
)

gplugin_gtk4_inc = include_directories('.')

pkgconfig.generate(
	gplugin_gtk4,
	name : 'gplugin-gtk4',
	description : 'GTK4 widgets for GPlugin',
	filebase : 'gplugin-gtk4',
	subdirs : 'gplugin-gtk4-1.0',
	libraries : [gplugin],
	libraries : [GLIB, GOBJECT, GMODULE, GTK4],
)

###############################################################################
# GObject Introspection
###############################################################################
if get_option('introspection')
	gplugin_gtk4_gir = gnome.generate_gir(
		gplugin_gtk4,
		sources : [
			GPLUGIN_GTK4_SOURCES,
			GPLUGIN_GTK4_HEADERS,
			GPLUGIN_GTK4_PUBLIC_BUILT_SOURCES,
			GPLUGIN_GTK4_PUBLIC_BUILT_HEADERS,
		],
		includes : [gplugin_gir[0], 'Gtk-4.0'],
		namespace : 'GPluginGtk4',
		nsversion : '1.0',
		symbol_prefix : 'gplugin_gtk',
		identifier_prefix : 'GPluginGtk',
		install : true,
		header : 'gplugin-gtk.h',
		export_packages: ['gplugin-gtk4'],
		dependencies : [gplugin_dep],
		extra_args : ['--quiet', '-DGPLUGIN_GTK_COMPILATION'])
	GPLUGIN_GTK4_GENERATED_TARGETS += gplugin_gtk4_gir
endif

###############################################################################
# Library Dependency Object
###############################################################################
gplugin_gtk4_dep = declare_dependency(
	include_directories: [toplevel_inc, gplugin_gtk4_inc],
	link_with : [gplugin, gplugin_gtk4],
	sources : [
		GPLUGIN_GTK4_PUBLIC_BUILT_HEADERS,
		GPLUGIN_GTK4_GENERATED_TARGETS,
	],
	dependencies : [GLIB, GOBJECT, GTK4],
)

meson.override_dependency('gplugin-gtk4', gplugin_gtk4_dep)

###############################################################################
# Install Stuff
###############################################################################
# install the normal includes into the gplugin-gtk subdirectory
install_headers(
	GPLUGIN_GTK4_HEADERS,
	subdir : 'gplugin-gtk4-1.0/gplugin-gtk'
)

###############################################################################
# subdirectories
###############################################################################
subdir('reference')

endif  # gtk4
