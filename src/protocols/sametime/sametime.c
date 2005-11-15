
/*
  Meanwhile Protocol Plugin for Gaim
  Adds Lotus Sametime support to Gaim using the Meanwhile library

  Copyright (C) 2004 Christopher (siege) O'Brien <siege@preoccupied.net>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at
  your option) any later version.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
  USA.
*/


/* system includes */
#include <stdlib.h>
#include <time.h>

/* glib includes */
#include <glib.h>
#include <glib/ghash.h>
#include <glib/glist.h>

/* gaim includes */
#include <internal.h>
#include <gaim.h>
#include <config.h>

#include <account.h>
#include <accountopt.h>
#include <conversation.h>
#include <debug.h>
#include <ft.h>
#include <imgstore.h>
#include <mime.h>
#include <notify.h>
#include <plugin.h>
#include <privacy.h>
#include <prpl.h>
#include <request.h>
#include <util.h>
#include <version.h>

/* meanwhile includes */
#include <mw_cipher.h>
#include <mw_common.h>
#include <mw_error.h>
#include <mw_service.h>
#include <mw_session.h>
#include <mw_srvc_aware.h>
#include <mw_srvc_conf.h>
#include <mw_srvc_ft.h>
#include <mw_srvc_im.h>
#include <mw_srvc_place.h>
#include <mw_srvc_resolve.h>
#include <mw_srvc_store.h>
#include <mw_st_list.h>

/* project includes */
#include "sametime.h"


/* considering that there's no display of this information for prpls,
   I don't know why I even bother providing these. Oh valiant reader,
   I do it all for you. */
/* scratch that, I just added it to the prpl options panel */
#define PLUGIN_ID        "prpl-meanwhile"
#define PLUGIN_NAME      "Sametime"
#define PLUGIN_SUMMARY   "Sametime Protocol Plugin"
#define PLUGIN_DESC      "Open implementation of a Lotus Sametime client"
#define PLUGIN_AUTHOR    "Christopher (siege) O'Brien <siege@preoccupied.net>"
#define PLUGIN_HOMEPAGE  "http://meanwhile.sourceforge.net/"


/* plugin preference names */
#define MW_PRPL_OPT_BASE          "/plugins/prpl/meanwhile"
#define MW_PRPL_OPT_BLIST_ACTION  MW_PRPL_OPT_BASE "/blist_action"
#define MW_PRPL_OPT_PSYCHIC       MW_PRPL_OPT_BASE "/psychic"
#define MW_PRPL_OPT_FORCE_LOGIN   MW_PRPL_OPT_BASE "/force_login"
#define MW_PRPL_OPT_SAVE_DYNAMIC  MW_PRPL_OPT_BASE "/save_dynamic"


/* stages of connecting-ness */
#define MW_CONNECT_STEPS  9


/* stages of conciousness */
#define MW_STATE_OFFLINE      "offline"
#define MW_STATE_ACTIVE       "active"
#define MW_STATE_AWAY         "away"
#define MW_STATE_BUSY         "dnd"
#define MW_STATE_MESSAGE      "message"
#define MW_STATE_ENLIGHTENED  "buddha"


/* keys to get/set chat information */
#define CHAT_KEY_CREATOR   "chat.creator"
#define CHAT_KEY_NAME      "chat.name"
#define CHAT_KEY_TOPIC     "chat.topic"
#define CHAT_KEY_INVITE    "chat.invite"
#define CHAT_KEY_IS_PLACE  "chat.is_place"


/* key for associating a mwLoginType with a buddy */
#define BUDDY_KEY_CLIENT  "meanwhile.client"

/* store the remote alias so that we can re-create it easily */
#define BUDDY_KEY_NAME    "meanwhile.shortname"

/* enum mwSametimeUserType */
#define BUDDY_KEY_TYPE    "meanwhile.type"


/* key for the real group name for a meanwhile group */
#define GROUP_KEY_NAME    "meanwhile.group"

/* enum mwSametimeGroupType */
#define GROUP_KEY_TYPE    "meanwhile.type"

/* NAB group owning account */
#define GROUP_KEY_OWNER   "meanwhile.account"

/* key gtk blist uses to indicate a collapsed group */
#define GROUP_KEY_COLLAPSED  "collapsed"


/* verification replacement */
#define mwSession_NO_SECRET  "meanwhile.no_secret"


/* keys to get/set gaim plugin information */
#define MW_KEY_HOST        "server"
#define MW_KEY_PORT        "port"
#define MW_KEY_ACTIVE_MSG  "active_msg"
#define MW_KEY_AWAY_MSG    "away_msg"
#define MW_KEY_BUSY_MSG    "busy_msg"
#define MW_KEY_MSG_PROMPT  "msg_prompt"
#define MW_KEY_INVITE      "conf_invite"
#define MW_KEY_ENCODING    "encoding"
#define MW_KEY_FORCE       "force_login"


/** number of seconds from the first blist change before a save to the
    storage service occurs. */
#define BLIST_SAVE_SECONDS  15


/** the possible buddy list storage settings */
enum blist_choice {
  blist_choice_LOCAL = 1, /**< local only */
  blist_choice_MERGE = 2, /**< merge from server */
  blist_choice_STORE = 3, /**< merge from and save to server */
  blist_choice_SYNCH = 4, /**< sync with server */
};


/** the default blist storage option */
#define BLIST_CHOICE_DEFAULT  blist_choice_SYNCH


/* testing for the above */
#define BLIST_PREF_IS(n) (gaim_prefs_get_int(MW_PRPL_OPT_BLIST_ACTION)==(n))
#define BLIST_PREF_IS_LOCAL()  BLIST_PREF_IS(blist_choice_LOCAL)
#define BLIST_PREF_IS_MERGE()  BLIST_PREF_IS(blist_choice_MERGE)
#define BLIST_PREF_IS_STORE()  BLIST_PREF_IS(blist_choice_STORE)
#define BLIST_PREF_IS_SYNCH()  BLIST_PREF_IS(blist_choice_SYNCH)


/* debugging output */
#define DEBUG_ERROR(a...)  gaim_debug_error(G_LOG_DOMAIN, a)
#define DEBUG_INFO(a...)   gaim_debug_info(G_LOG_DOMAIN, a)
#define DEBUG_MISC(a...)   gaim_debug_misc(G_LOG_DOMAIN, a)
#define DEBUG_WARN(a...)   gaim_debug_warning(G_LOG_DOMAIN, a)


/** ensure non-null strings */
#ifndef NSTR
# define NSTR(str) ((str)? (str): "(null)")
#endif


/** calibrates distinct secure channel nomenclature */
static const unsigned char no_secret[] = {
  0x2d, 0x2d, 0x20, 0x73, 0x69, 0x65, 0x67, 0x65,
  0x20, 0x6c, 0x6f, 0x76, 0x65, 0x73, 0x20, 0x6a,
  0x65, 0x6e, 0x6e, 0x69, 0x20, 0x61, 0x6e, 0x64,
  0x20, 0x7a, 0x6f, 0x65, 0x20, 0x2d, 0x2d, 0x00,
};


/** handler IDs from g_log_set_handler in mw_plugin_init */
static guint log_handler[2] = { 0, 0 };


/** the gaim plugin data.
    available as gc->proto_data and mwSession_getClientData */
struct mwGaimPluginData {
  struct mwSession *session;

  struct mwServiceAware *srvc_aware;
  struct mwServiceConference *srvc_conf;
  struct mwServiceFileTransfer *srvc_ft;
  struct mwServiceIm *srvc_im;
  struct mwServicePlace *srvc_place;
  struct mwServiceResolve *srvc_resolve;
  struct mwServiceStorage *srvc_store;

  /** map of GaimGroup:mwAwareList and mwAwareList:GaimGroup */
  GHashTable *group_list_map;

  /** event id for the buddy list save callback */
  guint save_event;

  /** socket fd */
  int socket;

  GaimConnection *gc;
};


/* blist and aware functions */

static void blist_export(GaimConnection *gc, struct mwSametimeList *stlist);

static void blist_store(struct mwGaimPluginData *pd);

static void blist_schedule(struct mwGaimPluginData *pd);

static void blist_merge(GaimConnection *gc, struct mwSametimeList *stlist);

static void blist_sync(GaimConnection *gc, struct mwSametimeList *stlist);

static gboolean buddy_is_external(GaimBuddy *b);

static void buddy_add(struct mwGaimPluginData *pd, GaimBuddy *buddy);

static GaimBuddy *
buddy_ensure(GaimConnection *gc, GaimGroup *group,
	     struct mwSametimeUser *stuser);

static void group_add(struct mwGaimPluginData *pd, GaimGroup *group);

static GaimGroup *
group_ensure(GaimConnection *gc, struct mwSametimeGroup *stgroup);

static struct mwAwareList *
list_ensure(struct mwGaimPluginData *pd, GaimGroup *group);


/* session functions */

static struct mwSession *
gc_to_session(GaimConnection *gc);

static GaimConnection *session_to_gc(struct mwSession *session);


/* conference functions */

static struct mwConference *
conf_find_by_id(struct mwGaimPluginData *pd, int id);


/* conversation functions */

struct convo_msg {
  enum mwImSendType type;
  gpointer data;
  GDestroyNotify clear;
};


struct convo_data {
  struct mwConversation *conv;
  GList *queue;   /**< outgoing message queue, list of convo_msg */
};

static void convo_data_new(struct mwConversation *conv);

static void convo_data_free(struct convo_data *conv);

static void convo_features(struct mwConversation *conv);

static GaimConversation *convo_get_gconv(struct mwConversation *conv);


/* resolved id */

struct resolved_id {
  char *id;
  char *name;
};

static struct resolved_id *resolved_id_new(const char *id, const char *name);

static void resolved_id_free(struct resolved_id *rid);


/* connection functions */

static void connect_cb(gpointer data, gint source, GaimInputCondition cond);


/* ----- session ------ */


/** resolves a mwSession from a GaimConnection */
static struct mwSession *gc_to_session(GaimConnection *gc) {
  struct mwGaimPluginData *pd;
  
  g_return_val_if_fail(gc != NULL, NULL);
  
  pd = gc->proto_data;
  g_return_val_if_fail(pd != NULL, NULL);
  
  return pd->session;
}


/** resolves a GaimConnection from a mwSession */
static GaimConnection *session_to_gc(struct mwSession *session) {
  struct mwGaimPluginData *pd;

  g_return_val_if_fail(session != NULL, NULL);

  pd = mwSession_getClientData(session);
  g_return_val_if_fail(pd != NULL, NULL);

  return pd->gc;
}


static int mw_session_io_write(struct mwSession *session,
			       const char *buf, gsize len) {
  struct mwGaimPluginData *pd;
  int ret = 0;

  pd = mwSession_getClientData(session);

  /* socket was already closed. */
  if(pd->socket == 0)
    return 1;

  while(len) {
    ret = write(pd->socket, buf, len);
    if(ret <= 0) break;
    len -= ret;
  }

  if(len > 0) {
    DEBUG_ERROR("write returned %i, %i bytes left unwritten\n", ret, len);
    gaim_connection_error(pd->gc, _("Connection closed (writing)"));

#if 0
    close(pd->socket);
    pd->socket = 0;
#endif

    return -1;
  }

  return 0;
}


static void mw_session_io_close(struct mwSession *session) {
  struct mwGaimPluginData *pd;
  GaimConnection *gc;

  pd = mwSession_getClientData(session);
  g_return_if_fail(pd != NULL);

  gc = pd->gc;
  
  if(pd->socket) {
    close(pd->socket);
    pd->socket = 0;
  }
    
  if(gc->inpa) {
    gaim_input_remove(gc->inpa);
    gc->inpa = 0;
  }
}


static void mw_session_clear(struct mwSession *session) {
  ; /* nothing for now */
}


/* ----- aware list ----- */


static void blist_resolve_alias_cb(struct mwServiceResolve *srvc,
				   guint32 id, guint32 code, GList *results,
				   gpointer data) {
  struct mwResolveResult *result;
  struct mwResolveMatch *match;

  g_return_if_fail(results != NULL);

  result = results->data;
  g_return_if_fail(result != NULL);
  g_return_if_fail(result->matches != NULL);

  match = result->matches->data;
  g_return_if_fail(match != NULL);

  gaim_blist_server_alias_buddy(data, match->name);
  gaim_blist_node_set_string(data, BUDDY_KEY_NAME, match->name);
}


static void mw_aware_list_on_aware(struct mwAwareList *list,
				   struct mwAwareSnapshot *aware) {

  GaimConnection *gc;
  GaimAccount *acct;
    
  struct mwGaimPluginData *pd;
  time_t idle;
  guint stat;
  const char *id;
  const char *status = MW_STATE_ACTIVE;

  gc = mwAwareList_getClientData(list);
  acct = gaim_connection_get_account(gc);

  pd = gc->proto_data;
  idle = aware->status.time;
  stat = aware->status.status;
  id = aware->id.user;

  /* not sure which client sends this yet */
  if(idle == 0xdeadbeef) {
    /* knock knock!
       who's there?
       rude interrupting cow.
       rude interr...
       MOO! */
    idle = -1;
  }

  switch(stat) {
  case mwStatus_ACTIVE:
    status = MW_STATE_ACTIVE;
    idle = 0;
    break;

  case mwStatus_IDLE:
    if(! idle) idle = -1;
    break;
    
  case mwStatus_AWAY:
    status = MW_STATE_AWAY;
    break;
    
  case mwStatus_BUSY:
    status = MW_STATE_BUSY;
    break;
  }
  
  /* NAB group members */
  if(aware->group) {
    GaimGroup *group;
    GaimBuddy *buddy;
    GaimBlistNode *bnode;

    group = g_hash_table_lookup(pd->group_list_map, list);
    buddy = gaim_find_buddy_in_group(acct, id, group);
    bnode = (GaimBlistNode *) buddy;

    if(! buddy) {
      struct mwServiceResolve *srvc;
      GList *query;

      buddy = gaim_buddy_new(acct, id, NULL);
      gaim_blist_add_buddy(buddy, NULL, group, NULL);

      bnode = (GaimBlistNode *) buddy;

      srvc = pd->srvc_resolve;
      query = g_list_append(NULL, (char *) id);

      mwServiceResolve_resolve(srvc, query, mwResolveFlag_USERS,
			       blist_resolve_alias_cb, buddy, NULL);
      g_list_free(query);
    }

    gaim_blist_node_set_int(bnode, BUDDY_KEY_TYPE, mwSametimeUser_NORMAL);
  }
  
  gaim_prpl_got_user_status(acct, id, status, NULL);
  gaim_prpl_got_user_idle(acct, id, !!idle, idle);
}


static void mw_aware_list_on_attrib(struct mwAwareList *list,
				    struct mwAwareIdBlock *id,
				    struct mwAwareAttribute *attrib) {

  ; /* nothing. We'll get attribute data as we need it */
}


static void mw_aware_list_clear(struct mwAwareList *list) {
  ; /* nothing for now */
}


static struct mwAwareListHandler mw_aware_list_handler = {
  .on_aware = mw_aware_list_on_aware,
  .on_attrib = mw_aware_list_on_attrib,
  .clear = mw_aware_list_clear,
};


/** Ensures that an Aware List is associated with the given group, and
    returns that list. */
static struct mwAwareList *
list_ensure(struct mwGaimPluginData *pd, GaimGroup *group) {
  
  struct mwAwareList *list;
  
  g_return_val_if_fail(pd != NULL, NULL);
  g_return_val_if_fail(group != NULL, NULL);
  
  list = g_hash_table_lookup(pd->group_list_map, group);
  if(! list) {
    list = mwAwareList_new(pd->srvc_aware, &mw_aware_list_handler);
    mwAwareList_setClientData(list, pd->gc, NULL);
    
    mwAwareList_watchAttributes(list,
				mwAttribute_AV_PREFS_SET,
				mwAttribute_MICROPHONE,
				mwAttribute_SPEAKERS,
				mwAttribute_VIDEO_CAMERA,
				mwAttribute_FILE_TRANSFER,
				NULL);

    g_hash_table_replace(pd->group_list_map, group, list);
    g_hash_table_insert(pd->group_list_map, list, group);
  }
  
  return list;
}


static void blist_export(GaimConnection *gc, struct mwSametimeList *stlist) {
  /* - find the account for this connection
     - iterate through the buddy list
     - add each buddy matching this account to the stlist
  */

  GaimAccount *acct;
  GaimBuddyList *blist;
  GaimBlistNode *gn, *cn, *bn;
  GaimGroup *grp;
  GaimBuddy *bdy;

  struct mwSametimeGroup *stg = NULL;
  struct mwIdBlock idb = { NULL, NULL };

  acct = gaim_connection_get_account(gc);
  g_return_if_fail(acct != NULL);

  blist = gaim_get_blist();
  g_return_if_fail(blist != NULL);

  for(gn = blist->root; gn; gn = gn->next) {
    const char *owner;
    const char *gname;
    enum mwSametimeGroupType gtype;
    gboolean gopen;

    if(! GAIM_BLIST_NODE_IS_GROUP(gn)) continue;
    grp = (GaimGroup *) gn;

    /* the group's type (normal or dynamic) */
    gtype = gaim_blist_node_get_int(gn, GROUP_KEY_TYPE);
    if(! gtype) gtype = mwSametimeGroup_NORMAL;

    /* if it's a normal group with none of our people in it, skip it */
    if(gtype == mwSametimeGroup_NORMAL && !gaim_group_on_account(grp, acct))
      continue;
    
    /* if the group has an owner and we're not it, skip it */
    owner = gaim_blist_node_get_string(gn, GROUP_KEY_OWNER);
    if(owner && strcmp(owner, gaim_account_get_username(acct)))
      continue;

    /* the group's actual name may be different from the gaim group's
       name. Find whichever is there */
    gname = gaim_blist_node_get_string(gn, GROUP_KEY_NAME);
    if(! gname) gname = grp->name;

    /* we save this, but never actually honor it */
    gopen = ! gaim_blist_node_get_bool(gn, GROUP_KEY_COLLAPSED);

    stg = mwSametimeGroup_new(stlist, gtype, gname);
    mwSametimeGroup_setAlias(stg, grp->name);
    mwSametimeGroup_setOpen(stg, gopen);

    /* don't attempt to put buddies in a dynamic group, it breaks
       other clients */
    if(gtype == mwSametimeGroup_DYNAMIC)
      continue;

    for(cn = gn->child; cn; cn = cn->next) {
      if(! GAIM_BLIST_NODE_IS_CONTACT(cn)) continue;

      for(bn = cn->child; bn; bn = bn->next) {
	if(! GAIM_BLIST_NODE_IS_BUDDY(bn)) continue;
	if(! GAIM_BLIST_NODE_SHOULD_SAVE(bn)) continue;

	bdy = (GaimBuddy *) bn;

	if(bdy->account == acct) {
	  struct mwSametimeUser *stu;
	  enum mwSametimeUserType utype;

	  idb.user = bdy->name;

	  utype = gaim_blist_node_get_int(bn, BUDDY_KEY_TYPE);
	  if(! utype) utype = mwSametimeUser_NORMAL;

	  stu = mwSametimeUser_new(stg, utype, &idb);
	  mwSametimeUser_setShortName(stu, bdy->server_alias);
	  mwSametimeUser_setAlias(stu, bdy->alias);
	}
      }
    }
  }  
}


static void blist_store(struct mwGaimPluginData *pd) {

  struct mwSametimeList *stlist;
  struct mwServiceStorage *srvc;
  struct mwStorageUnit *unit;

  GaimConnection *gc;

  struct mwPutBuffer *b;
  struct mwOpaque *o;

  g_return_if_fail(pd != NULL);

  srvc = pd->srvc_store;
  g_return_if_fail(srvc != NULL);

  gc = pd->gc;

  if(BLIST_PREF_IS_LOCAL() || BLIST_PREF_IS_MERGE()) {
    DEBUG_INFO("preferences indicate not to save remote blist\n");
    return;

  } else if(MW_SERVICE_IS_DEAD(srvc)) {
    DEBUG_INFO("aborting save of blist: storage service is not alive\n");
    return;

  } else if(BLIST_PREF_IS_STORE() || BLIST_PREF_IS_SYNCH()) {
    DEBUG_INFO("saving remote blist\n");

  } else {
    g_return_if_reached();
  }

  /* create and export to a list object */
  stlist = mwSametimeList_new();
  blist_export(gc, stlist);

  /* write it to a buffer */
  b = mwPutBuffer_new();
  mwSametimeList_put(b, stlist);
  mwSametimeList_free(stlist);

  /* put the buffer contents into a storage unit */
  unit = mwStorageUnit_new(mwStore_AWARE_LIST);
  o = mwStorageUnit_asOpaque(unit);
  mwPutBuffer_finalize(o, b);

  /* save the storage unit to the service */
  mwServiceStorage_save(srvc, unit, NULL, NULL, NULL);
}


static gboolean blist_save_cb(gpointer data) {
  struct mwGaimPluginData *pd = data;

  blist_store(pd);
  pd->save_event = 0;
  return FALSE;
}


/** schedules the buddy list to be saved to the server */
static void blist_schedule(struct mwGaimPluginData *pd) {
  if(pd->save_event) return;

  pd->save_event = gaim_timeout_add(BLIST_SAVE_SECONDS * 1000,
				    blist_save_cb, pd);
}


