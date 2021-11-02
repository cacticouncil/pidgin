if get_option('vapi')
	if not get_option('introspection')
		error('Vala generation requires GObject Introspection.')
	endif

	add_languages('vala', native : false)

	talkatu_vapi = gnome.generate_vapi('talkatu',
		sources : talkatu_gir[0],
		packages : [ 'gtk+-3.0' ],
		install : true,
		gir_dirs : meson.current_build_dir() / '..' / 'talkatu',
	)
endif # vala
