summary('perl5', get_option('perl5'), section : 'Loader Support', bool_yn : true)

if get_option('perl5')
	if not get_option('introspection')
		error('Perl plugin requires GObject Introspection.')
	endif

	GPLUGIN_PERL_SOURCES = [
		'gplugin-perl5-core.c',
		'gplugin-perl5-loader.c',
		'gplugin-perl5-plugin.c',
	]

	GPLUGIN_PERL_HEADERS = [
		'gplugin-perl5-loader.h',
		'gplugin-perl5-plugin.h',
	]

	# make sure we have the perl executable, we need it to figure out the
	# build arguments.
	PERL = find_program('perl')

	# make sure we have the perl library as well.
	LIBPERL = []
	if host_machine.system() != 'freebsd'
		INC = run_command(PERL, '-e print join("/CORE ",grep(/^\\//,@INC))."/CORE"')
		LIBPERL = compiler.find_library('libperl', dirs: INC.stdout().strip().split(' '))
	endif

	# make sure we have the gobject introspection perl module.
	run_command(
		PERL,
		'-e use Glib::Object::Introspection;',
		check : true,
	)

	perl_dep = declare_dependency(
		compile_args : run_command(PERL, '-MExtUtils::Embed', '-e', 'ccopts', check : true).stdout().split(),
		link_args : run_command(PERL, '-MExtUtils::Embed', '-e', 'ldopts', check : true).stdout().split(),
	)

	if host_machine.system() in ['freebsd', 'openbsd']
		perl_arch = 'sitearchexp'
	else
		perl_arch = 'vendorarchexp'
	endif

	perl_arch = run_command(PERL,
		'-MConfig',
		'-e', 'if ($Config{@0@}) { print "$Config{@0@}" } else { exit 1 }'.format(perl_arch),
		check : true).stdout().split()
	glib_perl_incdir = '-I@0@/Glib/Install'.format(perl_arch[0])
	glib_perl_libdir = '@0@/auto/Glib/'.format(perl_arch[0])
	glib_perl_dep = compiler.find_library(
		'Glib',
		dirs : glib_perl_libdir,
		has_headers : 'gperl.h',
		header_args : glib_perl_incdir
	)
	glib_perl_dep = declare_dependency(
		dependencies : glib_perl_dep,
		# Annoyingly, this is not transferred over from the header_args.
		compile_args : glib_perl_incdir
	)

	shared_library('gplugin-perl5',
		GPLUGIN_PERL_SOURCES,
		GPLUGIN_PERL_HEADERS,
		name_prefix : '',
		dependencies : [GMODULE, gplugin_dep, perl_dep, glib_perl_dep],
		install : true,
		install_dir : get_option('libdir') / 'gplugin'
	)

	# Add the loader's path to our environment variable
	devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_build_dir())
endif # perl5

subdir('tests')