static gboolean buddy_is_external(GaimBuddy *b) {
  g_return_val_if_fail(b != NULL, FALSE);
  return g_str_has_prefix(b->name, "@E ");
}


/** Actually add a buddy to the aware service, and schedule the buddy
    list to be saved to the server */
static void buddy_add(struct mwGaimPluginData *pd,
		      GaimBuddy *buddy) {

  struct mwAwareIdBlock idb = { mwAware_USER, (char *) buddy->name, NULL };
  struct mwAwareList *list;

  GaimGroup *group;
  GList *add;

  add = g_list_prepend(NULL, &idb);

  group = gaim_buddy_get_group(buddy);
  list = list_ensure(pd, group);

  if(mwAwareList_addAware(list, add)) {
    gaim_blist_remove_buddy(buddy);
  }

  blist_schedule(pd);

  g_list_free(add);  
}


/** ensure that a GaimBuddy exists in the group with data
    appropriately matching the st user entry from the st list */
static GaimBuddy *buddy_ensure(GaimConnection *gc, GaimGroup *group,
			       struct mwSametimeUser *stuser) {

  struct mwGaimPluginData *pd = gc->proto_data;
  GaimBuddy *buddy;
  GaimAccount *acct = gaim_connection_get_account(gc);

  const char *id = mwSametimeUser_getUser(stuser);
  const char *name = mwSametimeUser_getShortName(stuser);
  const char *alias = mwSametimeUser_getAlias(stuser);
  enum mwSametimeUserType type = mwSametimeUser_getType(stuser);

  g_return_val_if_fail(id != NULL, NULL);
  g_return_val_if_fail(strlen(id) > 0, NULL);

  buddy = gaim_find_buddy_in_group(acct, id, group);
  if(! buddy) {
    buddy = gaim_buddy_new(acct, id, alias);
  
    gaim_blist_add_buddy(buddy, NULL, group, NULL);
    buddy_add(pd, buddy);
  }
  
  gaim_blist_alias_buddy(buddy, alias);
  gaim_blist_server_alias_buddy(buddy, name);
  gaim_blist_node_set_string((GaimBlistNode *) buddy, BUDDY_KEY_NAME, name);
  gaim_blist_node_set_int((GaimBlistNode *) buddy, BUDDY_KEY_TYPE, type);

  return buddy;
}


/** add aware watch for a dynamic group */
static void group_add(struct mwGaimPluginData *pd,
		      GaimGroup *group) {

  struct mwAwareIdBlock idb = { mwAware_GROUP, NULL, NULL };
  struct mwAwareList *list;
  const char *n;
  GList *add;
  
  n = gaim_blist_node_get_string((GaimBlistNode *) group, GROUP_KEY_NAME);
  if(! n) n = group->name;

  idb.user = (char *) n;
  add = g_list_prepend(NULL, &idb);

  list = list_ensure(pd, group);
  mwAwareList_addAware(list, add);
  g_list_free(add);
}


/** ensure that a GaimGroup exists in the blist with data
    appropriately matching the st group entry from the st list */
static GaimGroup *group_ensure(GaimConnection *gc,
			       struct mwSametimeGroup *stgroup) {
  GaimAccount *acct;
  GaimGroup *group = NULL;
  GaimBuddyList *blist;
  GaimBlistNode *gn;
  const char *name, *alias, *owner;
  enum mwSametimeGroupType type;

  acct = gaim_connection_get_account(gc);
  owner = gaim_account_get_username(acct);

  blist = gaim_get_blist();
  g_return_val_if_fail(blist != NULL, NULL);

  name = mwSametimeGroup_getName(stgroup);
  alias = mwSametimeGroup_getAlias(stgroup);
  type = mwSametimeGroup_getType(stgroup);

  /* first attempt at finding the group, by the name key */
  for(gn = blist->root; gn; gn = gn->next) {
    const char *n;
    if(! GAIM_BLIST_NODE_IS_GROUP(gn)) continue;
    n = gaim_blist_node_get_string(gn, GROUP_KEY_NAME);

    if(n && !strcmp(n, name)) {
      group = (GaimGroup *) gn;
      break;
    }
  }  

  /* try again, by alias */
  if(! group) group = gaim_find_group(alias);

  /* oh well, no such group. Let's create it! */
  if(! group) {
    group = gaim_group_new(alias);
    gaim_blist_add_group(group, NULL);
  }

  gn = (GaimBlistNode *) group;
  gaim_blist_node_set_string(gn, GROUP_KEY_NAME, name);
  gaim_blist_node_set_int(gn, GROUP_KEY_TYPE, type);

  if(type == mwSametimeGroup_DYNAMIC) {
    gaim_blist_node_set_string(gn, GROUP_KEY_OWNER, owner);
    group_add(gc->proto_data, group);
  }
  
  return group;
}


/** merge the entries from a st list into the gaim blist */
static void blist_merge(GaimConnection *gc, struct mwSametimeList *stlist) {
  struct mwSametimeGroup *stgroup;
  struct mwSametimeUser *stuser;

  GaimGroup *group;
  GaimBuddy *buddy;

  GList *gl, *gtl, *ul, *utl;

  gl = gtl = mwSametimeList_getGroups(stlist);
  for(; gl; gl = gl->next) {

    stgroup = (struct mwSametimeGroup *) gl->data;
    group = group_ensure(gc, stgroup);

    ul = utl = mwSametimeGroup_getUsers(stgroup);
    for(; ul; ul = ul->next) {

      stuser = (struct mwSametimeUser *) ul->data;
      buddy = buddy_ensure(gc, group, stuser);
    }
    g_list_free(utl);
  }
  g_list_free(gtl);
}


/** remove all buddies on account from group. If del is TRUE and group
    is left empty, remove group as well */
static void group_clear(GaimGroup *group, GaimAccount *acct, gboolean del) {
  GaimConnection *gc;
  GList *prune = NULL;
  GaimBlistNode *gn, *cn, *bn;

  g_return_if_fail(group != NULL);

  DEBUG_INFO("clearing members from pruned group %s\n", NSTR(group->name));

  gc = gaim_account_get_connection(acct);
  g_return_if_fail(gc != NULL);

  gn = (GaimBlistNode *) group;

  for(cn = gn->child; cn; cn = cn->next) {
    if(! GAIM_BLIST_NODE_IS_CONTACT(cn)) continue;

    for(bn = cn->child; bn; bn = bn->next) {
      GaimBuddy *gb = (GaimBuddy *) bn;

      if(! GAIM_BLIST_NODE_IS_BUDDY(bn)) continue;
      
      if(gb->account == acct) {
	DEBUG_INFO("clearing %s from group\n", NSTR(gb->name));
	prune = g_list_prepend(prune, gb);
      }
    }
  }

  /* quickly unsubscribe from presence for the entire group */
  gaim_account_remove_group(acct, group);

  /* remove blist entries that need to go */
  while(prune) {
    gaim_blist_remove_buddy(prune->data);
    prune = g_list_delete_link(prune, prune);
  }
  DEBUG_INFO("cleared buddies\n");

  /* optionally remove group from blist */
  if(del && !gaim_blist_get_group_size(group, TRUE)) {
    DEBUG_INFO("removing empty group\n");
    gaim_blist_remove_group(group);
  }
}


/** prune out group members that shouldn't be there */
static void group_prune(GaimConnection *gc, GaimGroup *group,
			struct mwSametimeGroup *stgroup) {

  GaimAccount *acct;
  GaimBlistNode *gn, *cn, *bn;
  
  GHashTable *stusers;
  GList *prune = NULL;
  GList *ul, *utl;

  g_return_if_fail(group != NULL);

  DEBUG_INFO("pruning membership of group %s\n", NSTR(group->name));

  acct = gaim_connection_get_account(gc);
  g_return_if_fail(acct != NULL);

  stusers = g_hash_table_new(g_str_hash, g_str_equal);
  
  /* build a hash table for quick lookup while pruning the group
     contents */
  utl = mwSametimeGroup_getUsers(stgroup);
  for(ul = utl; ul; ul = ul->next) {
    const char *id = mwSametimeUser_getUser(ul->data);
    g_hash_table_insert(stusers, (char *) id, ul->data);
    DEBUG_INFO("server copy has %s\n", NSTR(id));
  }
  g_list_free(utl);

  gn = (GaimBlistNode *) group;

  for(cn = gn->child; cn; cn = cn->next) {
    if(! GAIM_BLIST_NODE_IS_CONTACT(cn)) continue;

    for(bn = cn->child; bn; bn = bn->next) {
      GaimBuddy *gb = (GaimBuddy *) bn;

      if(! GAIM_BLIST_NODE_IS_BUDDY(bn)) continue;

      /* if the account is correct and they're not in our table, mark
	 them for pruning */
      if(gb->account == acct && !g_hash_table_lookup(stusers, gb->name)) {
	DEBUG_INFO("marking %s for pruning\n", NSTR(gb->name));
	prune = g_list_prepend(prune, gb);
      }
    }
  }
  DEBUG_INFO("done marking\n");

  g_hash_table_destroy(stusers);

  if(prune) {
    gaim_account_remove_buddies(acct, prune, NULL);
    while(prune) {
      gaim_blist_remove_buddy(prune->data);
      prune = g_list_delete_link(prune, prune);
    }
  }
}


/** synch the entries from a st list into the gaim blist, removing any
    existing buddies that aren't in the st list */
static void blist_sync(GaimConnection *gc, struct mwSametimeList *stlist) {

  GaimAccount *acct;
  GaimBuddyList *blist;
  GaimBlistNode *gn;

  GHashTable *stgroups;
  GList *g_prune = NULL;

  GList *gl, *gtl;

  const char *acct_n;

  DEBUG_INFO("synchronizing local buddy list from server list\n");

  acct = gaim_connection_get_account(gc);
  g_return_if_fail(acct != NULL);

  acct_n = gaim_account_get_username(acct);

  blist = gaim_get_blist();
  g_return_if_fail(blist != NULL);

  /* build a hash table for quick lookup while pruning the local
     list, mapping group name to group structure */
  stgroups = g_hash_table_new(g_str_hash, g_str_equal);

  gtl = mwSametimeList_getGroups(stlist);
  for(gl = gtl; gl; gl = gl->next) {
    const char *name = mwSametimeGroup_getName(gl->data);
    g_hash_table_insert(stgroups, (char *) name, gl->data);
  }
  g_list_free(gtl);

  /* find all groups which should be pruned from the local list */
  for(gn = blist->root; gn; gn = gn->next) {
    GaimGroup *grp = (GaimGroup *) gn;
    const char *gname, *owner;
    struct mwSametimeGroup *stgrp;

    if(! GAIM_BLIST_NODE_IS_GROUP(gn)) continue;

    /* group not belonging to this account */
    if(! gaim_group_on_account(grp, acct))
      continue;

    /* dynamic group belonging to this account. don't prune contents */
    owner = gaim_blist_node_get_string(gn, GROUP_KEY_OWNER);
    if(owner && !strcmp(owner, acct_n))
       continue;

    /* we actually are synching by this key as opposed to the group
       title, which can be different things in the st list */
    gname = gaim_blist_node_get_string(gn, GROUP_KEY_NAME);
    if(! gname) gname = grp->name;

    stgrp = g_hash_table_lookup(stgroups, gname);
    if(! stgrp) {
      /* remove the whole group */
      DEBUG_INFO("marking group %s for pruning\n", grp->name);
      g_prune = g_list_prepend(g_prune, grp);

    } else {
      /* synch the group contents */
      group_prune(gc, grp, stgrp);
    }
  }
  DEBUG_INFO("done marking groups\n");

  /* don't need this anymore */
  g_hash_table_destroy(stgroups);

  /* prune all marked groups */
  while(g_prune) {
    GaimGroup *grp = g_prune->data;
    GaimBlistNode *gn = (GaimBlistNode *) grp;
    const char *owner;
    gboolean del = TRUE;

    owner = gaim_blist_node_get_string(gn, GROUP_KEY_OWNER);
    if(owner && strcmp(owner, acct_n)) {
      /* it's a specialty group belonging to another account with some
	 of our members in it, so don't fully delete it */
      del = FALSE;
    }
    
    group_clear(g_prune->data, acct, del);
    g_prune = g_list_delete_link(g_prune, g_prune);
  }

  /* done with the pruning, let's merge in the additions */
  blist_merge(gc, stlist);
}


/** callback passed to the storage service when it's told to load the
    st list */
static void fetch_blist_cb(struct mwServiceStorage *srvc,
			   guint32 result, struct mwStorageUnit *item,
			   gpointer data) {

  struct mwGaimPluginData *pd = data;
  struct mwSametimeList *stlist;
  struct mwSession *s;

  struct mwGetBuffer *b;

  g_return_if_fail(result == ERR_SUCCESS);

  /* check our preferences for loading */
  if(BLIST_PREF_IS_LOCAL()) {
    DEBUG_INFO("preferences indicate not to load remote buddy list\n");
    return;
  }

  b = mwGetBuffer_wrap(mwStorageUnit_asOpaque(item));

  stlist = mwSametimeList_new();
  mwSametimeList_get(b, stlist);

  s = mwService_getSession(MW_SERVICE(srvc));

  /* merge or synch depending on preferences */
  if(BLIST_PREF_IS_MERGE() || BLIST_PREF_IS_STORE()) {
    blist_merge(pd->gc, stlist);

  } else if(BLIST_PREF_IS_SYNCH()) {
    blist_sync(pd->gc, stlist);
  }

  mwSametimeList_free(stlist);
}


/** callback passed to the storage service when it's told to load one
    of the default status messages */
static void fetch_msg_cb(struct mwServiceStorage *srvc,
			 guint32 result, struct mwStorageUnit *item,
			 gpointer data) {

  struct mwGaimPluginData *pd = data;
  GaimConnection *gc;
  GaimAccount *acct;
  struct mwSession *session;
  char *msg, *m;

  /* it's no big deal if these entries don't exist on the server */
  if(result != ERR_SUCCESS) return;

  g_return_if_fail(pd != NULL);

  gc = pd->gc;
  g_return_if_fail(gc != NULL);

  acct = gaim_connection_get_account(gc);
  g_return_if_fail(acct != NULL);

  session = pd->session;
  g_return_if_fail(session != NULL);

  m = msg = mwStorageUnit_asString(item);

  /* only load the first (non-empty) line of the collection of
     status messages */
  if(m && *m) {
    while(*m && isspace(*m)) m++;
    if(*m) {
      char *tail;

      tail = strchr(m, '\r');
      if(tail) *tail = '\0';
      tail = strchr(m, '\n');
      if(tail) *tail = '\0';
    }
  }

  switch(mwStorageUnit_getKey(item)) {
  case mwStore_AWAY_MESSAGES:
    DEBUG_INFO("setting away message to \"%s\"\n", NSTR(m));
    gaim_account_set_string(acct, MW_KEY_AWAY_MSG, m);
    break;

  case mwStore_BUSY_MESSAGES:
    DEBUG_INFO("setting busy message to \"%s\"\n", NSTR(m));
    gaim_account_set_string(acct, MW_KEY_BUSY_MSG, m);
    break;

  case mwStore_ACTIVE_MESSAGES:
    DEBUG_INFO("setting active message to \"%s\"\n", NSTR(m));
    gaim_account_set_string(acct, MW_KEY_ACTIVE_MSG, m);
    break;

  default:
    g_free(msg);
    g_return_if_reached();
  }

  g_free(msg);
  msg = NULL;

#if 0
  /* XXX resets the status, thus updating the message */
  if(!gc->away_state || !strcmp(gc->away_state, MW_STATE_ACTIVE)) {
    msg = MW_STATE_ACTIVE;
  } else if(gc->away_state && !strcmp(gc->away_state, MW_STATE_AWAY)) {
    msg = MW_STATE_AWAY;
  } else if(gc->away_state && !strcmp(gc->away_state, MW_STATE_BUSY)) {
    msg = MW_STATE_BUSY;
  }

  if(msg) serv_set_away(gc, msg, NULL);
#endif
}


/** signal triggered when a conversation is opened in Gaim */
static void conversation_created_cb(GaimConversation *g_conv,
				    struct mwGaimPluginData *pd) {

  /* we need to tell the IM service to negotiate features for the
     conversation right away, otherwise it'll wait until the first
     message is sent before offering NotesBuddy features. Therefore
     whenever Gaim creates a conversation, we'll immediately open the
     channel to the other side and figure out what the target can
     handle. Unfortunately, this makes us vulnerable to Psychic Mode,
     whereas a more lazy negotiation based on the first message
     would not */

  GaimConnection *gc;
  struct mwIdBlock who = { 0, 0 };
  struct mwConversation *conv;

  gc = gaim_conversation_get_gc(g_conv);
  if(pd->gc != gc)
    return; /* not ours */

  if(gaim_conversation_get_type(g_conv) != GAIM_CONV_TYPE_IM)
    return; /* wrong type */

  who.user = (char *) gaim_conversation_get_name(g_conv);
  conv = mwServiceIm_getConversation(pd->srvc_im, &who);

  convo_features(conv);
    
  if(mwConversation_isClosed(conv))
    mwConversation_open(conv);
}


static void blist_menu_nab(GaimBlistNode *node, gpointer data) {
  struct mwGaimPluginData *pd = data;
  GaimConnection *gc;

  GaimGroup *group = (GaimGroup *) node;

  GString *str;
  char *tmp;

  g_return_if_fail(pd != NULL);

  gc = pd->gc;
  g_return_if_fail(gc != NULL);

  g_return_if_fail(GAIM_BLIST_NODE_IS_GROUP(node));

  str = g_string_new(NULL);

  tmp = (char *) gaim_blist_node_get_string(node, GROUP_KEY_NAME);

  g_string_append_printf(str, _("<b>Group Title:</b> %s<br>"), group->name);
  g_string_append_printf(str, _("<b>Notes Group ID:</b> %s<br>"), tmp);

  tmp = g_strdup_printf(_("Info for Group %s"), group->name);

  gaim_notify_formatted(gc, tmp, _("Notes Address Book Information"),
			NULL, str->str, NULL, NULL);

  g_free(tmp);
  g_string_free(str, TRUE);
}


/** The normal blist menu prpl function doesn't get called for groups,
    so we use the blist-node-extended-menu signal to trigger this
    handler */
static void blist_node_menu_cb(GaimBlistNode *node,
			       GList **menu, struct mwGaimPluginData *pd) {
  const char *owner;
  GaimGroup *group;
  GaimAccount *acct;
  GaimBlistNodeAction *act;

  /* we only want groups */
  if(! GAIM_BLIST_NODE_IS_GROUP(node)) return;
  group = (GaimGroup *) node;

  acct = gaim_connection_get_account(pd->gc);
  g_return_if_fail(acct != NULL);

  /* better make sure we're connected */
  if(! gaim_account_is_connected(acct)) return;

#if 0
  /* if there's anyone in the group for this acct, offer to invite
     them all to a conference */
  if(gaim_group_on_account(group, acct)) {
    act = gaim_blist_node_action_new(_("Invite Group to Conference..."),
				     blist_menu_group_invite, pd, NULL);
    *menu = g_list_append(*menu, NULL);
  }
#endif

  /* check if it's a NAB group for this account */
  owner = gaim_blist_node_get_string(node, GROUP_KEY_OWNER);
  if(owner && !strcmp(owner, gaim_account_get_username(acct))) {
    act = gaim_blist_node_action_new(_("Get Notes Address Book Info"),
				     blist_menu_nab, pd, NULL);
    *menu = g_list_append(*menu, act);
  }
}


/** Last thing to happen from a started session */
static void services_starting(struct mwGaimPluginData *pd) {

  GaimConnection *gc;
  GaimAccount *acct;
  struct mwStorageUnit *unit;
  GaimBuddyList *blist;
  GaimBlistNode *l;

  gc = pd->gc;
  acct = gaim_connection_get_account(gc);

  /* grab the buddy list from the server */
  unit = mwStorageUnit_new(mwStore_AWARE_LIST);
  mwServiceStorage_load(pd->srvc_store, unit, fetch_blist_cb, pd, NULL); 
  
  /* fetch the away/busy/active messages from the server */
  unit = mwStorageUnit_new(mwStore_AWAY_MESSAGES);
  mwServiceStorage_load(pd->srvc_store, unit, fetch_msg_cb, pd, NULL);

  unit = mwStorageUnit_new(mwStore_BUSY_MESSAGES);
  mwServiceStorage_load(pd->srvc_store, unit, fetch_msg_cb, pd, NULL);

  unit = mwStorageUnit_new(mwStore_ACTIVE_MESSAGES);
  mwServiceStorage_load(pd->srvc_store, unit, fetch_msg_cb, pd, NULL);

  /* start watching for new conversations */
  gaim_signal_connect(gaim_conversations_get_handle(),
		      "conversation-created", gc,
		      GAIM_CALLBACK(conversation_created_cb), pd);

  /* watch for group extended menu items */
  gaim_signal_connect(gaim_blist_get_handle(),
		      "blist-node-extended-menu", gc,
		      GAIM_CALLBACK(blist_node_menu_cb), pd);

  /* find all the NAB groups and subscribe to them */
  blist = gaim_get_blist();
  for(l = blist->root; l; l = l->next) {
    GaimGroup *group = (GaimGroup *) l;
    enum mwSametimeGroupType gt;
    const char *owner;

    if(! GAIM_BLIST_NODE_IS_GROUP(l)) continue;

    /* if the group is ownerless, or has an owner and we're not it,
       skip it */
    owner = gaim_blist_node_get_string(l, GROUP_KEY_OWNER);
    if(!owner || strcmp(owner, gaim_account_get_username(acct)))
      continue;

    gt = gaim_blist_node_get_int(l, GROUP_KEY_TYPE);
    if(gt == mwSametimeGroup_DYNAMIC)
      group_add(pd, group);
  }

  /* set the aware attributes */
  /* indicate we understand what AV prefs are, but don't support any */
  mwServiceAware_setAttributeBoolean(pd->srvc_aware,
				     mwAttribute_AV_PREFS_SET, TRUE);
  mwServiceAware_unsetAttribute(pd->srvc_aware, mwAttribute_MICROPHONE);
  mwServiceAware_unsetAttribute(pd->srvc_aware, mwAttribute_SPEAKERS);
  mwServiceAware_unsetAttribute(pd->srvc_aware, mwAttribute_VIDEO_CAMERA);

  /* ... but we can do file transfers! */
  mwServiceAware_setAttributeBoolean(pd->srvc_aware,
				     mwAttribute_FILE_TRANSFER, TRUE);
}


