/*
 * idevicecrashlog.c
 *
 *  Created on: May 1, 2012
 *      Author: benoit
 */

#include <stdio.h>
#include <errno.h>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/file_relay.h>

#include <archive.h>

#include <libgen.h>
#include <string.h>

static void print_usage(int argc, char **argv)
{
      char *name = NULL;
      name = strrchr(argv[0], '/');
      printf("Usage: %s [crashlog]\n", (name ? name + 1: argv[0]));
}

int extract_file(char* dumpFile, char* file) {
	struct archive_entry *entry;
	int r;

	struct archive *a = archive_read_new();
	archive_read_support_compression_gzip(a);
	archive_read_support_format_cpio(a);
	FILE *dump = fopen(dumpFile, "r");
	r = archive_read_open_FILE(a, dump);
	if (r != ARCHIVE_OK) {
		fprintf(stderr, "Fail to read cpio file");
		fclose(dump);
		return -1;
	}
	fclose(dump);

	FILE *outputFile = fopen(basename(file), "w");
	int notfound = 1;
	while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
		if(strcmp(basename(archive_entry_pathname(entry)), file) == 0) {
			notfound = 0;
			printf("extract %s\n",archive_entry_pathname(entry));
			archive_read_data_into_fd(a, fileno(outputFile));
		}
	}
	fclose(outputFile);
	archive_read_free(a);
	if(notfound) {
		sprintf(stderr, "The crash log %s was not found\n", file);
		return -1;
	}
	return 1;
}

int main(int argc, char **argv) {
	idevice_t phone = NULL;
	lockdownd_client_t client = NULL;
    uint16_t port = 0;

    char* crashLogFile;

    if(argc != 2) {
    	print_usage(argc, argv);
    	return -1;
    }
    crashLogFile = argv[1];

	if (idevice_new(&phone, NULL) != IDEVICE_E_SUCCESS) {
		printf("No device found, is it plugged in?\n");
		return -1;
	}

	if (lockdownd_client_new_with_handshake(phone, &client, "idevicecrashlog") !=  LOCKDOWN_E_SUCCESS) {
		fprintf(stderr, "Could not connect to lockdownd. Exiting.\n");
		return -1;
	}

	if ((lockdownd_start_service(client, "com.apple.mobile.file_relay", &port) !=
		 LOCKDOWN_E_SUCCESS) || !port) {
		fprintf(stderr, "Could not start com.apple.mobile.file_relay!\n");
		return -1;
	}

	const char *sources[] = {"CrashReporter", NULL};
	idevice_connection_t dump = NULL;
	file_relay_client_t frc = NULL;

	if (file_relay_client_new(phone, port, &frc) != FILE_RELAY_E_SUCCESS) {
		printf("could not connect to file_relay service!\n");
		return -1;
	}

	if (file_relay_request_sources(frc, sources, &dump) != FILE_RELAY_E_SUCCESS) {
		printf("could not get sources\n");
		return -1;
	}

	if (!dump) {
		printf("did not get connection!\n");
		return -1;
	}


	uint32_t cnt = 0;
	uint32_t len = 0;
	char buf[4096];
	char* dumpTmpFile = tmpnam(NULL);
	FILE *f = fopen(dumpTmpFile, "w");
	//receiving file
	while (idevice_connection_receive(dump, buf, 4096, &len) == IDEVICE_E_SUCCESS) {
		fwrite(buf, 1, len, f);
		cnt += len;
		len = 0;
	}
	fclose(f);

	extract_file(dumpTmpFile, crashLogFile);

	lockdownd_client_free(client);
}


