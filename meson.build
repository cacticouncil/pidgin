# UPDATING VERSION NUMBERS FOR RELEASES
#
# The version number is:
#   <major>.<minor>.<micro><extra>
#
# micro += 1
#
# If any functions have been added to libpurple, Pidgin, or Finch:
#   micro = 0
#   minor += 1
#
# If backwards compatibility has been broken in libpurple, Pidgin, or Finch:
#   micro = 0
#   minor = 0
#   major += 1
#   purple_soversion += 1
#
# extra should be similar to one of the following:
#   For beta releases:          '-beta2'
#   For code under development: '-devel'
#   For production releases:    ''
#
project('pidgin', 'c',
    version : '3.0.0-devel',
    meson_version : '>=0.58.0')
purple_soversion = 20

parts = meson.project_version().split('-')
if parts.length() > 1
  purple_extra_version = parts[1]
else
  purple_extra_version = ''
endif

parts = parts[0].split('.')
purple_major_version = parts[0]
purple_minor_version = parts[1]
purple_micro_version = parts[2]

GETTEXT_PACKAGE=meson.project_name()
find_program('gettext')
find_program('xgettext')

add_project_arguments([
	'-DHAVE_CONFIG_H=1',
	'-DVERSION="@0@"'.format(meson.project_version()),
	'-DDISPLAY_VERSION="@0@"'.format(meson.project_version()),
	'-DPURPLE_WEBSITE="https://pidgin.im/"',
	'-DGETTEXT_PACKAGE="@0@"'.format(GETTEXT_PACKAGE)],
	language : 'c')
conf = configuration_data()
man_conf = configuration_data()
version_conf = configuration_data()

conf.set_quoted('GETTEXT_PACKAGE', GETTEXT_PACKAGE)
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('PACKAGE_NAME', meson.project_name())
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('DISPLAY_VERSION', meson.project_version())

version_conf.set('PURPLE_MAJOR_VERSION', purple_major_version)
version_conf.set('PURPLE_MINOR_VERSION', purple_minor_version)
version_conf.set('PURPLE_MICRO_VERSION', purple_micro_version)
version_conf.set('PURPLE_EXTRA_VERSION', purple_extra_version)
version_conf.set('PURPLE_VERSION', meson.project_version())
version_conf.set('PURPLE_API_VERSION', purple_soversion)

PURPLE_LIB_VERSION = '@0@.@1@.@2@'.format(purple_soversion,
                                          purple_minor_version,
                                          purple_micro_version)

package_revision = vcs_tag(
    input : 'package_revision.h.in',
    output : 'package_revision.h',
    fallback : meson.project_version())

# For running `meson devenv`.
devenv = environment()

# For man pages.
man_conf.set('VERSION', meson.project_version())
man_conf.set('prefix', get_option('prefix'))

# Used for pkg-config files.
pkgconfig = import('pkgconfig')

# Storing build arguments
meson.add_postconf_script('mkmesonconf.py')
conf.set('HAVE_MESON_CONFIG', true)

# Checks for programs.
compiler = meson.get_compiler('c')

# Check for Sun compiler
SUNCC = compiler.compiles('void main() {__SUNPRO_C;};')

# Check for Win32
if host_machine.system() == 'windows'
	windows = import('windows')

	IS_WIN32 = true
	ws2_32 = compiler.find_library('ws2_32')
	dnsapi = compiler.find_library('dnsapi')
	if build_machine.system() != 'windows'
		conf.set('IS_WIN32_CROSS_COMPILED', true)
	endif
	conf.set('WIN32_LEAN_AND_MEAN', true)

	conf.set('LIBPIDGIN_DLL_NAMEW',
	    'L"libpidgin3-@0@.dll"'.format(purple_soversion))
else
	IS_WIN32 = false
	ws2_32 = []
	dnsapi = []
endif

# Checks for header files.
# AC_HEADER_SYS_WAIT:
conf.set('HAVE_SYS_WAIT_H', compiler.has_header('sys/wait.h'))
conf.set('HAVE_UNISTD_H', compiler.has_header('unistd.h'))

foreach h : ['fcntl.h', 'stdint.h']
	if compiler.has_header(h)
		conf.set('HAVE_' + h.to_upper().underscorify(), true)
	else
		error(h + ' is required.')
	endif
endforeach