static void session_loginRedirect(struct mwSession *session,
				  const char *host) {
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  GaimAccount *account;
  guint port;

  pd = mwSession_getClientData(session);
  gc = pd->gc;
  account = gaim_connection_get_account(gc);
  port = gaim_account_get_int(account, MW_KEY_PORT, MW_PLUGIN_DEFAULT_PORT);

  if(gaim_account_get_bool(account, MW_KEY_FORCE, FALSE) ||
     gaim_proxy_connect(account, host, port, connect_cb, pd)) {

    mwSession_forceLogin(session);
  }
}


/** called from mw_session_stateChange when the session's state is
    mwSession_STARTED. Any finalizing of start-up stuff should go
    here */
static void session_started(struct mwGaimPluginData *pd) {

  /* XXX setup status */

  /* use our services to do neat things */
  services_starting(pd);
}


static void mw_session_stateChange(struct mwSession *session,
				   enum mwSessionState state,
				   gpointer info) {
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  char *msg = NULL;

  pd = mwSession_getClientData(session);
  gc = pd->gc;

  switch(state) {
  case mwSession_STARTING:
    msg = _("Sending Handshake");
    gaim_connection_update_progress(gc, msg, 2, MW_CONNECT_STEPS);
    break;

  case mwSession_HANDSHAKE:
    msg = _("Waiting for Handshake Acknowledgement");
    gaim_connection_update_progress(gc, msg, 3, MW_CONNECT_STEPS);
    break;

  case mwSession_HANDSHAKE_ACK:
    msg = _("Handshake Acknowledged, Sending Login");
    gaim_connection_update_progress(gc, msg, 4, MW_CONNECT_STEPS);
    break;

  case mwSession_LOGIN:
    msg = _("Waiting for Login Acknowledgement");
    gaim_connection_update_progress(gc, msg, 5, MW_CONNECT_STEPS);
    break;

  case mwSession_LOGIN_REDIR:
    msg = _("Login Redirected");
    gaim_connection_update_progress(gc, msg, 6, MW_CONNECT_STEPS);
    session_loginRedirect(session, info);
    break;

  case mwSession_LOGIN_CONT:
    msg = _("Forcing Login");
    gaim_connection_update_progress(gc, msg, 7, MW_CONNECT_STEPS);

  case mwSession_LOGIN_ACK:
    msg = _("Login Acknowledged");
    gaim_connection_update_progress(gc, msg, 8, MW_CONNECT_STEPS);
    break;

  case mwSession_STARTED:
    msg = _("Connected to Sametime Community Server");
    gaim_connection_update_progress(gc, msg, 9, MW_CONNECT_STEPS);
    gaim_connection_set_state(gc, GAIM_CONNECTED);

    session_started(pd);
    break;

  case mwSession_STOPPING:
    if(GPOINTER_TO_UINT(info) & ERR_FAILURE) {
      msg = mwError(GPOINTER_TO_UINT(info));
      gaim_connection_error(gc, msg);
      g_free(msg);
    }
    break;

  case mwSession_STOPPED:
    break;

  case mwSession_UNKNOWN:
  default:
    DEBUG_WARN("session in unknown state\n");
  }
}


static void mw_session_setPrivacyInfo(struct mwSession *session) {
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  GaimAccount *acct;
  struct mwPrivacyInfo *privacy;
  GSList *l, **ll;
  guint count;

  DEBUG_INFO("privacy information set from server\n");

  g_return_if_fail(session != NULL);

  pd = mwSession_getClientData(session);
  g_return_if_fail(pd != NULL);

  gc = pd->gc;
  g_return_if_fail(gc != NULL);

  acct = gaim_connection_get_account(gc);
  g_return_if_fail(acct != NULL);

  privacy = mwSession_getPrivacyInfo(session);
  count = privacy->count;

  ll = (privacy->deny)? &acct->deny: &acct->permit;
  for(l = *ll; l; l = l->next) g_free(l->data);
  g_slist_free(*ll);
  l = *ll = NULL;

  while(count--) {
    struct mwUserItem *u = privacy->users + count;
    l = g_slist_prepend(l, g_strdup(u->id));
  }
  *ll = l;
}


static void mw_session_setUserStatus(struct mwSession *session) {
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  struct mwAwareIdBlock idb = { mwAware_USER, NULL, NULL };
  struct mwUserStatus *stat;

  g_return_if_fail(session != NULL);

  pd = mwSession_getClientData(session);
  g_return_if_fail(pd != NULL);

  gc = pd->gc;
  g_return_if_fail(gc != NULL);

  idb.user = mwSession_getProperty(session, mwSession_AUTH_USER_ID);
  stat = mwSession_getUserStatus(session);

  /* trigger an update of our own status if we're in the buddy list */
  mwServiceAware_setStatus(pd->srvc_aware, &idb, stat);
}


static void mw_session_admin(struct mwSession *session,
			     const char *text) {

  GaimConnection *gc = session_to_gc(session);
  g_return_if_fail(gc != NULL);

  /** @todo Admin alerts should probably be in a conversation window
      rather than a gaim_notify_message. Or in some sort of updating
      dialog, or something. */

  gaim_notify_message(gc, GAIM_NOTIFY_MSG_INFO, _("Admin Alert"),
		      text, NULL, NULL, NULL);
}


/** called from read_cb, attempts to read available data from sock and
    pass it to the session, passing back the return code from the read
    call for handling in read_cb */
static int read_recv(struct mwSession *session, int sock) {
  char buf[BUF_LEN];
  int len;

  len = read(sock, buf, BUF_LEN);
  if(len > 0) mwSession_recv(session, buf, len);

  return len;
}


/** callback triggered from gaim_input_add, watches the socked for
    available data to be processed by the session */
static void read_cb(gpointer data, gint source,
		    GaimInputCondition cond) {

  struct mwGaimPluginData *pd = data;
  int ret = 0, err = 0;

  /* How the heck can this happen? Fix submitted to Gaim so that it
     won't happen anymore. */
  if(! cond) return;

  g_return_if_fail(pd != NULL);
  g_return_if_fail(cond & GAIM_INPUT_READ);

  ret = read_recv(pd->session, pd->socket);

  /* normal operation ends here */
  if(ret > 0) return;

  /* fetch the global error value */
  err = errno;

  /* read problem occured if we're here, so we'll need to take care of
     it and clean up internal state */

  if(pd->socket) {
    close(pd->socket);
    pd->socket = 0;
  }

  if(pd->gc->inpa) {
    gaim_input_remove(pd->gc->inpa);
    pd->gc->inpa = 0;
  }

  if(! ret) {
    DEBUG_INFO("connection reset\n");
    gaim_connection_error(pd->gc, _("Connection reset"));

  } else if(ret < 0) {
    char *msg = strerror(err);

    DEBUG_INFO("error in read callback: %s\n", msg);

    msg = g_strdup_printf(_("Error reading from socket: %s"), msg);
    gaim_connection_error(pd->gc, msg);
    g_free(msg);
  }
}


/** Callback passed to gaim_proxy_connect when an account is logged
    in, and if the session logging in receives a redirect message */
static void connect_cb(gpointer data, gint source,
		       GaimInputCondition cond) {

  struct mwGaimPluginData *pd = data;
  GaimConnection *gc = pd->gc;

  if(! g_list_find(gaim_connections_get_all(), pd->gc)) {
    close(source);
    g_return_if_reached();
  }

  if(source < 0) {
    /* connection failed */

    if(pd->socket) {
      /* this is a redirect connect, force login on existing socket */
      mwSession_forceLogin(pd->session);

    } else {
      /* this is a regular connect, error out */
      gaim_connection_error(pd->gc, _("Unable to connect to host"));
    }

    return;
  }

  if(pd->socket) {
    /* stop any existing login attempt */
    mwSession_stop(pd->session, ERR_SUCCESS);
  }

  pd->socket = source;
  gc->inpa = gaim_input_add(source, GAIM_INPUT_READ, read_cb, pd);

  mwSession_start(pd->session);
}


static void mw_session_announce(struct mwSession *s,
				struct mwLoginInfo *from,
				gboolean may_reply,
				const char *text) {
  struct mwGaimPluginData *pd;
  GaimAccount *acct;
  GaimConversation *conv;
  GSList *buddies;
  char *who = from->user_id;
  char *msg;
  
  pd = mwSession_getClientData(s);
  acct = gaim_connection_get_account(pd->gc);
  conv = gaim_find_conversation_with_account(GAIM_CONV_TYPE_IM, who, acct);
  if(! conv) conv = gaim_conversation_new(GAIM_CONV_TYPE_IM, acct, who);

  buddies = gaim_find_buddies(acct, who);
  if(buddies) {
    who = (char *) gaim_buddy_get_contact_alias(buddies->data);
    g_slist_free(buddies);
  }

  who = g_strdup_printf(_("Announcement from %s"), who);
  msg = gaim_markup_linkify(text);

  gaim_conversation_write(conv, who, msg, GAIM_MESSAGE_RECV, time(NULL));
  g_free(who);
  g_free(msg);
}


static struct mwSessionHandler mw_session_handler = {
  .io_write = mw_session_io_write,
  .io_close = mw_session_io_close,
  .clear = mw_session_clear,
  .on_stateChange = mw_session_stateChange,
  .on_setPrivacyInfo = mw_session_setPrivacyInfo,
  .on_setUserStatus = mw_session_setUserStatus,
  .on_admin = mw_session_admin,
  .on_announce = mw_session_announce,
};


static void mw_aware_on_attrib(struct mwServiceAware *srvc,
			       struct mwAwareAttribute *attrib) {

  ; /** @todo handle server attributes.  There may be some stuff we
	actually want to look for, but I'm not aware of anything right
	now.*/
}


static void mw_aware_clear(struct mwServiceAware *srvc) {
  ; /* nothing for now */
}


static struct mwAwareHandler mw_aware_handler = {
  .on_attrib = mw_aware_on_attrib,
  .clear = mw_aware_clear,
};


static struct mwServiceAware *mw_srvc_aware_new(struct mwSession *s) {
  struct mwServiceAware *srvc;
  srvc = mwServiceAware_new(s, &mw_aware_handler);
  return srvc;
};


static void mw_conf_invited(struct mwConference *conf,
			    struct mwLoginInfo *inviter,
			    const char *invitation) {
  
  struct mwServiceConference *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;

  char *c_inviter, *c_name, *c_topic, *c_invitation;
  GHashTable *ht;

  srvc = mwConference_getService(conf);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;

  ht = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

  c_inviter = g_strdup(inviter->user_id);
  g_hash_table_insert(ht, CHAT_KEY_CREATOR, c_inviter);

  c_name = g_strdup(mwConference_getName(conf));
  g_hash_table_insert(ht, CHAT_KEY_NAME, c_name);

  c_topic = g_strdup(mwConference_getTitle(conf));
  g_hash_table_insert(ht, CHAT_KEY_TOPIC, c_topic);

  c_invitation = g_strdup(invitation);
  g_hash_table_insert(ht, CHAT_KEY_INVITE, c_invitation);

  DEBUG_INFO("received invitation from '%s' to join ('%s','%s'): '%s'\n",
	     NSTR(c_inviter), NSTR(c_name),
	     NSTR(c_topic), NSTR(c_invitation));

  serv_got_chat_invite(gc, c_topic, c_inviter, c_invitation, ht);
}


/* The following mess helps us relate a mwConference to a GaimConvChat
   in the various forms by which either may be indicated */

#define CONF_TO_ID(conf)   (GPOINTER_TO_INT(conf))
#define ID_TO_CONF(pd, id) (conf_find_by_id((pd), (id)))

#define CHAT_TO_ID(chat)   (gaim_conv_chat_get_id(chat))
#define ID_TO_CHAT(id)     (gaim_find_chat(id))

#define CHAT_TO_CONF(pd, chat)  (ID_TO_CONF((pd), CHAT_TO_ID(chat)))
#define CONF_TO_CHAT(conf)      (ID_TO_CHAT(CONF_TO_ID(conf)))


static struct mwConference *
conf_find_by_id(struct mwGaimPluginData *pd, int id) {

  struct mwServiceConference *srvc = pd->srvc_conf;
  struct mwConference *conf = NULL;
  GList *l, *ll;
  
  ll = mwServiceConference_getConferences(srvc);
  for(l = ll; l; l = l->next) {
    struct mwConference *c = l->data;
    GaimConvChat *h = mwConference_getClientData(c);

    if(CHAT_TO_ID(h) == id) {
      conf = c;
      break;
    }
  }
  g_list_free(ll);
  
  return conf;
}


static void mw_conf_opened(struct mwConference *conf, GList *members) {
  struct mwServiceConference *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  GaimConversation *g_conf;

  const char *n = mwConference_getName(conf);

  DEBUG_INFO("conf %s opened, %u initial members\n",
	     NSTR(n), g_list_length(members));

  srvc = mwConference_getService(conf);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;

  g_conf = serv_got_joined_chat(gc, CONF_TO_ID(conf),
				mwConference_getTitle(conf));

  mwConference_setClientData(conf, GAIM_CONV_CHAT(g_conf), NULL);

  for(; members; members = members->next) {
    struct mwLoginInfo *peer = members->data;
    gaim_conv_chat_add_user(GAIM_CONV_CHAT(g_conf), peer->user_id,
			    NULL, GAIM_CBFLAGS_NONE, FALSE);
  }
}


static void mw_conf_closed(struct mwConference *conf, guint32 reason) {
  struct mwServiceConference *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;

  const char *n = mwConference_getName(conf);
  char *msg = mwError(reason);

  DEBUG_INFO("conf %s closed, 0x%08x\n", NSTR(n), reason);

  srvc = mwConference_getService(conf);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;

  serv_got_chat_left(gc, CONF_TO_ID(conf));

  gaim_notify_error(gc, _("Conference Closed"), NULL, msg);
  g_free(msg);
}


static void mw_conf_peer_joined(struct mwConference *conf,
				struct mwLoginInfo *peer) {

  struct mwServiceConference *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  GaimConvChat *g_conf;

  const char *n = mwConference_getName(conf);

  DEBUG_INFO("%s joined conf %s\n", NSTR(peer->user_id), NSTR(n));

  srvc = mwConference_getService(conf);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;

  g_conf = mwConference_getClientData(conf);
  g_return_if_fail(g_conf != NULL);

  gaim_conv_chat_add_user(g_conf, peer->user_id,
			  NULL, GAIM_CBFLAGS_NONE, TRUE);
}


static void mw_conf_peer_parted(struct mwConference *conf,
				struct mwLoginInfo *peer) {
  
  struct mwServiceConference *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  GaimConvChat *g_conf;

  const char *n = mwConference_getName(conf);

  DEBUG_INFO("%s left conf %s\n", NSTR(peer->user_id), NSTR(n));

  srvc = mwConference_getService(conf);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;

  g_conf = mwConference_getClientData(conf);
  g_return_if_fail(g_conf != NULL);

  gaim_conv_chat_remove_user(g_conf, peer->user_id, NULL);
}


static void mw_conf_text(struct mwConference *conf,
			 struct mwLoginInfo *who, const char *text) {
  
  struct mwServiceConference *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  char *esc;

  srvc = mwConference_getService(conf);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;

  esc = g_markup_escape_text(text, -1);
  serv_got_chat_in(gc, CONF_TO_ID(conf), who->user_id, 0, esc, time(NULL));
  g_free(esc);
}


static void mw_conf_typing(struct mwConference *conf,
			   struct mwLoginInfo *who, gboolean typing) {

  /* gaim really has no good way to expose this to the user. */

  const char *n = mwConference_getName(conf);
  const char *w = who->user_id;

  if(typing) {
    DEBUG_INFO("%s in conf %s: <typing>\n", NSTR(w), NSTR(n));

  } else {
    DEBUG_INFO("%s in conf %s: <stopped typing>\n", NSTR(w), NSTR(n));
  }
}


static void mw_conf_clear(struct mwServiceConference *srvc) {
  ;
}


static struct mwConferenceHandler mw_conference_handler = {
  .on_invited = mw_conf_invited,
  .conf_opened = mw_conf_opened,
  .conf_closed = mw_conf_closed,
  .on_peer_joined = mw_conf_peer_joined,
  .on_peer_parted = mw_conf_peer_parted,
  .on_text = mw_conf_text,
  .on_typing = mw_conf_typing,
  .clear = mw_conf_clear,
};


static struct mwServiceConference *mw_srvc_conf_new(struct mwSession *s) {
  struct mwServiceConference *srvc;
  srvc = mwServiceConference_new(s, &mw_conference_handler);
  return srvc;
}


static void ft_incoming_cancel(GaimXfer *xfer) {
  /* incoming transfer rejected or canceled in-progress */
  struct mwFileTransfer *ft = xfer->data;
  if(ft) mwFileTransfer_reject(ft);
}


static void ft_incoming_init(GaimXfer *xfer) {
  /* incoming transfer accepted */
  
  /* - accept the mwFileTransfer
     - open/create the local FILE "wb"
     - stick the FILE's fp in xfer->dest_fp
  */

  struct mwFileTransfer *ft;
  FILE *fp;

  ft = xfer->data;

  fp = g_fopen(xfer->local_filename, "wb");
  if(! fp) {
    mwFileTransfer_cancel(ft);
    return;
  }

  xfer->dest_fp = fp;
  mwFileTransfer_accept(ft);
}


static void mw_ft_offered(struct mwFileTransfer *ft) {
  /*
    - create a gaim ft object
    - offer it
  */

  struct mwServiceFileTransfer *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  GaimAccount *acct;
  const char *who;
  GaimXfer *xfer;

  /* @todo add some safety checks */
  srvc = mwFileTransfer_getService(ft);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;
  acct = gaim_connection_get_account(gc);

  who = mwFileTransfer_getUser(ft)->user;

  DEBUG_INFO("file transfer %p offered\n", ft);
  DEBUG_INFO(" from: %s\n", NSTR(who));
  DEBUG_INFO(" file: %s\n", NSTR(mwFileTransfer_getFileName(ft)));
  DEBUG_INFO(" size: %u\n", mwFileTransfer_getFileSize(ft));
  DEBUG_INFO(" text: %s\n", NSTR(mwFileTransfer_getMessage(ft)));

  xfer = gaim_xfer_new(acct, GAIM_XFER_RECEIVE, who);

  gaim_xfer_ref(xfer);
  mwFileTransfer_setClientData(ft, xfer, (GDestroyNotify) gaim_xfer_unref);
  xfer->data = ft;

  gaim_xfer_set_init_fnc(xfer, ft_incoming_init);
  gaim_xfer_set_cancel_recv_fnc(xfer, ft_incoming_cancel);
  gaim_xfer_set_request_denied_fnc(xfer, ft_incoming_cancel);

  gaim_xfer_set_filename(xfer, mwFileTransfer_getFileName(ft));
  gaim_xfer_set_size(xfer, mwFileTransfer_getFileSize(ft));
  gaim_xfer_set_message(xfer, mwFileTransfer_getMessage(ft));

  gaim_xfer_request(xfer);
}


static void ft_send(struct mwFileTransfer *ft, FILE *fp) {
  char buf[BUF_LONG];
  struct mwOpaque o = { .data = buf, .len = BUF_LONG };
  guint32 rem;
  GaimXfer *xfer;

  xfer = mwFileTransfer_getClientData(ft);

  rem = mwFileTransfer_getRemaining(ft);
  if(rem < BUF_LONG) o.len = rem;
  
  if(fread(buf, (size_t) o.len, 1, fp)) {

    /* calculate progress first. update is displayed upon ack */
    xfer->bytes_sent += o.len;
    xfer->bytes_remaining -= o.len;

    /* ... send data second */
    mwFileTransfer_send(ft, &o);

  } else {
    int err = errno;
    DEBUG_WARN("problem reading from file %s: %s",
	       NSTR(mwFileTransfer_getFileName(ft)), strerror(err));

    mwFileTransfer_cancel(ft);
  }
}


