extern "C"
{
	void on_newdialog_entry_changed(GtkObject *object, AuthManager *authManager)
	{
	    
        
	}
    
	void on_editdialog_entry_changed(GtkObject *object, AuthManager *authManager)
	{
	    
        
	}
	
	void on_infodialog_button_ok_clicked(GtkObject *object, AuthManager *authManager)
	{
        gtk_widget_hide(authManager->infodialog.window);
	}
	
	void on_editdialog_button_update_clicked(GtkObject *object, AuthManager *authManager)
	{
	    gtk_widget_hide(authManager->editdialog.window);
	    
	    GtkTreeIter iter;
	    
	    gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(authManager->key_store), &iter, authManager->editdialog.current_row_path);
	    free(authManager->editdialog.current_row_path);
	    
        gtk_list_store_set(authManager->key_store, &iter, 
                                                   COL_AUTHNAME, gtk_entry_get_text(authManager->editdialog.entry_authname),
                                                   COL_PRIVKEY, gtk_entry_get_text(authManager->editdialog.entry_privkey),
                                                   COL_DOMAIN, gtk_entry_get_text(authManager->editdialog.entry_domain),
                                                   -1);
        
        write_row(authManager, &iter);
	}
	
	void on_editdialog_button_cancel_clicked(GtkObject *object, AuthManager *authManager)
	{
        gtk_widget_hide(authManager->editdialog.window);
	}

	void on_toolbutton_new_clicked(GtkObject *object, AuthManager *authManager)
	{
	    gtk_entry_set_text(authManager->newdialog.entry_authname, "");
	    gtk_entry_set_text(authManager->newdialog.entry_privkey, "");
	    gtk_entry_set_text(authManager->newdialog.entry_domain, "");
        gtk_widget_show(authManager->newdialog.window);
	}
	
	void on_newdialog_button_add_clicked(GtkObject *object, AuthManager *authManager)
	{
        gtk_widget_hide(authManager->newdialog.window);
        create_row(authManager, gtk_entry_get_text(authManager->newdialog.entry_authname), 
                                gtk_entry_get_text(authManager->newdialog.entry_privkey), 
                                gtk_entry_get_text(authManager->newdialog.entry_domain));
	}
	
	void on_newdialog_button_cancel_clicked(GtkObject *object, AuthManager *authManager)
	{
        gtk_widget_hide(authManager->newdialog.window);
	}
	
    void on_newdialog_clipboard_received(GtkClipboard *clipboard, const char *text_data, AuthManager *authManager)
	{
        char    authname[512];
        char    privkey[51];
        char    domain[512];
	    if(text_data)
	    {
	        printf("data = '%s'\n", text_data);
	        int values_read = sscanf(text_data, "%*12s %511s %51s %511s", authname, privkey, domain);
	        if(values_read >= 2)
	        {
	            gtk_entry_set_text(authManager->newdialog.entry_authname, authname);
	            gtk_entry_set_text(authManager->newdialog.entry_privkey, privkey);
	        }
	        if(values_read >= 3)
	            gtk_entry_set_text(authManager->newdialog.entry_domain, domain);
	        if(values_read <= 1)
	            printf("Clipboard contents does not appear to be an auth key.\n");
        }
        else
        {
            printf("Failed to get clipboard contents.\n");
        }
	}
	
    void on_newdialog_button_fromclip_clicked(GtkObject *object, AuthManager *authManager)
	{
	    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	    gtk_clipboard_request_text(clipboard, (GtkClipboardTextReceivedFunc)on_newdialog_clipboard_received, authManager);
	}
	
    bool on_KeyView_button_press_event(GtkObject *object, GdkEventButton  *event, AuthManager *authManager)
    {
        GtkCellRenderer *renderer = NULL;
        GtkTreeIter iter;

        if(event->button != 1) return true;

        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(object));

        if (gtk_tree_selection_count_selected_rows(selection) > 1) return true;
        
        GtkTreePath *path;

        if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(object), (gint) event->x, (gint) event->y, &path, NULL, NULL, NULL)) return true;
        
        gtk_tree_selection_unselect_all(selection);

        if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(authManager->key_store), &iter, path)) return true;

        gtk_tree_path_free(path);
        
        if (!tree_view_get_cell_from_pos(GTK_TREE_VIEW(object), event->x, event->y, &renderer)) return true;
        
        if(authManager->info_renderer == renderer) { info_row(authManager, &iter); return false;}
        else if(authManager->edit_renderer == renderer) { edit_row(authManager, &iter); return false;}
        else if(authManager->delete_renderer == renderer) { delete_row(authManager, &iter); return false;}
        else if(authManager->enabled_renderer == renderer) { toggle_row(authManager, &iter); return false;}
        return true;
    }
}