/* SPDX-License-Identifier: Zlib */

#include <girara/settings.h>
#include <girara/log.h>

#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "document.h"
#include "zathura.h"
#include "utils.h"
#include "dbus-interface.h"
#include "discord_game_sdk.h"
#ifdef WITH_SYNCTEX
#include "synctex.h"
#endif

/* Init locale */
static void
init_locale(void)
{
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
}

/* Set log level */
static void
set_log_level(const char* loglevel)
{
  if (loglevel == NULL || g_strcmp0(loglevel, "info") == 0) {
    girara_set_log_level(GIRARA_INFO);
  } else if (g_strcmp0(loglevel, "warning") == 0) {
    girara_set_log_level(GIRARA_WARNING);
  } else if (g_strcmp0(loglevel, "error") == 0) {
    girara_set_log_level(GIRARA_ERROR);
  }
}

/* Handle synctex forward synchronization */
#ifdef WITH_SYNCTEX
static int
run_synctex_forward(const char* synctex_fwd, const char* filename,
                    int synctex_pid)
{
  GFile* file = g_file_new_for_commandline_arg(filename);
  if (file == NULL) {
    girara_error("Unable to handle argument '%s'.", filename);
    return -1;
  }

  char* real_path = g_file_get_path(file);
  g_object_unref(file);
  if (real_path == NULL) {
    girara_error("Failed to determine path for '%s'", filename);
    return -1;
  }

  int   line       = 0;
  int   column     = 0;
  char* input_file = NULL;
  if (synctex_parse_input(synctex_fwd, &input_file, &line, &column) == false) {
    girara_error("Failed to parse argument to --synctex-forward.");
    g_free(real_path);
    return -1;
  }

  const int ret = zathura_dbus_synctex_position(real_path, input_file, line,
                                                column, synctex_pid);
  g_free(input_file);
  g_free(real_path);

  if (ret == -1) {
    /* D-Bus or SyncTeX failed */
    girara_error(
      "Got no usable data from SyncTeX or D-Bus failed in some way.");
  }

  return ret;
}
#endif

static zathura_t*
init_zathura(const char* config_dir, const char* data_dir,
             const char* cache_dir, const char* plugin_path, char** argv,
             const char* synctex_editor, Window embed)
{
  /* create zathura session */
  zathura_t* zathura = zathura_create();
  if (zathura == NULL) {
    return NULL;
  }

  zathura_set_xid(zathura, embed);
  zathura_set_config_dir(zathura, config_dir);
  zathura_set_data_dir(zathura, data_dir);
  zathura_set_cache_dir(zathura, cache_dir);
  zathura_set_plugin_dir(zathura, plugin_path);
  zathura_set_argv(zathura, argv);

  /* Init zathura */
  if (zathura_init(zathura) == false) {
    zathura_free(zathura);
    return NULL;
  }

  if (synctex_editor != NULL) {
    girara_setting_set(zathura->ui.session, "synctex-editor-command",
                       synctex_editor);
  }

  return zathura;
}


void UpdateActivityCallback(void* data, enum EDiscordResult result)
{
    DISCORD_REQUIRE(result);
}

int RelationshipPassFilter(void* data, struct DiscordRelationship* relationship)
{
    return (relationship->type == DiscordRelationshipType_Friend);
}

int RelationshipSnowflakeFilter(void* data, struct DiscordRelationship* relationship)
{
    struct Application* app = (struct Application*)data;

    return (relationship->type == DiscordRelationshipType_Friend &&
            relationship->user.id < app->user_id);
}
void OnRelationshipsRefresh(void* data)
{}

void OnUserUpdated(void* data)
{
    struct Application* app = (struct Application*)data;
    struct DiscordUser user;
    app->users->get_current_user(app->users, &user);
    app->user_id = user.id;
}

void OnOAuth2Token(void* data, enum EDiscordResult result, struct DiscordOAuth2Token* token)
{
    if (result == DiscordResult_Ok) {
        printf("OAuth2 token: %s\n", token->access_token);
    }
    else {
        printf("GetOAuth2Token failed with %d\n", (int)result);
    }
}

