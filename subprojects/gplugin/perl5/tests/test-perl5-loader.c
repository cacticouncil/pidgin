/*
 * Copyright (C) 2011-2020 Gary Kramlich <grim@reaperworld.com>
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

#include <glib.h>

#include <gplugin.h>
#include <gplugin/gplugin-loader-tests.h>

gint
main(gint argc, gchar **argv)
{
	g_test_init(&argc, &argv, NULL);

	gplugin_loader_tests_main(PERL5_LOADER_DIR, PERL5_PLUGIN_DIR, "perl5");

	return g_test_run();
}
