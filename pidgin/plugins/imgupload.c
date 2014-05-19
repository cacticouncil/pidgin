/*
 * Image Upload - an inline images implementation for protocols without
 * support for such feature.
 *
 * Copyright (C) 2014, Tomasz Wasilczyk <twasilczyk@pidgin.im>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301 USA
 */

#include "internal.h"

#include "debug.h"
#include "glibcompat.h"
#include "http.h"
#include "version.h"

#include "gtk3compat.h"
#include "gtkconv.h"
#include "gtkplugin.h"
#include "gtkutils.h"
#include "gtkwebviewtoolbar.h"
#include "pidginstock.h"

#include <json-glib/json-glib.h>

#define IMGUP_IMGUR_CLIENT_ID "b6d33c6bb80e1b6"

static void
imgup_upload_done(PidginWebView *webview, const gchar *url, const gchar *title);
static void
imgup_upload_failed(PidginWebView *webview);


/******************************************************************************
 * Helper functions
 ******************************************************************************/

static gboolean
imgup_conn_is_hooked(PurpleConnection *gc)
{
	return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(gc), "imgupload-set"));
}


/******************************************************************************
 * Imgur implementation
 ******************************************************************************/

static void
imgup_imgur_uploaded(PurpleHttpConnection *hc, PurpleHttpResponse *resp,
	gpointer _webview)
{
	JsonParser *parser;
	JsonObject *result;
	const gchar *data;
	gsize data_len;
	PidginWebView *webview = PIDGIN_WEBVIEW(_webview);
	const gchar *url;

	if (!purple_http_response_is_successful(resp)) {
		imgup_upload_failed(webview);
		return;
	}

	data = purple_http_response_get_data(resp, &data_len);
	parser = json_parser_new();
	if (!json_parser_load_from_data(parser, data, data_len, NULL)) {
		purple_debug_warning("imgupload", "Invalid json got from imgur");

		imgup_upload_failed(webview);
		return;
	}

	result = json_node_get_object(json_parser_get_root(parser));

	if (!json_object_get_boolean_member(result, "success")) {
		g_object_unref(parser);

		purple_debug_warning("imgupload", "imgur - not a success");

		imgup_upload_failed(webview);
		return;
	}

	result = json_object_get_object_member(result, "data");
	url = json_object_get_string_member(result, "link");

	/* TODO: pass image name here too */
	imgup_upload_done(webview, url, NULL);
}

static PurpleHttpConnection *
imgup_imgur_upload(PidginWebView *webview, PurpleImage *image)
{
	PurpleHttpRequest *req;
	PurpleHttpConnection *hc;
	gchar *req_data, *img_data, *img_data_e;

	req = purple_http_request_new("https://api.imgur.com/3/image");
	purple_http_request_set_method(req, "POST");
	purple_http_request_header_set(req, "Authorization",
		"Client-ID " IMGUP_IMGUR_CLIENT_ID);

	/* TODO: make it a plain, multipart/form-data request */
	img_data = purple_base64_encode(purple_image_get_data(image),
		purple_image_get_size(image));
	img_data_e = g_uri_escape_string(img_data, NULL, FALSE);
	g_free(img_data);
	req_data = g_strdup_printf("type=base64&image=%s", img_data_e);
	g_free(img_data_e);

	purple_http_request_header_set(req, "Content-Type",
		"application/x-www-form-urlencoded");
	purple_http_request_set_contents(req, req_data, -1);
	g_free(req_data);

	hc = purple_http_request(NULL, req, imgup_imgur_uploaded, webview);
	purple_http_request_unref(req);

	return hc;
}

/******************************************************************************
 * Image/link upload and insertion
 ******************************************************************************/

static void
imgup_upload_done(PidginWebView *webview, const gchar *url, const gchar *title)
{
	g_object_steal_data(G_OBJECT(webview), "imgupload-hc");

	/* TODO: insert text or insert link */
	pidgin_webview_insert_link(webview, url, title ? title : url);
}

static void
imgup_upload_failed(PidginWebView *webview)
{
	g_object_steal_data(G_OBJECT(webview), "imgupload-hc");

	purple_debug_error("imgupload", "Failed uploading image");
}

static gboolean
imgup_upload_start(PidginWebView *webview, PurpleImage *image, gpointer _gtkconv)
{
	PidginConversation *gtkconv = _gtkconv;
	PurpleConversation *conv = gtkconv->active_conv;
	PurpleHttpConnection *hc;

	if (!imgup_conn_is_hooked(purple_conversation_get_connection(conv)))
		return FALSE;

	hc = imgup_imgur_upload(webview, image);
	g_object_set_data_full(G_OBJECT(webview), "imgupload-hc",
		hc, (GDestroyNotify)purple_http_conn_cancel);

	/* TODO: please wait dialog */

	return TRUE;
}


/******************************************************************************
 * Plugin setup
 ******************************************************************************/

static void
imgup_pidconv_init(PidginConversation *gtkconv)
{
	PidginWebView *webview;

	webview = PIDGIN_WEBVIEW(gtkconv->entry);

	g_signal_connect(G_OBJECT(webview), "insert-image",
		G_CALLBACK(imgup_upload_start), gtkconv);
}

