#include "modules/log.h"
#include "modules/config.h"
#include "modules/filesystem.h"
#include "epicardium.h"

#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

#define MAX_LINE_LENGTH 80
#define KEYS_PER_BLOCK 16
#define KEY_LENGTH 16
#define NOT_INT_MAGIC 0x80000000

// one key-value pair representing a line in the config
typedef struct {
	char key[KEY_LENGTH];

	// the value in the config file, if it's an integer.
	// for strings it's set to NOT_INT_MAGIC
	int value;

	// the byte offset in the config file to read the value string
	size_t value_offset;
} config_slot;

// a block of 16 config slots
// if more are needed, this becomes a linked list
typedef struct {
	config_slot slots[KEYS_PER_BLOCK];
	void *next;
} config_block;

static config_block *config_data = NULL;

// returns the config slot for a key name
static config_slot *find_config_slot(const char *key)
{
	config_block *current = config_data;

	while (current) {
		for (int i = 0; i < KEYS_PER_BLOCK; i++) {
			config_slot *k = &current->slots[i];

			if (strcmp(k->key, key) == 0) {
				// found what we're looking for
				return k;

			} else if (*k->key == '\0') {
				// found the first empty key
				return NULL;
			}
		}
		current = current->next;
	}

	return NULL;
}

// returns the next available config slot, or allocates a new block if needed
static config_slot *allocate_config_slot()
{
	config_block *current;

	if (config_data == NULL) {
		config_data = malloc(sizeof(config_block));
		assert(config_data != NULL);
		memset(config_data, 0, sizeof(config_block));
	}

	current = config_data;

	while (true) {
		for (int i = 0; i < KEYS_PER_BLOCK; i++) {
			config_slot *k = &current->slots[i];
			if (*k->key == '\0') {
				return k;
			}
		}

		// this block is full and there's no next allocated block
		if (current->next == NULL) {
			current->next = malloc(sizeof(config_block));
			assert(current->next != NULL);
			memset(current->next, 0, sizeof(config_block));
		}
		current = current->next;
	}
}

// parses an int out of 'value' or returns NOT_INT_MAGIC
static int try_parse_int(const char *value)
{
	char *endptr;
	size_t len = strlen(value);
	int v      = strtol(value, &endptr, 0);

	if (endptr != (value + len)) {
		return NOT_INT_MAGIC;
	}

	return v;
}

// loads a key/value pair into a new config slot
static void add_config_pair(
	const char *key, const char *value, int line_number, size_t value_offset
) {
	if (strlen(key) > KEY_LENGTH - 1) {
		LOG_WARN(
			"card10.cfg",
			"line:%d: too long - aborting",
			line_number
		);
		return;
	}

	config_slot *slot = allocate_config_slot();
	strncpy(slot->key, key, KEY_LENGTH);
	slot->value        = try_parse_int(value);
	slot->value_offset = value_offset;
}

// parses one line of the config file
static void
parse_line(char *line, char *eol, int line_number, size_t line_offset)
{
	char *line_start = line;

	//skip leading whitespace
	while (*line && isspace((int)*line))
		++line;

	char *key = line;
	if (*key == '#') {
		//skip comments
		return;
	}

	char *eq = strchr(line, '=');
	if (!eq) {
		if (*key) {
			LOG_WARN(
				"card10.cfg",
				"line %d: syntax error",
				line_number
			);
		}
		return;
	}

	char *e_key = eq - 1;
	//skip trailing whitespace in key
	while (e_key > key && isspace((int)*e_key))
		--e_key;
	e_key[1] = '\0';
	if (*key == '\0') {
		LOG_WARN("card10.cfg", "line %d: empty key", line_number);
		return;
	}

	char *value = eq + 1;
	//skip leading whitespace
	while (*value && isspace((int)*value))
		++value;

	char *e_val = eol - 1;
	//skip trailing whitespace
	while (e_val > value && isspace((int)*e_val))
		--e_val;
	if (*value == '\0') {
		LOG_WARN(
			"card10.cfg",
			"line %d: empty value for option '%s'",
			line_number,
			key
		);
		return;
	}

	size_t value_offset = value - line_start + line_offset;

	add_config_pair(key, value, line_number, value_offset);
}

// convert windows line endings to unix line endings.
// we don't care about the extra empty lines
static void convert_crlf_to_lflf(char *buf, int n)
{
	while (n--) {
		if (*buf == '\r') {
			*buf = '\n';
		}
		buf++;
	}
}

