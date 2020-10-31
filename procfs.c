#define _GNU_SOURCE
#include "debug.h"
#include "procfs.h"
#include "helper.h"
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>

/**
 * Retrieves the hostname.
 *
 * Parameters:
 * - proc_dir: the proc directory.
 *
 * - hostname_buf: the buffer to add the hostname.
 *
 * - buf_sz: the buffer size.
 *
 * Returns: 0 if found or -1 if not found.
 **/
int pfs_hostname(char *proc_dir, char *hostname_buf, size_t buf_sz)
{
	// Set file path.
	char *dir = malloc(sizeof(char) * (strlen(proc_dir) + strlen("/sys/kernel/hostname") + 1));
	strcpy(dir, proc_dir);
	strcat(dir, "/sys/kernel/hostname");

	int fd = open(dir, O_RDONLY); // Open file.

	if (fd == -1)
	{
		close(fd);
		free(dir);
		return -1;
	}

	lineread(fd, hostname_buf, buf_sz);

	close(fd);
	free(dir);

	return 0;
}

/**
 * Retrieves the kernel version.
 *
 * Parameters:
 * - proc_dir: the proc directory.
 *
 * - version_buf: the buffer to add the kernel version.
 *
 * - buf_sz: the buffer size.
 *
 * Returns: 0 if found or -1 if not found.
 **/
int pfs_kernel_version(char *proc_dir, char *version_buf, size_t buf_sz)
{
	// Set file path.
	char *dir = malloc(sizeof(char) * (strlen(proc_dir) + strlen("/sys/kernel/osrelease") + 1));
	strcpy(dir, proc_dir);
	strcat(dir, "/sys/kernel/osrelease");

	int fd = open(dir, O_RDONLY); // Open file.

	if (fd == -1)
	{
		close(fd);
		free(dir);
		return -1;
	}

	lineread(fd, version_buf, buf_sz);

	strsep(&version_buf, "-"); // Keep the kernel version only.

	close(fd);
	free(dir);

	return 0;
}

/**
 * Retrieves the cpu model name.
 *
 * Parameters:
 * - proc_dir: the proc directory.
 *
 * - model_buf: the buffer to add the cpu model name.
 *
 * - buf_sz: the buffer size.
 *
 * Returns: 0 if found or -1 if not found.
 **/
int pfs_cpu_model(char *proc_dir, char *model_buf, size_t buf_sz)
{
	// Set file path.
	char *dir = malloc(sizeof(char) * (strlen(proc_dir) + strlen("/cpuinfo") + 1));
	strcpy(dir, proc_dir);
	strcat(dir, "/cpuinfo");

	int fd = open(dir, O_RDONLY);

	if (fd == -1)
	{
		close(fd);
		free(dir);
		return -1;
	}

	while (true)
	{
		char buf[128] = {0};

		ssize_t read_sz = lineread(fd, buf, 127);

		if (read_sz <= 0)
		{
			break;
		}

		if (strstr(buf, "model name") != NULL)
		{
			int location = strcspn(buf, ":"); // Get the location of the line after ':'.
			strcpy(model_buf, &buf[location + 2]);
		}
	}

	close(fd);
	free(dir);

	return 0;
}

/**
 * Retrieves the number of processing units.
 *
 * Parameters:
 * - proc_dir: the proc directory.
 *
 * Returns: the number of processing units.
 **/
int pfs_cpu_units(char *proc_dir)
{
	// Set file path.
	char *dir = malloc(sizeof(char) * (strlen(proc_dir) + strlen("/cpuinfo") + 1));
	strcpy(dir, proc_dir);
	strcat(dir, "/cpuinfo");

	int fd = open(dir, O_RDONLY);

	int processing_units = 0;

	while (true)
	{
		char buf[128] = {0};

		ssize_t read_sz = lineread(fd, buf, 127);

		if (read_sz <= 0)
		{
			break;
		}

		if (strstr(buf, "processor") != NULL)
		{
			processing_units++; // Increment processing_units everytime a processor line is found.
		}
	}

	close(fd);
	free(dir);

	return processing_units;

}

/**
 * Retrieves the uptime in seconds.
 *
 * Parameters:
 * - proc_dir: the proc directory.
 *
 * Returns: the uptime in seconds.
 **/
double pfs_uptime(char *proc_dir)
{
	char uptime[32] = {0};
	// Set file path.
	char *dir = malloc(sizeof(char) * (strlen(proc_dir) + strlen("/uptime") + 1));
	strcpy(dir, proc_dir);
	strcat(dir, "/uptime");

	int fd = open(dir, O_RDONLY);

	if (fd == -1)
	{
		close(fd);
		free(dir);
		return 0.0;
	}

	lineread(fd, uptime, 31);

	close(fd);
	free(dir);

	return atof(uptime);
}

/**
 * Formats the uptime in terms of years, days, hours, minutes and seconds.
 *
 * Parameters:
 * - time: the uptime in seconds.
 * - uptime_buf: the buffer to add the formatted uptime.
 *
 * Returns: 0 after formatting.
 **/
