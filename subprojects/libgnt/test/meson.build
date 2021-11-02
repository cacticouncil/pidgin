foreach prog : ['combo', 'focus', 'tv', 'multiwin', 'keys', 'menu', 'parse']
    executable(prog,
        prog + '.c',
        c_args : '-DSTANDALONE',
        dependencies : [libgnt_dep, gobject, gmodule])
endforeach

shared_library('wm',
    'wm.c',
    name_prefix : '',
    dependencies : [libgnt_dep, gobject, gmodule])
