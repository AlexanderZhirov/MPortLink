#include "mportlink.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#define DEV_PATH "/dev/"

typedef struct
{
    const gchar *manufacturer;
    const gchar *model;
    const gchar *imei;
    const gchar *device_identifier;
    MMModemPortInfo *ports;
    guint n_ports;
} MPLmodem;

MPLmodem *mpl_get_new_modem()
{
    MPLmodem *modem = (MPLmodem *)calloc(1, sizeof(MPLmodem));

    if (modem == NULL)
    {
        syslog(LOG_ERR, "mpl_get_new_modem, modem: memory allocation error");
        raise(SIGINT);
    }

    return modem;
}

void mpl_free_modem(MPLmodem *modem)
{
    if (modem->n_ports && modem->ports)
        mm_modem_port_info_array_free(modem->ports, modem->n_ports);
    free(modem);
}

void mpl_print_modem_info(MPLmodem *modem, bool connected)
{
    gchar *status;
    status = connected ? "Connected" : "Disconnected";

    g_print("%s %s %s (%s)\n", status, modem->manufacturer, modem->model, modem->device_identifier);
    g_print("`-- Count ports: %d\n", modem->n_ports);

    for (guint i = 0; i < modem->n_ports; i++)
    {
        if (i + 1 < modem->n_ports)
        {
            g_print("    |-- Port %d:\n", i);
            g_print("    |   |-- Name: %s\n", modem->ports[i].name);
            g_print("    |   `-- Link: %s-%d\n", modem->imei, i);
        }
        else
        {
            g_print("    `-- Port %d:\n", i);
            g_print("        |-- Name: %s\n", modem->ports[i].name);
            g_print("        `-- Link: %s-%d\n", modem->imei, i);
        }
    }
}

MPLmodem *mpl_get_new_modem_info(MMObject *obj)
{
    MPLmodem *mpl_modem = NULL;
    MMModem *mm_modem = mm_object_get_modem(obj);

    if (mm_modem == NULL)
    {
        syslog(LOG_ERR, "mpl_get_new_modem_info, mm_modem: modem does not implement the interface");
        return NULL;
    }

    mpl_modem = mpl_get_new_modem();

    mpl_modem->manufacturer = mm_modem_get_manufacturer(mm_modem);
    mpl_modem->model = mm_modem_get_model(mm_modem);
    mpl_modem->imei = mm_modem_get_equipment_identifier(mm_modem);
    mpl_modem->device_identifier = mm_modem_get_device_identifier(mm_modem);

    gboolean success = mm_modem_get_ports(mm_modem, &mpl_modem->ports, &mpl_modem->n_ports);

    g_object_unref(mm_modem);

    if (success)
        return mpl_modem;
    else
        syslog(LOG_WARNING, "mpl_get_new_modem_info, success: error receiving modem ports");

    mpl_free_modem(mpl_modem);

    return NULL;
}

char *mpl_get_dst_path(guint index, const gchar *imei)
{
    char *dst_path = NULL;

    guint size_id = snprintf(NULL, 0, "-%d", index);
    char *id = (char *)calloc(size_id + 1, sizeof(char));

    if (id == NULL)
    {
        syslog(LOG_ERR, "mpl_get_dst_path, id: memory allocation error");
        raise(SIGINT);
    }

    sprintf(id, "-%d", index);

    guint dst_path_size = strlen(DEV_PATH) + strlen(imei) + size_id;
    dst_path = (char *)calloc(dst_path_size + 1, sizeof(char));

    if (dst_path == NULL)
    {
        syslog(LOG_ERR, "mpl_get_dst_path, dst_path: memory allocation error");
        raise(SIGINT);
    }

    strcpy(dst_path, DEV_PATH);
    strcat(dst_path, imei);
    strcat(dst_path, id);

    free(id);

    return dst_path;
}

char *mpl_get_src_path(const gchar *port_name)
{
    char *src_path = NULL;

    guint src_path_size = strlen(DEV_PATH) + strlen(port_name);
    src_path = (char *)calloc(src_path_size + 1, sizeof(char));

    if (src_path == NULL)
    {
        syslog(LOG_ERR, "mpl_get_src_path, src_path: memory allocation error");
        raise(SIGINT);
    }

    strcpy(src_path, DEV_PATH);
    strcat(src_path, port_name);

    return src_path;
}

void mpl_symlink_ports(MPLmodem *modem)
{
    for (guint i = 0; i < modem->n_ports; i++)
    {
        char *src_path = mpl_get_src_path(modem->ports[i].name);
        char *dst_path = mpl_get_dst_path(i, modem->imei);
        struct stat st;

        if (access(src_path, F_OK) == -1)
        {
            syslog(LOG_ERR, "mpl_symlink_ports, src_path: device file is not available [%s]", src_path);
            free(src_path);
            free(dst_path);
            continue;
        }

        if (lstat(dst_path, &st) == 0)
        {
            syslog(LOG_WARNING, "mpl_symlink_ports, dst_path: a symbolic link to the device already exists [%s]", dst_path);
            if (unlink(dst_path) == -1)
            {
                syslog(LOG_ERR, "mpl_symlink_ports, dst_path: link could not be deleted [%s]", dst_path);
                free(src_path);
                free(dst_path);
                continue;
            }
        }

        if (symlink(src_path, dst_path) == 0)
            syslog(LOG_NOTICE, "mpl_symlink_ports, dst_path: the symbolic link has been successfully created [%s -> %s]", dst_path, src_path);
        else
            syslog(LOG_ERR, "mpl_symlink_ports, dst_path: error creating a symbolic link [%s -> %s]", dst_path, src_path);

        free(src_path);
        free(dst_path);
    }
}

void mpl_unlink_ports(MPLmodem *modem)
{
    for (guint i = 0; i < modem->n_ports; i++)
    {
        char *dst_path = mpl_get_dst_path(i, modem->imei);
        struct stat st;

        if (lstat(dst_path, &st) == -1)
        {
            syslog(LOG_WARNING, "mpl_unlink_ports, dst_path: a symbolic link to the device not exists [%s]", dst_path);
            free(dst_path);
            continue;
        }

        if (unlink(dst_path) == 0)
            syslog(LOG_NOTICE, "mpl_unlink_ports, dst_path: link was successfully deleted [%s]", dst_path);
        else
            syslog(LOG_ERR, "mpl_unlink_ports, dst_path: link could not be deleted [%s]", dst_path);

        free(dst_path);
    }
}

void mpl_device_added(MMManager *manager, MMObject *obj)
{
    MPLmodem *modem = mpl_get_new_modem_info(obj);

    if (modem == NULL)
        return;

    // mpl_print_modem_info(modem, true);
    mpl_symlink_ports(modem);

    mpl_free_modem(modem);
}

void mpl_device_removed(MMManager *manager, MMObject *obj)
{
    MPLmodem *modem = mpl_get_new_modem_info(obj);

    if (modem == NULL)
        return;

    // mpl_print_modem_info(modem, false);
    mpl_unlink_ports(modem);

    mpl_free_modem(modem);
}