static gboolean ft_idle_cb(struct mwFileTransfer *ft) {
  GaimXfer *xfer = mwFileTransfer_getClientData(ft);
  g_return_val_if_fail(xfer != NULL, FALSE);
  
  xfer->watcher = 0;
  ft_send(ft, xfer->dest_fp);

  return FALSE;
}


static void mw_ft_opened(struct mwFileTransfer *ft) {
  /*
    - get gaim ft from client data in ft
    - set the state to active
  */

  GaimXfer *xfer;

  xfer = mwFileTransfer_getClientData(ft);

  if(! xfer) {
    mwFileTransfer_cancel(ft);
    mwFileTransfer_free(ft);
    g_return_if_reached();
  }

  gaim_xfer_update_progress(xfer);

  if(gaim_xfer_get_type(xfer) == GAIM_XFER_SEND) {
    xfer->watcher = g_idle_add((GSourceFunc)ft_idle_cb, ft);
    xfer->dest_fp = g_fopen(xfer->local_filename, "rb");
  }  
}


static void mw_ft_closed(struct mwFileTransfer *ft, guint32 code) {
  /*
    - get gaim ft from client data in ft
    - indicate rejection/cancelation/completion
    - free the file transfer itself
  */

  GaimXfer *xfer;

  xfer = mwFileTransfer_getClientData(ft);
  if(xfer) {
    xfer->data = NULL;

    if(mwFileTransfer_isDone(ft)) {
      gaim_xfer_set_completed(xfer, TRUE);
      gaim_xfer_end(xfer);

    } else if(mwFileTransfer_isCancelLocal(ft)) {
      /* calling gaim_xfer_cancel_local is redundant, since that's
	 probably what triggered this function to be called */
      ;

    } else if(mwFileTransfer_isCancelRemote(ft)) {
      /* steal the reference for the xfer */
      mwFileTransfer_setClientData(ft, NULL, NULL);
      gaim_xfer_cancel_remote(xfer);

      /* drop the stolen reference */
      gaim_xfer_unref(xfer);
      return;
    }
  }

  mwFileTransfer_free(ft);
}


static void mw_ft_recv(struct mwFileTransfer *ft,
		       struct mwOpaque *data) {
  /*
    - get gaim ft from client data in ft
    - update transfered percentage
    - if done, destroy the ft, disassociate from gaim ft
  */

  GaimXfer *xfer;
  FILE *fp;

  xfer = mwFileTransfer_getClientData(ft);
  g_return_if_fail(xfer != NULL);

  fp = xfer->dest_fp;
  g_return_if_fail(fp != NULL);

  /* we must collect and save our precious data */
  fwrite(data->data, 1, data->len, fp);

  /* update the progress */
  xfer->bytes_sent += data->len;
  xfer->bytes_remaining -= data->len;
  gaim_xfer_update_progress(xfer);

  /* let the other side know we got it, and to send some more */
  mwFileTransfer_ack(ft);
}


static void mw_ft_ack(struct mwFileTransfer *ft) {
  GaimXfer *xfer;

  xfer = mwFileTransfer_getClientData(ft);
  g_return_if_fail(xfer != NULL);
  g_return_if_fail(xfer->watcher == 0);

  gaim_xfer_update_progress(xfer);

  if(mwFileTransfer_isOpen(ft))
    xfer->watcher = g_idle_add((GSourceFunc)ft_idle_cb, ft);
}


static void mw_ft_clear(struct mwServiceFileTransfer *srvc) {
  ;
}


static struct mwFileTransferHandler mw_ft_handler = {
  .ft_offered = mw_ft_offered,
  .ft_opened = mw_ft_opened,
  .ft_closed = mw_ft_closed,
  .ft_recv = mw_ft_recv,
  .ft_ack = mw_ft_ack,
  .clear = mw_ft_clear,
};


static struct mwServiceFileTransfer *mw_srvc_ft_new(struct mwSession *s) {
  struct mwServiceFileTransfer *srvc;
  GHashTable *ft_map;

  ft_map = g_hash_table_new(g_direct_hash, g_direct_equal);

  srvc = mwServiceFileTransfer_new(s, &mw_ft_handler);
  mwService_setClientData(MW_SERVICE(srvc), ft_map,
			  (GDestroyNotify) g_hash_table_destroy);

  return srvc;
}


static void convo_data_free(struct convo_data *cd) {
  GList *l;

  /* clean the queue */
  for(l = cd->queue; l; l = g_list_delete_link(l, l)) {
    struct convo_msg *m = l->data;
    if(m->clear) m->clear(m->data);
    g_free(m);
  }

  g_free(cd);
}


/** allocates a convo_data structure and associates it with the
    conversation in the client data slot */
static void convo_data_new(struct mwConversation *conv) {
  struct convo_data *cd;

  g_return_if_fail(conv != NULL);

  if(mwConversation_getClientData(conv))
    return;

  cd = g_new0(struct convo_data, 1);
  cd->conv = conv;

  mwConversation_setClientData(conv, cd, (GDestroyNotify) convo_data_free);
}


static GaimConversation *convo_get_gconv(struct mwConversation *conv) {
  struct mwServiceIm *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  GaimAccount *acct;

  struct mwIdBlock *idb;

  srvc = mwConversation_getService(conv);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;
  acct = gaim_connection_get_account(gc);

  idb = mwConversation_getTarget(conv);

  return gaim_find_conversation_with_account(GAIM_CONV_TYPE_IM,
					     idb->user, acct);
}


static void convo_queue(struct mwConversation *conv,
			enum mwImSendType type, gconstpointer data) {

  struct convo_data *cd;
  struct convo_msg *m;

  convo_data_new(conv);
  cd = mwConversation_getClientData(conv);

  m = g_new0(struct convo_msg, 1);
  m->type = type;

  switch(type) {
  case mwImSend_PLAIN:
    m->data = g_strdup(data);
    m->clear = g_free;
    break;
    
  case mwImSend_TYPING:
  default:
    m->data = (gpointer) data;
    m->clear = NULL;
  }

  cd->queue = g_list_append(cd->queue, m);
}


/* Does what it takes to get an error displayed for a conversation */
static void convo_error(struct mwConversation *conv, guint32 err) {
  GaimConversation *gconv;
  char *tmp, *text;
  struct mwIdBlock *idb;
  
  idb = mwConversation_getTarget(conv);
  
  tmp = mwError(err);
  text = g_strconcat(_("Unable to send message: "), tmp, NULL);
  
  gconv = convo_get_gconv(conv);
  if(gconv && !gaim_conv_present_error(idb->user, gconv->account, text)) {
    
    g_free(text);
    text = g_strdup_printf(_("Unable to send message to %s:"),
			   (idb->user)? idb->user: "(unknown)");
    gaim_notify_error(gaim_account_get_connection(gconv->account),
		      NULL, text, tmp);
  }
  
  g_free(tmp);
  g_free(text);
}


static void convo_queue_send(struct mwConversation *conv) {
  struct convo_data *cd;
  GList *l;
  
  cd = mwConversation_getClientData(conv);

  for(l = cd->queue; l; l = g_list_delete_link(l, l)) {
    struct convo_msg *m = l->data;

    mwConversation_send(conv, m->type, m->data);

    if(m->clear) m->clear(m->data);
    g_free(m);
  }

  cd->queue = NULL;
}


/**  called when a mw conversation leaves a gaim conversation to
     inform the gaim conversation that it's unsafe to offer any *cool*
     features. */
static void convo_nofeatures(struct mwConversation *conv) {
  GaimConversation *gconv;
  GaimConnection *gc;

  gconv = convo_get_gconv(conv);
  if(! gconv) return;

  gc = gaim_conversation_get_gc(gconv);
  if(! gc) return;

  gaim_conversation_set_features(gconv, gc->flags);
}


/** called when a mw conversation and gaim conversation come together,
    to inform the gaim conversation of what features to offer the
    user */
static void convo_features(struct mwConversation *conv) {
  GaimConversation *gconv;
  GaimConnectionFlags feat;

  gconv = convo_get_gconv(conv);
  if(! gconv) return;

  feat = gaim_conversation_get_features(gconv);

  if(mwConversation_isOpen(conv)) {
    if(mwConversation_supports(conv, mwImSend_HTML)) {
      feat |= GAIM_CONNECTION_HTML;
    } else {
      feat &= ~GAIM_CONNECTION_HTML;
    }

    if(mwConversation_supports(conv, mwImSend_MIME)) {
      feat &= ~GAIM_CONNECTION_NO_IMAGES;
    } else {
      feat |= GAIM_CONNECTION_NO_IMAGES;
    }

    DEBUG_INFO("conversation features set to 0x%04x\n", feat);
    gaim_conversation_set_features(gconv, feat);

  } else {
    convo_nofeatures(conv);
  }
}


#if 0
/** triggered from mw_conversation_opened if the appropriate plugin
    preference is set. This will open a window for the conversation
    before the first message is sent. */
static void convo_do_psychic(struct mwConversation *conv) {
  struct mwServiceIm *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  GaimAccount *acct;

  struct mwIdBlock *idb;

  GaimConversation *gconv;
  GaimConvWindow *win;

  srvc = mwConversation_getService(conv);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;
  acct = gaim_connection_get_account(gc);

  idb = mwConversation_getTarget(conv);

  gconv = gaim_find_conversation_with_account(GAIM_CONV_TYPE_IM,
					      idb->user, acct);
  if(! gconv) {
    gconv = gaim_conversation_new(GAIM_CONV_TYPE_IM, acct, idb->user);
  }

  g_return_if_fail(gconv != NULL);

  win = gaim_conversation_get_window(gconv);
  g_return_if_fail(win != NULL);

  gaim_conv_window_show(win);
}
#endif


static void mw_conversation_opened(struct mwConversation *conv) {
  struct mwServiceIm *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  GaimAccount *acct;

  struct convo_dat *cd;

  srvc = mwConversation_getService(conv);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;
  acct = gaim_connection_get_account(gc);

  /* set up the queue */
  cd = mwConversation_getClientData(conv);
  if(cd) {
    convo_queue_send(conv);
  
    if(! convo_get_gconv(conv)) {
      mwConversation_free(conv);
      return;
    }

  } else {
    convo_data_new(conv);

#if 0
    if(gaim_prefs_get_bool(MW_PRPL_OPT_PSYCHIC)) {
      convo_do_psychic(conv);
    }
#endif
  }

  { /* record the client key for the buddy */
    GaimBuddy *buddy;
    struct mwLoginInfo *info;
    info = mwConversation_getTargetInfo(conv);
    
    buddy = gaim_find_buddy(acct, info->user_id);
    if(buddy) {
      gaim_blist_node_set_int((GaimBlistNode *) buddy,
			      BUDDY_KEY_CLIENT, info->type);
    }
  }

  convo_features(conv);
}


static void mw_conversation_closed(struct mwConversation *conv,
				   guint32 reason) {

  struct convo_data *cd;

  g_return_if_fail(conv != NULL);

  /* if there's an error code and a non-typing message in the queue,
     print an error message to the conversation */
  cd = mwConversation_getClientData(conv);
  if(reason && cd && cd->queue) {
    GList *l;
    for(l = cd->queue; l; l = l->next) {
      struct convo_msg *m = l->data;
      if(m->type != mwImSend_TYPING) {
	convo_error(conv, reason);
	break;
      }
    }
  }

#if 0
  /* don't do this, to prevent the occasional weird sending of
     formatted messages as plaintext when the other end closes the
     conversation after we've begun composing the message */
  convo_nofeatures(conv);
#endif

  mwConversation_removeClientData(conv);
}



static char *im_decode(GaimConnection *gc, const char *msg) {
  return gaim_utf8_try_convert(msg);
}


static void im_recv_text(struct mwConversation *conv,
			 struct mwGaimPluginData *pd,
			 const char *msg) {

  struct mwIdBlock *idb;
  char *txt, *esc, *t;

  idb = mwConversation_getTarget(conv);
  txt = im_decode(pd->gc, msg);

  t = txt? txt: (char *) msg;

  esc = g_markup_escape_text(t, -1);
  serv_got_im(pd->gc, idb->user, esc, 0, time(NULL));
  g_free(esc);

  g_free(txt);
}


static void im_recv_typing(struct mwConversation *conv,
			   struct mwGaimPluginData *pd,
			   gboolean typing) {

  struct mwIdBlock *idb;
  idb = mwConversation_getTarget(conv);

  serv_got_typing(pd->gc, idb->user, 0,
		  typing? GAIM_TYPING: GAIM_NOT_TYPING);
}


static void im_recv_html(struct mwConversation *conv,
			 struct mwGaimPluginData *pd,
			 const char *msg) {

  struct mwIdBlock *idb;
  char *txt, *t;

  idb = mwConversation_getTarget(conv);
  txt = im_decode(pd->gc, msg);
  
  t = txt? txt: (char *) msg;

  serv_got_im(pd->gc, idb->user, t, 0, time(NULL));

  g_free(txt);
}


static void im_recv_subj(struct mwConversation *conv,
			 struct mwGaimPluginData *pd,
			 const char *subj) {

  /** @todo somehow indicate receipt of a conversation subject. It
      would also be nice if we added a /topic command for the
      protocol */
  ;
}


/** generate "cid:908@20582notesbuddy" from "<908@20582notesbuddy>" */
static char *make_cid(const char *cid) {
  gsize n;
  char *c, *d;

  g_return_val_if_fail(cid != NULL, NULL);

  n = strlen(cid);
  g_return_val_if_fail(n > 2, NULL);

  c = g_strndup(cid+1, n-2);
  d = g_strdup_printf("cid:%s", c);

  g_free(c);
  return d;
}


static void im_recv_mime(struct mwConversation *conv,
			 struct mwGaimPluginData *pd,
			 const char *data) {

  struct mwIdBlock *idb;

  GHashTable *img_by_cid;
  GList *images;

  GString *str;

  GaimMimeDocument *doc;
  const GList *parts;

  idb = mwConversation_getTarget(conv);

  img_by_cid = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  images = NULL;

  /* don't want the contained string to ever be NULL */
  str = g_string_new("");
  
  doc = gaim_mime_document_parse(data);

  /* handle all the MIME parts */
  parts = gaim_mime_document_get_parts(doc);
  for(; parts; parts = parts->next) {
    GaimMimePart *part = parts->data;
    const char *type;

    type = gaim_mime_part_get_field(part, "content-type");
    DEBUG_INFO("MIME part Content-Type: %s\n", NSTR(type));

    if(! type) {
      ; /* feh */
      
    } else if(g_str_has_prefix(type, "image")) {
      /* put images into the image store */

      guchar *d_dat;
      gsize d_len;
      char *cid;
      int img;

      /* obtain and unencode the data */
      gaim_mime_part_get_data_decoded(part, &d_dat, &d_len);
      
      /* look up the content id */
      cid = (char *) gaim_mime_part_get_field(part, "Content-ID");
      cid = make_cid(cid);

      /* add image to the gaim image store */
      img = gaim_imgstore_add(d_dat, d_len, cid);
      g_free(d_dat);

      /* map the cid to the image store identifier */
      g_hash_table_insert(img_by_cid, cid, GINT_TO_POINTER(img));

      /* recall the image for dereferencing later */
      images = g_list_append(images, GINT_TO_POINTER(img));
      
    } else if(g_str_has_prefix(type, "text")) {

      /* concatenate all the text parts together */
      guchar *data;
      char *txt;
      gsize len;

      gaim_mime_part_get_data_decoded(part, &data, &len);

      txt = im_decode(pd->gc, (const char *)data);
      g_string_append(str, txt?txt:(const char *)data);

      g_free(data);
      g_free(txt);
    }
  }  

  gaim_mime_document_free(doc);

  /* @todo should put this in its own function */
  { /* replace each IMG tag's SRC attribute with an ID attribute. This
       actually modifies the contents of str */
    GData *attribs;
    char *start, *end;
    char *tmp = str->str;

    while(*tmp && gaim_markup_find_tag("img", tmp, (const char **) &start,
				       (const char **) &end, &attribs)) {

      char *alt, *align, *border, *src;
      int img;

      alt = g_datalist_get_data(&attribs, "alt");
      align = g_datalist_get_data(&attribs, "align");
      border = g_datalist_get_data(&attribs, "border");
      src = g_datalist_get_data(&attribs, "src");

      img = GPOINTER_TO_INT(g_hash_table_lookup(img_by_cid, src));
      if(img) {
	GString *atstr;
	gsize len = (end - start);
	gsize mov;

	atstr = g_string_new("");
	if(alt) g_string_append_printf(atstr, " alt=\"%s\"", alt);
	if(align) g_string_append_printf(atstr, " align=\"%s\"", align);
	if(border) g_string_append_printf(atstr, " border=\"%s\"", border);

	mov = g_snprintf(start, len, "<img%s id=\"%i\"", atstr->str, img);
	while(mov < len) start[mov++] = ' ';

	g_string_free(atstr, TRUE);
      }

      g_datalist_clear(&attribs);
      tmp = end + 1;
    }
  }

  /* actually display the message */
  serv_got_im(pd->gc, idb->user, str->str, 0, time(NULL));

  g_string_free(str, TRUE);
  
  /* clean up the cid table */
  g_hash_table_destroy(img_by_cid);

  /* dereference all the imgages */
  while(images) {
    gaim_imgstore_unref(GPOINTER_TO_INT(images->data));
    images = g_list_delete_link(images, images);
  }
}


static void mw_conversation_recv(struct mwConversation *conv,
				 enum mwImSendType type,
				 gconstpointer msg) {
  struct mwServiceIm *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;

  srvc = mwConversation_getService(conv);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);

  switch(type) {
  case mwImSend_PLAIN:
    im_recv_text(conv, pd, msg);
    break;

  case mwImSend_TYPING:
    im_recv_typing(conv, pd, !! msg);
    break;

  case mwImSend_HTML:
    im_recv_html(conv, pd, msg);
    break;

  case mwImSend_SUBJECT:
    im_recv_subj(conv, pd, msg);
    break;

  case mwImSend_MIME:
    im_recv_mime(conv, pd, msg);
    break;

  default:
    DEBUG_INFO("conversation received strange type, 0x%04x\n", type);
    ; /* erm... */
  }
}


static void mw_place_invite(struct mwConversation *conv,
			    const char *message,
			    const char *title, const char *name) {
  struct mwServiceIm *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;

  struct mwIdBlock *idb;
  GHashTable *ht;

  srvc = mwConversation_getService(conv);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);

  idb = mwConversation_getTarget(conv);
  
  ht = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
  g_hash_table_insert(ht, CHAT_KEY_CREATOR, g_strdup(idb->user));
  g_hash_table_insert(ht, CHAT_KEY_NAME, g_strdup(name));
  g_hash_table_insert(ht, CHAT_KEY_TOPIC, g_strdup(title));
  g_hash_table_insert(ht, CHAT_KEY_INVITE, g_strdup(message));
  g_hash_table_insert(ht, CHAT_KEY_IS_PLACE, g_strdup("")); /* ugh */

  serv_got_chat_invite(pd->gc, title, idb->user, message, ht);

  mwConversation_close(conv, ERR_SUCCESS);
  mwConversation_free(conv);
}


static void mw_im_clear(struct mwServiceIm *srvc) {
  ;
}


static struct mwImHandler mw_im_handler = {
  .conversation_opened = mw_conversation_opened,
  .conversation_closed = mw_conversation_closed,
  .conversation_recv = mw_conversation_recv,
  .place_invite = mw_place_invite,
  .clear = mw_im_clear,
};


static struct mwServiceIm *mw_srvc_im_new(struct mwSession *s) {
  struct mwServiceIm *srvc;
  srvc = mwServiceIm_new(s, &mw_im_handler);
  mwServiceIm_setClientType(srvc, mwImClient_NOTESBUDDY);
  return srvc;
}


/* The following helps us relate a mwPlace to a GaimConvChat in the
   various forms by which either may be indicated. Uses some of
   the similar macros from the conference service above */

#define PLACE_TO_ID(place)   (GPOINTER_TO_INT(place))
#define ID_TO_PLACE(pd, id)  (place_find_by_id((pd), (id)))

#define CHAT_TO_PLACE(pd, chat)  (ID_TO_PLACE((pd), CHAT_TO_ID(chat)))
#define PLACE_TO_CHAT(place)     (ID_TO_CHAT(PLACE_TO_ID(place)))


static struct mwPlace *
place_find_by_id(struct mwGaimPluginData *pd, int id) {
  struct mwServicePlace *srvc = pd->srvc_place;
  struct mwPlace *place = NULL;
  GList *l;

  l = (GList *) mwServicePlace_getPlaces(srvc);
  for(; l; l = l->next) {
    struct mwPlace *p = l->data;
    GaimConvChat *h = GAIM_CONV_CHAT(mwPlace_getClientData(p));

    if(CHAT_TO_ID(h) == id) {
      place = p;
      break;
    }
  }

  return place;
}


static void mw_place_opened(struct mwPlace *place) {
  struct mwServicePlace *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  GaimConversation *gconf;

  GList *members, *l;

  const char *n = mwPlace_getName(place);

  srvc = mwPlace_getService(place);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;

  members = mwPlace_getMembers(place);

  DEBUG_INFO("place %s opened, %u initial members\n",
	     NSTR(n), g_list_length(members));

  gconf = serv_got_joined_chat(gc, PLACE_TO_ID(place),
			       mwPlace_getTitle(place));

  mwPlace_setClientData(place, gconf, NULL);

  for(l = members; l; l = l->next) {
    struct mwIdBlock *idb = l->data;
    gaim_conv_chat_add_user(GAIM_CONV_CHAT(gconf), idb->user,
			    NULL, GAIM_CBFLAGS_NONE, FALSE);
  }
  g_list_free(members);
}