// parses the entire config file
void load_config(void)
{
	LOG_DEBUG("card10.cfg", "loading...");
	int fd = epic_file_open("card10.cfg", "r");
	if (fd < 0) {
		LOG_DEBUG(
			"card10.cfg",
			"loading failed: %s (%d)",
			strerror(-fd),
			fd
		);
		return;
	}
	char buf[MAX_LINE_LENGTH + 1];
	int line_number    = 0;
	size_t file_offset = 0;
	int nread;
	do {
		nread = epic_file_read(fd, buf, MAX_LINE_LENGTH);
		convert_crlf_to_lflf(buf, nread);
		if (nread < MAX_LINE_LENGTH) {
			//add fake EOL to ensure termination
			buf[nread++] = '\n';
		}
		//zero-terminate buffer
		buf[nread]   = '\0';
		char *line   = buf;
		char *eol    = NULL;
		int last_eol = 0;
		while (line) {
			//line points one character past the last (if any) '\n' hence '- 1'
			last_eol = line - buf - 1;
			eol      = strchr(line, '\n');
			++line_number;
			if (eol) {
				*eol = '\0';
				parse_line(line, eol, line_number, file_offset);
				file_offset += eol - line + 1;
				line = eol + 1;
				continue;
			}
			if (line == buf) {
				//line did not fit into buf
				LOG_WARN(
					"card10.cfg",
					"line:%d: too long - aborting",
					line_number
				);
				return;
			}
			int seek_back = last_eol - nread;

			LOG_DEBUG(
				"card10.cfg",
				"nread, last_eol, seek_back: %d,%d,%d",
				nread,
				last_eol,
				seek_back
			);
			assert(seek_back <= 0);
			if (!seek_back) {
				break;
			}

			int rc = epic_file_seek(fd, seek_back, SEEK_CUR);
			if (rc < 0) {
				LOG_ERR("card10.cfg", "seek failed, aborting");
				return;
			}
			char newline;
			rc = epic_file_read(fd, &newline, 1);
			if (rc < 0 || (newline != '\n' && newline != '\r')) {
				LOG_ERR("card10.cfg", "seek failed, aborting");
				LOG_DEBUG(
					"card10.cfg",
					"seek failed at read-back of newline: rc: %d read: %d",
					rc,
					(int)newline
				);
				return;
			}
			break;
		}
	} while (nread == MAX_LINE_LENGTH);
	epic_file_close(fd);
}

// opens the config file, seeks to seek_offset and reads buf_len bytes
// used for reading strings without storing them in memory
// since we don't need to optimize for that use case as much
static size_t read_config_offset(size_t seek_offset, char *buf, size_t buf_len)
{
	int fd = epic_file_open("card10.cfg", "r");
	if (fd < 0) {
		LOG_DEBUG(
			"card10.cfg",
			"opening config failed: %s (%d)",
			strerror(-fd),
			fd
		);
		return 0;
	}

	int rc = epic_file_seek(fd, seek_offset, SEEK_SET);
	if (rc < 0) {
		LOG_ERR("card10.cfg", "seek failed, aborting");
		return 0;
	}

	// one byte less to accommodate the 0 termination
	int nread = epic_file_read(fd, buf, buf_len - 1);

	buf[nread] = '\0';

	epic_file_close(fd);

	return nread;
}

// returns error if not found or invalid
int epic_config_get_integer(const char *key, int *value)
{
	config_slot *slot = find_config_slot(key);
	if (slot && slot->value != NOT_INT_MAGIC) {
		*value = slot->value;
		return 0;
	}
	return -ENOENT;
}

// returns default_value if not found or invalid
int config_get_integer_with_default(const char *key, int default_value)
{
	int value;
	int ret = epic_config_get_integer(key, &value);
	if (ret) {
		return default_value;
	} else {
		return value;
	}
}

// returns error if not found
int epic_config_get_string(const char *key, char *buf, size_t buf_len)
{
	config_slot *slot = find_config_slot(key);
	if (!(slot && slot->value_offset)) {
		return -ENOENT;
	}

	size_t nread = read_config_offset(slot->value_offset, buf, buf_len);
	if (nread == 0) {
		return -ENOENT;
	}

	char *eol = strchr(buf, '\n');
	if (eol) {
		*eol = '\0';
	}

	return 0;
}

// returns dflt if not found, otherwise same pointer as buf
char *config_get_string_with_default(
	const char *key, char *buf, size_t buf_len, char *dflt
) {
	int ret = epic_config_get_string(key, buf, buf_len);
	if (ret) {
		return dflt;
	} else {
		return buf;
	}
}

// returns error if not found or invalid
int epic_config_get_boolean(const char *key, bool *value)
{
	int int_value;
	int ret = epic_config_get_integer(key, &int_value);

	if (ret == 0) {
		*value = !!int_value;
		return 0;
	}

	char buf[MAX_LINE_LENGTH + 1];
	epic_config_get_string(key, buf, MAX_LINE_LENGTH);

	if (buf == NULL) {
		return -ENOENT;
	}

	if (!strcmp(buf, "true")) {
		*value = true;
		return 0;
	} else if (!strcmp(buf, "false")) {
		*value = false;
		return 0;
	}

	return -ERANGE;
}

// returns default_value if not found or invalid
bool config_get_boolean_with_default(const char *key, bool default_value)
{
	bool value;
	int ret = epic_config_get_boolean(key, &value);
	if (ret) {
		return default_value;
	} else {
		return value;
	}
}
