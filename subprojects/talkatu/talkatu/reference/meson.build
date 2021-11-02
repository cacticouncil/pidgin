DOC_MODULE = 'talkatu'

# Header files or dirs to ignore when scanning. Use base file/dir names
ignore_hfiles = [
    'talkatu.h',
    'talkatuenums.h',
    'talkaturesources.h',
]
ignore_hfiles += TALKATU_PRIVATE_HEADERS

# Extra options to supply to gtkdoc-scan.
scan_args = [
    '--rebuild-types',
    '--rebuild-sections',
    '--ignore-headers=' + ' '.join(ignore_hfiles),
]

# Extra options to supply to gtkdoc-mkdb.
mkdb_args = [
    '--ignore-files=' + ' '.join(ignore_hfiles),
]

talkatu_version_xml = configure_file(
    input : 'version.xml.in',
    output : 'version.xml',
    configuration : version_conf)

content_files = [
]

gnome.gtkdoc(DOC_MODULE,
    main_xml : DOC_MODULE + '-docs.xml',
    src_dir : talkatu_inc,
    dependencies : talkatu_dep,
    install : true,
    scan_args : scan_args,
    mkdb_args : mkdb_args,
    gobject_typesfile : DOC_MODULE + '.types',
    content_files : content_files,
)
