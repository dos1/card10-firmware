#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "epicardium.h"
#include "modules/log.h"
#include "modules/stream.h"
#include "modules/mutex.h"

/* Internal buffer of registered streams */
static struct stream_info *stream_table[SD_MAX];

/* Lock for modifying the stream info table */
static struct mutex stream_table_lock;

int stream_init()
{
	memset(stream_table, 0x00, sizeof(stream_table));
	mutex_create(&stream_table_lock);
	return 0;
}

int stream_register(int sd, struct stream_info *stream)
{
	int ret = 0;

	mutex_lock(&stream_table_lock);

	if (sd < 0 || sd >= SD_MAX) {
		ret = -EINVAL;
		goto out;
	}

	if (stream_table[sd] != NULL) {
		/* Stream already registered */
		ret = -EACCES;
		goto out;
	}

	stream_table[sd] = stream;

out:
	mutex_unlock(&stream_table_lock);
	return ret;
}

int stream_deregister(int sd, struct stream_info *stream)
{
	int ret = 0;

	mutex_lock(&stream_table_lock);

	if (sd < 0 || sd >= SD_MAX) {
		ret = -EINVAL;
		goto out;
	}

	if (stream_table[sd] != stream) {
		/* Stream registered by someone else */
		ret = -EACCES;
		goto out;
	}

	stream_table[sd] = NULL;

out:
	mutex_unlock(&stream_table_lock);
	return ret;
}

int epic_stream_read(int sd, void *buf, size_t count)
{
	int ret = 0;
	/*
	 * TODO: In theory, multiple reads on different streams can happen
	 * simulaneously.  I don't know what the most efficient implementation
	 * of this would look like.
	 */
	mutex_lock(&stream_table_lock);

	if (sd < 0 || sd >= SD_MAX) {
		ret = -EBADF;
		goto out;
	}

	struct stream_info *stream = stream_table[sd];
	if (stream == NULL) {
		ret = -ENODEV;
		goto out;
	}

	/* Poll the stream, if a poll_stream function exists */
	if (stream->poll_stream != NULL) {
		ret = stream->poll_stream();
		if (ret < 0) {
			goto out;
		}
	}

	/* Check buffer size is a multiple of the data packet size */
	if (count % stream->item_size != 0) {
		ret = -EINVAL;
		goto out;
	}

	size_t i;
	for (i = 0; i < count; i += stream->item_size) {
		if (!xQueueReceive(stream->queue, buf + i, 0)) {
			break;
		}
	}

	ret = i / stream->item_size;

out:
	mutex_unlock(&stream_table_lock);
	return ret;
}
