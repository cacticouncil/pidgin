/**
 * @file icon.c Buddy Icon API
 * @ingroup core
 *
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "internal.h"
#include "buddyicon.h"
#include "cipher.h"
#include "conversation.h"
#include "dbus-maybe.h"
#include "debug.h"
#include "imgstore.h"
#include "util.h"

typedef struct _PurpleBuddyIconData PurpleBuddyIconData;

/* NOTE: Instances of this struct are allocated without zeroing the memory, so
 * NOTE: be sure to update purple_buddy_icon_new() if you add members. */
struct _PurpleBuddyIcon
{
	PurpleAccount *account;    /**< The account the user is on.          */
	char *username;            /**< The username the icon belongs to.    */
	PurpleStoredImage *img;    /**< The id of the stored image with the
	                                the icon data.                       */
	int ref_count;             /**< The buddy icon reference count.      */
};

static GHashTable *account_cache = NULL;
static GHashTable *icon_data_cache = NULL;
static GHashTable *icon_file_cache = NULL;
static GHashTable *custom_icon_cache = NULL;
static char       *cache_dir     = NULL;
static gboolean    icon_caching  = TRUE;

/* For ~/.gaim to ~/.purple migration. */
static char *old_icons_dir = NULL;

static void
ref_filename(const char *filename)
{
	int refs;

	g_return_if_fail(filename != NULL);

	refs = GPOINTER_TO_INT(g_hash_table_lookup(icon_file_cache, filename));
	printf("refs before increment = %d\n", refs);

	g_hash_table_insert(icon_file_cache, g_strdup(filename),
	                    GINT_TO_POINTER(refs + 1));
}

static void
unref_filename(const char *filename)
{
	int refs;

	if (filename == NULL)
		return;

	refs = GPOINTER_TO_INT(g_hash_table_lookup(icon_file_cache, filename));
	printf("refs before decrement = %d\n", refs);

	if (refs == 1)
	{
		g_hash_table_remove(icon_file_cache, filename);
	}
	else
	{
		g_hash_table_insert(icon_file_cache, g_strdup(filename),
		                    GINT_TO_POINTER(refs - 1));
	}
}

static char *
purple_buddy_icon_data_calculate_filename(guchar *icon_data, size_t icon_len)
{
	PurpleCipherContext *context;
	gchar digest[41];

	context = purple_cipher_context_new_by_name("sha1", NULL);
	if (context == NULL)
	{
		purple_debug_error("buddyicon", "Could not find sha1 cipher\n");
		g_return_val_if_reached(NULL);
	}

	/* Hash the icon data */
	purple_cipher_context_append(context, icon_data, icon_len);
	if (!purple_cipher_context_digest_to_str(context, sizeof(digest), digest, NULL))
	{
		purple_debug_error("buddyicon", "Failed to get SHA-1 digest.\n");
		g_return_val_if_reached(NULL);
	}
	purple_cipher_context_destroy(context);

	/* Return the filename */
	return g_strdup_printf("%s.%s", digest,
	                       purple_util_get_image_extension(icon_data, icon_len));
}

static void
purple_buddy_icon_data_cache(PurpleStoredImage *img)
{
	const char *dirname;
	char *path;
	FILE *file = NULL;

	g_return_if_fail(img != NULL);

	if (!purple_buddy_icons_is_caching())
		return;

	dirname  = purple_buddy_icons_get_cache_dir();
	path = g_build_filename(dirname, purple_imgstore_get_filename(img), NULL);

	if (!g_file_test(dirname, G_FILE_TEST_IS_DIR))
	{
		purple_debug_info("buddyicon", "Creating icon cache directory.\n");

		if (g_mkdir(dirname, S_IRUSR | S_IWUSR | S_IXUSR) < 0)
		{
			purple_debug_error("buddyicon",
			                   "Unable to create directory %s: %s\n",
			                   dirname, strerror(errno));
		}
	}

	if ((file = g_fopen(path, "wb")) != NULL)
	{
		if (!fwrite(purple_imgstore_get_data(img), purple_imgstore_get_size(img), 1, file))
		{
			purple_debug_error("buddyicon", "Error writing %s: %s\n",
			                   path, strerror(errno));
		}
		else
			purple_debug_info("buddyicon", "Wrote cache file: %s\n", path);

		fclose(file);
	}
	else
	{
		purple_debug_error("buddyicon", "Unable to create file %s: %s\n",
		                   path, strerror(errno));
		g_free(path);
		return;
	}
	g_free(path);
}