int pfs_format_uptime(double time, char *uptime_buf)
{
	int time_i = (int)time;

	int years = time_i / (365 * 24 * 3600);

	int days = time_i / (24 * 3600);
	time_i = time_i % (24 * 3600);

	int hours = time_i / 3600;
	time_i %= 3600;

	int mins = time_i / 60;
	time_i %= 60;

	int secs = time_i;

	int uptime_text = 0;

	if (years > 0)
	{
		uptime_text += sprintf(uptime_buf + uptime_text, "%d years, ", years);
	}
	if (days > 0)
	{
		uptime_text += sprintf(uptime_buf + uptime_text, "%d days, ", days);
	}
	if (hours > 0)
	{
		uptime_text += sprintf(uptime_buf + uptime_text, "%d hours, ", hours);
	}
	if (mins >= 0)
	{
		uptime_text += sprintf(uptime_buf + uptime_text, "%d minutes, ", mins);
	}
	if (secs >= 0)
	{
		uptime_text += sprintf(uptime_buf + uptime_text, "%d seconds", secs);
	}

	uptime_text = 0;

	return uptime_text;
}

/**
 * Stores the load average values into a loadavg struct.
 *
 * Parameters:
 * - proc_dir: the proc directory.
 *
 * Returns: the loadavg struct.
 **/
struct load_avg pfs_load_avg(char *proc_dir)
{
	// Set file path.
	char *dir = malloc(sizeof(char) * (strlen(proc_dir) + strlen("/loadavg") + 1));
	strcpy(dir, proc_dir);
	strcat(dir, "/loadavg");

	// Initialize struct.
	struct load_avg lavg = {0};

	char buf[64] = {0};

	int fd = open(dir, O_RDONLY);

	lineread(fd, buf, 63);

	char *buf_ptr = buf;

	for (size_t i = 0; i < 3; i++)
	{
		char *token = strsep(&buf_ptr, " ");

		if (token == NULL)
		{
			break;
		}

		if (i == 0)
		{
			lavg.one = atof(token);
		}
		else if (i == 1)
		{
			lavg.five = atof(token);
		}
		else if (i == 2)
		{
			lavg.fifteen = atof(token);
		}
	}

	close(fd);
	free(dir);

	return lavg;
}

/**
 * Calculates the CPU usage rate.
 *
 * Parameters:
 * - proc_dir: the proc directory.
 *
 * - prev: the previous CPU total and idle numbers.
 *
 * - curr: the current CPU total and idle numbers.
 *
 * Returns: the CPU usage number.
 **/
double pfs_cpu_usage(char *proc_dir, struct cpu_stats *prev, struct cpu_stats *curr)
{
	// Set file path.
	char *dir = malloc(sizeof(char) * (strlen(proc_dir) + strlen("/stat") + 1));
	strcpy(dir, proc_dir);
	strcat(dir, "/stat");

	int fd = open(dir, O_RDONLY);

	char buf[128] = {0};

	lineread(fd, buf, 127);

	char *buf_ptr = buf;


	curr->total = 0;
	curr->idle = 0;

	int count = 0;

	while (true)
	{
		char *token = strsep(&buf_ptr, " ");

		if (token == NULL)
		{
			break;
		}
		if (count == 5)
		{
			curr->idle = atol(token);
			curr->total += atol(token);
		}
		else if (count > 1)
		{
			curr->total += atol(token);
		}

		count++;
	}

	close(fd);
	free(dir);

	double usage = (double) 1 - ((double) (curr->idle - prev->idle) / (double) (curr->total - prev->total));

	if (isnan(usage) != 0 || (double) curr->idle - prev->idle < 0 || (double) curr->total - prev->total < 0)
	{
		// Return 0.0 if the calculation results in NaN or yielding negative values after reset.
		return 0.0;
	}

	return usage;
}

/**
 * Creates the memory usage struct.
 *
 * Parameters:
 * - proc_dir: the proc directory.
 *
 * Returns: the memory usage struct with updated values.
 **/
struct mem_stats pfs_mem_usage(char *proc_dir)
{
	// Initialize struct.
	struct mem_stats mstats = {0};

	char *dir = malloc(sizeof(char) * (strlen(proc_dir) + strlen("/meminfo") + 1));
	strcpy(dir, proc_dir);
	strcat(dir, "/meminfo");

	int fd = open(dir, O_RDONLY);

	double mem_total = 0;
	double mem_available = 0;

	while (true)
	{
		char buf[128] = {0};

		ssize_t read_sz = lineread(fd, buf, 127);

		if (read_sz <=0)
		{
			break;
		}

		if (strstr(buf, "MemTotal:") != NULL)
		{
			char *buf_ptr = buf;

			// Use next_token on the buf_ptr twice to retrieve the correct token.
			next_token(&buf_ptr, " ");

			mem_total = atof(next_token(&buf_ptr, " "));
		}

		if (strstr(buf, "MemAvailable:") != NULL)
		{
			char *buf_ptr = buf;

			// Use next_token on the buf_ptr twice to retrieve the correct token.
			next_token(&buf_ptr, " ");

			mem_available = atof(next_token(&buf_ptr, " "));
		}
	}

