/*
 * Copyright (C) 2017 La Ode Muh. Fadlun Akbar <fadlun.net@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <gtk/gtk.h>
#include <libudev.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "usbip.h"
#include "usbip_common.h"

typedef struct _usb_remote_info {
    const char* path;
    const char* idVendor;
    const char* idProduct;
    const char* bConfValue;
    const char* bNumIntfs;
    const char* busid;
    const char* manufact;
    const char* product_usb;
    char product_name[128];
} USBRemoteInfo;

typedef struct _ui_usb_list {
    GtkWidget* items;
    GSList* list;
    gboolean clear_state;
} USBListInterface;

static gboolean attach_usb_remote(void);
static gboolean detach_usb_remote(void);
static GSList* usb_devices_list_local(void);
static void interface_list_local(GtkWidget* window, gpointer user_data);
static GtkWidget* main_window(GtkWidget* window);

static gboolean
attach_usb_remote(void)
{
    gboolean control_status = TRUE;
    return control_status;
}

static gboolean
detach_usb_remote(void)
{
    gboolean control_status = TRUE;
    return control_status;
}

static GtkWidget*
control_usb_remote(GtkWidget* button, gpointer user_data)
{
    USBRemoteInfo* USBDevInfo = (USBRemoteInfo*)user_data;
    if (g_strcmp0(gtk_button_get_label(GTK_BUTTON(button)), "Attach") == 0) {
        if (attach_usb_remote()) {
            printf("Attach: %s\n", USBDevInfo->product_usb);
            gtk_button_set_label(GTK_BUTTON(button), "Detach");
        }
    } else {
        if (detach_usb_remote()) {
            printf("Detach: %s\n", USBDevInfo->product_usb);
            gtk_button_set_label(GTK_BUTTON(button), "Attach");
        }
    }
    return button;
}

static void
interface_list_local(GtkWidget* window, gpointer user_data)
{
    GtkWidget* devs_list = NULL;
    GtkWidget* button_box = NULL;
    GtkWidget* devs_info = NULL;
    GtkWidget* button = NULL;
    GtkWidget* label = NULL;
    gchar* devs_desc = NULL;
    GSList* iterator = NULL;

    USBListInterface* USBRefreshList = (USBListInterface*)user_data;

    USBRefreshList->list = usb_devices_list_local();
    window = USBRefreshList->items;

    devs_list = gtk_list_box_new();

    for (iterator = USBRefreshList->list; iterator; iterator = iterator->next) {
        devs_info = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        button_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

        devs_desc =
          g_strdup_printf("<b>%s</b>\nidVendor: %s\nidProduct: %s\n"
                          "Manufacturer: %s\nBUSID: %s\n",
                          ((USBRemoteInfo*)iterator->data)->product_usb,
                          ((USBRemoteInfo*)iterator->data)->idVendor,
                          ((USBRemoteInfo*)iterator->data)->idProduct,
                          ((USBRemoteInfo*)iterator->data)->manufact,
                          ((USBRemoteInfo*)iterator->data)->busid);

        label = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(label), devs_desc);

        button = gtk_button_new_with_label("Attach");
        gtk_box_set_center_widget(GTK_BOX(button_box), button);
        g_signal_connect(button, "clicked", G_CALLBACK(control_usb_remote),
                         ((USBRemoteInfo*)iterator->data));

        gtk_box_pack_start(GTK_BOX(devs_info), label, FALSE, FALSE, 0);
        gtk_box_pack_end(GTK_BOX(devs_info), button_box, FALSE, FALSE, 0);
        gtk_list_box_insert(GTK_LIST_BOX(devs_list), devs_info, 0);

        g_free(devs_desc);
    }

    g_slist_free(USBRefreshList->list);

    if (gtk_bin_get_child(GTK_BIN(window)) == NULL) {
        gtk_container_add(GTK_CONTAINER(window), devs_list);
    } else {
        gtk_widget_queue_draw(window);
    }

    printf("Refresh list\n");
    gtk_widget_show_all(window);
}

static GSList*
usb_devices_list_local()
{
    GSList* usb_dev_list = NULL;

    struct udev* udev;
    struct udev_enumerate* enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device* dev;

    udev = udev_new();
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "usb");
    udev_enumerate_add_nomatch_sysattr(enumerate, "bDeviceClass", "09");
    udev_enumerate_add_nomatch_sysattr(enumerate, "bInterfaceNumber", NULL);
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        USBRemoteInfo* USBDevInfo = g_new(USBRemoteInfo, 1);
        USBDevInfo->path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, USBDevInfo->path);

        USBDevInfo->idVendor = udev_device_get_sysattr_value(dev, "idVendor");
        USBDevInfo->idProduct = udev_device_get_sysattr_value(dev, "idProduct");
        USBDevInfo->bConfValue =
          udev_device_get_sysattr_value(dev, "bConfigurationValue");
        USBDevInfo->bNumIntfs =
          udev_device_get_sysattr_value(dev, "bNumInterfaces");
        USBDevInfo->busid = udev_device_get_sysname(dev);
        USBDevInfo->manufact =
          udev_device_get_sysattr_value(dev, "manufacturer");
        USBDevInfo->product_usb = udev_device_get_sysattr_value(dev, "product");

        if (!USBDevInfo->idVendor || !USBDevInfo->idProduct ||
            !USBDevInfo->bConfValue || !USBDevInfo->bNumIntfs) {
            err("problem getting device attributes: %s", strerror(errno));
            goto err_out;
        }

        usbip_names_get_product(USBDevInfo->product_name,
                                sizeof(USBDevInfo->product_name),
                                strtol(USBDevInfo->idVendor, NULL, 16),
                                strtol(USBDevInfo->idProduct, NULL, 16));

        usb_dev_list = g_slist_append(usb_dev_list, USBDevInfo);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return usb_dev_list;

err_out:
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return NULL;
}

static GtkWidget*
main_window(GtkWidget* window)
{
    GtkWidget* header = NULL;
    GtkWidget* button = NULL;
    GtkWidget* box = NULL;
    GtkWidget* image = NULL;
    GIcon* icon = NULL;

    USBListInterface* USBRefreshList = g_new(USBListInterface, 1);
    USBRefreshList->items = window;
    USBRefreshList->list = NULL;
    USBRefreshList->clear_state = TRUE;

    if (!USBRefreshList->items) {
        USBRefreshList->items = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        g_signal_connect(USBRefreshList->items, "destroy",
                         G_CALLBACK(gtk_widget_destroyed),
                         &USBRefreshList->items);

        header = gtk_header_bar_new();
        gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
        gtk_header_bar_set_title(GTK_HEADER_BAR(header),
                                 "USB Device in Your Area - Skripsi");
        gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header), FALSE);

        button = gtk_button_new();
        icon = g_themed_icon_new("view-refresh-symbolic");
        image = gtk_image_new_from_gicon(icon, GTK_ICON_SIZE_BUTTON);
        g_object_unref(icon);
        gtk_container_add(GTK_CONTAINER(button), image);
        gtk_header_bar_pack_end(GTK_HEADER_BAR(header), button);

        g_signal_connect(button, "clicked", G_CALLBACK(interface_list_local),
                         USBRefreshList);

        box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_style_context_add_class(gtk_widget_get_style_context(box),
                                    "linked");
        gtk_header_bar_pack_start(GTK_HEADER_BAR(header), box);

        gtk_window_set_titlebar(GTK_WINDOW(USBRefreshList->items), header);
        gtk_window_set_default_size(GTK_WINDOW(USBRefreshList->items), 600,
                                    400);
    }
    return USBRefreshList->items;
}

int
main(void)
{
    GtkWidget* window = NULL;

    gtk_init(0, NULL);
    window = main_window(window);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    gtk_main();

    return EXIT_SUCCESS;
}