static void
purple_buddy_icon_data_uncache_file(const char *filename)
{
	const char *dirname;
	char *path;

	g_return_if_fail(filename != NULL);

	/* It's possible that there are other references to this icon
	 * cache file that are not currently loaded into memory. */
	printf("file has %d refs\n", GPOINTER_TO_INT(g_hash_table_lookup(icon_file_cache, filename)));
	if (GPOINTER_TO_INT(g_hash_table_lookup(icon_file_cache, filename)))
		return;

	dirname  = purple_buddy_icons_get_cache_dir();
	path = g_build_filename(dirname, filename, NULL);

	printf("Going to unlink %s\n", path);
	if (g_file_test(path, G_FILE_TEST_EXISTS))
	{
		if (g_unlink(path))
		{
			printf("Failed to unlink %s\n", path);

			purple_debug_error("buddyicon", "Failed to delete %s: %s\n",
			                   path, strerror(errno));
		}
		else
		{
			printf("Unlinked %s\n", path);
			purple_debug_info("buddyicon", "Deleted cache file: %s\n", path);
		}
	}

	g_free(path);
}

static void
image_deleting_cb(PurpleStoredImage *img, gpointer data)
{
	const char *filename = purple_imgstore_get_filename(img);

	if (img == g_hash_table_lookup(icon_data_cache, filename))
	{
		purple_buddy_icon_data_uncache_file(filename);
		g_hash_table_remove(icon_data_cache, filename);
	}
}

static PurpleStoredImage *
purple_buddy_icon_data_new(guchar *icon_data, size_t icon_len, const char *filename)
{
	char *file;
	PurpleStoredImage *img;

	g_return_val_if_fail(icon_data != NULL, NULL);
	g_return_val_if_fail(icon_len  > 0,     NULL);

	if (filename == NULL)
	{
		file = purple_buddy_icon_data_calculate_filename(icon_data, icon_len);
		if (file == NULL)
			return NULL;
	}
	else
		file = g_strdup(filename);

	if ((img = g_hash_table_lookup(icon_data_cache, file)))
	{
		g_free(file);
		return purple_imgstore_ref(img);
	}

	img = purple_imgstore_add(icon_data, icon_len, file);

	/* This will take ownership of file and g_free it either now or later. */
	g_hash_table_insert(icon_data_cache, file, img);

	purple_buddy_icon_data_cache(img);

	return img;
}

static PurpleBuddyIcon *
purple_buddy_icon_create(PurpleAccount *account, const char *username)
{
	PurpleBuddyIcon *icon;
	GHashTable *icon_cache;

	/* This does not zero.  See purple_buddy_icon_new() for
	 * information on which function allocates which member. */
	icon = g_slice_new(PurpleBuddyIcon);
	PURPLE_DBUS_REGISTER_POINTER(icon, PurpleBuddyIcon);

	icon->account = account;
	icon->username = g_strdup(username);

	icon_cache = g_hash_table_lookup(account_cache, account);

	if (icon_cache == NULL)
	{
		icon_cache = g_hash_table_new(g_str_hash, g_str_equal);

		g_hash_table_insert(account_cache, account, icon_cache);
	}

	g_hash_table_insert(icon_cache,
	                    (char *)purple_buddy_icon_get_username(icon), icon);
	return icon;
}

PurpleBuddyIcon *
purple_buddy_icon_new(PurpleAccount *account, const char *username,
                      void *icon_data, size_t icon_len)
{
	PurpleBuddyIcon *icon;

	g_return_val_if_fail(account   != NULL, NULL);
	g_return_val_if_fail(username  != NULL, NULL);
	g_return_val_if_fail(icon_data != NULL, NULL);
	g_return_val_if_fail(icon_len  > 0,    NULL);

	/* purple_buddy_icons_find() does allocation, so be
	 * sure to update it as well when members are added. */
	icon = purple_buddy_icons_find(account, username);

	/* purple_buddy_icon_create() sets account & username */
	if (icon == NULL)
		icon = purple_buddy_icon_create(account, username);

	/* Take a reference for the caller of this function. */
	icon->ref_count = 1;

	/* purple_buddy_icon_set_data() sets img, but it
	 * references img first, so we need to initialize it */
	icon->img = NULL;
	purple_buddy_icon_set_data(icon, icon_data, icon_len);

	return icon;
}

