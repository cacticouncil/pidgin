if get_option('gtk3')

if not get_option('install-gplugin-gtk3-viewer')
summary('gplugin-gtk3-viewer',
        'You have disabled installation of gplugin-gtk-viewer which is a ' +
        'very useful diagnostics tool.',
        section : 'Warnings')
endif

gplugin_gtk3_viewer_resources = gnome.compile_resources(
	'gplugin-gtk-viewer-resources',
	'data/gplugin-gtk-viewer.gresource.xml',
	c_name: 'gplugin_gtk_viewer',
	source_dir: 'data')

gplugin_gtk3_viewer = executable('gplugin-gtk3-viewer',
	'gplugin-gtk-viewer.c',
	'gplugin-gtk-viewer-window.c',
	'gplugin-gtk-viewer-window.h',
	dependencies : [gplugin_dep, gplugin_gtk3_dep],
	install : get_option('install-gplugin-gtk3-viewer')
)

if get_option('help2man')
	gplugin_gtk_viewer_man = custom_target('gplugin-gtk3-viewer.1',
		command : [help2man,
		           '--name=View installed plugins', '--section=1',
		           '--help-option=--help-all', '--no-info',
		           '--output', '@OUTPUT@',
		           gplugin_gtk3_viewer],
		output : 'gplugin-gtk3-viewer.1',
		install : get_option('install-gplugin-gtk3-viewer'),
		install_dir : get_option('mandir') / 'man1')
endif

endif
