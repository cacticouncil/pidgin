if get_option('gtk3')

###############################################################################
# Build Info
###############################################################################
GTK3 = dependency('gtk+-3.0', version : '>=3.0.0')

GPLUGIN_GTK3_LIBRARY_VERSION = '0.1.0'

GPLUGIN_GTK3_SOURCES = [
	'gplugin-gtk-plugin-info.c',
	'gplugin-gtk-store.c',
	'gplugin-gtk-view.c',
]

GPLUGIN_GTK3_HEADERS = [
	'gplugin-gtk-plugin-info.h',
	'gplugin-gtk-store.h',
	'gplugin-gtk-view.h',
]

GPLUGIN_GTK3_PRIVATE_SOURCES = [
]

GPLUGIN_GTK3_PRIVATE_HEADERS = [
]

GPLUGIN_GTK3_PUBLIC_BUILT_SOURCES = [
]

GPLUGIN_GTK3_PUBLIC_BUILT_HEADERS = [
]

GPLUGIN_GTK3_GENERATED_TARGETS = [
]

###############################################################################
# Configure Files
###############################################################################
GPLUGIN_GTK3_H_INCLUDES = ''
foreach header : GPLUGIN_GTK3_HEADERS
	GPLUGIN_GTK3_H_INCLUDES = '@0@\n#include <@1@>'.format(
		GPLUGIN_GTK3_H_INCLUDES,
		header)
endforeach

conf = configuration_data()
conf.set('GPLUGIN_GTK_H_INCLUDES', GPLUGIN_GTK3_H_INCLUDES)
gplugin_gtk3_h = configure_file(
	input : 'gplugin-gtk.h.in',
	output : 'gplugin-gtk.h',
	configuration : conf,
	install : true,
	install_dir : get_option('includedir') / 'gplugin-gtk3-1.0')

gplugin_gtk3_xml = configure_file(
	input : 'gplugin-gtk3.xml.in',
	output: 'gplugin-gtk3.xml',
	configuration : version_conf,
	install: true,
	install_dir : get_option('datadir') / 'glade' / 'catalogs',
)

###############################################################################
# Resource Target
###############################################################################
gplugin_gtk3_resources = gnome.compile_resources(
	'gplugin-gtk3-resources',
	'data/gplugin-gtk.gresource.xml',
	c_name: 'gplugin_gtk',
	source_dir: 'data')

###############################################################################
# Library Target
###############################################################################
gplugin_gtk3 = shared_library('gplugin-gtk3',
	GPLUGIN_GTK3_SOURCES,
	GPLUGIN_GTK3_HEADERS,
	GPLUGIN_GTK3_PRIVATE_SOURCES,
	GPLUGIN_GTK3_PRIVATE_HEADERS,
	GPLUGIN_GTK3_PUBLIC_BUILT_SOURCES,
	GPLUGIN_GTK3_PUBLIC_BUILT_HEADERS,
	gplugin_gtk3_resources,
	c_args : ['-DGPLUGIN_GTK_COMPILATION', '-DG_LOG_DOMAIN="GPluginGtk"'],
	include_directories : toplevel_inc,
	dependencies : [gplugin_dep, GTK3],
	version : GPLUGIN_GTK3_LIBRARY_VERSION,
	install : true
)

gplugin_gtk3_inc = include_directories('.')

pkgconfig.generate(
	gplugin_gtk3,
	name : 'gplugin-gtk3',
	description : 'Gtk3 widgets for GPlugin',
	filebase : 'gplugin-gtk3',
	subdirs : 'gplugin-gtk3-1.0',
	libraries : [gplugin],
	requires : [GLIB, GOBJECT, GMODULE, GTK3],
)

###############################################################################
# GObject Introspection
###############################################################################
if get_option('introspection')
	gplugin_gtk3_gir = gnome.generate_gir(
		gplugin_gtk3,
		sources : GPLUGIN_GTK3_SOURCES + GPLUGIN_GTK3_HEADERS +
		          GPLUGIN_GTK3_PUBLIC_BUILT_SOURCES +
		          GPLUGIN_GTK3_PUBLIC_BUILT_HEADERS,
		includes : [gplugin_gir[0], 'Gtk-3.0'],
		namespace : 'GPluginGtk3',
		nsversion : '1.0',
		symbol_prefix : 'gplugin_gtk',
		identifier_prefix : 'GPluginGtk',
		install : true,
		header : 'gplugin-gtk.h',
		export_packages: ['gplugin-gtk3'],
		dependencies : [gplugin_dep],
		extra_args : ['--quiet', '-DGPLUGIN_GTK_COMPILATION'])
	GPLUGIN_GTK3_GENERATED_TARGETS += gplugin_gtk3_gir
endif

###############################################################################
# Library Dependency Object
###############################################################################
gplugin_gtk3_dep = declare_dependency(
	include_directories: [toplevel_inc, gplugin_gtk3_inc],
	link_with : [gplugin, gplugin_gtk3],
	sources : GPLUGIN_GTK3_PUBLIC_BUILT_HEADERS + GPLUGIN_GTK3_GENERATED_TARGETS,
	dependencies : [GLIB, GOBJECT, GTK3],
)

meson.override_dependency('gplugin-gtk3', gplugin_gtk3_dep)

###############################################################################
# Install Stuff
###############################################################################
# install the normal includes into the gplugin-gtk subdirectory
install_headers(
	GPLUGIN_GTK3_HEADERS,
	subdir : 'gplugin-gtk3-1.0/gplugin-gtk'
)

###############################################################################
# subdirectories
###############################################################################
subdir('reference')

endif  # gtk3
