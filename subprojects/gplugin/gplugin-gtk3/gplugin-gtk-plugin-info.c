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

#include <glib/gi18n-lib.h>

#include <gplugin.h>

#include <gplugin-gtk-plugin-info.h>

/**
 * GPluginGtkPluginInfo:
 *
 * A widget that displays a [class@GPlugin.PluginInfo] in a user friendly way.
 */

/******************************************************************************
 * Structs
 *****************************************************************************/
struct _GPluginGtkPluginInfo {
	GtkBox parent;

	GPluginPlugin *plugin;
	gulong signal_id;

	GtkWidget *name;
	GtkWidget *version;
	GtkWidget *authors_box;
	GtkWidget *website;
	GtkWidget *summary;
	GtkWidget *description;
	GtkWidget *dependencies_box;
	GtkWidget *error;
	GtkWidget *expander;
	GtkWidget *id;
	GtkWidget *filename;
	GtkWidget *abi_version;
	GtkWidget *loader;
	GtkWidget *internal;
	GtkWidget *load_on_query;
};

/*****************************************************************************s
 * Enums
 *****************************************************************************/
enum {
	PROP_ZERO,
	PROP_PLUGIN,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {
	NULL,
};

G_DEFINE_TYPE(GPluginGtkPluginInfo, gplugin_gtk_plugin_info, GTK_TYPE_BOX)

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
gplugin_gtk_plugin_info_expander_activate(
	GtkExpander *expander,
	G_GNUC_UNUSED gpointer data)
{
	if(gtk_expander_get_expanded(expander))
		gtk_expander_set_label(expander, "More");
	else
		gtk_expander_set_label(expander, "Less");
}

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
_gplugin_gtk_plugin_info_refresh(GPluginGtkPluginInfo *info)
{
	GtkWidget *widget = NULL;
	GError *error = NULL;
	GList *children = NULL, *iter = NULL;
	gchar *markup = NULL;
	gchar *name = NULL, *version = NULL, *website = NULL;
	gchar *summary = NULL, *description = NULL, *id = NULL, *abi_version = NULL;
	gchar *loader = NULL;
	gchar **authors = NULL;
	gchar **dependencies = NULL;
	guint32 abi_version_uint;
	gboolean loq = FALSE, internal = FALSE;
	const gchar *filename = NULL;

	/* remove all the children from the authors box */
	children = gtk_container_get_children(GTK_CONTAINER(info->authors_box));
	for(iter = children; iter; iter = iter->next)
		gtk_widget_destroy(GTK_WIDGET(iter->data));
	g_list_free(children);

	/* remove all the children from the dependencies box */
	children =
		gtk_container_get_children(GTK_CONTAINER(info->dependencies_box));
	for(iter = children; iter; iter = iter->next)
		gtk_widget_destroy(GTK_WIDGET(iter->data));
	g_list_free(children);

	/* now get the info if we can */
	if(GPLUGIN_IS_PLUGIN(info->plugin)) {
		GPluginPluginInfo *plugin_info = gplugin_plugin_get_info(info->plugin);
		GPluginLoader *plugin_loader = gplugin_plugin_get_loader(info->plugin);

		filename = gplugin_plugin_get_filename(info->plugin);
		error = gplugin_plugin_get_error(info->plugin);

		if(plugin_loader && GPLUGIN_IS_LOADER(plugin_loader)) {
			const char *loader_name = G_OBJECT_TYPE_NAME(plugin_loader);
			loader = g_strdup(loader_name);
			g_object_unref(G_OBJECT(plugin_loader));
		}

		/* clang-format off */
		g_object_get(
			G_OBJECT(plugin_info),
			"abi_version", &abi_version_uint,
			"authors", &authors,
			"summary", &summary,
			"description", &description,
			"dependencies", &dependencies,
			"id", &id,
			"internal", &internal,
			"load-on-query", &loq,
			"name", &name,
			"version", &version,
			"website", &website,
			NULL);
		/* clang-format on */

		/* fanagle the plugin name */
		markup = g_markup_printf_escaped(
			"<span font_size=\"large\" "
			"font_weight=\"bold\">%s</span>",
			(name) ? name : "Unnamed");
		g_free(name);
		name = markup;

		/* fanagle the website */
		if(website) {
			markup = g_markup_printf_escaped(
				"<a href=\"%s\">%s</a>",
				website,
				website);
			g_free(website);
			website = markup;
		}

		/* fanagle the abi_version */
		abi_version = g_strdup_printf("%08x", abi_version_uint);

		g_object_unref(G_OBJECT(plugin_info));
	}

	gtk_label_set_markup(GTK_LABEL(info->name), (name) ? name : "Unnamed");
	gtk_label_set_text(GTK_LABEL(info->version), (version) ? version : "");
	gtk_label_set_markup(GTK_LABEL(info->website), (website) ? website : "");
	gtk_label_set_text(GTK_LABEL(info->summary), (summary) ? summary : "");
	gtk_label_set_text(
		GTK_LABEL(info->description),
		(description) ? description : "");
	gtk_label_set_text(GTK_LABEL(info->id), (id) ? id : "");
	gtk_label_set_text(
		GTK_LABEL(info->error),
		(error) ? error->message : "(none)");
	gtk_label_set_text(GTK_LABEL(info->filename), (filename) ? filename : "");
	gtk_label_set_text(
		GTK_LABEL(info->abi_version),
		(abi_version) ? abi_version : "");
	gtk_label_set_text(GTK_LABEL(info->loader), (loader) ? loader : "Unknown");
	gtk_label_set_text(GTK_LABEL(info->internal), (internal) ? "Yes" : "No");
	gtk_label_set_text(GTK_LABEL(info->load_on_query), (loq) ? "Yes" : "No");

	/* set the authors */
	if(authors) {
		gint i = 0;

		for(i = 0; authors[i]; i++) {
			widget = gtk_label_new(authors[i]);
			gtk_widget_set_halign(widget, GTK_ALIGN_START);
			gtk_widget_set_valign(widget, GTK_ALIGN_START);
			gtk_box_pack_start(
				GTK_BOX(info->authors_box),
				widget,
				TRUE,
				TRUE,
				0);
			gtk_widget_show(widget);
		}
	}

	/* set the dependencies */
	if(dependencies) {
		gint i = 0;

		for(i = 0; dependencies[i]; i++) {
			widget = gtk_label_new(dependencies[i]);
			gtk_widget_set_halign(widget, GTK_ALIGN_START);
			gtk_widget_set_valign(widget, GTK_ALIGN_START);
			gtk_box_pack_start(
				GTK_BOX(info->dependencies_box),
				widget,
				TRUE,
				TRUE,
				0);
			gtk_widget_show(widget);
		}
	} else {
		widget = gtk_label_new(_("(none)"));
		gtk_box_pack_start(
			GTK_BOX(info->dependencies_box),
			widget,
			TRUE,
			TRUE,
			0);
		gtk_widget_show(widget);
	}

	g_clear_error(&error);
	g_free(loader);
	g_free(abi_version);
	g_strfreev(authors);
	g_free(summary);
	g_free(description);
	g_strfreev(dependencies);
	g_free(id);
	g_free(name);
	g_free(version);
	g_free(website);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
gplugin_gtk_plugin_info_state_cb(
	G_GNUC_UNUSED GObject *obj,
	G_GNUC_UNUSED GParamSpec *pspec,
	gpointer data)
{
	_gplugin_gtk_plugin_info_refresh(GPLUGIN_GTK_PLUGIN_INFO(data));
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
gplugin_gtk_plugin_info_set_property(
	GObject *obj,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkPluginInfo *info = GPLUGIN_GTK_PLUGIN_INFO(obj);

	switch(prop_id) {
		case PROP_PLUGIN:
			gplugin_gtk_plugin_info_set_plugin(info, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_plugin_info_get_property(
	GObject *obj,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkPluginInfo *info = GPLUGIN_GTK_PLUGIN_INFO(obj);

	switch(prop_id) {
		case PROP_PLUGIN:
			g_value_set_object(value, gplugin_gtk_plugin_info_get_plugin(info));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_plugin_info_finalize(GObject *obj)
{
	GPluginGtkPluginInfo *info = GPLUGIN_GTK_PLUGIN_INFO(obj);

	if(info->signal_id != 0 && GPLUGIN_IS_PLUGIN(info->plugin)) {
		g_signal_handler_disconnect(G_OBJECT(info->plugin), info->signal_id);
	}

	g_clear_object(&info->plugin);

	G_OBJECT_CLASS(gplugin_gtk_plugin_info_parent_class)->finalize(obj);
}

static void
gplugin_gtk_plugin_info_init(GPluginGtkPluginInfo *info)
{
	gtk_widget_init_template(GTK_WIDGET(info));

	info->signal_id = 0;
}

static void
gplugin_gtk_plugin_info_class_init(GPluginGtkPluginInfoClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = gplugin_gtk_plugin_info_get_property;
	obj_class->set_property = gplugin_gtk_plugin_info_set_property;
	obj_class->finalize = gplugin_gtk_plugin_info_finalize;

	/* properties */
	properties[PROP_PLUGIN] = g_param_spec_object(
		"plugin",
		"plugin",
		"The GPluginPlugin who's info should be displayed",
		G_TYPE_OBJECT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

	/* template stuff */
	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/gplugin/gplugin-gtk/plugin-info.ui");

	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		name);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		version);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		authors_box);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		website);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		summary);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		description);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		dependencies_box);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		error);

	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		expander);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		id);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		filename);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		abi_version);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		loader);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		internal);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginInfo,
		load_on_query);

	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_plugin_info_expander_activate);
}