PurpleBuddyIcon *
purple_buddy_icon_ref(PurpleBuddyIcon *icon)
{
	g_return_val_if_fail(icon != NULL, NULL);

	icon->ref_count++;

	return icon;
}

PurpleBuddyIcon *
purple_buddy_icon_unref(PurpleBuddyIcon *icon)
{
	if (icon == NULL)
		return NULL;

	g_return_val_if_fail(icon->ref_count > 0, NULL);

	icon->ref_count--;

	if (icon->ref_count == 0)
	{
		GHashTable *icon_cache = g_hash_table_lookup(account_cache, purple_buddy_icon_get_account(icon));

		if (icon_cache != NULL)
			g_hash_table_remove(icon_cache, purple_buddy_icon_get_username(icon));

		g_free(icon->username);
		purple_imgstore_unref(icon->img);

		PURPLE_DBUS_UNREGISTER_POINTER(icon);
		g_slice_free(PurpleBuddyIcon, icon);

		return NULL;
	}

	return icon;
}

void
purple_buddy_icon_update(PurpleBuddyIcon *icon)
{
	PurpleConversation *conv;
	PurpleAccount *account;
	const char *username;
	PurpleBuddyIcon *icon_to_set;
	GSList *sl, *list;

	g_return_if_fail(icon != NULL);

	account  = purple_buddy_icon_get_account(icon);
	username = purple_buddy_icon_get_username(icon);

	/* If no data exists, then call the functions below with NULL to
	 * unset the icon.  They will then unref the icon and it should be
	 * destroyed.  The only way it wouldn't be destroyed is if someone
	 * else is holding a reference to it, in which case they can kill
	 * the icon when they realize it has no data. */
	icon_to_set = icon->img ? icon : NULL;

	for (list = sl = purple_find_buddies(account, username);
	     sl != NULL;
	     sl = sl->next)
	{
		PurpleBuddy *buddy = (PurpleBuddy *)sl->data;
		char *old_icon;

		purple_buddy_set_icon(buddy, icon_to_set);


		old_icon = g_strdup(purple_blist_node_get_string((PurpleBlistNode *)buddy,
		                                                 "buddy_icon"));
		if (icon->img)
		{
			const char *filename = purple_imgstore_get_filename(icon->img);
			purple_blist_node_set_string((PurpleBlistNode *)buddy,
			                             "buddy_icon",
			                             filename);
			printf("Calling ref_filename(%s)\n", filename);
			ref_filename(filename);
		}
		else
		{
			purple_blist_node_remove_setting((PurpleBlistNode *)buddy, "buddy_icon");
		}
		printf("Calling unref_filename(%s)\n", old_icon);
		unref_filename(old_icon);
		g_free(old_icon);
	}

	g_slist_free(list);

	conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, username, account);

	if (conv != NULL)
		purple_conv_im_set_icon(PURPLE_CONV_IM(conv), icon_to_set);
}

void
purple_buddy_icon_set_data(PurpleBuddyIcon *icon, guchar *data, size_t len)
{
	PurpleStoredImage *old_img;

	g_return_if_fail(icon != NULL);

	old_img = icon->img;
	icon->img = NULL;

	if (data != NULL && len > 0)
		icon->img = purple_buddy_icon_data_new(data, len, NULL);

	printf("Calling purple_buddy_icon_update\n");
	purple_buddy_icon_update(icon);

	printf("Calling purple_imgstore_unref\n");
	purple_imgstore_unref(old_img);
}

PurpleAccount *
purple_buddy_icon_get_account(const PurpleBuddyIcon *icon)
{
	g_return_val_if_fail(icon != NULL, NULL);

	return icon->account;
}

const char *
purple_buddy_icon_get_username(const PurpleBuddyIcon *icon)
{
	g_return_val_if_fail(icon != NULL, NULL);

	return icon->username;
}

gconstpointer
purple_buddy_icon_get_data(const PurpleBuddyIcon *icon, size_t *len)
{
	g_return_val_if_fail(icon != NULL, NULL);

	if (icon->img)
	{
		if (len != NULL)
			*len = purple_imgstore_get_size(icon->img);

		return purple_imgstore_get_data(icon->img);
	}

	return NULL;
}