# Checks for typedefs, structures, and compiler characteristics.
time_t_size = compiler.sizeof('time_t',
    prefix : '''
#include <stdio.h>
#include <time.h>
''')
conf.set('SIZEOF_TIME_T', time_t_size)

conf.set('WORDS_BIGENDIAN', host_machine.endian() != 'little')

# Check for directories
if IS_WIN32
	foreach dir : ['bin', 'lib', 'data', 'sysconf', 'locale']
		path = get_option('prefix') / get_option(dir + 'dir')
		conf.set_quoted('WIN32_FHS_@0@DIR'.format(dir.to_upper()), path)
	endforeach

	conf.set('PURPLE_LIBDIR',
		 'wpurple_lib_dir("purple-@0@")'.format(purple_major_version))
	conf.set('PIDGIN_LIBDIR',
		 'wpurple_lib_dir("pidgin-@0@")'.format(purple_major_version))
	conf.set('FINCH_LIBDIR',
		 'wpurple_lib_dir("finch-@0@")'.format(purple_major_version))

	conf.set('PURPLE_DATADIR', 'wpurple_data_dir()')
	conf.set('PURPLE_SYSCONFDIR', 'wpurple_sysconf_dir()')
	conf.set('PURPLE_LOCALEDIR', 'wpurple_locale_dir()')
else
	foreach dir : ['data', 'sysconf', 'locale']
		path = get_option('prefix') / get_option(dir + 'dir')
		conf.set_quoted('PURPLE_@0@DIR'.format(dir.to_upper()), path)
	endforeach

	common_libdir = get_option('prefix') / get_option('libdir')
	conf.set_quoted('PURPLE_LIBDIR',
	                common_libdir / 'purple-@0@'.format(purple_major_version))
	conf.set_quoted('PIDGIN_LIBDIR',
	                common_libdir / 'pidgin-@0@'.format(purple_major_version))
	conf.set_quoted('FINCH_LIBDIR',
	                common_libdir / 'finch-@0@'.format(purple_major_version))
endif

abslibdir = get_option('prefix') / get_option('libdir')
PURPLE_PLUGINDIR = abslibdir / 'purple-@0@'.format(purple_major_version)
conf.set_quoted('PURPLE_PLUGINDIR', PURPLE_PLUGINDIR)
PIDGIN_PLUGINDIR = abslibdir / 'pidgin-@0@'.format(purple_major_version)
conf.set_quoted('PIDGIN_PLUGINDIR', PIDGIN_PLUGINDIR)
FINCH_PLUGINDIR = abslibdir / 'finch-@0@'.format(purple_major_version)
conf.set_quoted('FINCH_PLUGINDIR', FINCH_PLUGINDIR)

# Check for socklen_t (in Unix98)
if IS_WIN32
	socket_header = 'ws2tcpip.h'
else
	socket_header = 'sys/socket.h'
endif
if not compiler.has_header_symbol(socket_header, 'socklen_t')
	code = '''
#include <sys/types.h>
#include <@0@>
int accept(int, struct sockaddr *, size_t *);
int main() {}
'''.format(socket_header)
	if compiler.compiles(code, name : 'socklen_t is size_t')
		conf.set('socklen_t', 'size_t')
	else
		conf.set('socklen_t', 'int')
	endif
endif

# Some systems do not have sa_len field for struct sockaddr.
conf.set('HAVE_STRUCT_SOCKADDR_SA_LEN',
    compiler.has_member('struct sockaddr', 'sa_len',
        prefix : '#include <@0@>'.format(socket_header)))

# Windows and Haiku do not use libm for the math functions, they are part
# of the C library
math = compiler.find_library('m', required: false)

IOKIT = []
if host_machine.system() == 'darwin'
	IOKIT = dependency('appleframeworks',
	                   modules : ['IOKit', 'CoreFoundation'])

	conf.set('HAVE_IOKIT', true)
endif

#######################################################################
# Check for GLib (required)
#######################################################################
glib = dependency('glib-2.0', version : '>= 2.70.0')
gio = dependency('gio-2.0')
gobject = dependency('gobject-2.0')
gthread = dependency('gthread-2.0')
gnome = import('gnome')

add_project_arguments(
	'-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_70',
	'-DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_70',
	language : 'c',)

#######################################################################
# Check for gdk-pixbuf (required)
#######################################################################
gdk_pixbuf = dependency('gdk-pixbuf-2.0')

