/*BK based on a simple systray applet example by Rodrigo De Castro, 2007*/
/*sys tray applet to monitor free personal storage space
  GPL license /usr/share/doc/legal/gpl-2.0.txt.
  freememapplet_tray is started in /usr/sbin/delayedrun and pfbpanel.
  2.1: changed from png to included xpm images, passed param not needed.
  2.2: 100517: always read free space in save file, not RAM space.
  2.2: pet pkg has moved executable to /root/Startup.
  2.3: 100820: 01micko: PUPMODE==2 fix.
  2.3.1: BK 110805 testing with PUPMODE=2, needs fix, there is no /dev/root.
   (puppy has a fixed df that returns actual partition instead of /dev/root)
  2.4 (20120519): rodin.s: added gettext.
  
  modified by mistfire
*/

#include <libintl.h>
#include <locale.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <libayatana-appindicator/app-indicator.h>
#include <sys/statfs.h>
#include <string.h>

#define MAXCHARNUM 16
#define MAXCHARMSG 256

#define _(STRING)    gettext(STRING)

#define PFILE "/etc/rc.d/PUPSTATE"

#define ICON_PATH "/usr/share/pixmaps/freememapplet"

#define QUIT _("Quit")
#define RESIZE _("Resize personal storage")
#define STORAGE_MSG _("personal storage, free space")
#define RIGHT_MENU _("Resize personal storage, right click for menu.")

AppIndicator *tray_icon;
GtkWidget *menu;
GtkWidget *mem_item_total;
GtkWidget *mem_item_free;
GtkWidget *mem_item_type;

guint interval = 10; /*update interval in seconds*/
unsigned long long sizetotal;
unsigned long long sizefree;
unsigned long long sizefreeprev = 0;


gchar memdisplayfree[MAXCHARNUM];
gchar memdisplaytotal[MAXCHARNUM];

gchar mnudisplayfree[MAXCHARMSG];
gchar mnudisplaytotal[MAXCHARMSG];
gchar mnudisplaytype[MAXCHARMSG];


gchar memdisplaylong[MAXCHARMSG];

gchar *save_layer_dir;
gchar *save_layer_type;

unsigned int ptoffset;
int percentfree;
gboolean pupSavefile = FALSE;

gboolean Update(gpointer ptr);

void getFileSystemData(const char *fln)
{
    struct statfs vfs;
    int retval;
    retval = statfs(fln, &vfs);
    if (retval == 0)
    {
        sizetotal = vfs.f_blocks;
        sizetotal = (sizetotal * vfs.f_bsize) / 1000000;
        sizefree = vfs.f_bavail;
        sizefree = (sizefree * vfs.f_bsize) / 1000000;
    }
    else
    {
        sizetotal = sizefree = 0;
    }
}

gboolean Update(gpointer ptr)
{
    getFileSystemData(save_layer_dir);

    if(sizefreeprev == sizefree){
        return TRUE;
    }
        
    sizefreeprev = sizefree;

    if(sizefree < 1000){
        g_snprintf(memdisplayfree, MAXCHARNUM, "%lldMB", sizefree);
    }
    else if(sizefree >= 10000){
        g_snprintf(memdisplayfree, MAXCHARNUM, "%.0fGB", (float)(sizefree / 1000.0));
    }
    else{
        g_snprintf(memdisplayfree, MAXCHARNUM, "%.0fGB", (float)(sizefree / 1000.0));
    }    
        
    if (sizetotal < 1000){
        g_snprintf(memdisplaytotal, MAXCHARNUM, "%lldMB", sizetotal);
    }
    else if (sizetotal >= 10000){
        g_snprintf(memdisplaytotal, MAXCHARNUM, "%.0fGB", (float)(sizetotal / 1000.0));
    }
    else{
        g_snprintf(memdisplaytotal, MAXCHARNUM, "%.0fGB", (float)(sizetotal / 1000.0));
	}
	
    if (pupSavefile){
        g_snprintf(memdisplaylong, MAXCHARMSG, "%s %s %s\n%s", memdisplaytotal, STORAGE_MSG, memdisplayfree, RIGHT_MENU);
    }
    else{
        g_snprintf(memdisplaylong, MAXCHARMSG, "%s %s %s", memdisplaytotal, STORAGE_MSG, memdisplayfree);
	}
	
    percentfree = sizetotal == 0 ? 0 : (sizefree * 100) / sizetotal;

    if (percentfree < 20){
        app_indicator_set_icon_full(tray_icon, ICON_PATH "/container_1.svg", memdisplaylong);
    }
    else if (percentfree < 45){
        app_indicator_set_icon_full(tray_icon, ICON_PATH "/container_2.svg", memdisplaylong);
    }
    else if (percentfree < 70){
        app_indicator_set_icon_full(tray_icon, ICON_PATH "/container_3.svg", memdisplaylong);
    }
    else{
        app_indicator_set_icon_full(tray_icon, ICON_PATH "/container_4.svg", memdisplaylong);
	}

	
	g_snprintf(mnudisplaytotal, MAXCHARMSG, "Total Space: %s", memdisplaytotal);
	g_snprintf(mnudisplayfree, MAXCHARMSG, "Free Space: %s", memdisplayfree);
	 
    gtk_menu_item_set_label(GTK_MENU_ITEM(mem_item_total), mnudisplaytotal);  
    gtk_menu_item_set_label(GTK_MENU_ITEM(mem_item_free), mnudisplayfree);    

    return TRUE;
}