const char *
purple_buddy_icon_get_extension(const PurpleBuddyIcon *icon)
{
	if (icon->img != NULL)
		return purple_imgstore_get_extension(icon->img);

	return NULL;
}

void
purple_buddy_icons_set_for_user(PurpleAccount *account, const char *username,
                                void *icon_data, size_t icon_len)
{
	g_return_if_fail(account  != NULL);
	g_return_if_fail(username != NULL);

	if (icon_data != NULL && icon_len > 0)
	{
		PurpleBuddyIcon *icon;

		icon = purple_buddy_icons_find(account, username);

		if (icon != NULL)
			purple_buddy_icon_set_data(icon, icon_data, icon_len);
	}
	else
	{
		PurpleBuddyIcon *icon = purple_buddy_icon_new(account, username, icon_data, icon_len);
		purple_buddy_icon_unref(icon);
	}
}

static gboolean
read_icon_file(const char *path, guchar **data, size_t *len)
{
	struct stat st;

	if (!g_stat(path, &st))
	{
		FILE *f = g_fopen(path, "rb");
		if (f)
		{
			*data = g_malloc(st.st_size);
			if (!fread(*data, st.st_size, 1, f))
			{
				purple_debug_error("buddyicon", "Error reading %s: %s\n",
				                   path, strerror(errno));
				g_free(*data);
				return FALSE;
			}
			fclose(f);

			*len = st.st_size;
			return TRUE;
		}
		else
		{
			purple_debug_error("buddyicon", "Unable to open file %s for reading: %s\n",
			                   path, strerror(errno));
		}
	}
	return FALSE;
}

PurpleBuddyIcon *
purple_buddy_icons_find(PurpleAccount *account, const char *username)
{
	GHashTable *icon_cache;
	PurpleBuddyIcon *icon = NULL;

	g_return_val_if_fail(account  != NULL, NULL);
	g_return_val_if_fail(username != NULL, NULL);

	icon_cache = g_hash_table_lookup(account_cache, account);

	if ((icon_cache == NULL) || ((icon = g_hash_table_lookup(icon_cache, username)) == NULL))
	{
		PurpleBuddy *b = purple_find_buddy(account, username);
		const char *protocol_icon_file;
		const char *dirname;
		gboolean caching;
		guchar *data;
		size_t len;

		if (!b)
			return NULL;

		protocol_icon_file = purple_blist_node_get_string((PurpleBlistNode*)b, "buddy_icon");

		if (protocol_icon_file == NULL)
			return NULL;

		dirname = purple_buddy_icons_get_cache_dir();

		caching = purple_buddy_icons_is_caching();
		/* By disabling caching temporarily, we avoid a loop
		 * and don't have to add special code through several
		 * functions. */
		purple_buddy_icons_set_caching(FALSE);

		if (protocol_icon_file != NULL)
		{
			char *path = g_build_filename(dirname, protocol_icon_file, NULL);
			if (read_icon_file(path, &data, &len))
			{
				if (icon == NULL)
					icon = purple_buddy_icon_create(account, username);
				icon->ref_count = 0;
				icon->img = NULL;
				purple_buddy_icon_set_data(icon, data, len);
			}
			g_free(path);
		}

		purple_buddy_icons_set_caching(caching);
	}

	return icon;
}

gboolean
purple_buddy_icons_has_custom_icon(PurpleContact *contact)
{
	g_return_val_if_fail(contact != NULL, FALSE);

	return (purple_blist_node_get_string((PurpleBlistNode*)contact, "custom_buddy_icon") != NULL);
}

PurpleStoredImage *
purple_buddy_icons_find_custom_icon(PurpleContact *contact)
{
	PurpleStoredImage *img;
	const char *custom_icon_file;
	const char *dirname;
	char *path;
	guchar *data;
	size_t len;

	g_return_val_if_fail(contact != NULL, NULL);

	if ((img = g_hash_table_lookup(custom_icon_cache, contact)))
	{
		return purple_imgstore_ref(img);
	}

	custom_icon_file = purple_blist_node_get_string((PurpleBlistNode*)contact, "custom_buddy_icon");

	if (custom_icon_file == NULL)
		return NULL;

	dirname = purple_buddy_icons_get_cache_dir();
	path = g_build_filename(dirname, custom_icon_file, NULL);

	if (read_icon_file(path, &data, &len))
	{
		g_free(path);
		img = purple_buddy_icon_data_new(data, len, custom_icon_file);
		g_hash_table_insert(custom_icon_cache, contact, img);
		return img;
	}
	g_free(path);

	return NULL;
}

