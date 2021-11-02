if get_option('vapi')
	shared_library('vala-basic-plugin', 'basic.vala',
		name_prefix : '',
		dependencies : [gplugin_dep, gplugin_vapi])
	shared_library('vala-dependent-plugin', 'dependent.vala',
		name_prefix : '',
		dependencies : [gplugin_dep, gplugin_vapi])
	shared_library('vala-load-exception', 'load-exception.vala',
		name_prefix : '',
		dependencies : [gplugin_dep, gplugin_vapi])
	shared_library('vala-load-failed', 'load-failed.vala',
		name_prefix : '',
		dependencies : [gplugin_dep, gplugin_vapi])
	shared_library('vala-unload-failed', 'unload-failed.vala',
		name_prefix : '',
		dependencies : [gplugin_dep, gplugin_vapi])
	shared_library('vala-unload-shutdown', 'unload-shutdown.vala',
		name_prefix : '',
		dependencies : [gplugin_dep, gplugin_vapi])

	# Add the test plugins path to our environment variable
	devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_build_dir())
endif # vapi
