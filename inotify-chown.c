// Fonte: https://www.thegeekstuff.com/2010/04/inotify-c-program-example/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/inotify.h>

#define EVENT_SIZE (sizeof (struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16 ))
#define MAX_FILENAME 4096
int fd;

void close_inotify();
void get_user_info(char *username, struct passwd *pwd);

int main()
{
	int length, i = 0, result_chown;
	int wd;
	char buffer[EVENT_BUF_LEN];
	struct passwd *pwd = NULL;
	char filename[MAX_FILENAME];

	// Parâmetros fixos, depois vão virar parâmetros
	char *directory = "/home/pi/bluetooth";
	char *user = "pi";

	// Criar instância INOTIFY
	fd = inotify_init();
	if (fd < 0)
	{
		perror("inotify_init");
		exit(1);
	}

	// Registrar sinais de finalizar processo
	signal(SIGTERM, close_inotify);
	signal(SIGINT, close_inotify);
	signal(SIGHUP, close_inotify);
	signal(SIGKILL, close_inotify);

	// Colocar diretório no watch list
	// TODO: Validar se diretório existe
	wd = inotify_add_watch(fd, directory, IN_CREATE);

	while (1)
	{
		// Ler notificação
		length = read(fd, buffer, EVENT_BUF_LEN);

		if (length < 0)
		{
			perror("read");
			close_inotify();
			exit(1);
		}

		i = 0;
		while (i < length)
		{
			struct inotify_event *event = (struct inotify_event *) &buffer[i];
			if (event->len)
			{
				if (event->mask & IN_CREATE)
				{
					if (event->mask & IN_ISDIR)
					{
						printf("New directory created: %s\n.", event->name);
					}
					else
					{
						printf("New file created: %s\n", event->name);
					}

					pwd = calloc(1, sizeof(struct passwd));
					if (pwd == NULL)
					{
						fprintf(stderr, "Failed to allocate struct passwd for getpwnam_r\n");
						close_inotify();
						exit(1);
					}

					get_user_info(user, pwd);
					sprintf(filename, "%s/%s", directory, event->name);
					result_chown = chown(filename, pwd->pw_uid, pwd->pw_gid);
					if (result_chown == 0)
					{
						printf("Changed ownership of file '%s' to user '%s'\n", filename, user);
					}
					else
					{
						fprintf(stderr, "Failed to change ownership of file '%s' to user '%s', result_chown = %s\n", filename, user, strerror(errno));
					}

					free(pwd);
				}
			}
			i += EVENT_SIZE + event->len;
		}
	}

	close_inotify();

}

void close_inotify()
{
	if (fd >= 0)
	{
		close(fd);
	}
}

/* https://stackoverflow.com/a/1009398 */
void get_user_info(char *username, struct passwd *pwd)
{
	size_t buffer_len = sysconf(_SC_GETPW_R_SIZE_MAX) * sizeof(char);
	char *buffer = malloc(buffer_len);
	if (buffer == NULL)
	{
		fprintf(stderr, "Failed to allocate buffer for getpwnam_r\n");
		close_inotify();
		exit(1);
	}

	getpwnam_r(username, pwd, buffer, buffer_len, &pwd);
	free(buffer);

	if (pwd == NULL)
	{
		fprintf(stderr, "get_pwnam_r failed to find requested entry for user '%s'\n", username);
		close_inotify();
		exit(1);
	}
}