#######################################################################
# Check for GObject Introspection
#######################################################################
if get_option('introspection')
	enable_introspection = dependency('gobject-introspection-1.0', version : '>= 1.39.0').found()
else
	enable_introspection = false
endif
conf.set('ENABLE_INTROSPECTION', enable_introspection)

#######################################################################
# Check for libnice
#######################################################################
nice = dependency('nice',
	version : '>=0.1.13',
	allow_fallback : get_option('build-dependencies'))

#######################################################################
# Check Pidgin dependencies
#######################################################################
if get_option('gtkui')
	gtk = dependency('gtk4', version : '>= 4.0.0')
	libadwaita = dependency(
  'libadwaita-1',
  version: '>= 1.0.0',
  fallback: ['libadwaita', 'libadwaita_dep'],
  default_options: [
    'examples=false',
    'introspection=disabled',
    'tests=false',
    'vapi=false',
  ]
)

	talkatu_dep = dependency('talkatu',
		version: '>=0.1.0',
		fallback: ['talkatu', 'talkatu_dep'])
endif	# GTK

ENABLE_GTK = get_option('gtkui')


#######################################################################
# Check if we should compile with X support
#######################################################################
if IS_WIN32
	x11 = disabler()
else
	x11 = dependency('x11', required : get_option('x'))
endif
conf.set('HAVE_X11', x11.found())

#######################################################################
# Check for LibXML2 (required)
#######################################################################
libxml = dependency('libxml-2.0', version : '>= 2.6.0')
if libxml.version().version_compare('<2.6.18')
	message('Versions of libxml2 < 2.6.18 may contain bugs that could cause XMPP messages to be discarded.')
endif

#######################################################################
# Check for JSON-GLib (required)
#######################################################################

json = dependency('json-glib-1.0', version : '>= 0.14.0')

#######################################################################
# Check for gUPnP (required)
#######################################################################

gupnp = dependency('gupnp-1.2', version : '>= 1.2.0')
gupnp_igd = dependency('gupnp-igd-1.0',
	version : '>= 1.2.0',
	allow_fallback : get_option('build-dependencies'))

#######################################################################
# Check for libsoup (required)
#######################################################################

libsoup = disabler()
if get_option('soup3')
	libsoup = dependency('libsoup-3.0', version : '>= 3')
	add_project_arguments(
		'-DSOUP_VERSION_MIN_REQUIRED=SOUP_VERSION_3_0',
		'-DSOUP_VERSION_MAX_ALLOWED=SOUP_VERSION_3_0',
		language : 'c')
else
	libsoup = dependency('libsoup-2.4', version : '>= 2.42')
	add_project_arguments(
		'-DSOUP_VERSION_MIN_REQUIRED=SOUP_VERSION_2_42',
		'-DSOUP_VERSION_MAX_ALLOWED=SOUP_VERSION_2_42',
		language : 'c')
endif

#######################################################################
# Check for sqlite3 (required)
#######################################################################
sqlite3 = dependency('sqlite3', version : '>= 3.27.0')

#######################################################################
# Check for GStreamer
#######################################################################

gstreamer = dependency('gstreamer-1.0', version : '>=1.14')

#######################################################################
# Check for Raw data streams support in Farstream
#######################################################################
gstreamer_app = dependency('gstreamer-app-1.0')

#######################################################################
# Check for Meanwhile headers (for Sametime)
#######################################################################
meanwhile = dependency('meanwhile', version : ['>= 1.0.0', '< 2.0.0'], required : get_option('meanwhile'))
gmime = dependency('gmime-3.0', version : '>= 3.0.0', required : get_option('meanwhile'))
enable_meanwhile = meanwhile.found() and gmime.found()

#######################################################################
# Check for Native Avahi headers (for Bonjour)
#######################################################################

if IS_WIN32
	# Just keep enabled.
	enable_avahi = get_option('avahi').enabled() or get_option('avahi').auto()
	avahi = []
else
	# Attempt to autodetect Avahi
	avahi_client = dependency('avahi-client', required : get_option('avahi'))
	avahi_glib = dependency('avahi-glib', required : get_option('avahi'))
	avahi = [avahi_client, avahi_glib]
	enable_avahi = avahi_client.found() and avahi_glib.found()
endif


