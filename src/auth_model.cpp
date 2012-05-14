typedef struct {
    bool    enabled;
    char    authname[512];
    char    privkey[51];
    char    domain[512];
} AuthData;

bool read_auth_line(AuthData *auth, const char *line)
{
    if(!strncmp(line, "//authkey", 9) || !strncmp(line, "authkey", 7))
    {
        auth->enabled = (line[0] != '/');
        auth->domain[0] = '\0';
        int values_read = sscanf(line, "%*12s %511s %51s %511s", auth->authname, auth->privkey, auth->domain);
        return (values_read >= 2);
    }
    else return false;
}

void read_config_buffer(AuthManager *authManager, FileBuffer *fb)
{
    AuthData auth;
    int lx;
    for(lx = 0; lx < fb->line_count; lx++)
    {
        if(read_auth_line(&auth, fb->lines[lx]))
        {
            add_row(authManager, auth.enabled, auth.authname, auth.privkey, auth.domain, fb, lx);
        }
    }
}

void load_auth_information(AuthManager *authManager)
{
    char autoexec_filename[512];
    char authcfg_filename[512];

    #if defined __GNUC__
        const char *homeDir = getenv("HOME");
    #elif defined _WIN32
        const char homeDir[MAX_PATH];
        const DWORD ret = GetEnvironmentVariableA("USERPROFILE",buff,MAX_PATH);
        if (ret==0 || ret>MAX_PATH)
            homeDir = 0;
    #endif
    
    if(!homeDir) return;
    
    printf("Sauerbraten home dir = "SAUERBRATEN_HOME_PATH"\n", homeDir);
    
    snprintf(autoexec_filename, 512, SAUERBRATEN_HOME_PATH"autoexec.cfg", homeDir);
    snprintf(authcfg_filename, 512, SAUERBRATEN_HOME_PATH"auth.cfg", homeDir);

    readfb(&authManager->auth_cfg_buffer, authcfg_filename);    
    readfb(&authManager->autoexec_cfg_buffer, autoexec_filename);
    
    printf("finished reading files.\n");
    
    read_config_buffer(authManager, &authManager->auth_cfg_buffer);
    read_config_buffer(authManager, &authManager->autoexec_cfg_buffer);
    
    printf("finished reading auth keys.\n");
}

void write_row(AuthManager *authManager, GtkTreeIter *iter)
{
    gboolean    enabled;
    char        *authname;
    char        *privkey;
    char        *domain;
    FileBuffer  *fb;
    int         line_index;
    
    gtk_tree_model_get(GTK_TREE_MODEL(authManager->key_store), iter, 
                                                                    COL_ENABLED, &enabled, 
                                                                    COL_AUTHNAME, &authname,
                                                                    COL_PRIVKEY, &privkey,
                                                                    COL_DOMAIN, &domain,
                                                                    COL_CONFIG_BUFFER, &fb,
                                                                    COL_CONFIG_LINE, &line_index,
                                                                    -1);
    
    if(fb->lines[line_index]) free(fb->lines[line_index]);
    
    int line_length = (enabled ? strlen("authkey ") : strlen("//authkey ")) + strlen(authname) + strlen(privkey) + strlen(domain) + 2 + 1;
    fb->lines[line_index] = (char *)malloc(sizeof(char)*line_length);
    if(enabled)
        snprintf(fb->lines[line_index], line_length, "authkey %s %s %s", authname, privkey, domain);
    else
        snprintf(fb->lines[line_index], line_length, "//authkey %s %s %s", authname, privkey, domain);
        
    free(authname);
    free(privkey);
    free(domain);
        
    writefb(fb);
}

void delete_row(AuthManager *authManager, GtkTreeIter *iter)
{
    FileBuffer  *fb;
    int         line_index;
    
    gtk_tree_model_get(GTK_TREE_MODEL(authManager->key_store), iter, 
                                                                    COL_CONFIG_BUFFER, &fb,
                                                                    COL_CONFIG_LINE, &line_index,
                                                                    -1);
    
    gtk_list_store_remove(GTK_LIST_STORE(authManager->key_store), iter);
    
    free(fb->lines[line_index]);
    fb->lines[line_index] = NULL;
    
    writefb(fb);
}