static void mw_place_closed(struct mwPlace *place, guint32 code) {
  struct mwServicePlace *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;

  const char *n = mwPlace_getName(place);
  char *msg = mwError(code);

  DEBUG_INFO("place %s closed, 0x%08x\n", NSTR(n), code);

  srvc = mwPlace_getService(place);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;

  serv_got_chat_left(gc, PLACE_TO_ID(place));

  gaim_notify_error(gc, _("Place Closed"), NULL, msg);
  g_free(msg);
}


static void mw_place_peerJoined(struct mwPlace *place,
				const struct mwIdBlock *peer) {
  struct mwServicePlace *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  GaimConversation *gconf;

  const char *n = mwPlace_getName(place);

  DEBUG_INFO("%s joined place %s\n", NSTR(peer->user), NSTR(n));

  srvc = mwPlace_getService(place);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;

  gconf = mwPlace_getClientData(place);
  g_return_if_fail(gconf != NULL);

  gaim_conv_chat_add_user(GAIM_CONV_CHAT(gconf), peer->user,
			  NULL, GAIM_CBFLAGS_NONE, TRUE);
}


static void mw_place_peerParted(struct mwPlace *place,
				const struct mwIdBlock *peer) {
  struct mwServicePlace *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  GaimConversation *gconf;

  const char *n = mwPlace_getName(place);

  DEBUG_INFO("%s left place %s\n", NSTR(peer->user), NSTR(n));

  srvc = mwPlace_getService(place);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;

  gconf = mwPlace_getClientData(place);
  g_return_if_fail(gconf != NULL);

  gaim_conv_chat_remove_user(GAIM_CONV_CHAT(gconf), peer->user, NULL);
}


static void mw_place_peerSetAttribute(struct mwPlace *place,
				      const struct mwIdBlock *peer,
				      guint32 attr, struct mwOpaque *o) {
  ;
}


static void mw_place_peerUnsetAttribute(struct mwPlace *place,
					const struct mwIdBlock *peer,
					guint32 attr) {
  ;
}


static void mw_place_message(struct mwPlace *place,
			     const struct mwIdBlock *who,
			     const char *msg) {
  struct mwServicePlace *srvc;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;
  char *esc;

  srvc = mwPlace_getService(place);
  session = mwService_getSession(MW_SERVICE(srvc));
  pd = mwSession_getClientData(session);
  gc = pd->gc;

  esc = g_markup_escape_text(msg, -1);
  serv_got_chat_in(gc, PLACE_TO_ID(place), who->user, 0, esc, time(NULL));
  g_free(esc);
}


static void mw_place_clear(struct mwServicePlace *srvc) {
  ;
}


static struct mwPlaceHandler mw_place_handler = {
  .opened = mw_place_opened,
  .closed = mw_place_closed,
  .peerJoined = mw_place_peerJoined,
  .peerParted = mw_place_peerParted,
  .peerSetAttribute = mw_place_peerSetAttribute,
  .peerUnsetAttribute = mw_place_peerUnsetAttribute,
  .message = mw_place_message,
  .clear = mw_place_clear,
};


static struct mwServicePlace *mw_srvc_place_new(struct mwSession *s) {
  struct mwServicePlace *srvc;
  srvc = mwServicePlace_new(s, &mw_place_handler);
  return srvc;
}


static struct mwServiceResolve *mw_srvc_resolve_new(struct mwSession *s) {
  struct mwServiceResolve *srvc;
  srvc = mwServiceResolve_new(s);
  return srvc;
}


static struct mwServiceStorage *mw_srvc_store_new(struct mwSession *s) {
  struct mwServiceStorage *srvc;
  srvc = mwServiceStorage_new(s);
  return srvc;
}


/** allocate and associate a mwGaimPluginData with a GaimConnection */
static struct mwGaimPluginData *mwGaimPluginData_new(GaimConnection *gc) {
  struct mwGaimPluginData *pd;

  g_return_val_if_fail(gc != NULL, NULL);

  pd = g_new0(struct mwGaimPluginData, 1);
  pd->gc = gc;
  pd->session = mwSession_new(&mw_session_handler);
  pd->srvc_aware = mw_srvc_aware_new(pd->session);
  pd->srvc_conf = mw_srvc_conf_new(pd->session);
  pd->srvc_ft = mw_srvc_ft_new(pd->session);
  pd->srvc_im = mw_srvc_im_new(pd->session);
  pd->srvc_place = mw_srvc_place_new(pd->session);
  pd->srvc_resolve = mw_srvc_resolve_new(pd->session);
  pd->srvc_store = mw_srvc_store_new(pd->session);
  pd->group_list_map = g_hash_table_new(g_direct_hash, g_direct_equal);

  mwSession_addService(pd->session, MW_SERVICE(pd->srvc_aware));
  mwSession_addService(pd->session, MW_SERVICE(pd->srvc_conf));
  mwSession_addService(pd->session, MW_SERVICE(pd->srvc_ft));
  mwSession_addService(pd->session, MW_SERVICE(pd->srvc_im));
  mwSession_addService(pd->session, MW_SERVICE(pd->srvc_place));
  mwSession_addService(pd->session, MW_SERVICE(pd->srvc_resolve));
  mwSession_addService(pd->session, MW_SERVICE(pd->srvc_store));

  mwSession_addCipher(pd->session, mwCipher_new_RC2_40(pd->session));
  mwSession_addCipher(pd->session, mwCipher_new_RC2_128(pd->session));

  mwSession_setClientData(pd->session, pd, NULL);
  gc->proto_data = pd;

  return pd;
}


static void mwGaimPluginData_free(struct mwGaimPluginData *pd) {
  g_return_if_fail(pd != NULL);

  pd->gc->proto_data = NULL;

  mwSession_removeService(pd->session, mwService_AWARE);
  mwSession_removeService(pd->session, mwService_CONFERENCE);
  mwSession_removeService(pd->session, mwService_FILE_TRANSFER);
  mwSession_removeService(pd->session, mwService_IM);
  mwSession_removeService(pd->session, mwService_PLACE);
  mwSession_removeService(pd->session, mwService_RESOLVE);
  mwSession_removeService(pd->session, mwService_STORAGE);

  mwService_free(MW_SERVICE(pd->srvc_aware));
  mwService_free(MW_SERVICE(pd->srvc_conf));
  mwService_free(MW_SERVICE(pd->srvc_ft));
  mwService_free(MW_SERVICE(pd->srvc_im));
  mwService_free(MW_SERVICE(pd->srvc_place));
  mwService_free(MW_SERVICE(pd->srvc_resolve));
  mwService_free(MW_SERVICE(pd->srvc_store));

  mwCipher_free(mwSession_getCipher(pd->session, mwCipher_RC2_40));
  mwCipher_free(mwSession_getCipher(pd->session, mwCipher_RC2_128));

  mwSession_free(pd->session);

  g_hash_table_destroy(pd->group_list_map);

  g_free(pd);
}


static const char *mw_prpl_list_icon(GaimAccount *a, GaimBuddy *b) {
  /* my little green dude is a chopped up version of the aim running
     guy.  First, cut off the head and store someplace safe. Then,
     take the left-half side of the body and throw it away. Make a
     copy of the remaining body, and flip it horizontally. Now attach
     the two pieces into an X shape, and drop the head back on the
     top, being careful to center it. Then, just change the color
     saturation to bring the red down a bit, and voila! */

  /* then, throw all of that away and use sodipodi to make a new
     icon. You know, LIKE A REAL MAN. */

  return "meanwhile";
}


static void mw_prpl_list_emblems(GaimBuddy *b,
				 const char **se, const char **sw,
				 const char **nw, const char **ne) {

  /* speaking of custom icons, the external icon here is an ugly
     little example of what happens when I use Gimp */

  GaimPresence *presence;
  GaimStatus *status;
  const char *status_id;

  presence = gaim_buddy_get_presence(b);
  status = gaim_presence_get_active_status(presence);
  status_id = gaim_status_get_id(status);

  if(! GAIM_BUDDY_IS_ONLINE(b)) {
    *se = "offline";
  } else if(!strcmp(status_id, MW_STATE_AWAY)) {
    *se = "away";
  } else if(!strcmp(status_id, MW_STATE_BUSY)) {
    *se = "dnd";
  }  

  if(buddy_is_external(b)) {
    /* best assignment ever */
    *(*se?sw:se) = "external";
  }
}


static char *mw_prpl_status_text(GaimBuddy *b) {
  GaimConnection *gc;
  struct mwGaimPluginData *pd;
  struct mwAwareIdBlock t = { mwAware_USER, b->name, NULL };
  const char *ret;

  gc = b->account->gc;
  pd = gc->proto_data;

  ret = mwServiceAware_getText(pd->srvc_aware, &t);
  return g_strdup(ret);
}


static const char *status_text(GaimBuddy *b) {
  GaimPresence *presence;
  GaimStatus *status;

  presence = gaim_buddy_get_presence(b);
  status = gaim_presence_get_active_status(presence);

  return gaim_status_get_name(status);
}


static gboolean user_supports(struct mwServiceAware *srvc,
			      const char *who, guint32 feature) {

  const struct mwAwareAttribute *attr;
  struct mwAwareIdBlock idb = { mwAware_USER, (char *) who, NULL };

  attr = mwServiceAware_getAttribute(srvc, &idb, feature);
  return (attr != NULL) && mwAwareAttribute_asBoolean(attr);
}


char *user_supports_text(struct mwServiceAware *srvc, const char *who) {
    char *feat[] = {NULL, NULL, NULL, NULL, NULL};
    char **f = feat;

    if(user_supports(srvc, who, mwAttribute_AV_PREFS_SET)) {
      gboolean mic, speak, video;

      mic = user_supports(srvc, who, mwAttribute_MICROPHONE);
      speak = user_supports(srvc, who, mwAttribute_SPEAKERS);
      video = user_supports(srvc, who, mwAttribute_VIDEO_CAMERA);

      if(mic) *f++ = _("Microphone");
      if(speak) *f++ = _("Speakers");
      if(video) *f++ = _("Video Camera");
    }

    if(user_supports(srvc, who, mwAttribute_FILE_TRANSFER))
      *f++ = _("File Transfer");

    return (*feat)? g_strjoinv(", ", feat): NULL;
    /* jenni loves siege */
}


static char *mw_prpl_tooltip_text(GaimBuddy *b) {
  GaimConnection *gc;
  struct mwGaimPluginData *pd;
  struct mwAwareIdBlock idb = { mwAware_USER, b->name, NULL };

  GString *str;
  const char *tmp;

  gc = b->account->gc;
  pd = gc->proto_data;

  str = g_string_new(NULL);

  tmp = status_text(b);
  g_string_append_printf(str, _("\n<b>Status</b>: %s"), tmp);

  tmp = mwServiceAware_getText(pd->srvc_aware, &idb);
  if(tmp) {
    tmp = g_markup_escape_text(tmp, -1);
    g_string_append_printf(str, _("\n<b>Message</b>: %s"), tmp);
    g_free((char *) tmp);
  }

  tmp = user_supports_text(pd->srvc_aware, b->name);
  if(tmp) {
    g_string_append_printf(str, _("\n<b>Supports</b>: %s"), tmp);
    g_free((char *) tmp);
  }

  if(buddy_is_external(b)) {
    g_string_append(str, _("\n<b>External User</b>"));
  }

  tmp = str->str;
  g_string_free(str, FALSE);
  return (char *) tmp;
}


static GList *mw_prpl_status_types(GaimAccount *acct) {
  GList *types = NULL;
  GaimStatusType *type;

  type = gaim_status_type_new(GAIM_STATUS_OFFLINE, MW_STATE_OFFLINE,
			      _("Offline"), TRUE);
  types = g_list_append(types, type);

  type = gaim_status_type_new(GAIM_STATUS_AVAILABLE, MW_STATE_ACTIVE,
			      _("Active"), TRUE);
  gaim_status_type_add_attr(type, MW_STATE_MESSAGE, _("Message"),
			    gaim_value_new(GAIM_TYPE_STRING));
  types = g_list_append(types, type);

  type = gaim_status_type_new(GAIM_STATUS_AWAY, MW_STATE_AWAY,
			      _("Away"), TRUE);
  gaim_status_type_add_attr(type, MW_STATE_MESSAGE, _("Message"),
			    gaim_value_new(GAIM_TYPE_STRING));
  types = g_list_append(types, type);
  
  type = gaim_status_type_new(GAIM_STATUS_UNAVAILABLE, MW_STATE_BUSY,
			      _("Do Not Disturb"), TRUE);
  gaim_status_type_add_attr(type, MW_STATE_MESSAGE, _("Message"),
			    gaim_value_new(GAIM_TYPE_STRING));
  types = g_list_append(types, type);
  
  return types;
}


static void conf_create_prompt_cancel(GaimBuddy *buddy,
				      GaimRequestFields *fields) {
  ; /* nothing to do */
}


static void conf_create_prompt_join(GaimBuddy *buddy,
				    GaimRequestFields *fields) {
  GaimAccount *acct;
  GaimConnection *gc;
  struct mwGaimPluginData *pd;
  struct mwServiceConference *srvc;

  GaimRequestField *f;

  const char *topic, *invite;
  struct mwConference *conf;
  struct mwIdBlock idb = { NULL, NULL };

  acct = buddy->account;
  gc = gaim_account_get_connection(acct);
  pd = gc->proto_data;
  srvc = pd->srvc_conf;

  f = gaim_request_fields_get_field(fields, CHAT_KEY_TOPIC);
  topic = gaim_request_field_string_get_value(f);

  f = gaim_request_fields_get_field(fields, CHAT_KEY_INVITE);
  invite = gaim_request_field_string_get_value(f);

  conf = mwConference_new(srvc, topic);
  mwConference_open(conf);

  idb.user = buddy->name;
  mwConference_invite(conf, &idb, invite);
}


static void blist_menu_conf_create(GaimBuddy *buddy, const char *msg) {

  GaimRequestFields *fields;
  GaimRequestFieldGroup *g;
  GaimRequestField *f;

  GaimAccount *acct;
  GaimConnection *gc;

  char *msgA, *msgB;
  
  g_return_if_fail(buddy != NULL);

  acct = buddy->account;
  g_return_if_fail(acct != NULL);

  gc = gaim_account_get_connection(acct);
  g_return_if_fail(gc != NULL);
  
  fields = gaim_request_fields_new();

  g = gaim_request_field_group_new(NULL);
  gaim_request_fields_add_group(fields, g);
  
  f = gaim_request_field_string_new(CHAT_KEY_TOPIC, _("Topic"), NULL, FALSE);
  gaim_request_field_group_add_field(g, f);

  f = gaim_request_field_string_new(CHAT_KEY_INVITE, _("Message"), msg, FALSE);
  gaim_request_field_group_add_field(g, f);
  
  msgA = _("Create conference with user");
  msgB = _("Please enter a topic for the new conference, and an invitation"
	   " message to be sent to %s");
  msgB = g_strdup_printf(msgB, buddy->name);

  gaim_request_fields(gc, _("New Conference"),
		      msgA, msgB, fields,
		      _("Create"), G_CALLBACK(conf_create_prompt_join),
		      _("Cancel"), G_CALLBACK(conf_create_prompt_cancel),
		      buddy);
  g_free(msgB);
}


static void conf_select_prompt_cancel(GaimBuddy *buddy,
				      GaimRequestFields *fields) {
  ;
}


static void conf_select_prompt_invite(GaimBuddy *buddy,
				      GaimRequestFields *fields) {
  GaimRequestField *f;
  const GList *l;
  const char *msg;
  
  f = gaim_request_fields_get_field(fields, CHAT_KEY_INVITE);
  msg = gaim_request_field_string_get_value(f);

  f = gaim_request_fields_get_field(fields, "conf");
  l = gaim_request_field_list_get_selected(f);

  if(l) {
    gpointer d = gaim_request_field_list_get_data(f, l->data);
    
    if(GPOINTER_TO_INT(d) == 0x01) {
      blist_menu_conf_create(buddy, msg);

    } else {
      struct mwIdBlock idb = { buddy->name, NULL };
      mwConference_invite(d, &idb, msg);
    }
  }
}


static void blist_menu_conf_list(GaimBuddy *buddy,
				 GList *confs) {
  
  GaimRequestFields *fields;
  GaimRequestFieldGroup *g;
  GaimRequestField *f;

  GaimAccount *acct;
  GaimConnection *gc;

  char *msgA, *msgB;

  acct = buddy->account;
  g_return_if_fail(acct != NULL);

  gc = gaim_account_get_connection(acct);
  g_return_if_fail(gc != NULL);

  fields = gaim_request_fields_new();
  
  g = gaim_request_field_group_new(NULL);
  gaim_request_fields_add_group(fields, g);

  f = gaim_request_field_list_new("conf", _("Available Conferences"));
  gaim_request_field_list_set_multi_select(f, FALSE);
  for(; confs; confs = confs->next) {
    struct mwConference *c = confs->data;
    gaim_request_field_list_add(f, mwConference_getTitle(c), c);
  }
  gaim_request_field_list_add(f, _("Create New Conference..."),
			      GINT_TO_POINTER(0x01));
  gaim_request_field_group_add_field(g, f);
  
  f = gaim_request_field_string_new(CHAT_KEY_INVITE, "Message", NULL, FALSE);
  gaim_request_field_group_add_field(g, f);
  
  msgA = _("Invite user to a conference");
  msgB = _("Select a conference from the list below to send an invite to"
	   " user %s. Select \"Create New Conference\" if you'd like to"
	   " create a new conference to invite this user to.");
  msgB = g_strdup_printf(msgB, buddy->name);

  gaim_request_fields(gc, _("Invite to Conference"),
		      msgA, msgB, fields,
		      _("Invite"), G_CALLBACK(conf_select_prompt_invite),
		      _("Cancel"), G_CALLBACK(conf_select_prompt_cancel),
		      buddy);
  g_free(msgB);
}


static void blist_menu_conf(GaimBlistNode *node, gpointer data) {
  GaimBuddy *buddy = (GaimBuddy *) node;
  GaimAccount *acct;
  GaimConnection *gc;
  struct mwGaimPluginData *pd;
  GList *l;

  g_return_if_fail(node != NULL);
  g_return_if_fail(GAIM_BLIST_NODE_IS_BUDDY(node));

  acct = buddy->account;
  g_return_if_fail(acct != NULL);

  gc = gaim_account_get_connection(acct);
  g_return_if_fail(gc != NULL);

  pd = gc->proto_data;
  g_return_if_fail(pd != NULL);

  /*
    - get a list of all conferences on this session
    - if none, prompt to create one, and invite buddy to it
    - else, prompt to select a conference or create one
  */

  l = mwServiceConference_getConferences(pd->srvc_conf);
  if(l) {
    blist_menu_conf_list(buddy, l);
    g_list_free(l);

  } else {
    blist_menu_conf_create(buddy, NULL);
  }
}


static GList *mw_prpl_blist_node_menu(GaimBlistNode *node) {
  GList *l = NULL;
  GaimBlistNodeAction *act;

  if(! GAIM_BLIST_NODE_IS_BUDDY(node))
    return l;

  l = g_list_append(l, NULL);

  act = gaim_blist_node_action_new(_("Invite to Conference..."),
				   blist_menu_conf, NULL, NULL);
  l = g_list_append(l, act);

  /** note: this never gets called for a GaimGroup, have to use the
      blist-node-extended-menu signal for that. The function
      blist_node_menu_cb is assigned to this signal in the function
      services_starting */

  return l;
}


static GList *mw_prpl_chat_info(GaimConnection *gc) {
  GList *l = NULL;
  struct proto_chat_entry *pce;
  
  pce = g_new0(struct proto_chat_entry, 1);
  pce->label = _("Topic:");
  pce->identifier = CHAT_KEY_TOPIC;
  l = g_list_append(l, pce);
  
  return l;
}


static GHashTable *mw_prpl_chat_info_defaults(GaimConnection *gc,
					      const char *name) {
  GHashTable *table;

  g_return_val_if_fail(gc != NULL, NULL);

  table = g_hash_table_new_full(g_str_hash, g_str_equal,
				NULL, g_free);

  g_hash_table_insert(table, CHAT_KEY_NAME, g_strdup(name));
  g_hash_table_insert(table, CHAT_KEY_INVITE, NULL);

  return table;
}


static void mw_prpl_login(GaimAccount *acct);


static void prompt_host_cancel_cb(GaimConnection *gc) {
  gaim_connection_error(gc, _("No Sametime Community Server specified"));
}


static void prompt_host_ok_cb(GaimConnection *gc, const char *host) {
  if(host && *host) {
    GaimAccount *acct = gaim_connection_get_account(gc);
    gaim_account_set_string(acct, MW_KEY_HOST, host);
    mw_prpl_login(acct);

  } else {
    prompt_host_cancel_cb(gc);
  }
}


