#include <libmm-glib.h>
#include <syslog.h>

typedef struct
{
    MMManager *manager;
    GCancellable *cancellable;
} Context;

MMManager *mmcli_get_manager_finish(GAsyncResult *res);
void mmcli_get_manager(GDBusConnection *connection, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