void
purple_buddy_icons_set_custom_icon(PurpleContact *contact,
                                   guchar *icon_data, size_t icon_len)
{
	PurpleStoredImage *old_img;
	PurpleStoredImage *img = NULL;
	char *old_icon;

	old_img = g_hash_table_lookup(custom_icon_cache, contact);

	if (icon_data != NULL && icon_len > 0)
		img = purple_buddy_icon_data_new(icon_data, icon_len, NULL);

	old_icon = g_strdup(purple_blist_node_get_string((PurpleBlistNode *)contact,
	                                                 "custom_buddy_icon"));
	if (img)
	{
		const char *filename = purple_imgstore_get_filename(img);
		purple_blist_node_set_string((PurpleBlistNode *)contact,
		                             "custom_buddy_icon",
		                             filename);
		ref_filename(filename);
	}
	else
	{
		purple_blist_node_remove_setting((PurpleBlistNode *)contact,
		                                 "custom_buddy_icon");
	}
	unref_filename(old_icon);
	g_free(old_icon);


	g_hash_table_insert(custom_icon_cache, contact, img);
	purple_imgstore_unref(old_img);
}

void
purple_buddy_icon_set_old_icons_dir(const char *dirname)
{
	old_icons_dir = g_strdup(dirname);
}

static void
migrate_buddy_icon(PurpleBlistNode *node, const char *setting_name,
                   const char *dirname, const char *filename)
{
	char *path;

	if (filename[0] != '/')
	{
		path = g_build_filename(dirname, filename, NULL);
		if (g_file_test(path, G_FILE_TEST_EXISTS))
		{
			g_free(path);
			return;
		}
		g_free(path);

		path = g_build_filename(old_icons_dir, filename, NULL);
	}
	else
		path = g_strdup(filename);

	if (g_file_test(path, G_FILE_TEST_EXISTS))
	{
		guchar *icon_data;
		size_t icon_len;
		FILE *file;
		char *new_filename;

		if (!read_icon_file(path, &icon_data, &icon_len))
		{
			g_free(path);
			return;
		}

		g_free(path);

		new_filename = purple_buddy_icon_data_calculate_filename(icon_data, icon_len);
		if (new_filename == NULL)
		{
			return;
		}

		path = g_build_filename(dirname, new_filename, NULL);
		if ((file = g_fopen(path, "wb")) != NULL)
		{
			if (!fwrite(icon_data, icon_len, 1, file))
			{
				purple_debug_error("buddyicon", "Error writing %s: %s\n",
				                   path, strerror(errno));
			}
			else
				purple_debug_info("buddyicon", "Wrote migrated cache file: %s\n", path);

			fclose(file);
		}
		else
		{
			purple_debug_error("buddyicon", "Unable to create file %s: %s\n",
			                   path, strerror(errno));
			g_free(new_filename);
			g_free(path);
			return;
		}
		g_free(path);

		purple_blist_node_set_string(node,
		                             setting_name,
		                             new_filename);
		ref_filename(new_filename);

		g_free(new_filename);
	}
	else
	{
		/* If the icon is gone, drop the setting... */
		purple_blist_node_remove_setting(node,
		                                 setting_name);
		g_free(path);
	}
}

