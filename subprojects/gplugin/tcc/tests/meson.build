if get_option('tcc')

e = executable('test-tcc-loader', 'test-tcc-loader.c',
	c_args : [
		'-DTCC_LOADER_DIR="@0@/.."'.format(meson.current_build_dir()),
		'-DTCC_PLUGIN_DIR="@0@/plugins"'.format(
			meson.current_source_dir()),
	],
	link_with : gplugin_loader_tests,
	dependencies : [GLIB, GOBJECT, TCC, gplugin_dep])
test('TCC Loader', e)

endif  # tcc
