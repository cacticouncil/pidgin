Title: Perl5 Plugins
Slug: perl5

## Perl5 Plugins

> You **MUST** have the Perl5 loader plugin installed and working as well as
> the gobject-introspection package for GPlugin installed to use Perl5 plugins.

### Example Perl5 Plugin

Like all plugins in GPlugin, Perl5 plugins must also implement the
`gplugin_query`, `gplugin_load`, and `glugin_unload` functions.

The following is a basic Perl5 plugin.

```perl
use strict;

use Glib::Object::Introspection;

Glib::Object::Introspection->setup(basename => "GPlugin", version => "1.0", package=> "GPlugin");

sub gplugin_query {
	return GPlugin::PluginInfo->new(
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
}

sub gplugin_load {
	return 0;
}

sub gplugin_unload {
	return 0;
}
```

