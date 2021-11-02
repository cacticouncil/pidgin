plugindir = get_option('libdir') / 'gnt3'

irssi = library('irssi', 'irssi.c',
    dependencies : [ncurses, gobject, libgnt_dep],
    name_prefix : '',
    install : true, install_dir : plugindir)

s = library('s', 's.c',
    dependencies : [ncurses, gobject, libgnt_dep],
    name_prefix : '',
    install : true, install_dir : plugindir)
