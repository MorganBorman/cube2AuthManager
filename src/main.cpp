#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "filebuffer.h"
#include "cube2crypto.h"

#if defined _WIN32 || defined _WIN64
    #define FS_DELIM "\\"
    #include <Windows.h>
    #include <shlobj.h>
#else
    #define FS_DELIM "/"
#endif

enum{PROGDIR, HOMEDIR};

typedef struct {
    int    relative_to;
    char   *path;
} HomeLocation;

#if defined _WIN32 || defined _WIN64

/* Array of sauerbraten home directories and what they're relative to. */
static HomeLocation home_locations[] =  {
        {HOMEDIR, "My Games\Sauerbraten"},
        {PROGDIR, "Sauerbraten\\"}
};

#elif __APPLE__

static HomeLocation home_locations[] =  {
        {HOMEDIR, "Library/Application Support/sauerbraten"}
};

#else

static HomeLocation home_locations[] =  {
        {HOMEDIR, ".sauerbraten"}
};

#endif

#define NUMLOCATIONS sizeof(home_locations)/sizeof(HomeLocation)

bool directory_exists(const char *path)
{
    struct stat stat_data;
    if(stat(path, &stat_data)) return false;

    if (stat_data.st_mode & S_IFDIR)
    {
        return true;
    }
    else
    {
        return false;
    }
}

enum
{
  COL_ENABLED = 0,      /*  gboolean    */
  COL_AUTHNAME,         /*  gchararray  */
  COL_PRIVKEY,          /*  gchararray  */
  COL_DOMAIN,           /*  gchararray  */
  COL_ACTION_INFO,      /*  gint        */
  COL_ACTION_EDIT,      /*  gint        */
  COL_ACTION_DELETE,    /*  gint        */
  COL_CONFIG_BUFFER,    /*  gpointer    */
  COL_CONFIG_LINE,      /*  gint        */
  NUM_COLS
};

typedef struct {
    GtkWidget		*window;
    
    GtkEntry        *entry_authname;
    GtkEntry        *entry_privkey;
    GtkEntry        *entry_domain;
} NewDialog;

typedef struct {
    GtkWidget		*window;
    
    GtkEntry        *entry_authname;
    GtkEntry        *entry_privkey;
    GtkEntry        *entry_domain;
    
    char            *current_row_path;
} EditDialog;

typedef struct {
    GtkWidget		*window;
    
    GtkLabel        *label_authname;
    GtkLabel        *label_privkey;
    GtkLabel        *label_pubkey;
    GtkLabel        *label_domain;
    GtkLabel        *label_location;
} InfoDialog;

typedef struct
{
        GtkWidget		*window;
        GtkWidget       *errordialog_nodir;

        GtkListStore    *key_store;
        GtkTreeView     *key_view;
        
        GdkPixbuf *info_icon_pixbuf;
        GdkPixbuf *edit_icon_pixbuf;
        GdkPixbuf *delete_icon_pixbuf;
        
        GtkCellRenderer *enabled_renderer;
        GtkCellRenderer *info_renderer;
        GtkCellRenderer *edit_renderer;
        GtkCellRenderer *delete_renderer;
        
        NewDialog       newdialog;
        EditDialog      editdialog;
        InfoDialog      infodialog;
        
        FileBuffer auth_cfg_buffer;
        FileBuffer autoexec_cfg_buffer;
        
} AuthManager;

GdkPixbuf *get_stock_icon(GtkIconTheme *icon_theme, const char *icon_name, int size=24)
{
    GError		*error			= NULL;
    GdkPixbuf   *return_pixbuf  = NULL;
    
    return_pixbuf = gdk_pixbuf_new_from_file_at_size(icon_name, size /*px*/, size /*px*/, &error);
    if(!return_pixbuf) {
        g_print("Couldn't load icon: %s\n", error->message);
        g_free(error);
        return NULL;
    }
    return return_pixbuf;
}

void add_row(AuthManager *authManager, bool enabled, const char *authname, const char *privkey, const char *domain, FileBuffer *config_buffer, int config_line)
{
	GtkTreeIter current_entry;
	gtk_list_store_append(authManager->key_store, &current_entry);
	gtk_list_store_set(authManager->key_store, &current_entry, 
	                                                    COL_ENABLED, enabled, 
	                                                    COL_AUTHNAME, authname, 
	                                                    COL_PRIVKEY, privkey, 
	                                                    COL_DOMAIN, domain, 
	                                                    COL_ACTION_INFO, authManager->info_icon_pixbuf, 
	                                                    COL_ACTION_EDIT, authManager->edit_icon_pixbuf, 
	                                                    COL_ACTION_DELETE, authManager->delete_icon_pixbuf, 
	                                                    COL_CONFIG_BUFFER, (gpointer)config_buffer, 
	                                                    COL_CONFIG_LINE, config_line, -1);
}

#include "tree_view_get_cell_from_pos.cpp"
#include "auth_model.cpp"
#include "event_handlers.cpp"

