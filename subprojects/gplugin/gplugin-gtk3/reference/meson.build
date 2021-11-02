if get_option('doc')
	gplugin_gtk3_toml = configure_file(
		input : 'gplugin-gtk3.toml.in',
		output : 'gplugin-gtk3.toml',
		configuration : version_conf,
		install : true,
		install_dir : docs_dir / 'gplugin-gtk3',
	)

	gplugin_gtk3_doc = custom_target('gplugin-gtk3-doc',
		input : [ gplugin_gtk3_toml, gplugin_gtk3_gir[0] ],
		output : 'gplugin-gtk3',
		command : [
			gidocgen,
			'generate',
			'--quiet',
			'--fatal-warnings',
			'--add-include-path=@0@'.format(meson.project_build_root() / 'gplugin'),
			'--config=@INPUT0@',
			'--output-dir=@OUTPUT@',
			'--no-namespace-dir',
			'--content-dir=@0@'.format(meson.current_source_dir()),
			'@INPUT1@'
		],
		depends : [ gplugin_gir[0] ],
		build_by_default : true,
		install : true,
		install_dir : docs_dir,
	)
endif