void
purple_buddy_icons_blist_loaded_cb()
{
	PurpleBlistNode *node = purple_blist_get_root();
	const char *dirname = purple_buddy_icons_get_cache_dir();

	// TODO: TEMP
	old_icons_dir = g_strdup("/home/rlaager/.gaim/icons");

	/* Doing this once here saves having to check it inside a loop. */
	if (old_icons_dir != NULL)
	{
		if (!g_file_test(dirname, G_FILE_TEST_IS_DIR))
		{
			purple_debug_info("buddyicon", "Creating icon cache directory.\n");

			if (g_mkdir(dirname, S_IRUSR | S_IWUSR | S_IXUSR) < 0)
			{
				purple_debug_error("buddyicon",
				                   "Unable to create directory %s: %s\n",
				                   dirname, strerror(errno));
			}
		}
	}

        while (node != NULL)
	{
		if (PURPLE_BLIST_NODE_IS_BUDDY(node))
		{
			const char *filename;

			filename = purple_blist_node_get_string(node, "buddy_icon");
			if (filename != NULL)
			{
				if (old_icons_dir != NULL)
				{
					migrate_buddy_icon(node,
					                   "buddy_icon",
					                   dirname, filename);
				}
				else
				{
					if (!g_file_test(filename, G_FILE_TEST_EXISTS))
					{
						purple_blist_node_remove_setting(node,
						                                 "buddy_icon");
					}
					ref_filename(filename);
				}
			}
		}
		else if (PURPLE_BLIST_NODE_IS_CONTACT(node))
		{
			const char *filename;

			filename = purple_blist_node_get_string(node, "custom_buddy_icon");
			if (filename != NULL)
			{
				if (old_icons_dir != NULL)
				{
					migrate_buddy_icon(node,
					                   "custom_buddy_icon",
					                    dirname, filename);
				}
				else
				{
					if (!g_file_test(filename, G_FILE_TEST_EXISTS))
					{
						purple_blist_node_remove_setting(node,
						                                 "custom_buddy_icon");
					}
					ref_filename(filename);
				}
			}
		}
		node = purple_blist_node_next(node, TRUE);
        }
}

void
purple_buddy_icons_set_caching(gboolean caching)
{
	icon_caching = caching;
}

gboolean
purple_buddy_icons_is_caching(void)
{
	return icon_caching;
}

void
purple_buddy_icons_set_cache_dir(const char *dir)
{
	g_return_if_fail(dir != NULL);

	g_free(cache_dir);
	cache_dir = g_strdup(dir);
}

const char *
purple_buddy_icons_get_cache_dir(void)
{
	return cache_dir;
}

// TODO: Deal with this
char *purple_buddy_icons_get_full_path(const char *icon) {
	if (icon == NULL)
		return NULL;

	if (g_file_test(icon, G_FILE_TEST_IS_REGULAR))
		return g_strdup(icon);
	else
		return g_build_filename(purple_buddy_icons_get_cache_dir(), icon, NULL);
}

void *
purple_buddy_icons_get_handle()
{
	static int handle;

	return &handle;
}

void
purple_buddy_icons_init()
{
	account_cache = g_hash_table_new_full(
		g_direct_hash, g_direct_equal,
		NULL, (GFreeFunc)g_hash_table_destroy);

	icon_data_cache = g_hash_table_new(g_str_hash, g_str_equal);
	icon_file_cache = g_hash_table_new_full(g_str_hash, g_str_equal,
	                                        g_free, NULL);
	custom_icon_cache = g_hash_table_new(g_direct_hash, g_direct_equal);

	cache_dir = g_build_filename(purple_user_dir(), "icons", NULL);

	purple_signal_connect(purple_imgstore_get_handle(), "image-deleting",
	                      purple_buddy_icons_get_handle(),
	                      G_CALLBACK(image_deleting_cb), NULL);
}

void
purple_buddy_icons_uninit()
{
	purple_signals_disconnect_by_handle(purple_buddy_icons_get_handle());

	g_hash_table_destroy(account_cache);
	g_hash_table_destroy(icon_data_cache);
	g_hash_table_destroy(icon_file_cache);
	g_hash_table_destroy(custom_icon_cache);
	g_free(old_icons_dir);
}

void purple_buddy_icon_get_scale_size(PurpleBuddyIconSpec *spec, int *width, int *height)
{
	int new_width, new_height;

	new_width = *width;
	new_height = *height;

	if (*width < spec->min_width)
		new_width = spec->min_width;
	else if (*width > spec->max_width)
		new_width = spec->max_width;

	if (*height < spec->min_height)
		new_height = spec->min_height;
	else if (*height > spec->max_height)
		new_height = spec->max_height;

	/* preserve aspect ratio */
	if ((double)*height * (double)new_width >
		(double)*width * (double)new_height) {
			new_width = 0.5 + (double)*width * (double)new_height / (double)*height;
	} else {
			new_height = 0.5 + (double)*height * (double)new_width / (double)*width;
	}

	*width = new_width;
	*height = new_height;
}