#######################################################################
# Check for SILC client includes and libraries
#######################################################################
silc = dependency('silcclient', version : '>= 1.1.1', required : get_option('silc'))

#######################################################################
# Check for Gadu-Gadu protocol library (libgadu)
#######################################################################

libgadu = dependency('libgadu', version : '>= 1.12.0', required : get_option('libgadu'))

if libgadu.found()
	if not compiler.has_function('gg_is_gpl_compliant', dependencies : libgadu)
		if get_option('libgadu').auto()
			libgadu = disabler()
		else
			message('''
libgadu is not compatible with the GPL when compiled with OpenSSL support.

To link against libgadu, please recompile it using:
./configure --with-openssl=no
Then rerun this Meson build
			''')
		endif
	endif
endif


DEFAULT_PRPLS = ['bonjour', 'demo', 'facebook', 'gg', 'irc', 'ircv3', 'jabber',
                 'novell', 'null', 'sametime', 'silc', 'zephyr']
ALL_PRPLS = DEFAULT_PRPLS + ['null']

dynamic_list = get_option('dynamic-prpls').split(',')
if dynamic_list == ['all']
	dynamic_list = DEFAULT_PRPLS
endif
DYNAMIC_PRPLS = []
foreach prpl : dynamic_list
	if prpl == ''
		# The list was empty; do nothing.
	elif prpl == 'sametime' and not enable_meanwhile
		# Do nothing.
	elif prpl == 'bonjour' and not enable_avahi
		# Do nothing.
	elif prpl == 'silc' and not silc.found()
		# Do nothing.
	elif prpl == 'gg' and not libgadu.found()
		# Do nothing.
	elif prpl == 'zephyr' and IS_WIN32
		# Do nothing.
	else
		DYNAMIC_PRPLS += [prpl]
	endif
endforeach

DYNAMIC_BONJOUR = DYNAMIC_PRPLS.contains('bonjour')
DYNAMIC_DEMO = DYNAMIC_PRPLS.contains('demo')
DYNAMIC_FACEBOOK = DYNAMIC_PRPLS.contains('facebook')
DYNAMIC_GG  = DYNAMIC_PRPLS.contains('gg')
DYNAMIC_IRC = DYNAMIC_PRPLS.contains('irc')
DYNAMIC_IRCV3 = DYNAMIC_PRPLS.contains('ircv3')
DYNAMIC_JABBER = DYNAMIC_PRPLS.contains('jabber')
DYNAMIC_NOVELL = DYNAMIC_PRPLS.contains('novell')
DYNAMIC_NULL = DYNAMIC_PRPLS.contains('null')
DYNAMIC_SAMETIME = DYNAMIC_PRPLS.contains('sametime')
DYNAMIC_SILC = DYNAMIC_PRPLS.contains('silc')
DYNAMIC_ZEPHYR = DYNAMIC_PRPLS.contains('zephyr')

conf.set('HAVE_SYS_UTSNAME_H',
    compiler.has_header('sys/utsname.h'))
conf.set('HAVE_UNAME',
    compiler.has_function('uname'))


add_project_arguments(
    '-DPURPLE_DISABLE_DEPRECATED',
    '-DPIDGIN_DISABLE_DEPRECATED',
    '-DFINCH_DISABLE_DEPRECATED',
    '-DGNT_DISABLE_DEPRECATED',
    language : 'c')
if get_option('buildtype') != 'plain' and compiler.get_id() == 'gcc'
	# We enable -Wall later.
	# If it's set after the warning CFLAGS in the compiler invocation, it counteracts the -Wno... flags.
	# This leads to warnings we don't want.
#	CFLAGS=`echo $CFLAGS |$sedpath 's/-Wall//'`

	# ENABLE WARNINGS SUPPORTED BY THE VERSION OF GCC IN USE
	#
	# Future Possibilities
	#
	# Consider adding -Wbad-function-cast.
	#	This leads to spurious warnings using GPOINTER_TO_INT(), et al. directly on a function call.
	#		We'd need an intermediate variable.
	#
	foreach newflag : [
			'-Waggregate-return',
			'-Wcast-align',
			'-Wdeclaration-after-statement',
			'-Wendif-labels',
			'-Werror-implicit-function-declaration',
			'-Wextra -Wno-unused-parameter',
			'-Wformat',
			'-Wformat-security',
			'-Werror=format-security',
			'-Winit-self',
			'-Wmissing-declarations',
			'-Wmissing-noreturn',
			'-Wmissing-prototypes',
			'-Wpointer-arith',
			'-Wfloat-equal',
			'-Wundef']
		if compiler.has_argument(newflag)
			add_project_arguments(newflag, language : 'c')
		endif
	endforeach
