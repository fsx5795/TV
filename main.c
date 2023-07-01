#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gst/video/videooverlay.h>

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
static GtkWidget* create_menu(void);
static void on_app_activate(GtkApplication*, gpointer);
static void on_activate(GtkWidget*, gpointer);
static gboolean on_configure_event(GtkWidget*, GdkEventConfigure*, gpointer);
//鼠标右击显示菜单
static gboolean on_button_press_event(GtkWidget*, GdkEvent*, gpointer);
static void on_realize(GtkWidget*, GstElement*);
static GstBusSyncReply bus_sync_handler(GstBus*, GstMessage*, gpointer);
static gboolean on_bus(GstBus*, GstMessage*, gpointer);

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
	GtkWidget *video = gtk_drawing_area_new();
	g_signal_connect(video, "realize", G_CALLBACK(on_realize), NULL);

	GtkWidget *menuBar = gtk_menu_bar_new();
	GtkWidget *channelItem = gtk_menu_item_new_with_label("频道");
	gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), channelItem);
	GtkWidget *channelMenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(channelItem), channelMenu);
	GtkWidget *firstItem = gtk_menu_item_new_with_label("中央1套");
	g_signal_connect(G_OBJECT(firstItem), "activate", G_CALLBACK(on_activate),  "https://cntv.sbs/live?auth=230612&id=cctv1");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), firstItem);
	GtkWidget *secondItem = gtk_menu_item_new_with_label("中央2套");
	g_signal_connect(G_OBJECT(secondItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv2");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), secondItem);
	GtkWidget *thirdItem = gtk_menu_item_new_with_label("中央3套");
	g_signal_connect(G_OBJECT(thirdItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv3");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), thirdItem);
	GtkWidget *fourthItem = gtk_menu_item_new_with_label("中央4套");
	g_signal_connect(G_OBJECT(fourthItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv4");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), fourthItem);
	GtkWidget *fifthItem = gtk_menu_item_new_with_label("中央5套");
	g_signal_connect(G_OBJECT(fifthItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv5");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), fifthItem);
	GtkWidget *sixthItem = gtk_menu_item_new_with_label("中央6套");
	g_signal_connect(G_OBJECT(sixthItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv6");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), sixthItem);
	GtkWidget *seventhItem = gtk_menu_item_new_with_label("中央7套");
	g_signal_connect(G_OBJECT(seventhItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv7");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), seventhItem);
	GtkWidget *eighthItem = gtk_menu_item_new_with_label("中央8套");
	g_signal_connect(G_OBJECT(eighthItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv8");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), eighthItem);
	GtkWidget *ninthItem = gtk_menu_item_new_with_label("中央9套");
	g_signal_connect(G_OBJECT(ninthItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv9");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), ninthItem);
	GtkWidget *tenthItem = gtk_menu_item_new_with_label("中央10套");
	g_signal_connect(G_OBJECT(tenthItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv10");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), tenthItem);
	GtkWidget *eleventhItem = gtk_menu_item_new_with_label("中央11套");
	g_signal_connect(G_OBJECT(eleventhItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv11");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), eleventhItem);
	GtkWidget *twelfthItem = gtk_menu_item_new_with_label("中央12套");
	g_signal_connect(G_OBJECT(twelfthItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv12");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), twelfthItem);
	GtkWidget *thirteenthItem = gtk_menu_item_new_with_label("中央13套");
	g_signal_connect(G_OBJECT(thirteenthItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv13");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), thirteenthItem);
	GtkWidget *fourteenthItem = gtk_menu_item_new_with_label("中央14套");
	g_signal_connect(G_OBJECT(fourteenthItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv14");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), fourteenthItem);
	GtkWidget *fifteenthItem = gtk_menu_item_new_with_label("中央15套");
	g_signal_connect(G_OBJECT(fifteenthItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv15");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), fifteenthItem);
	GtkWidget *sixteenthItem = gtk_menu_item_new_with_label("中央16套");
	g_signal_connect(G_OBJECT(sixteenthItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv16");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), sixteenthItem);
	GtkWidget *sixtethkItem = gtk_menu_item_new_with_label("中央16套4k");
	g_signal_connect(G_OBJECT(sixtethkItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv164k");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), sixtethkItem);
	GtkWidget *seventeenthItem = gtk_menu_item_new_with_label("中央17套");
	g_signal_connect(G_OBJECT(seventeenthItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv17");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), seventeenthItem);
	GtkWidget *kItem = gtk_menu_item_new_with_label("中央4k");
	g_signal_connect(G_OBJECT(kItem), "activate", G_CALLBACK(on_activate), "https://cntv.sbs/live?auth=230612&id=cctv4k");
	gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), kItem);

	GtkWidget *menu = gtk_menu_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), firstItem);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), secondItem);
	g_signal_connect(window, "button-press-event", G_CALLBACK(on_button_press_event), menu);
	g_signal_connect(window, "configure_event", G_CALLBACK(on_configure_event), NULL);

	GtkWidget *mainbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(mainbox), GTK_WIDGET(menuBar), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(mainbox), GTK_WIDGET(video), TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), mainbox);

	gtk_widget_set_size_request(window, 400, 300);
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
	g_object_set(G_OBJECT(videoElements.playbin), "uri", "https://cntv.sbs/live?auth=230612&id=cctv1", NULL);
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