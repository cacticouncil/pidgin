/*
 * Copyright (C) 2011-2014 Gary Kramlich <grim@reaperworld.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include <glib.h>

#include <gplugin.h>
#include <gplugin-native.h>

/******************************************************************************
 * Tests
 *****************************************************************************/
/* load on query */
static void
test_load_on_query(void)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GPluginPlugin *plugin = NULL;

	gplugin_manager_remove_paths(manager);
	gplugin_manager_append_path(manager, TEST_LOAD_ON_QUERY_PASS_DIR);
	gplugin_manager_refresh(manager);

	plugin = gplugin_manager_find_plugin(manager, "gplugin/load-on-query");
	g_assert_nonnull(plugin);
	g_assert_true(GPLUGIN_IS_PLUGIN(plugin));

	g_assert_cmpint(
		gplugin_plugin_get_state(plugin),
		==,
		GPLUGIN_PLUGIN_STATE_LOADED);
}

static void
test_load_on_query_fail_subprocess(void)
{
	GPluginManager *manager = gplugin_manager_get_default();

	/* this test is very simple since we can't get the exact error condition
	 * that we want.
	 *
	 * There's an error condition where a plugin will be stored twice, but we
	 * can't test for it since a g_warning gets output that kills our fork, so
	 * we lose the internal state of the plugin manager and thus can't see the
	 * plugin stored twice.  This has been fixed in the code, but it has to be
	 * looked for manually.
	 */
	gplugin_manager_remove_paths(manager);
	gplugin_manager_append_path(manager, TEST_DIR);
	gplugin_manager_append_path(manager, TEST_LOAD_ON_QUERY_FAIL_DIR);
	gplugin_manager_refresh(manager);
}

static void
test_load_on_query_fail(void)
{
	g_test_trap_subprocess(
		"/loaders/native/load-on-query/fail/subprocess",
		0,
		0);

	g_test_trap_assert_stderr("*failed to load*during query*");
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{

	g_test_init(&argc, &argv, NULL);

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	/* test the load on query flag */
	g_test_add_func("/loaders/native/load-on-query/pass", test_load_on_query);
	g_test_add_func(
		"/loaders/native/load-on-query/fail",
		test_load_on_query_fail);
	g_test_add_func(
		"/loaders/native/load-on-query/fail/subprocess",
		test_load_on_query_fail_subprocess);

	return g_test_run();
}