void create_row(AuthManager *authManager, const char *authname, const char *privkey, const char *domain)
{
    FileBuffer  *fb = &authManager->auth_cfg_buffer;
    char       **lines_temp = NULL;
    
    lines_temp = (char **)realloc(fb->lines, sizeof(char *)*(fb->line_count+2));
    
    if(!lines_temp) return;
    
    fb->lines = lines_temp;
    fb->line_count++;
    
    fb->lines[fb->line_count] = NULL;
    
	static GtkTreeIter current_entry;
	gtk_list_store_append(authManager->key_store, &current_entry);
	gtk_list_store_set(authManager->key_store, &current_entry, 
	                                                    COL_ENABLED, 1, 
	                                                    COL_AUTHNAME, authname, 
	                                                    COL_PRIVKEY, privkey, 
	                                                    COL_DOMAIN, domain, 
	                                                    COL_ACTION_INFO, authManager->info_icon_pixbuf, 
	                                                    COL_ACTION_EDIT, authManager->edit_icon_pixbuf, 
	                                                    COL_ACTION_DELETE, authManager->delete_icon_pixbuf, 
	                                                    COL_CONFIG_BUFFER, (gpointer)fb, 
	                                                    COL_CONFIG_LINE, fb->line_count-1, -1);
	                                                    
    int line_length = strlen("authkey ") + strlen(authname) + strlen(privkey) + strlen(domain) + 2 + 1;
    fb->lines[fb->line_count-1] = (char *)malloc(sizeof(char)*line_length);
    snprintf(fb->lines[fb->line_count-1], line_length, "authkey %s %s %s", authname, privkey, domain);
    
    writefb(fb);
}

void toggle_row(AuthManager *authManager, GtkTreeIter *iter)
{
    gboolean enabled;
    gtk_tree_model_get(GTK_TREE_MODEL(authManager->key_store), iter, COL_ENABLED, &enabled, -1);
    
    gtk_list_store_set(authManager->key_store, iter, COL_ENABLED, !enabled, -1);
    
    write_row(authManager, iter);
}

void info_row(AuthManager *authManager, GtkTreeIter *iter)
{
    char        *authname;
    char        *privkey;
    char        *domain;
    FileBuffer  *fb;
    int         line_index;
    
    gtk_tree_model_get(GTK_TREE_MODEL(authManager->key_store), iter, 
                                                                    COL_AUTHNAME, &authname,
                                                                    COL_PRIVKEY, &privkey,
                                                                    COL_DOMAIN, &domain,
                                                                    COL_CONFIG_BUFFER, &fb,
                                                                    COL_CONFIG_LINE, &line_index,
                                                                    -1);

    char *pubkey = cube2crypto_getpubkey(privkey);

    gtk_label_set_text(authManager->infodialog.label_authname, authname);
    gtk_label_set_text(authManager->infodialog.label_privkey, privkey);
    gtk_label_set_text(authManager->infodialog.label_pubkey, pubkey);
    gtk_label_set_text(authManager->infodialog.label_domain, domain);
    gtk_label_set_text(authManager->infodialog.label_location, fb->filename);

    free(authname);
    free(privkey);
    free(pubkey);
    free(domain);

    gtk_widget_show(authManager->infodialog.window);
}

void edit_row(AuthManager *authManager, GtkTreeIter *iter)
{
    char        *authname;
    char        *privkey;
    char        *domain;
    
    gtk_tree_model_get(GTK_TREE_MODEL(authManager->key_store), iter, 
                                                                    COL_AUTHNAME, &authname,
                                                                    COL_PRIVKEY, &privkey,
                                                                    COL_DOMAIN, &domain,
                                                                    -1);
                                                                    
    GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(authManager->key_store), iter);
    
    authManager->editdialog.current_row_path = gtk_tree_path_to_string(path);
    
    gtk_tree_path_free(path);
    
    gtk_entry_set_text(authManager->editdialog.entry_authname, authname);
    gtk_entry_set_text(authManager->editdialog.entry_privkey, privkey);
    gtk_entry_set_text(authManager->editdialog.entry_domain, domain);
    
    gtk_widget_show(authManager->editdialog.window);
}
