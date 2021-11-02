TEST_WRAPPER = find_program('./test-wrapper.py', required : true)

if get_option('tests')

e = executable(
	'test-html-serialization',
	'talkatutesthtmlserialization.c',
	dependencies : [talkatu_dep, GLIB, GTK3]
)
test('html', TEST_WRAPPER, args : e, is_parallel : false)

e = executable(
	'test-action-group',
	'talkatutestactiongroup.c',
	dependencies : [talkatu_dep, GLIB, GTK3]
)
test('action-group', TEST_WRAPPER, args : e, is_parallel : false)

e = executable(
	'test-html-pango-renderer',
	'talkatutesthtmlpangorenderer.c',
	dependencies : [talkatu_dep, GLIB]
)
test('html-pango-renderer', TEST_WRAPPER, args : e, is_parallel : false)

e = executable(
	'test-html-renderer',
	'talkatutesthtmlrenderer.c',
	dependencies : [talkatu_dep, GLIB]
)
test('html-renderer', TEST_WRAPPER, args : e, is_parallel : false)

endif
