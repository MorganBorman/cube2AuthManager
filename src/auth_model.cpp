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
    int lx = 0;
    for(std::vector<std::string>::iterator it = fb->lines.begin(); it != fb->lines.end(); ++it)
    {
        if(read_auth_line(&auth, (*it).c_str()))
        {
            add_row(authManager, auth.enabled, auth.authname, auth.privkey, auth.domain, fb, lx);
        }
        lx++;
    }
}

bool load_auth_information(AuthManager *authManager)
{
    const char **rel_prefix;

    int sauer_home_length;
    char *sauer_home;

    char *autoexec_filename;
    char *authcfg_filename;
	
	char *homeDir;
	char *progDir;

    #if defined _WIN32 || defined _WIN64
		int ret = GetEnvironmentVariableA("USERPROFILE", NULL, 0);
		homeDir = (char *)malloc(sizeof(char)*ret);
        ret = GetEnvironmentVariableA("USERPROFILE", homeDir, ret);
		
		progDir = (char *)malloc(sizeof(char)*MAX_PATH);
		ret = SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, progDir);
        if(ret) return false;
	#else
        homeDir = getenv("HOME");
        progDir = (char *)"";
    #endif
    
    if(!homeDir || !progDir) 
	{
		#if defined _WIN32 || defined _WIN64
			free(homeDir);
			free(progDir);
		#endif
		return false;
	}
    
    for(unsigned int lx = 0; lx < NUMLOCATIONS; lx++) {
        //home_locations[lx].relative_to
        //home_locations[lx].path

        if(home_locations[lx].relative_to == PROGDIR)
            rel_prefix = (const char **)&progDir;
        else if(home_locations[lx].relative_to == HOMEDIR)
            rel_prefix = (const char **)&homeDir;
        else
		{
			#if defined _WIN32 || defined _WIN64
				free(homeDir);
				free(progDir);
			#endif
			printf("Error in home locations. Unknown relative designation.\n");
            return false;
		}

        sauer_home_length = strlen(*rel_prefix) + strlen(FS_DELIM) + strlen(home_locations[lx].path) + 1;

        sauer_home = (char *)malloc(sizeof(char)*(sauer_home_length));

        snprintf(sauer_home, sauer_home_length, "%s%s%s", *rel_prefix, FS_DELIM, home_locations[lx].path);

        printf("Checking for Sauerbraten home dir: '%s'\n", sauer_home);

        if(directory_exists(sauer_home)) break;

        free(sauer_home);
        sauer_home = NULL;
    }
	
	#if defined _WIN32 || defined _WIN64
		free(homeDir);
		free(progDir);
	#endif

    if(!sauer_home) return false;

    printf("Found Sauerbraten home dir: '%s'\n", sauer_home);

    int autoexec_filename_length = sauer_home_length + strlen(FS_DELIM) + strlen("autoexec.cfg") + 1;
    int authcfg_filename_length = sauer_home_length + strlen(FS_DELIM) + strlen("auth.cfg") + 1;

    autoexec_filename = (char *)malloc(sizeof(char)*(autoexec_filename_length));
    authcfg_filename = (char *)malloc(sizeof(char)*(authcfg_filename_length));

    snprintf(autoexec_filename, autoexec_filename_length, "%s%sautoexec.cfg", sauer_home, FS_DELIM);
    snprintf(authcfg_filename, authcfg_filename_length, "%s%sauth.cfg", sauer_home, FS_DELIM);

    authManager->auth_cfg_file = g_file_new_for_path(authcfg_filename);
    authManager->autoexec_cfg_file = g_file_new_for_path(autoexec_filename);

    authManager->auth_cfg_monitor = g_file_monitor_file(authManager->auth_cfg_file, G_FILE_MONITOR_NONE, NULL, NULL);
    authManager->autoexec_cfg_monitor = g_file_monitor_file(authManager->autoexec_cfg_file, G_FILE_MONITOR_NONE, NULL, NULL);

    g_signal_connect(authManager->auth_cfg_monitor, "changed", (GCallback)on_file_changed, authManager);
    g_signal_connect(authManager->autoexec_cfg_monitor, "changed", (GCallback)on_file_changed, authManager);

    readfb(&authManager->auth_cfg_buffer, authcfg_filename);
    readfb(&authManager->autoexec_cfg_buffer, autoexec_filename);
    
    free(autoexec_filename);
    free(authcfg_filename);
	
	authManager->sauer_home = sauer_home;

    printf("finished reading files.\n");
    
    read_config_buffer(authManager, &authManager->auth_cfg_buffer);
    read_config_buffer(authManager, &authManager->autoexec_cfg_buffer);
    
    printf("finished reading auth keys.\n");
    return true;
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
    
    int line_length = (enabled ? strlen("authkey ") : strlen("//authkey ")) + strlen(authname) + strlen(privkey) + strlen(domain) + 2 + 1;
    char *temp = (char *)malloc(sizeof(char)*line_length);
    if(enabled)
        snprintf(temp, line_length, "authkey %s %s %s", authname, privkey, domain);
    else
        snprintf(temp, line_length, "//authkey %s %s %s", authname, privkey, domain);

    fb->lines.at(line_index) = std::string(temp);

    free(temp);
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
    
    fb->lines.at(line_index) = std::string("");
    
    writefb(fb);
}

void create_row(AuthManager *authManager, const char *authname, const char *privkey, const char *domain)
{
    FileBuffer  *fb = &authManager->auth_cfg_buffer;
    
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
	                                                    COL_CONFIG_LINE, fb->lines.size(), -1);
	                                                    

    int line_length = strlen("authkey ") + strlen(authname) + strlen(privkey) + strlen(domain) + 2 + 1;
    char *temp = (char *)malloc(sizeof(char)*line_length);
    snprintf(temp, line_length, "authkey %s %s %s", authname, privkey, domain);
    
    fb->lines.push_back(std::string(temp));

    free(temp);

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