int main (int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	GtkBuilder	*builder 		= gtk_builder_new();
	AuthManager	*authManager 	= g_slice_new(AuthManager);
	GError		*error			= NULL;

    if(!gtk_builder_add_from_file(builder, "authmanager.glade", &error))
    {
        g_print("Error occurred while loading UI description from file (authmanager.glade)!\n");
        g_print("Message: %s\n", error->message);
        g_free(error);
        g_slice_free(AuthManager, authManager);
        return(1);
    }

	authManager->window      	            = GTK_WIDGET(gtk_builder_get_object(builder, "mainwindow"));
	authManager->key_store     	            = GTK_LIST_STORE(gtk_builder_get_object(builder, "KeyStore"));
	authManager->key_view     	            = GTK_TREE_VIEW(gtk_builder_get_object(builder, "KeyView"));
	
	authManager->errordialog_nodir          = GTK_WIDGET(gtk_builder_get_object(builder, "errordialog_nodir"));

	authManager->enabled_renderer     	    = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cellrenderertoggle_enabled"));
	authManager->info_renderer     	        = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cellrendererpixbuf_info"));
	authManager->edit_renderer     	        = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cellrendererpixbuf_edit"));
	authManager->delete_renderer   	        = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cellrendererpixbuf_delete"));
	
	authManager->newdialog.window           = GTK_WIDGET(gtk_builder_get_object(builder, "newdialog_window"));
    authManager->newdialog.entry_authname   = GTK_ENTRY(gtk_builder_get_object (builder, "newdialog_entry_authname"));
    authManager->newdialog.entry_privkey    = GTK_ENTRY(gtk_builder_get_object (builder, "newdialog_entry_privkey"));
    authManager->newdialog.entry_domain     = GTK_ENTRY(gtk_builder_get_object (builder, "newdialog_entry_domain"));
	
    authManager->editdialog.window         	= GTK_WIDGET(gtk_builder_get_object(builder, "editdialog_window"));
    authManager->editdialog.entry_authname  = GTK_ENTRY(gtk_builder_get_object (builder, "editdialog_entry_authname"));
    authManager->editdialog.entry_privkey   = GTK_ENTRY(gtk_builder_get_object (builder, "editdialog_entry_privkey"));
    authManager->editdialog.entry_domain    = GTK_ENTRY(gtk_builder_get_object (builder, "editdialog_entry_domain"));
    
    authManager->infodialog.window         	= GTK_WIDGET(gtk_builder_get_object(builder, "infodialog_window"));
    
    authManager->infodialog.label_authname  = GTK_LABEL(gtk_builder_get_object (builder, "infodialog_info_authname"));
    authManager->infodialog.label_privkey   = GTK_LABEL(gtk_builder_get_object (builder, "infodialog_info_privkey"));
    authManager->infodialog.label_pubkey    = GTK_LABEL(gtk_builder_get_object (builder, "infodialog_info_pubkey"));
    authManager->infodialog.label_domain    = GTK_LABEL(gtk_builder_get_object (builder, "infodialog_info_domain"));
    authManager->infodialog.label_location  = GTK_LABEL(gtk_builder_get_object (builder, "infodialog_info_location"));

	gtk_builder_connect_signals (builder, authManager);

	g_object_unref(G_OBJECT(builder));

	gtk_widget_show(authManager->window);
	
	GTK_CELL_RENDERER_TOGGLE(authManager->enabled_renderer)->activatable = true;
	
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    
    authManager->info_icon_pixbuf = get_stock_icon(icon_theme, "Icons"FS_DELIM"200px-Gnome-dialog-information.png");
    authManager->edit_icon_pixbuf = get_stock_icon(icon_theme, "Icons"FS_DELIM"200px-Gnome-accessories-text-editor.png");
    authManager->delete_icon_pixbuf = get_stock_icon(icon_theme, "Icons"FS_DELIM"200px-Gnome-edit-delete.png");
    
    if(!authManager->info_icon_pixbuf || !authManager->edit_icon_pixbuf || !authManager->delete_icon_pixbuf)
    {
        if(authManager->info_icon_pixbuf) g_object_unref(authManager->info_icon_pixbuf);
        if(authManager->edit_icon_pixbuf) g_object_unref(authManager->edit_icon_pixbuf);
        if(authManager->delete_icon_pixbuf) g_object_unref(authManager->delete_icon_pixbuf);
        g_slice_free(AuthManager, authManager);
        return(1);
    }

    if(!load_auth_information(authManager))
    {
        gtk_dialog_run(GTK_DIALOG(authManager->errordialog_nodir));
    }
    else
    {
        gtk_main();
    }

    freefb(&authManager->auth_cfg_buffer);
    freefb(&authManager->autoexec_cfg_buffer);
    
	if(authManager->info_icon_pixbuf) g_object_unref(authManager->info_icon_pixbuf);
	if(authManager->edit_icon_pixbuf) g_object_unref(authManager->edit_icon_pixbuf);
	if(authManager->delete_icon_pixbuf) g_object_unref(authManager->delete_icon_pixbuf);
	
	g_slice_free(AuthManager, authManager);
	return 0;
}