void OnLobbyConnect(void* data, enum EDiscordResult result, struct DiscordLobby* lobby)
{
    printf("LobbyConnect returned %d\n", (int)result);
}
gboolean refresh(gpointer data)
{
    struct DiscordActivity activity;
    memset(&activity, 0, sizeof(activity));
    sprintf(activity.state, "Reading : %s (page %d/%d)", bname, cpage, pages);

    app.activities->update_activity(app.activities, &activity, &app, UpdateActivityCallback);
    DISCORD_REQUIRE(app.core->run_callbacks(app.core));
    return 1;
}
/* main function */
GIRARA_VISIBLE int
main(int argc, char* argv[])
{
    zathura_document_t* doc;
    gpointer data;
    memset(&app, 0, sizeof(app));

    struct IDiscordUserEvents users_events;
    memset(&users_events, 0, sizeof(users_events));
    users_events.on_current_user_update = OnUserUpdated;

    struct IDiscordActivityEvents activities_events;
    memset(&activities_events, 0, sizeof(activities_events));
    struct IDiscordRelationshipEvents relationships_events;
    struct DiscordCreateParams params;
    DiscordCreateParamsSetDefault(&params);
    params.client_id = 733236293969903676;
    params.flags = DiscordCreateFlags_Default;
    params.event_data = &app;
    params.activity_events = &activities_events;
    params.relationship_events = &relationships_events;
    params.user_events = &users_events;
    DISCORD_REQUIRE(DiscordCreate(DISCORD_VERSION, &params, &app.core));

    app.users = app.core->get_user_manager(app.core);
    app.achievements = app.core->get_achievement_manager(app.core);
    app.activities = app.core->get_activity_manager(app.core);
    app.application = app.core->get_application_manager(app.core);
    app.lobbies = app.core->get_lobby_manager(app.core);

    app.lobbies->connect_lobby_with_activity_secret(
      app.lobbies, "invalid_secret", &app, OnLobbyConnect);

    app.application->get_oauth2_token(app.application, &app, OnOAuth2Token);

    DiscordBranch branch;
    app.application->get_current_branch(app.application, &branch);
    printf("Current branch %s\n", branch);
    g_thread_init(NULL);
    gdk_threads_init();
    gdk_threads_enter ();
    gdk_threads_add_idle(refresh, data);
    app.relationships = app.core->get_relationship_manager(app.core);
  init_locale();

  /* parse command line arguments */
  gchar* config_dir     = NULL;
  gchar* data_dir       = NULL;
  gchar* cache_dir      = NULL;
  gchar* plugin_path    = NULL;
  gchar* loglevel       = NULL;
  gchar* password       = NULL;
  gchar* synctex_editor = NULL;
  gchar* synctex_fwd    = NULL;
  gchar* mode           = NULL;
  bool   forkback       = false;
  bool   print_version  = false;
  int    page_number    = ZATHURA_PAGE_NUMBER_UNSPECIFIED;
  int    synctex_pid    = -1;
  Window embed          = 0;

  GOptionEntry entries[] = {
    { "reparent",               'e',  0, G_OPTION_ARG_INT,      &embed,          _("Reparents to window specified by xid (X11)"),        "xid"  },
    { "config-dir",             'c',  0, G_OPTION_ARG_FILENAME, &config_dir,     _("Path to the config directory"),                      "path" },
    { "data-dir",               'd',  0, G_OPTION_ARG_FILENAME, &data_dir,       _("Path to the data directory"),                        "path" },
    { "cache-dir",              '\0', 0, G_OPTION_ARG_FILENAME, &cache_dir,      _("Path to the cache directory"),                       "path"},
    { "plugins-dir",            'p',  0, G_OPTION_ARG_STRING,   &plugin_path,    _("Path to the directories containing plugins"),        "path" },
    { "fork",                   '\0', 0, G_OPTION_ARG_NONE,     &forkback,       _("Fork into the background"),                          NULL },
    { "password",               'w',  0, G_OPTION_ARG_STRING,   &password,       _("Document password"),                                 "password" },
    { "page",                   'P',  0, G_OPTION_ARG_INT,      &page_number,    _("Page number to go to"),                              "number" },
    { "log-level",              'l',  0, G_OPTION_ARG_STRING,   &loglevel,       _("Log level (debug, info, warning, error)"),           "level" },
    { "version",                'v',  0, G_OPTION_ARG_NONE,     &print_version,  _("Print version information"),                         NULL },
    { "synctex-editor-command", 'x',  0, G_OPTION_ARG_STRING,   &synctex_editor, _("Synctex editor (forwarded to the synctex command)"), "cmd" },
    { "synctex-forward",        '\0', 0, G_OPTION_ARG_STRING,   &synctex_fwd,    _("Move to given synctex position"),                    "position" },
    { "synctex-pid",            '\0', 0, G_OPTION_ARG_INT,      &synctex_pid,    _("Highlight given position in the given process"),     "pid" },
    { "mode",                   '\0', 0, G_OPTION_ARG_STRING,   &mode,           _("Start in a non-default mode"),                       "mode" },
    { NULL, '\0', 0, 0, NULL, NULL, NULL }
  };

  GOptionContext* context = g_option_context_new(" [file1] [file2] [...]");
  g_option_context_add_main_entries(context, entries, NULL);

  GError* error = NULL;
  if (g_option_context_parse(context, &argc, &argv, &error) == false) {
    girara_error("Error parsing command line arguments: %s\n", error->message);
    g_option_context_free(context);
    g_error_free(error);

    return -1;
  }
  g_option_context_free(context);

  int ret = 0;
  set_log_level(loglevel);

#ifdef WITH_SYNCTEX
  /* handle synctex forward synchronization */
  if (synctex_fwd != NULL) {
    if (argc != 2) {
      girara_error("Too many arguments or missing filename while running with "
                   "--synctex-forward");
      ret = -1;
      goto free_and_ret;
    }

    ret = run_synctex_forward(synctex_fwd, argv[1], synctex_pid);
    if (ret > 0) {
      /* Instance found. */
      ret = 0;
      goto free_and_ret;
    }
    else if (ret < 0) {
      /* Error occurred. */
      ret = -1;
      goto free_and_ret;
    }

    girara_debug("No instance found. Starting new one.");
  }
#else
  if (synctex_fwd != NULL || synctex_editor != NULL || synctex_pid != -1) {
    girara_error("Built without synctex support, but synctex specific option was specified.");
    ret = -1;
    goto free_and_ret;
  }
#endif

  /* check mode */
  if (mode != NULL && g_strcmp0(mode, "presentation") != 0 &&
      g_strcmp0(mode, "fullscreen") != 0) {
    girara_error("Invalid argument for --mode: %s", mode);
    ret = -1;
    goto free_and_ret;
  }

  /* g_option_context_parse has some funny (documented) behavior:
   * * for "-- a b c" you get no -- in argv
   * * for "-- --" you get -- in argv twice
   * * for "-- -a" you get -- in argv
   *
   * So if there is one -- in argv, we need to ignore it. */
  const bool has_double_dash = argc > 1 && g_strcmp0(argv[1], "--") == 0;
  const int  file_idx_base   = has_double_dash ? 2 : 1;

  int file_idx = argc > file_idx_base ? file_idx_base : 0;
  /* Fork instances for other files. */
  if (print_version == false && argc > file_idx_base + 1) {
    for (int idx = file_idx_base + 1; idx < argc; ++idx) {
      const pid_t pid = fork();
      if (pid == 0) { /* child */
        file_idx = idx;
        if (setsid() == -1) {
          girara_error("Could not start new process group: %s",
                       strerror(errno));
          ret = -1;
          goto free_and_ret;
        }
        break;
      }
      else if (pid < 0) { /* error */
        girara_error("Could not fork: %s", strerror(errno));
        ret = -1;
        goto free_and_ret;
      }
    }
  }

  /* Fork into the background if the user really wants to ... */
  if (print_version == false && forkback == true &&
      file_idx < file_idx_base + 1) {
    const pid_t pid = fork();
    if (pid > 0) { /* parent */
      goto free_and_ret;
    }
    else if (pid < 0) { /* error */
      girara_error("Could not fork: %s", strerror(errno));
      ret = -1;
      goto free_and_ret;
    }

    if (setsid() == -1) {
      girara_error("Could not start new process group: %s", strerror(errno));
      ret = -1;
      goto free_and_ret;
    }
  }

  /* Initialize GTK+ */
  gtk_init(&argc, &argv);

  /* Create zathura session */
  zathura_t* zathura = init_zathura(config_dir, data_dir, cache_dir,
                                    plugin_path, argv, synctex_editor, embed);
  if (zathura == NULL) {
    girara_error("Could not initialize zathura.");
    ret = -1;
    goto free_and_ret;
  }

  /* Print version */
  if (print_version == true) {
    char* string = zathura_get_version_string(zathura, false);
    if (string != NULL) {
      fprintf(stdout, "%s\n", string);
      g_free(string);
    }
    zathura_free(zathura);

    goto free_and_ret;
  }

  /* open document if passed */
  if (file_idx != 0) {
    if (page_number > 0) {
      --page_number;
    }
    document_open_idle(zathura, argv[file_idx], password, page_number, mode,
                       synctex_fwd);
  }
  doc=zathura->document;
  memset(&relationships_events, 0, sizeof(relationships_events));
  relationships_events.on_refresh = OnRelationshipsRefresh;

  /* run zathura */
  
  gtk_main();
  gdk_threads_leave();
  /* free zathura */
  zathura_free(zathura);

free_and_ret:
  g_free(config_dir);
  g_free(data_dir);
  g_free(cache_dir);
  g_free(plugin_path);
  g_free(loglevel);
  g_free(password);
  g_free(synctex_editor);
  g_free(synctex_fwd);
  g_free(mode);

  return ret;
}