endif
if get_option('buildtype') != 'plain' and SUNCC
	add_project_arguments('-features=extensions', language : 'c')
endif

pidgin3path = find_program('pidgin3', required : false)

#######################################################################
# Check for Unity and Messaging Menu
# Remove when Ubuntu 16.04 is EOL
#######################################################################
UNITY = [
	dependency('unity', version : '>= 6.8', required : get_option('unity-integration')),
	dependency('messaging-menu', version : '>= 12.10', required : get_option('unity-integration'))
]
enable_unity = UNITY[0].found() and UNITY[1].found()
if enable_unity
	conf.set('USES_MM_CHAT_SECTION', 'X-MessagingMenu-UsesChatSection=true')
else
	conf.set('USES_MM_CHAT_SECTION', '')
endif

#######################################################################
# Check for Secret Service headers
#######################################################################

if IS_WIN32
	libsecret = disabler()
else
	libsecret = dependency('libsecret-1', required : get_option('libsecret'))
endif

#######################################################################
# Check for KWallet headers
#######################################################################

if IS_WIN32 or not add_languages('cpp', required : get_option('kwallet'), native: false)
	kwallet = disabler()
else
	# Use C++ compiler
	cxx_compiler = meson.get_compiler('cpp')
	add_project_arguments([
		'-DHAVE_CONFIG_H=1',
		'-DDISPLAY_VERSION="@0@"'.format(meson.project_version()),
		'-DPURPLE_WEBSITE="https://pidgin.im/"',
		'-DGETTEXT_PACKAGE="@0@"'.format(GETTEXT_PACKAGE)],
		language : 'cpp')

	qt5 = import('qt5')

	qt5_dep = dependency('qt5', modules: ['Core'], required : get_option('kwallet'))

	kwallet = dependency('KF5Wallet', required : get_option('kwallet'))
endif

#######################################################################
# Check for GPlugin
#######################################################################
gplugin_version = ['>=0.38.0', '<0.39.0']
gplugin_dep = dependency('gplugin',
	version : gplugin_version,
	fallback : ['gplugin', 'gplugin_dep'])

if get_option('gtkui')
	gplugin_gtk_dep = dependency('gplugin-gtk4',
		version : gplugin_version,
		fallback : ['gplugin-gtk4', 'gplugin_gtk4_dep'])
endif

#######################################################################
# Check for Cyrus-SASL (for xmpp/irc)
#######################################################################
foreach func : ['snprintf']
	conf.set('HAVE_' + func.to_upper(),
	    compiler.has_function(func))
endforeach
sasl = dependency('libsasl2', version : '>= 2.0')

#######################################################################
# Check for external libzephyr
#######################################################################
ext_zephyr = dependency('zephyr', required : get_option('zephyr'))
EXTERNAL_LIBZEPHYR = ext_zephyr.found()
conf.set('LIBZEPHYR_EXT', EXTERNAL_LIBZEPHYR)

#######################################################################
# Check for Kerberos (for Zephyr)
#######################################################################
conf.set('ZEPHYR_INT32', 'long')
#AC_SUBST(KRB4_CFLAGS)
#AC_SUBST(KRB4_LDFLAGS)
#AC_SUBST(KRB4_LIBS)
kerberos = get_option('krb4')
if kerberos
	if kerberos != 'yes'
#		KRB4_CFLAGS='-I${kerberos}/include'
#		if test -d '$kerberos/include/kerberosIV' ; then
#			KRB4_CFLAGS='$KRB4_CFLAGS -I${kerberos}/include/kerberosIV'
#		fi
#		KRB4_LDFLAGS='-L${kerberos}/lib'
	elif run_command('test', '-d', '/usr/local/include/kerberosIV').returncode() == 0
#		KRB4_CFLAGS='-I/usr/local/include/kerberosIV'
	elif run_command('test', '-d', '/usr/include/kerberosIV').returncode() == 0
#		KRB4_CFLAGS='-I/usr/include/kerberosIV'
	endif
	conf.set('ZEPHYR_USES_KERBEROS', true)

