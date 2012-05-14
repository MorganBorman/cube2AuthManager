static gboolean tree_view_get_cell_from_pos(GtkTreeView *view, guint x, guint y, GtkCellRenderer **cell)
{
        GtkTreeViewColumn *col = NULL;
        GList             *node, *columns, *cells;
        guint              colx = 0;
 
        g_return_val_if_fail ( view != NULL, FALSE );
        g_return_val_if_fail ( cell != NULL, FALSE );
 
        /* (1) find column and column x relative to tree view coordinates */
 
        columns = gtk_tree_view_get_columns(view);
 
        for (node = columns;  node != NULL && col == NULL;  node = node->next)
        {
                GtkTreeViewColumn *checkcol = (GtkTreeViewColumn*) node->data;
 
                if (x >= colx  &&  x < (colx + checkcol->width))
                        col = checkcol;
                else
                        colx += checkcol->width;
        }
 
        g_list_free(columns);
 
        if (col == NULL)
                return FALSE; /* not found */
 
        /* (2) find the cell renderer within the column */
 
        cells = gtk_tree_view_column_get_cell_renderers(col);
 
        for (node = cells;  node != NULL;  node = node->next)
        {
                GtkCellRenderer *checkcell = (GtkCellRenderer*) node->data;
                guint            width = 0;
 
                /* Will this work for all packing modes? doesn't that
                 *  return a random width depending on the last content
                 * rendered? */
                gtk_cell_renderer_get_size(checkcell, GTK_WIDGET(view), NULL, NULL, NULL, (gint*)&width, NULL);
 
                if (x >= colx && x < (colx + width))
                {
                        *cell = checkcell;
                        g_list_free(cells);
                        return TRUE;
                }
 
                colx += width;
        }
 
        g_list_free(cells);
        return FALSE; /* not found */
}