static void
imgup_pidconv_uninit(PidginConversation *gtkconv)
{
	PidginWebView *webview;

	webview = PIDGIN_WEBVIEW(gtkconv->entry);

	g_signal_handlers_disconnect_by_func(G_OBJECT(webview),
		G_CALLBACK(imgup_upload_start), gtkconv);
}

static void
imgup_conv_init(PurpleConversation *conv)
{
	PurpleConnection *gc;

	gc = purple_conversation_get_connection(conv);
	if (!gc)
		return;
	if (!imgup_conn_is_hooked(gc))
		return;

	purple_conversation_set_features(conv,
		purple_conversation_get_features(conv) &
		~PURPLE_CONNECTION_FLAG_NO_IMAGES);

	g_object_set_data(G_OBJECT(conv), "imgupload-set", GINT_TO_POINTER(TRUE));
}

static void
imgup_conv_uninit(PurpleConversation *conv)
{
	PurpleConnection *gc;

	gc = purple_conversation_get_connection(conv);
	if (gc) {
		if (!imgup_conn_is_hooked(gc))
			return;
	} else {
		if (!g_object_get_data(G_OBJECT(conv), "imgupload-set"))
			return;
	}

	purple_conversation_set_features(conv,
		purple_conversation_get_features(conv) |
		PURPLE_CONNECTION_FLAG_NO_IMAGES);

	g_object_set_data(G_OBJECT(conv), "imgupload-set", NULL);
}

static void
imgup_conn_init(PurpleConnection *gc)
{
	PurpleConnectionFlags flags;

	flags = purple_connection_get_flags(gc);

	if (!(flags & PURPLE_CONNECTION_FLAG_NO_IMAGES))
		return;

	flags &= ~PURPLE_CONNECTION_FLAG_NO_IMAGES;
	purple_connection_set_flags(gc, flags);

	g_object_set_data(G_OBJECT(gc), "imgupload-set", GINT_TO_POINTER(TRUE));
}

static void
imgup_conn_uninit(PurpleConnection *gc)
{
	if (!imgup_conn_is_hooked(gc))
		return;

	purple_connection_set_flags(gc, purple_connection_get_flags(gc) |
		PURPLE_CONNECTION_FLAG_NO_IMAGES);

	g_object_set_data(G_OBJECT(gc), "imgupload-set", NULL);
}

static gboolean
imgup_plugin_load(PurplePlugin *plugin)
{
	GList *it;

	it = purple_connections_get_all();
	for (; it; it = g_list_next(it)) {
		PurpleConnection *gc = it->data;
		imgup_conn_init(gc);
	}

	it = purple_conversations_get_all();
	for (; it; it = g_list_next(it)) {
		PurpleConversation *conv = it->data;
		imgup_conv_init(conv);
		if (PIDGIN_IS_PIDGIN_CONVERSATION(conv))
			imgup_pidconv_init(PIDGIN_CONVERSATION(conv));
	}

	purple_signal_connect(purple_connections_get_handle(),
		"signed-on", plugin,
		PURPLE_CALLBACK(imgup_conn_init), NULL);
	purple_signal_connect(purple_connections_get_handle(),
		"signing-off", plugin,
		PURPLE_CALLBACK(imgup_conn_uninit), NULL);
	purple_signal_connect(pidgin_conversations_get_handle(),
		"conversation-displayed", plugin,
		PURPLE_CALLBACK(imgup_pidconv_init), NULL);

	return TRUE;
}

static gboolean
imgup_plugin_unload(PurplePlugin *plugin)
{
	GList *it;

	it = purple_conversations_get_all();
	for (; it; it = g_list_next(it)) {
		PurpleConversation *conv = it->data;
		imgup_conv_uninit(conv);
		if (PIDGIN_IS_PIDGIN_CONVERSATION(conv))
			imgup_pidconv_uninit(PIDGIN_CONVERSATION(conv));
	}

	it = purple_connections_get_all();
	for (; it; it = g_list_next(it)) {
		PurpleConnection *gc = it->data;
		imgup_conn_uninit(gc);
	}

	return TRUE;
}

static PurplePluginInfo imgup_info =
{
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,
	PIDGIN_PLUGIN_TYPE,
	0,
	NULL,
	PURPLE_PRIORITY_DEFAULT,
	"gtk-imgupload",
	N_("Image Upload"),
	DISPLAY_VERSION,
	N_("Inline images implementation for protocols without such feature."),
	N_("Adds inline images support for protocols lacking this feature by "
		"uploading them to the external service."),
	"Tomasz Wasilczyk <twasilczyk@pidgin.im>",
	PURPLE_WEBSITE,
	imgup_plugin_load,
	imgup_plugin_unload,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	/* padding */
	NULL, NULL, NULL, NULL
};

static void
imgup_init_plugin(PurplePlugin *plugin)
{
#if 0
	purple_prefs_add_none("/plugins");
	purple_prefs_add_none("/plugins/gtk");
	purple_prefs_add_none("/plugins/gtk/imgupload");
#endif
}

PURPLE_INIT_PLUGIN(imgupload, imgup_init_plugin, imgup_info)
