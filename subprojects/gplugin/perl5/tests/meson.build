if get_option('perl5')

e = executable('test-perl5-loader', 'test-perl5-loader.c',
	c_args : [
		'-DPERL5_LOADER_DIR="@0@/.."'.format(meson.current_build_dir()),
		'-DPERL5_PLUGIN_DIR="@0@/plugins"'.format(meson.current_source_dir()),
	],
	link_with : gplugin_loader_tests,
	dependencies : [GLIB, GOBJECT, gplugin_dep, perl_dep])
test('Perl5 Loader', e)

# Add the test plugins path to our environment variable
devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_source_dir() / 'plugins')

endif  # perl5