static void prompt_host(GaimConnection *gc) {
  GaimAccount *acct;
  char *msg;
  
  acct = gaim_connection_get_account(gc);
  msg = _("No host or IP address has been configured for the"
	  " Meanwhile account %s. Please enter one below to"
	  " continue logging in.");
  msg = g_strdup_printf(msg, NSTR(gaim_account_get_username(acct)));
  
  gaim_request_input(gc, _("Meanwhile Connection Setup"),
		     _("No Sametime Community Server Specified"), msg,
		     MW_PLUGIN_DEFAULT_HOST, FALSE, FALSE, NULL,
		     _("Connect"), G_CALLBACK(prompt_host_ok_cb),
		     _("Cancel"), G_CALLBACK(prompt_host_cancel_cb),
		     gc);

  g_free(msg);
}


static void mw_prpl_login(GaimAccount *account) {
  GaimConnection *gc;
  struct mwGaimPluginData *pd;

  char *user, *pass, *host;
  guint port;

  gc = gaim_account_get_connection(account);
  pd = mwGaimPluginData_new(gc);

  /* while we do support images, the default is to not offer it */
  gc->flags |= GAIM_CONNECTION_NO_IMAGES;

  user = g_strdup(gaim_account_get_username(account));
  pass = (char *) gaim_account_get_password(account);

  host = strrchr(user, ':');
  if(host) {
    /* annoying user split from 1.2.0, need to undo it */
    *host++ = '\0';
    gaim_account_set_string(account, MW_KEY_HOST, host);
    gaim_account_set_username(account, user);
    
  } else {
    host = (char *) gaim_account_get_string(account, MW_KEY_HOST,
					    MW_PLUGIN_DEFAULT_HOST);
  }

  if(! host || ! *host) {
    /* somehow, we don't have a host to connect to. Well, we need one
       to actually continue, so let's ask the user directly. */
    prompt_host(gc);
    return;
  }

  port = gaim_account_get_int(account, MW_KEY_PORT, MW_PLUGIN_DEFAULT_PORT);

  DEBUG_INFO("user: '%s'\n", user);
  DEBUG_INFO("host: '%s'\n", host);
  DEBUG_INFO("port: %u\n", port);

  mwSession_setProperty(pd->session, mwSession_NO_SECRET,
			(char *) no_secret, NULL);
  mwSession_setProperty(pd->session, mwSession_AUTH_USER_ID, user, g_free);
  mwSession_setProperty(pd->session, mwSession_AUTH_PASSWORD, pass, NULL);
  mwSession_setProperty(pd->session, mwSession_CLIENT_TYPE_ID,
			GUINT_TO_POINTER(MW_CLIENT_TYPE_ID), NULL);

  gaim_connection_update_progress(gc, _("Connecting"), 1, MW_CONNECT_STEPS);

  if(gaim_proxy_connect(account, host, port, connect_cb, pd)) {
    gaim_connection_error(gc, _("Unable to connect to host"));
  }
}


static void mw_prpl_close(GaimConnection *gc) {
  struct mwGaimPluginData *pd;

  g_return_if_fail(gc != NULL);

  pd = gc->proto_data;
  g_return_if_fail(pd != NULL);

  /* get rid of the blist save timeout */
  if(pd->save_event) {
    gaim_timeout_remove(pd->save_event);
    pd->save_event = 0;
    blist_store(pd);
  }

  /* stop the session */
  mwSession_stop(pd->session, 0x00);

  /* no longer necessary */
  gc->proto_data = NULL;

  /* stop watching the socket */
  if(gc->inpa) {
    gaim_input_remove(gc->inpa);
    gc->inpa = 0;
  }

  /* clean up the rest */
  mwGaimPluginData_free(pd);
}


/** generates a random-ish content id string */
static char *im_mime_content_id() {
  const char *c = "%03x@%05xmeanwhile";
  srand(time(0) ^ rand());
  return g_strdup_printf(c, rand() & 0xfff, rand() & 0xfffff);
}


/** generates a multipart/related content type with a random-ish
    boundary value */
static char *im_mime_content_type() {
  const char *c = "multipart/related; boundary=related_MW%03x_%04x";
  srand(time(0) ^ rand());
  return g_strdup_printf(c, rand() & 0xfff, rand() & 0xffff);
}


/** determine content type from extension. Not so happy about this,
    but I don't want to actually write image type detection */
static char *im_mime_img_content_type(GaimStoredImage *img) {
  const char *fn = gaim_imgstore_get_filename(img);
  const char *ct = NULL;

  ct = strrchr(fn, '.');
  if(! ct) {
    ct = "image";

  } else if(! strcmp(".png", ct)) {
    ct = "image/png";

  } else if(! strcmp(".jpg", ct)) {
    ct = "image/jpeg";

  } else if(! strcmp(".jpeg", ct)) {
    ct = "image/jpeg";

  } else if(! strcmp(".gif", ct)) {
    ct = "image/gif";

  } else {
    ct = "image";
  }

  return g_strdup_printf("%s; name=\"%s\"", ct, fn);
}


static char *im_mime_img_content_disp(GaimStoredImage *img) {
  const char *fn = gaim_imgstore_get_filename(img);
  return g_strdup_printf("attachment; filename=\"%s\"", fn);
}


static char *nb_im_encode(GaimConnection *gc, const char *message) {
  GaimAccount *acct;
  const char *enc;
  char *ret;
  GError *error = NULL;
  
  acct = gaim_connection_get_account(gc);
  g_return_val_if_fail(acct != NULL, NULL);

  enc = gaim_account_get_string(acct, MW_KEY_ENCODING,
				MW_PLUGIN_DEFAULT_ENCODING);
  g_return_val_if_fail(enc != NULL, NULL);

  ret = g_convert_with_fallback(message, strlen(message),
				enc, "UTF-8", "?",
				NULL, NULL, &error);
 
 if(error) {
    DEBUG_INFO("problem converting to %s: %s\n",
	       enc, NSTR(error->message));
    g_error_free(error);
 }
 
 /* something went so wrong that not even the fallback worked */
 if(! ret) ret = g_strdup(message);

 return ret;
}


static gboolean is_nb(struct mwConversation *conv) {
  struct mwLoginInfo *info;

  info = mwConversation_getTargetInfo(conv);
  if(! info) return FALSE;

  /* NotesBuddy can be at least three different type IDs (all in the
     0x1400 range), or it can show up as 0x1002. However, if we're
     calling this check, then we're already in HTML or MIME mode, so
     we can discount the real 0x1002 */
  /* I tried to avoid having any client-type-dependant code in here, I
     really did. Oh well. CURSE YOU NOTESBUDDY */
  return ((info->type == 0x1002) || ((info->type & 0xff00) == 0x1400));
}


/** turn an IM with embedded images into a multi-part mime document */
static char *im_mime_convert(GaimConnection *gc,
			     struct mwConversation *conv,
			     const char *message) {
  GString *str;
  GaimMimeDocument *doc;
  GaimMimePart *part;

  GData *attr;
  char *tmp, *start, *end;

  str = g_string_new(NULL);

  doc = gaim_mime_document_new();

  gaim_mime_document_set_field(doc, "Mime-Version", "1.0");
  gaim_mime_document_set_field(doc, "Content-Disposition", "inline");

  tmp = im_mime_content_type();
  gaim_mime_document_set_field(doc, "Content-Type", tmp);
  g_free(tmp);

  tmp = (char *) message;
  while(*tmp && gaim_markup_find_tag("img", tmp, (const char **) &start,
				     (const char **) &end, &attr)) {
    char *id;
    GaimStoredImage *img = NULL;
    
    gsize len = (start - tmp);

    /* append the in-between-tags text */
    if(len) g_string_append_len(str, tmp, len);

    /* find the imgstore data by the id tag */
    id = g_datalist_get_data(&attr, "id");
    if(id && *id)
      img = gaim_imgstore_get(atoi(id));

    if(img) {
      char *cid;
      gpointer data;
      size_t size;

      part = gaim_mime_part_new(doc);

      data = im_mime_img_content_disp(img);
      gaim_mime_part_set_field(part, "Content-Disposition", data);
      g_free(data);

      data = im_mime_img_content_type(img);
      gaim_mime_part_set_field(part, "Content-Type", data);
      g_free(data);

      cid = im_mime_content_id();
      data = g_strdup_printf("<%s>", cid);
      gaim_mime_part_set_field(part, "Content-ID", data);
      g_free(data);

      gaim_mime_part_set_field(part, "Content-transfer-encoding", "base64");

      /* obtain and base64 encode the image data, and put it in the
	 mime part */
      data = gaim_imgstore_get_data(img);
      size = gaim_imgstore_get_size(img);
      data = gaim_base64_encode(data, (gsize) size);
      gaim_mime_part_set_data(part, data);
      g_free(data);

      /* append the modified tag */
      g_string_append_printf(str, "<img src=\"cid:%s\">", cid);
      g_free(cid);
      
    } else {
      /* append the literal image tag, since we couldn't find a
	 relative imgstore object */
      gsize len = (end - start) + 1;
      g_string_append_len(str, start, len);
    }

    g_datalist_clear(&attr);
    tmp = end + 1;
  }

  /* append left-overs */
  g_string_append(str, tmp);

  /* add the text/html part */
  part = gaim_mime_part_new(doc);
  gaim_mime_part_set_field(part, "Content-Disposition", "inline");

  if(is_nb(conv)) {
    GaimAccount *acct = gaim_connection_get_account(gc);

    tmp = (char *) gaim_account_get_string(acct, MW_KEY_ENCODING,
					   MW_PLUGIN_DEFAULT_ENCODING);
    tmp = g_strdup_printf("text/html; charset=\"%s\"", tmp);
    gaim_mime_part_set_field(part, "Content-Type", tmp);
    g_free(tmp);
    
    gaim_mime_part_set_field(part, "Content-Transfer-Encoding", "7bit");

    tmp = nb_im_encode(gc, str->str);
    gaim_mime_part_set_data(part, tmp);
    g_free(tmp);

  } else {
    gaim_mime_part_set_field(part, "Content-Type", "text/html");
    gaim_mime_part_set_field(part, "Content-Transfer-Encoding", "8bit");
    gaim_mime_part_set_data(part, str->str);
  }

  g_string_free(str, TRUE);

  str = g_string_new(NULL);
  gaim_mime_document_write(doc, str);
  tmp = str->str;
  g_string_free(str, FALSE);

  return tmp;
}


static int mw_prpl_send_im(GaimConnection *gc,
			   const char *name,
			   const char *message,
			   GaimConvImFlags flags) {

  struct mwGaimPluginData *pd;
  struct mwIdBlock who = { (char *) name, NULL };
  struct mwConversation *conv;

  g_return_val_if_fail(gc != NULL, 0);
  pd = gc->proto_data;

  g_return_val_if_fail(pd != NULL, 0);

  conv = mwServiceIm_getConversation(pd->srvc_im, &who);

  /* this detection of features to determine how to send the message
     (plain, html, or mime) is flawed because the other end of the
     conversation could close their channel at any time, rendering any
     existing formatting in an outgoing message innapropriate. The end
     result is that it may be possible that the other side of the
     conversation will receive a plaintext message with html contents,
     which is bad. I'm not sure how to fix this correctly. */

  if(strstr(message, "<img ") || strstr(message, "<IMG "))
    flags |= GAIM_CONV_IM_IMAGES;

  if(mwConversation_isOpen(conv)) {
    char *tmp;
    int ret;

    if((flags & GAIM_CONV_IM_IMAGES) &&
       mwConversation_supports(conv, mwImSend_MIME)) {
      /* send a MIME message */

      tmp = im_mime_convert(gc, conv, message);
      ret = mwConversation_send(conv, mwImSend_MIME, tmp);
      g_free(tmp);
      
    } else if(mwConversation_supports(conv, mwImSend_HTML)) {
      /* send an HTML message */

      /* need to do this to get the \n to <br> conversion */
      if(is_nb(conv)) {

	/* html messages need the notesbuddy hack */
	char *msg = nb_im_encode(gc, message);
	tmp = gaim_strdup_withhtml(msg);	
	g_free(msg);

      } else {
	tmp = gaim_strdup_withhtml(message);
      }

      ret = mwConversation_send(conv, mwImSend_HTML, tmp);
      g_free(tmp);

    } else {
      /* default to text */
      ret = mwConversation_send(conv, mwImSend_PLAIN, message);
    }
    
    return !ret;

  } else {

    /* queue up the message safely as plain text */
    char *tmp = gaim_markup_strip_html(message);
    convo_queue(conv, mwImSend_PLAIN, tmp);
    g_free(tmp);

    if(! mwConversation_isPending(conv))
      mwConversation_open(conv);

    return 1;
  }
}


static int mw_prpl_send_typing(GaimConnection *gc, const char *name,
			       int typing) {
  
  struct mwGaimPluginData *pd;
  struct mwIdBlock who = { (char *) name, NULL };
  struct mwConversation *conv;

  gpointer t = GINT_TO_POINTER(!! typing);

  g_return_val_if_fail(gc != NULL, 0);
  pd = gc->proto_data;

  g_return_val_if_fail(pd != NULL, 0);

  conv = mwServiceIm_getConversation(pd->srvc_im, &who);

  if(mwConversation_isOpen(conv))
    return ! mwConversation_send(conv, mwImSend_TYPING, t);

  if(typing) {
    /* let's only open a channel for typing, not for not-typing.
       Otherwise two users in psychic mode will continually open
       conversations to each other, never able to get rid of them, as
       when the other person closes, it psychicaly opens again */

    convo_queue(conv, mwImSend_TYPING, t);

    if(! mwConversation_isPending(conv))
      mwConversation_open(conv);
  }

  return 1;
}


static void mw_prpl_get_info(GaimConnection *gc, const char *who) {

  struct mwAwareIdBlock idb = { mwAware_USER, (char *) who, NULL };

  struct mwGaimPluginData *pd;
  GaimAccount *acct;
  GaimBuddy *b;
  
  GString *str;
  const char *tmp;

  g_return_if_fail(who != NULL);
  g_return_if_fail(*who != '\0');

  pd = gc->proto_data;

  acct = gaim_connection_get_account(gc);
  b = gaim_find_buddy(acct, who);

  str = g_string_new(NULL);

  if(g_str_has_prefix(who, "@E ")) {
    g_string_append(str, _("<b>External User</b><br>"));
  }

  g_string_append_printf(str, _("<b>User ID:</b> %s<br>"), who);

  if(b) {
    guint32 type;

    if(b->server_alias) {
      g_string_append_printf(str, _("<b>Full Name:</b> %s<br>"),
			     b->server_alias);
    }

    type = gaim_blist_node_get_int((GaimBlistNode *) b, BUDDY_KEY_CLIENT);
    if(type) {
      g_string_append(str, _("<b>Last Known Client:</b> "));

      tmp = mwLoginType_getName(type);
      if(tmp) {
	g_string_append(str, tmp);
	g_string_append(str, "<br>");
	
      } else {
	g_string_append_printf(str, _("Unknown (0x%04x)<br>"), type);
      }
    }
  }

  tmp = user_supports_text(pd->srvc_aware, who);
  if(tmp) {
    g_string_append_printf(str, _("<b>Supports:</b> %s<br>"), tmp);
    g_free((char *) tmp);
  }

  if(b) {
    tmp = status_text(b);
    g_string_append_printf(str, _("<b>Status:</b> %s"), tmp);

    g_string_append(str, "<hr>");
    
    tmp = mwServiceAware_getText(pd->srvc_aware, &idb);
    if(tmp) g_string_append(str, tmp);
  }

  /* @todo emit a signal to allow a plugin to override the display of
     this notification, so that it can create its own */

  gaim_notify_userinfo(gc, who, str->str, NULL, NULL);

  g_string_free(str, TRUE);
}
 
 
static void mw_prpl_set_status(GaimAccount *acct, GaimStatus *status) {
  GaimConnection *gc;
  const char *state;
  char *message = NULL;
  struct mwSession *session;
  struct mwUserStatus stat;
  
  g_return_if_fail(acct != NULL);
  gc = gaim_account_get_connection(acct);
  
  state = gaim_status_get_id(status);
  
  DEBUG_INFO("Set status to %s\n", gaim_status_get_name(status));
  
  g_return_if_fail(gc != NULL);
  
  session = gc_to_session(gc);
  g_return_if_fail(session != NULL);
  
  /* get a working copy of the current status */
  mwUserStatus_clone(&stat, mwSession_getUserStatus(session));
  
  /* determine the state */
  if(! strcmp(state, MW_STATE_ACTIVE)) {
    stat.status = mwStatus_ACTIVE;
    
  } else if(! strcmp(state, MW_STATE_AWAY)) {
    stat.status = mwStatus_AWAY;
    
  } else if(! strcmp(state, MW_STATE_BUSY)) {
    stat.status = mwStatus_BUSY;
  }
  
  /* determine the message */
  message = (char *) gaim_status_get_attr_string(status, MW_STATE_MESSAGE);
  
  if(message) {
    /* all the possible non-NULL values of message up to this point
       are const, so we don't need to free them */
    message = gaim_markup_strip_html(message);
  }
  
  /* out with the old */
  g_free(stat.desc);
  
  /* in with the new */
  stat.desc = (char *) message;
  
  mwSession_setUserStatus(session, &stat);
  mwUserStatus_clear(&stat);
}

 
static void mw_prpl_set_idle(GaimConnection *gc, int t) {
  struct mwSession *session;
  struct mwUserStatus stat;
 

  session = gc_to_session(gc);
  g_return_if_fail(session != NULL);

  mwUserStatus_clone(&stat, mwSession_getUserStatus(session));

  if(t) {
    time_t now = time(NULL);
    stat.time = now - t;

  } else {
    stat.time = 0;
  }

  if(t > 0 && stat.status == mwStatus_ACTIVE) {
    /* we were active and went idle, so change the status to IDLE. */
    stat.status = mwStatus_IDLE;

  } else if(t == 0 && stat.status == mwStatus_IDLE) {
    /* we only become idle automatically, so change back to ACTIVE */
    stat.status = mwStatus_ACTIVE;
  }

  mwSession_setUserStatus(session, &stat);
  mwUserStatus_clear(&stat);
 }


static struct resolved_id *resolved_id_new(const char *id,
					   const char *name) {

  struct resolved_id *rid = g_new0(struct resolved_id, 1);
  rid->id = g_strdup(id);
  rid->name = g_strdup(name);
  return rid;
}


static void resolved_id_free(struct resolved_id *rid) {
  if(rid) {
    g_free(rid->id);
    g_free(rid->name);
    g_free(rid);
  }
}


static void add_resolved_done(const char *id, const char *name,
			      GaimBuddy *buddy) {
  GaimAccount *acct;
  GaimConnection *gc;
  struct mwGaimPluginData *pd;

  g_return_if_fail(id != NULL);

  g_return_if_fail(buddy != NULL);
  acct = buddy->account;

  g_return_if_fail(acct != NULL);
  gc = gaim_account_get_connection(acct);

  g_return_if_fail(gc != NULL);
  pd = gc->proto_data;

  gaim_blist_rename_buddy(buddy, id);
  
  gaim_blist_server_alias_buddy(buddy, name);
  gaim_blist_node_set_string((GaimBlistNode *) buddy, BUDDY_KEY_NAME, name);
  
  buddy_add(pd, buddy);
}


static void multi_resolved_cleanup(GaimRequestFields *fields) {
  GaimRequestField *f;
  const GList *l;

  f = gaim_request_fields_get_field(fields, "user");
  l = gaim_request_field_list_get_items(f);

  for(; l; l = l->next) {
    const char *i = l->data;
    struct resolved_id *res;

    res = gaim_request_field_list_get_data(f, i);
    resolved_id_free(res);
  }
}


static void multi_resolved_cancel(GaimBuddy *buddy,
				  GaimRequestFields *fields) {
  GaimConnection *gc;
  struct mwGaimPluginData *pd;

  gc = gaim_account_get_connection(buddy->account);
  pd = gc->proto_data;

  gaim_blist_remove_buddy(buddy);
  multi_resolved_cleanup(fields);

  blist_schedule(pd);
}


static void multi_resolved_cb(GaimBuddy *buddy,
			      GaimRequestFields *fields) {
  GaimRequestField *f;
  const GList *l;

  f = gaim_request_fields_get_field(fields, "user");
  l = gaim_request_field_list_get_selected(f);

  if(l) {
    const char *i = l->data;
    struct resolved_id *res;

    res = gaim_request_field_list_get_data(f, i);

    add_resolved_done(res->id, res->name, buddy);
    multi_resolved_cleanup(fields);

  } else {
    multi_resolved_cancel(buddy, fields);
  }
}


static void foreach_resolved_id(char *key, char *val, GList **l) {
  struct resolved_id *res = resolved_id_new(key, val);
  *l = g_list_prepend(*l, res);
}


static gint resolved_id_comp(struct resolved_id *a, struct resolved_id *b) {
  return g_ascii_strcasecmp(a->name, b->name);
}


