#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gst/video/videooverlay.h>
#include <curl/curl.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#elif defined GDK_WINDOWING_WIN32
#include <gdk/gdkwin32.h>
#elif defined GDK_WINDOWING_QUARTZ
#include <gdk/gdkquartz.h>
#endif

static struct VideoElements
{
	GstElement *pipeline;
	GstElement *playbin;
} videoElements;

static guintptr handler = 0;
static char *initUrl = NULL;
static GtkWidget* create_menu(void);
static void on_app_activate(GtkApplication*, gpointer);
static void on_activate(GtkWidget*, gpointer);
static gboolean on_configure_event(GtkWidget*, GdkEventConfigure*, gpointer);
//鼠标右击显示菜单
static gboolean on_button_press_event(GtkWidget*, GdkEvent*, gpointer);
static void on_realize(GtkWidget*, GstElement*);
static GstBusSyncReply bus_sync_handler(GstBus*, GstMessage*, gpointer);
static gboolean on_bus(GstBus*, GstMessage*, gpointer);
#ifdef WIN32
static ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif
static size_t write_func(void*, size_t, size_t, FILE*);
static size_t read_func(char*, size_t, size_t, FILE*);

int main(int argc, char *argv[])
{
	gst_init(&argc, &argv);
	GtkApplication *app  = gtk_application_new("nook.inc", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return status;
}
static void on_app_activate(GtkApplication *app, gpointer data)
{
	GtkWidget *window = gtk_application_window_new(app);

	GtkWidget *menuBar = gtk_menu_bar_new();
	GtkWidget *channelItem = gtk_menu_item_new_with_label("频道");
	gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), channelItem);
	GtkWidget *channelMenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(channelItem), channelMenu);
	GtkWidget *menu = gtk_menu_new();

	g_signal_connect(window, "button-press-event", G_CALLBACK(on_button_press_event), menu);
	g_signal_connect(window, "configure_event", G_CALLBACK(on_configure_event), NULL);

	GtkWidget *mainbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *video = gtk_drawing_area_new();
	g_signal_connect(video, "realize", G_CALLBACK(on_realize), NULL);
	gtk_box_pack_start(GTK_BOX(mainbox), GTK_WIDGET(menuBar), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(mainbox), GTK_WIDGET(video), TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), mainbox);

	gtk_widget_set_size_request(window, 400, 300);

	CURL *curl = curl_easy_init();
	if (curl) {
		char *curDir = g_get_current_dir();
		char *path = malloc(strlen(curDir) + 12);
		strcpy(path, curDir);
		strcat(path, "/global.m3u");
		FILE *fp = fopen(path, "w");
		curl_easy_setopt(curl, CURLOPT_URL, "https://live.fanmingming.com/tv/m3u/global.m3u");
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_func);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_perform(curl);
		fclose(fp);
		curl_easy_cleanup(curl);

		fp = fopen(path, "r");
		char *line = NULL;
		char name[20] = {0}, uri[100] = {0};
		size_t leng = 0;
		while (getline(&line, &leng, fp) > 0) {
			char *nameStart = strstr(line, "tvg-name=\"");
			if (nameStart) {
				nameStart += strlen("tvg-name=\"");
				char *nameEnd = strchr(nameStart, '\"');
				int nameleng = nameEnd - nameStart;
				memset(name, 0, sizeof name);
				strncpy(name, nameStart, nameleng);
			} else if (strncmp(line, "https://", strlen("https://")) == 0 && name[0] != 0) {
				if (!initUrl) {
					initUrl = calloc(1, strlen(line) - 1);
					strncpy(initUrl, line, strlen(line) - 1);
				}
				memset(uri, 0, sizeof uri);
				strncpy(uri, line, strlen(line) - 1);
				GtkWidget *item = gtk_menu_item_new_with_label(name);
				g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(on_activate), uri);
				gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), item);
				//gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
				memset(name, 0, 10);
			}
			leng = 0;
		}
		free(line);
	}
	gtk_widget_show_all(window);
	return;
}
static void on_activate(GtkWidget *widget, gpointer data)
{
	gst_element_set_state(videoElements.pipeline, GST_STATE_NULL);
	g_object_set(G_OBJECT(videoElements.playbin), "uri", data, NULL);
	gst_element_set_state(videoElements.pipeline, GST_STATE_PLAYING);
	return;
}
static void on_realize(GtkWidget *video, GstElement *data)
{
	GdkWindow *window = gtk_widget_get_window(video);
	if (!gdk_window_ensure_native(window))
		puts("error!");
	#ifdef GDK_WINDOWING_WIN32
	handler = (guintptr)GDK_WINDOW_HWND(window);
	#elif defined GDK_WINDOWING_QUARTZ
	handler = gdk_quartz_window_get_nsview(window);
	#elif defined GDK_WINDOWING_X11
	handler = GDK_WINDOW_XID(window);
	#endif

	videoElements.pipeline = gst_pipeline_new("gtk-pipeline");
    videoElements.playbin = gst_element_factory_make("playbin", "gtk_playbin");
	if (videoElements.playbin == NULL) {
		puts("gst_element_factory_make failed!");
		exit(EXIT_FAILURE);
	}
	gst_bin_add(GST_BIN(videoElements.pipeline), videoElements.playbin);
	g_object_set(G_OBJECT(videoElements.playbin), "uri", initUrl, NULL);
	gst_element_set_state(videoElements.pipeline, GST_STATE_PLAYING);
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(videoElements.pipeline));
	gst_bus_set_sync_handler(bus, (GstBusSyncHandler)bus_sync_handler, videoElements.pipeline, NULL);
	gst_bus_add_watch(bus, on_bus, videoElements.pipeline);
	gst_object_unref(bus);
	return;
}
/*!
 @brief 鼠标右击显示菜单
 @param widget 触发事件的主界面
 @param event 鼠标点击事件
 @param data 要显示的菜单
 @return 是否已自行处理
*/
static gboolean on_button_press_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_MENU(data), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);
	if (event->type == GDK_BUTTON_PRESS) {
		GdkEventButton *mouse = (GdkEventButton*)event;
		if (mouse->button == GDK_BUTTON_SECONDARY) {
			gtk_widget_show_all(GTK_WIDGET(data));
			gtk_menu_popup_at_pointer(GTK_MENU(data), event);
			return TRUE;
		}
	}
	return FALSE;
}
static gboolean on_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	gint x = event->x;
	gint y = event->y;
	gint w = event->width;
	gint h = event->height;
	return TRUE;
}
static GstBusSyncReply bus_sync_handler(GstBus * bus, GstMessage * message, gpointer data)
{
	if (!gst_is_video_overlay_prepare_window_handle_message(message))
		return GST_BUS_PASS;
	if (handler != 0) {
		GstVideoOverlay *overlay = GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message));
		gst_video_overlay_set_window_handle(overlay, handler);
	} else {
		g_warning("handle failed!");
	}
	gst_message_unref(message);
	return GST_BUS_DROP;
}
static gboolean on_bus(GstBus *bus, GstMessage *msg, gpointer data)
{
	switch (GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_ERROR: {
		GError *err;
		char *debug;
		gst_message_parse_error(msg, &err, &debug);
		gst_message_unref(msg);
		puts(err->message);
		g_error_free(err);
		free(debug);
		gtk_main_quit();
	}
		break;
	case GST_MESSAGE_EOS:
		gst_element_set_state(GST_ELEMENT(data), GST_STATE_NULL);
		break;
	default:
		break;
	}
	return TRUE;
}
#ifdef WIN32
#define GETLINE_BUFLEN 128
static ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
	char* bufptr;
	char* p;
	size_t size;
	int c;
	if (!lineptr || !n || !stream)
		return -1;
	bufptr = *lineptr;
	size = *n;
	c = fgetc(stream);
	if (c == EOF)
		return -1;
	if (!bufptr) {
		if ((bufptr = (char*)malloc(GETLINE_BUFLEN)) == NULL)
			return -1;
		size = GETLINE_BUFLEN;
	}
	p = bufptr;
	while (c != EOF) {
		if ((p - bufptr) > (size - 1)) {
			size = size + GETLINE_BUFLEN;
			if ((bufptr = (char*)realloc(bufptr, size)) == NULL)
				return -1;
		}
		*p++ = c;
		if (c == '\n')
			break;
		c = fgetc(stream);
	}
	*p++ = 0;
	*lineptr = bufptr;
	*n = size;
	return p - bufptr - 1;
}
#endif
static size_t write_func(void *ptr, size_t size, size_t memb, FILE *fp)
{
	return fwrite(ptr, size, memb, fp);
}
static size_t read_func(char *ptr, size_t size, size_t memb, FILE *fp)
{
	return fread(ptr, size, memb, fp);
}