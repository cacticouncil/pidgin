if get_option('vapi')
	if not get_option('introspection')
		error('Vala generation requires GObject Introspection.')
	endif

	add_languages('vala', native: false)

	gplugin_vapi = gnome.generate_vapi('gplugin',
		sources : gplugin_gir[0],
		install : true,
	)

	if get_option('gtk3')
		gplugin_gtk_vapi = gnome.generate_vapi('gplugin-gtk3',
			sources : gplugin_gtk3_gir[0],
			packages : [ 'gtk+-3.0' ],
			install : true,
			gir_dirs : meson.current_build_dir() / '..' / 'gplugin',
		)
	endif

	subdir('tests')
endif # vala
