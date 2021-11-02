summary('tcc', get_option('tcc'), section : 'Loader Support', bool_yn : true)

if get_option('tcc')
	GPLUGIN_TCC_SOURCES = [
		'gplugin-tcc-core.c',
		'gplugin-tcc-loader.c',
		'gplugin-tcc-plugin.c',
	]

	GPLUGIN_TCC_HEADERS = [
		'gplugin-tcc-loader.h',
		'gplugin-tcc-plugin.h',
	]

	if not compiler.has_header('libtcc.h')
		error('libtcc.h not found')
	endif

	TCC = compiler.find_library('tcc')

	gplugin_tcc = shared_library('gplugin-tcc',
		GPLUGIN_TCC_SOURCES,
		GPLUGIN_TCC_HEADERS,
		name_prefix : '',
		dependencies : [TCC, gplugin_dep],
		install : true,
		install_dir : get_option('libdir') / 'gplugin'
	)

	# Add the loader's path to our environment variable
	devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_build_dir())
endif # tcc

subdir('tests')