#	AC_CHECK_LIB(krb4, krb_rd_req,
#			[KRB4_LIBS='-lkrb4 -ldes425 -lkrb5 -lk5crypto -lcom_err'],
#			[AC_CHECK_LIB(krb, krb_rd_req,
#				[KRB4_LIBS='-lkrb -ldes'],
#				[AC_MSG_ERROR([Kerberos 4 libraries not found])],
#				-ldes)],
#			-ldes425 -lkrb5 -lk5crypto -lcom_err)
#	AC_CHECK_FUNCS(krb_set_key krb_rd_req krb_get_lrealm)
#	AC_CHECK_FUNCS(krb_get_err_text krb_log)
	krb4 = []
endif
if not kerberos
	krb4 = []
endif

#AC_MSG_CHECKING(for me pot o' gold)
#AC_MSG_RESULT(no)
foreach func : ['timegm']
	conf.set('HAVE_' + func.to_upper(),
	    compiler.has_function(func))
endforeach
foreach header : 'sgtty.h sys/cdefs.h sys/file.h sys/filio.h sys/msgbuf.h sys/select.h sys/wait.h termios.h'.split()
	conf.set('HAVE_' + header.to_upper().underscorify(),
	    compiler.has_header(header))
endforeach

# OpenBSD 6.7 and FreeBSD 12 require including sys/types.h before sys/sysctl.h.
# NetBSD 9.0 doesn't care.
if host_machine.system() in ['freebsd', 'openbsd']
	prefix = '''
#include <sys/types.h>
'''

	conf.set('HAVE_SYS_SYSCTL_H',
	    compiler.has_header('sys/sysctl.h', prefix : prefix))
	conf.set('HAVE_SYS_SOCKET_H',
	    compiler.has_header('sys/socket.h'))
endif

#AC_VAR_TIMEZONE_EXTERNALS

#######################################################################
# Documentation
#######################################################################

if get_option('doc') and not get_option('introspection')
    error('Documentation requires GObject Introspection.')
endif

gidocgen_dep = dependency(
    'gi-docgen', version: '>= 2021.1',
    fallback: ['gi-docgen', 'dummy_dep'],
    required: get_option('doc')
)

gidocgen = find_program('gi-docgen', required : get_option('doc'))
docs_dir = get_option('prefix') / get_option('datadir') / 'doc'

#######################################################################
# Random Stuff
#######################################################################
if ['debug', 'debugoptimize'].contains(get_option('buildtype'))
	enable_debug = true
else
	enable_debug = get_option('console-logging')
endif
conf.set('DEBUG', enable_debug)

# So that config.h may be found.
toplevel_inc = include_directories('.')

subdir('libpurple')
subdir('purple-history')
subdir('finch')
subdir('pidgin')
subdir('doc')
subdir('po')

configure_file(output : 'config.h',
    configuration : conf)

config_home = get_option('devenv-config-dir')
if config_home == ''
	config_home = meson.global_build_root() / 'config'
endif
devenv.set('XDG_CONFIG_HOME', config_home)

meson.add_devenv(devenv)

summary({
    'prefix': get_option('prefix'),
    'bindir': get_option('bindir'),
    'libdir': get_option('libdir'),
    'datadir': get_option('datadir'),
}, section : 'Directories')

summary({
    'GTK': get_option('gtkui'),
    'console': enable_consoleui,
    'X11 support': x11.found(),
}, section: 'User Interfaces', bool_yn: true)

summary({
    'Dynamic protocols': DYNAMIC_PRPLS,
    'Cyrus SASL support (IRC and XMPP)': sasl.found(),
    'External libzephyr': EXTERNAL_LIBZEPHYR,
    'Use kerberos 4 with zephyr': kerberos,
}, section: 'Protocol Support', bool_yn: true, list_sep: ', ')

summary({
    'KWallet credential provider': kwallet.found(),
    'libsecret credential provider': libsecret.found(),
    'Unity integration': enable_unity,
}, section: 'Plugin support', bool_yn: true)

summary({
    'Enable Introspection': enable_introspection,
    'Generate documentation': get_option('doc'),
    'Has you': true,
    'Print debugging messages': enable_debug,
}, section: 'Miscellaneous', bool_yn: true)

if pidgin3path.found()
    summary('You have an old copy of pidgin3 at', pidgin3path.full_path(),
            section: 'Warnings')
endif