/******************************************************************************
 * API
 *****************************************************************************/

/**
 * gplugin_gtk_plugin_info_new:
 *
 * Create a new widget which can be used to display info about a
 * [class@GPlugin.Plugin].
 *
 * Returns: (transfer full): The new display widget.
 */
GtkWidget *
gplugin_gtk_plugin_info_new(void)
{
	return GTK_WIDGET(g_object_new(GPLUGIN_GTK_TYPE_PLUGIN_INFO, NULL));
}

/**
 * gplugin_gtk_plugin_info_set_plugin:
 * @info: The widget instance.
 * @plugin: The plugin instance.
 *
 * Sets the plugin that should be displayed.
 *
 * A @plugin value of %NULL will clear the widget.
 */
void
gplugin_gtk_plugin_info_set_plugin(
	GPluginGtkPluginInfo *info,
	GPluginPlugin *plugin)
{
	g_return_if_fail(GPLUGIN_GTK_IS_PLUGIN_INFO(info));

	if(info->signal_id != 0 && GPLUGIN_IS_PLUGIN(info->plugin)) {
		g_signal_handler_disconnect(info->plugin, info->signal_id);
		info->signal_id = 0;
	}

	if(g_set_object(&info->plugin, plugin) && GPLUGIN_IS_PLUGIN(plugin)) {
		_gplugin_gtk_plugin_info_refresh(info);

		/* Connect a signal to refresh when the plugin's state changes.  We
		 * can't use g_signal_connect_object because the plugin object never
		 * gets destroyed, as the manager and the loader both keep a reference
		 * to it and the GPluginGtkPluginInfo widget is reused for all plugins
		 * so that all means that we just have to manage the callback
		 * ourselves.
		 */
		info->signal_id = g_signal_connect(
			G_OBJECT(plugin),
			"notify::state",
			G_CALLBACK(gplugin_gtk_plugin_info_state_cb),
			info);
	}
}

/**
 * gplugin_gtk_plugin_info_get_plugin:
 * @info: The widget instance.
 *
 * Returns the plugin that's being displayed.
 *
 * Returns: (transfer full): The plugin that's being displayed.
 */
GPluginPlugin *
gplugin_gtk_plugin_info_get_plugin(GPluginGtkPluginInfo *info)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_PLUGIN_INFO(info), NULL);

	return (info->plugin) ? GPLUGIN_PLUGIN(g_object_ref(G_OBJECT(info->plugin)))
						  : NULL;
}