static void multi_resolved_query(struct mwResolveResult *result,
				 GaimBuddy *buddy) {
  GaimRequestFields *fields;
  GaimRequestFieldGroup *g;
  GaimRequestField *f;
  GHashTable *hash;
  GList *l;
  char *msgA, *msgB;

  GaimAccount *acct;
  GaimConnection *gc;

  g_return_if_fail(buddy != NULL);

  acct = buddy->account;
  g_return_if_fail(acct != NULL);

  gc = gaim_account_get_connection(acct);
  g_return_if_fail(gc != NULL);

  fields = gaim_request_fields_new();

  g = gaim_request_field_group_new(NULL);

  /* note that Gaim segfaults if you don't add the group to the fields
     before you add a required field to the group. Feh. */
  gaim_request_fields_add_group(fields, g);

  f = gaim_request_field_list_new("user", _("Possible Matches"));
  gaim_request_field_list_set_multi_select(f, FALSE);
  gaim_request_field_set_required(f, TRUE);

  /* collect results into a set of identities */
  hash = g_hash_table_new(g_str_hash, g_str_equal);
  for(l = result->matches; l; l = l->next) {
    struct mwResolveMatch *match = l->data;
    
    if(!match->id || !match->name)
      continue;
    
    g_hash_table_insert(hash, match->id, match->name);
  }
  
  /* collect set into a list of structures */
  l = NULL;
  g_hash_table_foreach(hash, (GHFunc) foreach_resolved_id, &l);
  g_list_sort(l, (GCompareFunc) resolved_id_comp);

  /* populate choices in request field */
  for(; l; l = l->next) {
    struct resolved_id *res = l->data;
    char *label;
    
    /* fixes bug 1178603 by making the selection label a combination
       of the full name and the user id. Problems arrise when multiple
       entries have identical labels */
    label = g_strdup_printf("%s (%s)", NSTR(res->name), NSTR(res->id));
    gaim_request_field_list_add(f, label, res);
    g_free(label);
  }

  g_list_free(l);

  gaim_request_field_group_add_field(g, f);

  msgA = _("An ambiguous user ID was entered");
  msgB = _("The identifier '%s' may possibly refer to any of the following"
	   " users. Please select the correct user from the list below to"
	   " add them to your buddy list.");
  msgB = g_strdup_printf(msgB, result->name);

  gaim_request_fields(gc, _("Select User to Add"),
		      msgA, msgB, fields,
		      _("Add User"), G_CALLBACK(multi_resolved_cb),
		      _("Cancel"), G_CALLBACK(multi_resolved_cancel),
		      buddy);
  g_free(msgB);
}


static void add_buddy_resolved(struct mwServiceResolve *srvc,
			       guint32 id, guint32 code, GList *results,
			       gpointer b) {

  struct mwResolveResult *res = NULL;
  GaimBuddy *buddy = b;
  GaimConnection *gc;
  struct mwGaimPluginData *pd;

  gc = gaim_account_get_connection(buddy->account);
  pd = gc->proto_data;

  if(results)
    res = results->data;

  if(!code && res && res->matches) {
    if(g_list_length(res->matches) == 1) {
      struct mwResolveMatch *match = res->matches->data;
      
      DEBUG_INFO("searched for %s, got only %s\n",
		 NSTR(res->name), NSTR(match->id));
      
      /* only one? that might be the right one! */
      if(strcmp(res->name, match->id)) {
	/* uh oh, the single result isn't identical to the search
	   term, better safe then sorry, so let's make sure it's who
	   the user meant to add */
	multi_resolved_query(res, buddy);
	
      } else {
	/* same person, add 'em */
	add_resolved_done(match->id, match->name, buddy);
      }
      
    } else {
      /* prompt user if more than one match was returned */
      multi_resolved_query(res, buddy);
    }
    
    return;
  }

  /* fall-through indicates that we couldn't find a matching user in
     the resolve service (ether error or zero results), so we remove
     this buddy */

  DEBUG_INFO("no such buddy in community\n");
  gaim_blist_remove_buddy(buddy);
  blist_schedule(pd);

  if(res && res->name) {
    /* compose and display an error message */
    char *msgA, *msgB;

    msgA = _("Unable to add user: user not found");

    msgB = _("The identifier '%s' did not match any users in your"
	     " Sametime community. This entry has been removed from"
	     " your buddy list.");
    msgB = g_strdup_printf(msgB, NSTR(res->name));

    gaim_notify_error(gc, _("Unable to add user"), msgA, msgB);

    g_free(msgB);
  }
}


static void mw_prpl_add_buddy(GaimConnection *gc,
			      GaimBuddy *buddy,
			      GaimGroup *group) {

  struct mwGaimPluginData *pd;
  struct mwServiceResolve *srvc;
  GList *query;
  enum mwResolveFlag flags;
  guint32 req;

  pd = gc->proto_data;
  srvc = pd->srvc_resolve;

  /* catch external buddies. They won't be in the resolve service */
  if(g_str_has_prefix(buddy->name, "@E ")) {
    buddy_add(pd, buddy);
    return;
  }

  query = g_list_prepend(NULL, buddy->name);
  flags = mwResolveFlag_FIRST | mwResolveFlag_USERS;

  req = mwServiceResolve_resolve(srvc, query, flags, add_buddy_resolved,
				 buddy, NULL);
  g_list_free(query);

  if(req == SEARCH_ERROR) {
    gaim_blist_remove_buddy(buddy);
    blist_schedule(pd);
  }
}


static void foreach_add_buddies(GaimGroup *group, GList *buddies,
				struct mwGaimPluginData *pd) {
  struct mwAwareList *list;

  list = list_ensure(pd, group);
  mwAwareList_addAware(list, buddies);
  g_list_free(buddies);
}


static void mw_prpl_add_buddies(GaimConnection *gc,
				GList *buddies,
				GList *groups) {

  struct mwGaimPluginData *pd;
  GHashTable *group_sets;
  struct mwAwareIdBlock *idbs, *idb;

  pd = gc->proto_data;

  /* map GaimGroup:GList of mwAwareIdBlock */
  group_sets = g_hash_table_new(g_direct_hash, g_direct_equal);

  /* bunch of mwAwareIdBlock allocated at once, free'd at once */
  idb = idbs = g_new(struct mwAwareIdBlock, g_list_length(buddies));

  /* first pass collects mwAwareIdBlock lists for each group */
  for(; buddies; buddies = buddies->next) {
    GaimBuddy *b = buddies->data;
    GaimGroup *g;
    const char *fn;
    GList *l;

    /* nab the saved server alias and stick it on the buddy */
    fn = gaim_blist_node_get_string((GaimBlistNode *) b, BUDDY_KEY_NAME);
    gaim_blist_server_alias_buddy(b, fn);

    /* convert GaimBuddy into a mwAwareIdBlock */
    idb->type = mwAware_USER;
    idb->user = (char *) b->name;
    idb->community = NULL;

    /* put idb into the list associated with the buddy's group */
    g = gaim_buddy_get_group(b);
    l = g_hash_table_lookup(group_sets, g);
    l = g_list_prepend(l, idb++);
    g_hash_table_insert(group_sets, g, l);
  }

  /* each group's buddies get added in one shot, and schedule the blist
     for saving */
  g_hash_table_foreach(group_sets, (GHFunc) foreach_add_buddies, pd);
  blist_schedule(pd);

  /* cleanup */
  g_hash_table_destroy(group_sets);
  g_free(idbs);
}


static void mw_prpl_remove_buddy(GaimConnection *gc,
				 GaimBuddy *buddy, GaimGroup *group) {

  struct mwGaimPluginData *pd;
  struct mwAwareIdBlock idb = { mwAware_USER, buddy->name, NULL };
  struct mwAwareList *list;

  GList *rem = g_list_prepend(NULL, &idb);

  pd = gc->proto_data;
  group = gaim_buddy_get_group(buddy);
  list = list_ensure(pd, group);

  mwAwareList_removeAware(list, rem);
  blist_schedule(pd);

  g_list_free(rem);
}


static void privacy_fill(struct mwPrivacyInfo *priv,
			 GSList *members) {
  
  struct mwUserItem *u;
  guint count;

  count = g_slist_length(members);
  DEBUG_INFO("privacy_fill: %u members\n", count);

  priv->count = count;
  priv->users = g_new0(struct mwUserItem, count);

  while(count--) {
    u = priv->users + count;
    u->id = members->data;
    members = members->next;
  }
}


static void mw_prpl_set_permit_deny(GaimConnection *gc) {
  GaimAccount *acct;
  struct mwGaimPluginData *pd;
  struct mwSession *session;

  struct mwPrivacyInfo privacy = {
    .deny = FALSE,
    .count = 0,
    .users = NULL,
  };

  g_return_if_fail(gc != NULL);

  acct = gaim_connection_get_account(gc);
  g_return_if_fail(acct != NULL);

  pd = gc->proto_data;
  g_return_if_fail(pd != NULL);

  session = pd->session;
  g_return_if_fail(session != NULL);

  switch(acct->perm_deny) {
  case GAIM_PRIVACY_DENY_USERS:
    DEBUG_INFO("GAIM_PRIVACY_DENY_USERS\n");
    privacy_fill(&privacy, acct->deny);
    privacy.deny = TRUE;
    break;

  case GAIM_PRIVACY_ALLOW_ALL:
    DEBUG_INFO("GAIM_PRIVACY_ALLOW_ALL\n");
    privacy.deny = TRUE;
    break;

  case GAIM_PRIVACY_ALLOW_USERS:
    DEBUG_INFO("GAIM_PRIVACY_ALLOW_USERS\n");
    privacy_fill(&privacy, acct->permit);
    privacy.deny = FALSE;
    break;

  case GAIM_PRIVACY_DENY_ALL:
    DEBUG_INFO("GAIM_PRIVACY_DENY_ALL\n");
    privacy.deny = FALSE;
    break;
    
  default:
    DEBUG_INFO("acct->perm_deny is 0x%x\n", acct->perm_deny);
    return;
  }

  mwSession_setPrivacyInfo(session, &privacy);
  g_free(privacy.users);
}


static void mw_prpl_add_permit(GaimConnection *gc, const char *name) {
  mw_prpl_set_permit_deny(gc);
}


static void mw_prpl_add_deny(GaimConnection *gc, const char *name) {
  mw_prpl_set_permit_deny(gc);
}


static void mw_prpl_rem_permit(GaimConnection *gc, const char *name) {
  mw_prpl_set_permit_deny(gc);
}


static void mw_prpl_rem_deny(GaimConnection *gc, const char *name) {
  mw_prpl_set_permit_deny(gc);
}


static struct mwConference *conf_find(struct mwServiceConference *srvc,
				      const char *name) {
  GList *l, *ll;
  struct mwConference *conf = NULL;

  ll = mwServiceConference_getConferences(srvc);
  for(l = ll; l; l = l->next) {
    struct mwConference *c = l->data;
    if(! strcmp(name, mwConference_getName(c))) {
      conf = c;
      break;
    }
  }
  g_list_free(ll);

  return conf;
}


static void mw_prpl_join_chat(GaimConnection *gc,
			      GHashTable *components) {

  struct mwGaimPluginData *pd;
  char *c, *t;
  
  pd = gc->proto_data;

  c = g_hash_table_lookup(components, CHAT_KEY_NAME);
  t = g_hash_table_lookup(components, CHAT_KEY_TOPIC);
  
  if(g_hash_table_lookup(components, CHAT_KEY_IS_PLACE)) {
    /* use place service */
    struct mwServicePlace *srvc;
    struct mwPlace *place = NULL;

    srvc = pd->srvc_place;
    place = mwPlace_new(srvc, c, t);
    mwPlace_open(place);
     
  } else {
    /* use conference service */
    struct mwServiceConference *srvc;
    struct mwConference *conf = NULL;

    srvc = pd->srvc_conf;
    if(c) conf = conf_find(srvc, c);

    if(conf) {
      DEBUG_INFO("accepting conference invitation\n");
      mwConference_accept(conf);
      
    } else {
      DEBUG_INFO("creating new conference\n");
      conf = mwConference_new(srvc, t);
      mwConference_open(conf);
    }
  }
}


static void mw_prpl_reject_chat(GaimConnection *gc,
				GHashTable *components) {

  struct mwGaimPluginData *pd;
  struct mwServiceConference *srvc;
  char *c;
  
  pd = gc->proto_data;
  srvc = pd->srvc_conf;

  if(g_hash_table_lookup(components, CHAT_KEY_IS_PLACE)) {
    ; /* nothing needs doing */

  } else {
    /* reject conference */
    c = g_hash_table_lookup(components, CHAT_KEY_NAME);
    if(c) {
      struct mwConference *conf = conf_find(srvc, c);
      if(conf) mwConference_reject(conf, ERR_SUCCESS, "Declined");
    }
  }
}


static char *mw_prpl_get_chat_name(GHashTable *components) {
  return g_hash_table_lookup(components, CHAT_KEY_NAME);
}


static void mw_prpl_chat_invite(GaimConnection *gc,
				int id,
				const char *invitation,
				const char *who) {

  struct mwGaimPluginData *pd;
  struct mwConference *conf;
  struct mwPlace *place;
  struct mwIdBlock idb = { (char *) who, NULL };

  pd = gc->proto_data;
  g_return_if_fail(pd != NULL);

  conf = ID_TO_CONF(pd, id);

  if(conf) {
    mwConference_invite(conf, &idb, invitation);
    return;
  }

  place = ID_TO_PLACE(pd, id);
  g_return_if_fail(place != NULL);

  /* @todo: use the IM service for invitation */
  mwPlace_legacyInvite(place, &idb, invitation);
}


static void mw_prpl_chat_leave(GaimConnection *gc,
			       int id) {

  struct mwGaimPluginData *pd;
  struct mwConference *conf;

  pd = gc->proto_data;

  g_return_if_fail(pd != NULL);
  conf = ID_TO_CONF(pd, id);

  if(conf) {
    mwConference_destroy(conf, ERR_SUCCESS, "Leaving");

  } else {
    struct mwPlace *place = ID_TO_PLACE(pd, id);
    g_return_if_fail(place != NULL);

    mwPlace_destroy(place, ERR_SUCCESS);
  }
}


static void mw_prpl_chat_whisper(GaimConnection *gc,
				 int id,
				 const char *who,
				 const char *message) {

  mw_prpl_send_im(gc, who, message, 0);
}


static int mw_prpl_chat_send(GaimConnection *gc,
			     int id,
			     const char *message) {

  struct mwGaimPluginData *pd;
  struct mwConference *conf;

  pd = gc->proto_data;

  g_return_val_if_fail(pd != NULL, 0);
  conf = ID_TO_CONF(pd, id);

  if(conf) {
    return ! mwConference_sendText(conf, message);

  } else {
    struct mwPlace *place = ID_TO_PLACE(pd, id);
    g_return_val_if_fail(place != NULL, 0);

    return ! mwPlace_sendText(place, message);
  }
}


static void mw_prpl_keepalive(GaimConnection *gc) {
  struct mwSession *session;

  g_return_if_fail(gc != NULL);

  session = gc_to_session(gc);
  g_return_if_fail(session != NULL);

  mwSession_sendKeepalive(session);
}


static void mw_prpl_alias_buddy(GaimConnection *gc,
				const char *who,
				const char *alias) {

  struct mwGaimPluginData *pd = gc->proto_data;
  g_return_if_fail(pd != NULL);

  /* it's a change to the buddy list, so we've gotta reflect that in
     the server copy */

  blist_schedule(pd);
}


static void mw_prpl_group_buddy(GaimConnection *gc,
				const char *who,
				const char *old_group,
				const char *new_group) {

  struct mwAwareIdBlock idb = { mwAware_USER, (char *) who, NULL };
  GList *gl = g_list_prepend(NULL, &idb);

  struct mwGaimPluginData *pd = gc->proto_data;
  GaimGroup *group;
  struct mwAwareList *list;

  /* add who to new_group's aware list */
  group = gaim_find_group(new_group);
  list = list_ensure(pd, group);
  mwAwareList_addAware(list, gl);

  /* remove who from old_group's aware list */
  group = gaim_find_group(old_group);
  list = list_ensure(pd, group);
  mwAwareList_removeAware(list, gl);

  g_list_free(gl);

  /* schedule the changes to be saved */
  blist_schedule(pd);
}


static void mw_prpl_rename_group(GaimConnection *gc,
				 const char *old,
				 GaimGroup *group,
				 GList *buddies) {

  struct mwGaimPluginData *pd = gc->proto_data;
  g_return_if_fail(pd != NULL);

  /* it's a change in the buddy list, so we've gotta reflect that in
     the server copy. Also, having this function should prevent all
     those buddies from being removed and re-added. We don't really
     give a crap what the group is named in Gaim other than to record
     that as the group name/alias */

  blist_schedule(pd);
}


static void mw_prpl_buddy_free(GaimBuddy *buddy) {
  /* I don't think we have any cleanup for buddies yet */
  ;
}


static void mw_prpl_convo_closed(GaimConnection *gc, const char *who) {
  struct mwGaimPluginData *pd = gc->proto_data;
  struct mwServiceIm *srvc;
  struct mwConversation *conv;
  struct mwIdBlock idb = { (char *) who, NULL };

  g_return_if_fail(pd != NULL);

  srvc = pd->srvc_im;
  g_return_if_fail(srvc != NULL);

  conv = mwServiceIm_findConversation(srvc, &idb);
  if(! conv) return;

  if(mwConversation_isOpen(conv))
    mwConversation_free(conv);
}


static const char *mw_prpl_normalize(const GaimAccount *account,
				     const char *id) {

  /* code elsewhere assumes that the return value points to different
     memory than the passed value, but it won't free the normalized
     data. wtf? */

  static char buf[BUF_LEN];
  strncpy(buf, id, sizeof(buf));
  return buf;
}


static void mw_prpl_remove_group(GaimConnection *gc, GaimGroup *group) {
  struct mwGaimPluginData *pd;
  struct mwAwareList *list;

  pd = gc->proto_data;
  g_return_if_fail(pd != NULL);
  g_return_if_fail(pd->group_list_map != NULL);

  list = g_hash_table_lookup(pd->group_list_map, group);

  if(list) {
    g_hash_table_remove(pd->group_list_map, list);
    g_hash_table_remove(pd->group_list_map, group);
    mwAwareList_free(list);

    blist_schedule(pd);
  }
}


static gboolean mw_prpl_can_receive_file(GaimConnection *gc,
					 const char *who) {
  struct mwGaimPluginData *pd;
  struct mwServiceAware *srvc;
  GaimAccount *acct;

  g_return_val_if_fail(gc != NULL, FALSE);

  pd = gc->proto_data;
  g_return_val_if_fail(pd != NULL, FALSE);

  srvc = pd->srvc_aware;
  g_return_val_if_fail(srvc != NULL, FALSE);
  
  acct = gaim_connection_get_account(gc);
  g_return_val_if_fail(acct != NULL, FALSE);

  return gaim_find_buddy(acct, who) &&
    user_supports(srvc, who, mwAttribute_FILE_TRANSFER);
}


static void ft_outgoing_init(GaimXfer *xfer) {
  GaimAccount *acct;
  GaimConnection *gc;

  struct mwGaimPluginData *pd;
  struct mwServiceFileTransfer *srvc;
  struct mwFileTransfer *ft;

  const char *filename;
  gsize filesize;
  FILE *fp;

  struct mwIdBlock idb = { NULL, NULL };

  DEBUG_INFO("ft_outgoing_init\n");

  acct = gaim_xfer_get_account(xfer);
  gc = gaim_account_get_connection(acct);
  pd = gc->proto_data;
  srvc = pd->srvc_ft;

  filename = gaim_xfer_get_local_filename(xfer);
  filesize = gaim_xfer_get_size(xfer);
  idb.user = xfer->who;

  /* test that we can actually send the file */
  fp = g_fopen(filename, "rb");
  if(! fp) {
    char *msg = g_strdup_printf(_("Error reading file %s: \n%s\n"),
				filename, strerror(errno));
    gaim_xfer_error(gaim_xfer_get_type(xfer), acct, xfer->who, msg);
    g_free(msg);
    return;
  }
  fclose(fp);

  {
    char *tmp = strrchr(filename, G_DIR_SEPARATOR);
    if(tmp++) filename = tmp;
  }
  
  ft = mwFileTransfer_new(srvc, &idb, NULL, filename, filesize);

  gaim_xfer_ref(xfer);
  mwFileTransfer_setClientData(ft, xfer, (GDestroyNotify) gaim_xfer_unref);
  xfer->data = ft;

  mwFileTransfer_offer(ft);
}


static void ft_outgoing_cancel(GaimXfer *xfer) {
  struct mwFileTransfer *ft = xfer->data;
  if(ft) mwFileTransfer_cancel(ft);
}


static void mw_prpl_send_file(GaimConnection *gc,
			      const char *who, const char *file) {

  GaimAccount *acct;
  GaimXfer *xfer;

  acct = gaim_connection_get_account(gc);

  xfer = gaim_xfer_new(acct, GAIM_XFER_SEND, who);
  gaim_xfer_set_init_fnc(xfer, ft_outgoing_init);
  gaim_xfer_set_cancel_send_fnc(xfer, ft_outgoing_cancel);

  if(file) {
    DEBUG_INFO("file != NULL\n");
    gaim_xfer_request_accepted(xfer, file);

  } else {
    DEBUG_INFO("file == NULL\n");
    gaim_xfer_request(xfer);
  }
}


