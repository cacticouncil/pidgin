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
use Scalar::Util qw(blessed);

Glib::Object::Introspection->setup(basename => "GPlugin",
                                   version => "1.0",
                                   package => "GPlugin");

sub gplugin_query {
	my $info = GPlugin::PluginInfo->new(
		id => "gplugin/perl5-basic-plugin",
		abi_version => 0x01020304,
		name => "basic plugin",
		authors => ("author1"),
		category => "test",
		version => "version",
		license_id => "license",
		summary => "summary",
		website => "website",
		description => "description",
	);

	return $info;
}

sub gplugin_load {
	my $plugin = shift;

	if(!defined($plugin) or (blessed($plugin) and !$plugin->isa("GPlugin::Plugin"))) {
		die("plugin is not a GPlugin::Plugin");
	}

	return 0;
}

sub gplugin_unload {
	my $plugin = shift;
	my $shutdown = shift;

	if(!defined($plugin) or (blessed($plugin) and !$plugin->isa("GPlugin::Plugin"))) {
		die("plugin is not a GPlugin::Plugin");
	}

	if($shutdown) {
		die("shutdown was true");
	}

	return 0;
}