	// Calculate in gigabytes from kilobytes.
	mstats.total = mem_total / 1048576.00;
	mstats.used = (mem_total - mem_available) / 1048576.00;

	close(fd);
	free(dir);

	return mstats;
}

/**
 * Initializes, creates, and allocates memory for a task_stats struct along with its values.
 *
 * Returns: an initialized task_stats struct.
 **/
struct task_stats *pfs_create_tstats()
{
	// Allocate memory for task stats.
	struct task_stats *tstats = calloc(1, sizeof(struct task_stats));

	if (tstats == NULL)
	{
		free (tstats);
		return NULL;
	}
	else
	{
		tstats->active_tasks = calloc(1, sizeof(struct task_info));
	}

	return tstats;
}

/**
 * Removes all task stats.
 **/
void pfs_destroy_tstats(struct task_stats *tstats)
{
	free(tstats->active_tasks);
	free(tstats);
}


/**
 * Adds task info into the tast_stats struct.
 *
 * Parameters:
 * - proc_dir: the proc directory.
 *
 * - tstats: the task_stats struct.
 *
 * Returns: 1 when info is added to the task_stats struct, or 0 if info can't be added.
 **/
int pfs_tasks(char *proc_dir, struct task_stats *tstats)
{
	DIR *dir;

	if ((dir = opendir(proc_dir)) == NULL)
	{
		return 0;
	}

	struct dirent *file;

	int count = 0; // Count for active_tasks.

	while ((file = readdir(dir)) != NULL)
	{
		// Check if file type is directory and file name contains only digits.
		if (file->d_type == DT_DIR && dir_is_digits(file->d_name))
		{
			tstats->active_tasks = realloc(tstats->active_tasks, sizeof(struct task_info) * (count + 1)); // Reallocate memory for a new task.
			tstats->active_tasks[count].pid = atoi(file->d_name); // Set PID.

			// Set file path.
			char *path = malloc(sizeof(char) * (strlen(proc_dir) + strlen("/") + strlen(file->d_name) + strlen("/status") + 1));
			strcpy(path, proc_dir);
			strcat(path, "/");
			strcat(path, file->d_name); 
			strcat(path, "/status");

			int fd = open(path, O_RDONLY);

			while (true)
			{
				char buf[128] = {0};

				ssize_t	read_sz = lineread(fd, buf, 127);

				if (read_sz <= 0)
				{
					break;
				}

				if (strstr(buf, "Name:") != NULL)
				{
					// Set task name.
					char task_name[26];
					int index = strcspn(buf, ":");

					strcpy(task_name, &buf[index + 2]); // Get only the name from the line.
					strcpy(tstats->active_tasks[count].name, task_name);

					int length = strlen(tstats->active_tasks[count].name);

					if (length >= 25)
					{
						tstats->active_tasks[count].name[25] = '\0';
					}
					else
					{
						tstats->active_tasks[count].name[length + 1] = '\0';
					}
				}


				if (strstr(buf, "Uid:") != NULL)
				{
					// Set UID
					char uid_str[13];

					int index = strcspn(buf, ":");

					strcpy(uid_str, &buf[index + 2]);

					char *uid_ptr = uid_str;

					tstats->active_tasks[count].uid = atoi(strsep(&uid_ptr, "\t"));
				}

				if (strstr(buf, "State:") != NULL)
				{
					// Set state and don't increment active tasks if current task's state is sleeping.
					char task_state[13];
					int index = strcspn(buf, ":");

					strcpy(task_state, &buf[index + 2]);

					if (strstr(task_state, "running") != NULL)
					{
						tstats->running++;
						strcpy(tstats->active_tasks[count].state, "running");
						count++;
					}
					else if (strstr(task_state, "disk sleep") != NULL)
					{
						tstats->waiting++;
						strcpy(tstats->active_tasks[count].state, "disk sleep");
						count++;
					}
					else if (strstr(task_state, "sleeping") != NULL || strstr(task_state, "idle"))
					{
						tstats->sleeping++;
					}
					else if (strstr(task_state, "tracing stop") != NULL)
					{
						tstats->stopped++;
						strcpy(tstats->active_tasks[count].state, "tracing stop");
						count++;
					}
					else if (strstr(task_state, "stopped") != NULL)
					{
						tstats->stopped++;
						strcpy(tstats->active_tasks[count].state, "stopped");
						count++;
					}
					else if (strstr(task_state, "zombie") != NULL)
					{
						tstats->zombie++;
						strcpy(tstats->active_tasks[count].state, "zombie");
						count++;
					}
				}
			}

			tstats->total++;
			close(fd);
			free(path);
		}
	}

	closedir(dir);
	return 1;
}