static GaimPluginProtocolInfo mw_prpl_info = {
  .options                   = OPT_PROTO_IM_IMAGE,
  .user_splits               = NULL, /*< set in mw_plugin_init */
  .protocol_options          = NULL, /*< set in mw_plugin_init */
  .icon_spec                 = NO_BUDDY_ICONS,
  .list_icon                 = mw_prpl_list_icon,
  .list_emblems              = mw_prpl_list_emblems,
  .status_text               = mw_prpl_status_text,
  .tooltip_text              = mw_prpl_tooltip_text,
  .status_types              = mw_prpl_status_types,
  .blist_node_menu           = mw_prpl_blist_node_menu,
  .chat_info                 = mw_prpl_chat_info,
  .chat_info_defaults        = mw_prpl_chat_info_defaults,
  .login                     = mw_prpl_login,
  .close                     = mw_prpl_close,
  .send_im                   = mw_prpl_send_im,
  .set_info                  = NULL,
  .send_typing               = mw_prpl_send_typing,
  .get_info                  = mw_prpl_get_info,
  .set_status                = mw_prpl_set_status,
  .set_idle                  = mw_prpl_set_idle,
  .change_passwd             = NULL,
  .add_buddy                 = mw_prpl_add_buddy,
  .add_buddies               = mw_prpl_add_buddies,
  .remove_buddy              = mw_prpl_remove_buddy,
  .remove_buddies            = NULL,
  .add_permit                = mw_prpl_add_permit,
  .add_deny                  = mw_prpl_add_deny,
  .rem_permit                = mw_prpl_rem_permit,
  .rem_deny                  = mw_prpl_rem_deny,
  .set_permit_deny           = mw_prpl_set_permit_deny,
  .join_chat                 = mw_prpl_join_chat,
  .reject_chat               = mw_prpl_reject_chat,
  .get_chat_name             = mw_prpl_get_chat_name,
  .chat_invite               = mw_prpl_chat_invite,
  .chat_leave                = mw_prpl_chat_leave,
  .chat_whisper              = mw_prpl_chat_whisper,
  .chat_send                 = mw_prpl_chat_send,
  .keepalive                 = mw_prpl_keepalive,
  .register_user             = NULL,
  .get_cb_info               = NULL,
  .get_cb_away               = NULL,
  .alias_buddy               = mw_prpl_alias_buddy,
  .group_buddy               = mw_prpl_group_buddy,
  .rename_group              = mw_prpl_rename_group,
  .buddy_free                = mw_prpl_buddy_free,
  .convo_closed              = mw_prpl_convo_closed,
  .normalize                 = mw_prpl_normalize,
  .set_buddy_icon            = NULL,
  .remove_group              = mw_prpl_remove_group,
  .get_cb_real_name          = NULL,
  .set_chat_topic            = NULL,
  .find_blist_chat           = NULL,
  .roomlist_get_list         = NULL,
  .roomlist_expand_category  = NULL,
  .can_receive_file          = mw_prpl_can_receive_file,
  .send_file                 = mw_prpl_send_file,
};


static GaimPluginPrefFrame *
mw_plugin_get_plugin_pref_frame(GaimPlugin *plugin) {
  GaimPluginPrefFrame *frame;
  GaimPluginPref *pref;

  frame = gaim_plugin_pref_frame_new();
  
  pref = gaim_plugin_pref_new_with_label(_("Remotely Stored Buddy List"));
  gaim_plugin_pref_frame_add(frame, pref);
  

  pref = gaim_plugin_pref_new_with_name(MW_PRPL_OPT_BLIST_ACTION);
  gaim_plugin_pref_set_label(pref, _("Buddy List Storage Mode"));

  gaim_plugin_pref_set_type(pref, GAIM_PLUGIN_PREF_CHOICE);
  gaim_plugin_pref_add_choice(pref, _("Local Buddy List Only"),
			      GINT_TO_POINTER(blist_choice_LOCAL));
  gaim_plugin_pref_add_choice(pref, _("Merge List from Server"),
			      GINT_TO_POINTER(blist_choice_MERGE));
  gaim_plugin_pref_add_choice(pref, _("Merge and Save List to Server"),
			      GINT_TO_POINTER(blist_choice_STORE));
  gaim_plugin_pref_add_choice(pref, _("Synchronize List with Server"),
			      GINT_TO_POINTER(blist_choice_SYNCH));

  gaim_plugin_pref_frame_add(frame, pref);

  return frame;
}


static GaimPluginUiInfo mw_plugin_ui_info = {
  .get_plugin_pref_frame = mw_plugin_get_plugin_pref_frame,
};


static void status_msg_action_cb(GaimConnection *gc,
				 GaimRequestFields *fields) {
  GaimAccount *acct;
  GaimRequestField *f;
  const char *msg;

  struct mwGaimPluginData *pd;
  struct mwServiceStorage *srvc;
  struct mwStorageUnit *unit;
  
  pd = gc->proto_data;
  srvc = pd->srvc_store;
  
  acct = gaim_connection_get_account(gc);

  f = gaim_request_fields_get_field(fields, "active");
  msg = gaim_request_field_string_get_value(f);
  gaim_account_set_string(acct, MW_KEY_ACTIVE_MSG, msg);
  unit = mwStorageUnit_newString(mwStore_ACTIVE_MESSAGES, msg);
  mwServiceStorage_save(srvc, unit, NULL, NULL, NULL);

  f = gaim_request_fields_get_field(fields, "away");
  msg = gaim_request_field_string_get_value(f);
  gaim_account_set_string(acct, MW_KEY_AWAY_MSG, msg);
  unit = mwStorageUnit_newString(mwStore_AWAY_MESSAGES, msg);
  mwServiceStorage_save(srvc, unit, NULL, NULL, NULL);

  f = gaim_request_fields_get_field(fields, "busy");
  msg = gaim_request_field_string_get_value(f);
  gaim_account_set_string(acct, MW_KEY_BUSY_MSG, msg);  
  unit = mwStorageUnit_newString(mwStore_BUSY_MESSAGES, msg);
  mwServiceStorage_save(srvc, unit, NULL, NULL, NULL);

#if 0
  /* XXX */
  /* need to propagate the message change if we're in any of those
     default states */
  msg = NULL;
  if(!gc->away_state || !strcmp(gc->away_state, MW_STATE_ACTIVE)) {
    msg = MW_STATE_ACTIVE;
  } else if(gc->away_state && !strcmp(gc->away_state, MW_STATE_AWAY)) {
    msg = MW_STATE_AWAY;
  } else if(gc->away_state && !strcmp(gc->away_state, MW_STATE_BUSY)) {
    msg = MW_STATE_BUSY;
  }
  if(msg)
    serv_set_away(gc, msg, NULL);
#endif
}


/** Prompt for messages for the three default status types. These
    values should be mirrored as strings in the storage service */
static void status_msg_action(GaimPluginAction *act) {
  GaimConnection *gc;
  GaimAccount *acct;

  GaimRequestFields *fields;
  GaimRequestFieldGroup *g;
  GaimRequestField *f;
  
  char *msgA, *msgB;
  const char *val;

  gc = act->context;
  acct = gaim_connection_get_account(gc);

  fields = gaim_request_fields_new();

  g = gaim_request_field_group_new(NULL);
  gaim_request_fields_add_group(fields, g);

  val = gaim_account_get_string(acct, MW_KEY_ACTIVE_MSG,
				MW_PLUGIN_DEFAULT_ACTIVE_MSG);
  f = gaim_request_field_string_new("active", _("Active Message"), val, FALSE);
  gaim_request_field_set_required(f, FALSE);
  gaim_request_field_group_add_field(g, f);
  
  val = gaim_account_get_string(acct, MW_KEY_AWAY_MSG,
				MW_PLUGIN_DEFAULT_AWAY_MSG);
  f = gaim_request_field_string_new("away", _("Away Message"), val, FALSE);
  gaim_request_field_set_required(f, FALSE);
  gaim_request_field_group_add_field(g, f);

  val = gaim_account_get_string(acct, MW_KEY_BUSY_MSG,
				MW_PLUGIN_DEFAULT_BUSY_MSG);
  f = gaim_request_field_string_new("busy", _("Busy Message"), val, FALSE);
  gaim_request_field_set_required(f, FALSE);
  gaim_request_field_group_add_field(g, f);

  msgA = _("Default status messages");
  msgB = ("");

  gaim_request_fields(gc, _("Edit Status Messages"),
		      msgA, msgB, fields,
		      _("OK"), G_CALLBACK(status_msg_action_cb),
		      _("Cancel"), NULL,
		      gc);
}


static void st_import_action_cb(GaimConnection *gc, char *filename) {
  struct mwSametimeList *l;

  FILE *file;
  char buf[BUF_LEN];
  size_t len;

  GString *str;

  file = fopen(filename, "r");
  g_return_if_fail(file != NULL);

  str = g_string_new(NULL);
  while( (len = fread(buf, 1, BUF_LEN, file)) ) {
    g_string_append_len(str, buf, len);
  }

  fclose(file);

  l = mwSametimeList_load(str->str);
  g_string_free(str, TRUE);

  blist_merge(gc, l);
  mwSametimeList_free(l);
}


/** prompts for a file to import blist from */
static void st_import_action(GaimPluginAction *act) {
  GaimConnection *gc;
  GaimAccount *account;
  char *title;

  gc = act->context;
  account = gaim_connection_get_account(gc);
  title = g_strdup_printf(_("Import Sametime List for Account %s"),
			  gaim_account_get_username(account));

  gaim_request_file(gc, title, NULL, FALSE,
		    G_CALLBACK(st_import_action_cb), NULL,
		    gc);

  g_free(title);
}


static void st_export_action_cb(GaimConnection *gc, char *filename) {
  struct mwSametimeList *l;
  char *str;
  FILE *file;

  file = fopen(filename, "w");
  g_return_if_fail(file != NULL);

  l = mwSametimeList_new();
  blist_export(gc, l);
  str = mwSametimeList_store(l);
  mwSametimeList_free(l);

  fprintf(file, "%s", str);
  fclose(file);

  g_free(str);
}


/** prompts for a file to export blist to */
static void st_export_action(GaimPluginAction *act) {
  GaimConnection *gc;
  GaimAccount *account;
  char *title;

  gc = act->context;
  account = gaim_connection_get_account(gc);
  title = g_strdup_printf(_("Export Sametime List for Account %s"),
			  gaim_account_get_username(account));

  gaim_request_file(gc, title, NULL, TRUE,
		    G_CALLBACK(st_export_action_cb), NULL,
		    gc);

  g_free(title);
}


static void remote_group_multi_cleanup(gpointer ignore,
				       GaimRequestFields *fields) {
  
  GaimRequestField *f;
  const GList *l;

  f = gaim_request_fields_get_field(fields, "group");
  l = gaim_request_field_list_get_items(f);

  for(; l; l = l->next) {
    const char *i = l->data;
    struct resolved_id *res;

    res = gaim_request_field_list_get_data(f, i);

    g_free(res->id);
    g_free(res->name);
    g_free(res);
  }
}


static void remote_group_done(struct mwGaimPluginData *pd,
			      const char *id, const char *name) {
  GaimConnection *gc;
  GaimAccount *acct;
  GaimGroup *group;
  GaimBlistNode *gn;
  const char *owner;

  g_return_if_fail(pd != NULL);

  gc = pd->gc;
  acct = gaim_connection_get_account(gc);
  
  /* collision checking */
  group = gaim_find_group(name);
  if(group) {
    char *msgA, *msgB;

    msgA = _("Unable to add group: group exists");
    msgB = _("A group named '%s' already exists in your buddy list.");
    msgB = g_strdup_printf(msgB, name);

    gaim_notify_error(gc, _("Unable to add group"), msgA, msgB);

    g_free(msgB);
    return;
  }

  group = gaim_group_new(name);
  gn = (GaimBlistNode *) group;

  owner = gaim_account_get_username(acct);

  gaim_blist_node_set_string(gn, GROUP_KEY_NAME, id);
  gaim_blist_node_set_int(gn, GROUP_KEY_TYPE, mwSametimeGroup_DYNAMIC);
  gaim_blist_node_set_string(gn, GROUP_KEY_OWNER, owner);
  gaim_blist_add_group(group, NULL);

  group_add(pd, group);
  blist_schedule(pd);
}


static void remote_group_multi_cb(struct mwGaimPluginData *pd,
				  GaimRequestFields *fields) {
  GaimRequestField *f;
  const GList *l;

  f = gaim_request_fields_get_field(fields, "group");
  l = gaim_request_field_list_get_selected(f);

  if(l) {
    const char *i = l->data;
    struct resolved_id *res;

    res = gaim_request_field_list_get_data(f, i);
    remote_group_done(pd, res->id, res->name);
  }

  remote_group_multi_cleanup(NULL, fields);
}


static void remote_group_multi(struct mwResolveResult *result,
			       struct mwGaimPluginData *pd) {

  GaimRequestFields *fields;
  GaimRequestFieldGroup *g;
  GaimRequestField *f;
  GList *l;
  char *msgA, *msgB;

  GaimConnection *gc = pd->gc;

  fields = gaim_request_fields_new();

  g = gaim_request_field_group_new(NULL);
  gaim_request_fields_add_group(fields, g);

  f = gaim_request_field_list_new("group", _("Possible Matches"));
  gaim_request_field_list_set_multi_select(f, FALSE);
  gaim_request_field_set_required(f, TRUE);

  for(l = result->matches; l; l = l->next) {
    struct mwResolveMatch *match = l->data;
    struct resolved_id *res = g_new0(struct resolved_id, 1);

    res->id = g_strdup(match->id);
    res->name = g_strdup(match->name);

    gaim_request_field_list_add(f, res->name, res);
  }

  gaim_request_field_group_add_field(g, f);

  msgA = _("Notes Address Book group results");
  msgB = _("The identifier '%s' may possibly refer to any of the following"
	  " Notes Address Book groups. Please select the correct group from"
	  " the list below to add it to your buddy list.");
  msgB = g_strdup_printf(msgB, result->name);

  gaim_request_fields(gc, _("Select Notes Address Book"),
		      msgA, msgB, fields,
		      _("Add Group"), G_CALLBACK(remote_group_multi_cb),
		      _("Cancel"), G_CALLBACK(remote_group_multi_cleanup),
		      pd);

  g_free(msgB);
}


static void remote_group_resolved(struct mwServiceResolve *srvc,
				  guint32 id, guint32 code, GList *results,
				  gpointer b) {

  struct mwResolveResult *res = NULL;
  struct mwSession *session;
  struct mwGaimPluginData *pd;
  GaimConnection *gc;

  session = mwService_getSession(MW_SERVICE(srvc));
  g_return_if_fail(session != NULL);

  pd = mwSession_getClientData(session);
  g_return_if_fail(pd != NULL);

  gc = pd->gc;
  g_return_if_fail(gc != NULL);
  
  if(!code && results) {
    res = results->data;

    if(res->matches) {
      remote_group_multi(res, pd);
      return;
    }
  }

  if(res && res->name) {
    char *msgA, *msgB;

    msgA = _("Unable to add group: group not found");

    msgB = _("The identifier '%s' did not match any Notes Address Book"
	    " groups in your Sametime community.");
    msgB = g_strdup_printf(msgB, res->name);

    gaim_notify_error(gc, _("Unable to add group"), msgA, msgB);

    g_free(msgB);
  }
}


static void remote_group_action_cb(GaimConnection *gc, const char *name) {
  struct mwGaimPluginData *pd;
  struct mwServiceResolve *srvc;
  GList *query;
  enum mwResolveFlag flags;
  guint32 req;

  pd = gc->proto_data;
  srvc = pd->srvc_resolve;

  query = g_list_prepend(NULL, (char *) name);
  flags = mwResolveFlag_FIRST | mwResolveFlag_GROUPS;
  
  req = mwServiceResolve_resolve(srvc, query, flags, remote_group_resolved,
				 NULL, NULL);
  g_list_free(query);

  if(req == SEARCH_ERROR) {
    /** @todo display error */
  }
}


static void remote_group_action(GaimPluginAction *act) {
  GaimConnection *gc;
  const char *msgA, *msgB;

  gc = act->context;

  msgA = _("Notes Address Book Group");
  msgB = _("Enter the name of a Notes Address Book group in the field below"
	  " to add the group and its members to your buddy list.");

  gaim_request_input(gc, _("Add Group"), msgA, msgB, NULL,
		     FALSE, FALSE, NULL,
		     _("Add"), G_CALLBACK(remote_group_action_cb),
		     _("Cancel"), NULL,
		     gc);
}


static GList *mw_plugin_actions(GaimPlugin *plugin, gpointer context) {
  GaimPluginAction *act;
  GList *l = NULL;

  act = gaim_plugin_action_new(_("Set Status Messages..."),
			       status_msg_action);
  l = g_list_append(l, act);

  act = gaim_plugin_action_new(_("Import Sametime List..."),
			       st_import_action);
  l = g_list_append(l, act);

  act = gaim_plugin_action_new(_("Export Sametime List..."),
			       st_export_action);
  l = g_list_append(l, act);

  act = gaim_plugin_action_new(_("Add Notes Address Book Group..."),
			       remote_group_action);
  l = g_list_append(l, act);

  return l;
}


static gboolean mw_plugin_load(GaimPlugin *plugin) {
  return TRUE;
}


static gboolean mw_plugin_unload(GaimPlugin *plugin) {
  return TRUE;
}


static void mw_plugin_destroy(GaimPlugin *plugin) {
  g_log_remove_handler(G_LOG_DOMAIN, log_handler[0]);
  g_log_remove_handler("meanwhile", log_handler[1]);
}


static GaimPluginInfo mw_plugin_info = {
  .magic           = GAIM_PLUGIN_MAGIC,
  .major_version   = GAIM_MAJOR_VERSION,
  .minor_version   = GAIM_MINOR_VERSION,
  .type            = GAIM_PLUGIN_PROTOCOL,
  .ui_requirement  = NULL,
  .flags           = 0,
  .dependencies    = NULL,
  .priority        = GAIM_PRIORITY_DEFAULT,
  .id              = PLUGIN_ID,
  .name            = PLUGIN_NAME,
  .version         = VERSION,
  .summary         = PLUGIN_SUMMARY,
  .description     = PLUGIN_DESC,
  .author          = PLUGIN_AUTHOR,
  .homepage        = PLUGIN_HOMEPAGE,
  .load            = mw_plugin_load,
  .unload          = mw_plugin_unload,
  .destroy         = mw_plugin_destroy,
  .ui_info         = NULL,
  .extra_info      = &mw_prpl_info,
  .prefs_info      = &mw_plugin_ui_info,
  .actions         = mw_plugin_actions,
};


static void mw_log_handler(const gchar *domain, GLogLevelFlags flags,
			   const gchar *msg, gpointer data) {

  if(! (msg && *msg)) return;

  /* handle g_log requests via gaim's built-in debug logging */
  if(flags & G_LOG_LEVEL_ERROR) {
    gaim_debug_error(domain, "%s\n", msg);

  } else if(flags & G_LOG_LEVEL_WARNING) {
    gaim_debug_warning(domain, "%s\n", msg);

  } else {
    gaim_debug_info(domain, "%s\n", msg);
  }
}


static void mw_plugin_init(GaimPlugin *plugin) {
  GaimAccountOption *opt;
  GList *l = NULL;

  GLogLevelFlags logflags =
    G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION;

  /* set up the preferences */
  gaim_prefs_add_none(MW_PRPL_OPT_BASE);
  gaim_prefs_add_int(MW_PRPL_OPT_BLIST_ACTION, BLIST_CHOICE_DEFAULT);

  /* remove dead preferences */
  gaim_prefs_remove(MW_PRPL_OPT_PSYCHIC);
  gaim_prefs_remove(MW_PRPL_OPT_SAVE_DYNAMIC);

  /* host to connect to */
  opt = gaim_account_option_string_new(_("Server"), MW_KEY_HOST,
				       MW_PLUGIN_DEFAULT_HOST);
  l = g_list_append(l, opt);

  /* port to connect to */
  opt = gaim_account_option_int_new(_("Port"), MW_KEY_PORT,
				    MW_PLUGIN_DEFAULT_PORT);
  l = g_list_append(l, opt);

  /* notesbuddy hack encoding */
  opt = gaim_account_option_string_new(_("NotesBuddy Encoding"),
				       MW_KEY_ENCODING,
				       MW_PLUGIN_DEFAULT_ENCODING);
  l = g_list_append(l, opt);

  { /* copy the old force login setting from prefs if it's
       there. Don't delete the preference, since there may be more
       than one account that wants to check for it. */
    gboolean b = FALSE;
    const char *label = _("Force Login (Ignore Server Redirects)");

    if(gaim_prefs_exists(MW_PRPL_OPT_FORCE_LOGIN))
      b = gaim_prefs_get_bool(MW_PRPL_OPT_FORCE_LOGIN);

    opt = gaim_account_option_bool_new(label, MW_KEY_FORCE, b);
    l = g_list_append(l, opt);
  }

  mw_prpl_info.protocol_options = l;
  l = NULL;

  /* forward all our g_log messages to gaim. Generally all the logging
     calls are using gaim_log directly, but the g_return macros will
     get caught here */
  log_handler[0] = g_log_set_handler(G_LOG_DOMAIN, logflags,
				     mw_log_handler, NULL);
  
  /* redirect meanwhile's logging to gaim's */
  log_handler[1] = g_log_set_handler("meanwhile", logflags,
				     mw_log_handler, NULL);
}


GAIM_INIT_PLUGIN(sametime, mw_plugin_init, mw_plugin_info);
/* The End. */

