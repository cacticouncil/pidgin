###############################################################################
# Subdirectories
###############################################################################
subdir('bad-plugins')
subdir('bind-global')
subdir('dynamic-type')
subdir('id-collision')
subdir('load-on-query-fail')
subdir('load-on-query-pass')
subdir('newest-version')
subdir('plugins')
subdir('unresolved-symbol')
subdir('versioned-dependencies')

###############################################################################
# Tests
###############################################################################

# Add the normal plugins to the plugin path envvar
devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_build_dir() / 'plugins')

#######################################
# Simple Tests (single file)
#######################################
e = executable('test-core', 'test-core.c',
	c_args : [
		'-DTEST_DIR="@0@/plugins/"'.format(
			meson.current_build_dir()),
		'-DTEST_ID_DIR="@0@/id-collision/"'.format(
			meson.current_build_dir()),
	],
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Core', e)

e = executable('test-find-plugins', 'test-find-plugins.c',
	c_args : ['-DTEST_DIR="@0@/plugins/"'.format(meson.current_build_dir())],
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Find Plugins', e)

e = executable('test-id-normalize', 'test-id-normalize.c',
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('ID Normalization', e)

e = executable('test-loader', 'test-loader.c',
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Loader', e)

e = executable('test-loader-registration', 'test-loader-registration.c',
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Loader Registration', e)

e = executable('test-newest-version', 'test-newest-version.c',
	c_args : ['-DTEST_DIR="@0@/newest-version/"'.format(meson.current_build_dir())],
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Find Plugins Newest Version', e)

e = executable('test-option-group', 'test-option-group.c',
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Option Group', e)

e = executable('test-plugin-manager-paths', 'test-plugin-manager-paths.c',
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Plugin Manager Paths', e)

e = executable('test-plugin-info', 'test-plugin-info.c',
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Plugin Info', e)

e = executable('test-signals', 'test-signals.c',
	c_args : ['-DTEST_DIR="@0@/plugins/"'.format(meson.current_build_dir())],
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Signals', e)

e = executable('test-version-compare', 'test-version-compare.c',
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Version Compare', e)

#######################################
# Dynamic Type
#######################################
e = executable('test-dynamic-type', 'test-dynamic-type.c',
	c_args : [
		'-DTEST_DYNAMIC_DIR="@0@/dynamic-type/"'.format(
			meson.current_build_dir())
	],
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Dynamic Type', e)

#######################################
# ID Collision
#######################################
e = executable('test-id-collision', 'test-id-collision.c',
	c_args : [
		'-DTEST_ID_DIR="@0@/id-collision/"'.format(
			meson.current_build_dir()),
	],
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('ID Collision', e)

#######################################
# Load On Query
#######################################
e = executable('test-load-on-query', 'test-load-on-query.c',
	c_args : [
		'-DTEST_DIR="@0@/plugins/"'.format(
			meson.current_build_dir()),
		'-DTEST_LOAD_ON_QUERY_PASS_DIR="@0@/load-on-query-pass"'.format(
			meson.current_build_dir()),
		'-DTEST_LOAD_ON_QUERY_FAIL_DIR="@0@/load-on-query-fail"'.format(
			meson.current_build_dir())
	],
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Load On Query', e)

#######################################
# Versioned Dependencies
#######################################
e = executable('test-versioned-dependencies', 'test-versioned-dependencies.c',
	c_args : [
		'-DTEST_VERSIONED_DEPENDENCY_DIR="@0@/versioned-dependencies"'.format(
			meson.current_build_dir()),
	],
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Versioned Dependencies', e)

#######################################
# Native Loader
#######################################
e = executable('test-native-loader', 'test-native-loader.c',
	c_args : [
		'-DTEST_DIR="@0@/plugins/"'.format(
			meson.current_build_dir()),
		'-DTEST_BAD_DIR="@0@/bad-plugins/"'.format(
			meson.current_build_dir()),
		'-DTEST_ID_DIR="@0@/id-collision/"'.format(
			meson.current_build_dir()),
	],
	link_with : gplugin_loader_tests,
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Native Loader', e)

###############################################################################
# Bind Local
###############################################################################
e = executable('test-bind-global', 'test-bind-global.c',
	c_args : [
		'-DTEST_BIND_GLOBAL_DIR="@0@/bind-global/"'.format(
			meson.current_build_dir()),
		'-DTEST_BAD_DIR="@0@/bad-plugins/"'.format(
			meson.current_build_dir()),
		'-DTEST_ID_DIR="@0@/id-collision/"'.format(
			meson.current_build_dir()),
	],
	dependencies : [gplugin_dep, GLIB, GOBJECT])
test('Bind Global', e)

###############################################################################
# Unresolved Symbol
###############################################################################
e = executable('test-unresolved-symbol', 'test-unresolved-symbol.c',
	c_args : [
		'-DPLUGIN_DIR="@0@/unresolved-symbol/"'.format(
			meson.current_build_dir()),
	],
	dependencies : [gplugin_dep, GLIB, GOBJECT])
# test('Unresolved Symbol', e)
