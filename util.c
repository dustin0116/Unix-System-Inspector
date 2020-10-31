#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "debug.h"
#include "util.h"
#include "helper.h"


/**
 * Draws the percentage bar.
 *
 * Parameters:
 * - buf: the buffer to add the percentage bar.
 *
 * - frac: the number to calculate in percentage.
 **/
void draw_percbar(char *buf, double frac)
{
	double perc = frac * 100; // Percentage.

	if (isnan(perc) || perc<= 0.0)
	{
		// Set to 0.0 if not a number or yielding a negative value.
		perc = 0.0;
	}

	if (perc >= 100.0)
	{
		// Always set 100% if greater than or equal to 100.0.
		perc = 100.0;
	}
	double inc =  perc / 5; // Get value by increments of 5 to draw out either '#' or '-'.

	for (size_t i = 0; i < 28; i++)
	{
		// Set up the bar style.
		if (i == 0)
		{
			buf[i] = '[';
		}
		else if (i == 21)
		{
			buf[i] = ']';
		}
		else if (i == 22)
		{
			buf[i] = ' ';
		}
	
		else {
			// Set up the bar status.
			if (inc >= 0.89)
			{
				buf[i] = '#';
				inc--;
			}
			else
			{
				buf[i] = '-';
			}
		}
	}

	// Add percentage to buffer.
	sprintf(&buf[23], "%.1f", perc);
	strcat(buf, "%");
}

/**
 * Retrieves the user from the task's UID.
 *
 * Parameters:
 * - name_buf: the buffer to add the user.
 *
 * - uid: the UID.
 **/
void uid_to_uname(char *name_buf, uid_t uid)
{
	char buf[128] = {0};

	char user[32] = {0};

	int fd = open("/etc/passwd", O_RDONLY);

	if (fd == -1)
	{
		return;
	}

	sprintf(name_buf,"%x", uid); // Store UID into the buffer in case no user is found.

	while (true)
	{
		ssize_t read_sz = lineread(fd, buf, 127);

		if (read_sz <= 0)
		{
			return;
		}

		char *uid_ptr = buf;

		strcpy(user, strsep(&uid_ptr, ":")); // Store username.

		strsep(&uid_ptr, ":"); // Tokenize to the number.

		int found_uid = atol(uid_ptr); // Convert to long int so it fits.

		if (uid == found_uid)
		{
			// UID matches.
			strcpy(name_buf, user);
			name_buf[15] = '\0';
		}
	}
	close(fd);
}