void quit()
{
    gtk_main_quit();
}

void resize_pupsave()
{
    system("resizepfile.sh &");
}

GtkWidget *create_menu()
{
    GtkWidget *menu = gtk_menu_new();
    
	// Dynamic memory label
	g_snprintf(mnudisplaytype, MAXCHARMSG, "Storage Type: %s", save_layer_type);
	g_snprintf(mnudisplaytotal, MAXCHARMSG, "Memory Capacity: %s", memdisplaytotal);
	g_snprintf(mnudisplayfree, MAXCHARMSG, "Free Memory: %s", memdisplayfree);

	mem_item_type = gtk_menu_item_new_with_label(mnudisplaytype);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), mem_item_type);
	
	mem_item_total = gtk_menu_item_new_with_label(mnudisplaytotal);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), mem_item_total);
	
	mem_item_free = gtk_menu_item_new_with_label(mnudisplayfree);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), mem_item_free);

	// Separator (optional)
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    

    if (pupSavefile)
    {
        GtkWidget *menuitem = gtk_menu_item_new_with_label(RESIZE);
        g_signal_connect(menuitem, "activate", G_CALLBACK(resize_pupsave), NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
    }

    GtkWidget *menuitem_quit = gtk_menu_item_new_with_label(QUIT);
    g_signal_connect(menuitem_quit, "activate", G_CALLBACK(quit), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_quit);

    gtk_widget_show_all(menu);
    return menu;
}

int main(int argc, char **argv)
{
    gchar **linesArray = NULL;
    gchar *linesString = NULL;
    gchar *newLine = "\n";
    gchar *curLine;
    gchar *curVal;
    guint numLines;
    gchar **chunksArray = NULL;
    gchar *chunkSep = "'";
    guint psave;
    gboolean havePupstate = FALSE;
    gboolean haveSaveLayer = FALSE;

    setlocale(LC_ALL, "");
    bindtextdomain("freememapplet_tray", "/usr/share/locale");
    textdomain("freememapplet_tray");

    save_layer_dir = NULL;
    if (g_file_get_contents(PFILE, &linesString, NULL, NULL))
    {
        havePupstate = TRUE;
        linesArray = g_strsplit(linesString, newLine, 0);
        numLines = g_strv_length(linesArray);
        for (psave = 0; psave < numLines; psave++)
        {
            curLine = linesArray[psave];
            if (g_str_has_prefix(curLine, "PUPSAVE="))
            {
                chunksArray = g_strsplit(curLine, chunkSep, 0);
                if (g_strv_length(chunksArray) > 1)
                {
                    curVal = chunksArray[1];
                    if ((g_str_has_suffix(curVal, ".2fs")) ||
                        (g_str_has_suffix(curVal, ".3fs")) ||
                        (g_str_has_suffix(curVal, ".4fs")))
                        pupSavefile = TRUE;
                }
                g_strfreev(chunksArray);
            }
            else if (g_str_has_prefix(curLine, "SAVE_LAYER="))
            {
                chunksArray = g_strsplit(curLine, chunkSep, 0);
                if (g_strv_length(chunksArray) > 1)
                {
                    curVal = chunksArray[1];
                    if (strlen(curVal) > 0)
                        save_layer_dir = g_strconcat("/initrd", curVal, NULL);
                }
                g_strfreev(chunksArray);
            }
        }
        if (save_layer_dir == NULL)
        {
            save_layer_dir = g_strdup("/");
            save_layer_type = g_strdup("Folder");
            haveSaveLayer = TRUE;
        }
        else
        {
            if (g_file_test(save_layer_dir, G_FILE_TEST_EXISTS))
            {
                haveSaveLayer = TRUE;
                
                if(pupSavefile){
					save_layer_type = g_strdup("File");
				}
				else{
					save_layer_type = g_strdup("Folder");					
				}
			
			}	
            else
            {
				save_layer_type = g_strdup("RAM"); 
                g_free(save_layer_dir);
                save_layer_dir = NULL;
            }
        }
    }

    g_free(linesString);
    g_strfreev(linesArray);

    if ((havePupstate) && (haveSaveLayer))
    {
        gtk_init(&argc, &argv);

        tray_icon = app_indicator_new("freememapplet", ICON_PATH "/container_0.svg", APP_INDICATOR_CATEGORY_SYSTEM_SERVICES);
        app_indicator_set_status(tray_icon, APP_INDICATOR_STATUS_ACTIVE);
        app_indicator_set_title(tray_icon,"Storage Space Status");
        menu = create_menu();
        app_indicator_set_menu(tray_icon, GTK_MENU(menu));

        g_timeout_add_seconds(interval, Update, NULL);
        Update(NULL);

        gtk_main();

        g_free(save_layer_dir);
        g_free(save_layer_type);

        return 0;
    }

    if (havePupstate)
        printf("SAVE_LAYER line is badly formatted.\n");
    else
        printf("PUPSTATE file is missing.\n");

    return 1;
}
