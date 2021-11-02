if get_option('vapi')
	shared_library('genie-basic-plugin', 'basic.gs',
		name_prefix : '',
		dependencies : [gplugin_dep, gplugin_vapi])
	shared_library('genie-dependent-plugin', 'dependent.gs',
		name_prefix : '',
		dependencies : [gplugin_dep, gplugin_vapi])
	shared_library('load-exception-plugin', 'load-exception.gs',
		name_prefix : '',
		dependencies : [gplugin_dep, gplugin_vapi])
	shared_library('load-failed-plugin', 'load-failed.gs',
		name_prefix : '',
		dependencies : [gplugin_dep, gplugin_vapi])
	shared_library('unload-shutdown-plugin', 'unload-shutdown.gs',
		name_prefix : '',
		dependencies : [gplugin_dep, gplugin_vapi])
	shared_library('unload-failed-plugin', 'unload-failed.gs',
		name_prefix : '',
		dependencies : [gplugin_dep, gplugin_vapi])

	# Add the test plugins path to our environment variable
	devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_build_dir())
endif # vapi
