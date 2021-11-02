if get_option('rs')

e = executable('test-rust-loading', 'test-rust-loading.c',
	include_directories : include_directories('.'),
	c_args: [
		'-DRUST_PLUGIN_DIR="@0@/plugins"'.format(meson.current_build_dir()),
	],
	link_with : gplugin_loader_tests,
	dependencies : [GLIB, GOBJECT, gplugin_dep])
test('Rust loading', e)

subdir('plugins')

endif # rs

