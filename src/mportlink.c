#include "mportlink.h"
#include <sys/file.h>
#include <fcntl.h>

#define LOCKFILE "/run/lock/mportlink.lock"
#define MPL "mportlink"

static Context *ctx;
static GCancellable *cancellable;
static GMainLoop *loop;

static void signals_handler(int signum)
{
    if (cancellable)
    {
        if (!g_cancellable_is_cancelled(cancellable))
        {
            syslog(LOG_WARNING, "signals_handler: cancelling the operation...");
            g_cancellable_cancel(cancellable);
        }
    }

    if (loop && g_main_loop_is_running(loop))
    {
        syslog(LOG_WARNING, "signals_handler: cancelling the main loop...");
        g_main_loop_quit(loop);
    }
}

static void get_manager_ready(GObject *source, GAsyncResult *result, gpointer none)
{
    ctx->manager = mmcli_get_manager_finish(result);

    g_signal_connect(ctx->manager, "object-added", G_CALLBACK(mpl_device_added), NULL);
    g_signal_connect(ctx->manager, "object-removed", G_CALLBACK(mpl_device_removed), NULL);
}

int main()
{
    openlog(MPL, LOG_PID, LOG_SYSLOG);

    int lock_fd = open(LOCKFILE, O_CREAT | O_RDWR, 0666);
    int rc = flock(lock_fd, LOCK_EX | LOCK_NB);

    if (rc || errno == EWOULDBLOCK)
    {
        syslog(LOG_ERR, "main: only one instance of %s is allowed", MPL);
        exit(EXIT_FAILURE);
    }

    syslog(LOG_NOTICE, "main: starting the %s daemon", MPL);

    mpl_create_dongle_dir();

    GError *error = NULL;
    GDBusConnection *connection = NULL;

    signal(SIGINT, signals_handler);
    signal(SIGHUP, signals_handler);
    signal(SIGTERM, signals_handler);

    connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (!connection)
    {
        syslog(LOG_ERR, "main, connection: couldn't get bus: %s", error->message);
        exit(EXIT_FAILURE);
    }

    cancellable = g_cancellable_new();
    loop = g_main_loop_new(NULL, FALSE);

    ctx = g_new0(Context, 1);
    if (cancellable)
        ctx->cancellable = g_object_ref(cancellable);

    mmcli_get_manager(connection, cancellable, (GAsyncReadyCallback)get_manager_ready, NULL);
    g_main_loop_run(loop);

    if (cancellable)
        g_object_unref(cancellable);
    g_main_loop_unref(loop);
    g_object_unref(connection);

    flock(lock_fd, LOCK_UN);

    syslog(LOG_NOTICE, "main: stopping the %s daemon", MPL);
    closelog();

    return EXIT_SUCCESS;
}
