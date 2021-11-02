# Copyright (C) 2011-2020 Gary Kramlich <grim@reaperworld.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, see <https://www.gnu.org/licenses/>.

use strict;

use Glib::Object::Introspection;

Glib::Object::Introspection->setup(basename => "GPlugin",
                                   version => "1.0",
                                   package => "GPlugin");

sub gplugin_query {
	return GPlugin::PluginInfo->new(
		id => "gplugin/perl5-load-exception",
	);
}

sub gplugin_load {
	die("you be dead");

	return 0;
}

sub gplugin_unload {
	return 0;
}
