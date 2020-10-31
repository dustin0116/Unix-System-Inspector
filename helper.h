/**
 * @file
 *
 * Helper functions for inspector processes.
 */

#ifndef _HELPER_H_
#define _HELPER_H_

#include <sys/types.h>
#include <stdbool.h>

ssize_t lineread(int fd, char *buf, size_t sz);

char *next_token(char **str_ptr, const char *delim);

bool dir_is_digits(char *dir_name);

#endif
