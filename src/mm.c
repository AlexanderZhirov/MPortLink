#include "mportlink.h"

MMManager *mmcli_get_manager_finish(GAsyncResult *res)
{
    return g_task_propagate_pointer(G_TASK(res), NULL);
}

void manager_new_ready(GDBusConnection *connection, GAsyncResult *res, GTask *task)
{
    MMManager *manager;
    gchar *name_owner;
    GError *error = NULL;

    manager = mm_manager_new_finish(res, &error);
    if (!manager)
    {
        syslog(LOG_ERR, "manager_new_ready, manager: couldn't create manager: %s\n", error ? error->message : "unknown error");
        exit(EXIT_FAILURE);
    }

    name_owner = g_dbus_object_manager_client_get_name_owner(G_DBUS_OBJECT_MANAGER_CLIENT(manager));
    if (!name_owner)
    {
        syslog(LOG_ERR, "manager_new_ready, name_owner: couldn't find the ModemManager process in the bus");
        exit(EXIT_FAILURE);
    }

    g_free(name_owner);

    g_task_return_pointer(task, manager, g_object_unref);
    g_object_unref(task);
}

void mmcli_get_manager(GDBusConnection *connection, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data)
{
    GTask *task;

    task = g_task_new(connection, cancellable, callback, user_data);

    mm_manager_new(connection, G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_DO_NOT_AUTO_START, cancellable, (GAsyncReadyCallback)manager_new_ready, task);
}
