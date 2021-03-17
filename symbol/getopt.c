#include "libgotcha/libgotcha_api.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int getopts(int argc, char *const *argv, const char *optstring) {
	static libgotcha_group_t group = 0;
	static char **optarg1;
	static int *optind1, *opterr1, *optopt1;
	if(!group) {
		group = libgotcha_group_new();
		assert(group);

		optarg1 = libgotcha_group_symbol(group, "optarg");
		optind1 = libgotcha_group_symbol(group, "optind");
		opterr1 = libgotcha_group_symbol(group, "opterr");
		optopt1 = libgotcha_group_symbol(group, "optopt");

		assert(optarg1);
		assert(optind1);
		assert(opterr1);
		assert(optopt1);

		assert(optarg1 != &optarg);
		assert(optind1 != &optind);
		assert(opterr1 != &opterr);
		assert(optopt1 != &optopt);

		opterr = 0;
		*opterr1 = 0;
	}

	char *argv1[argc];
	for(int arg = 0; arg < argc; ++arg)
		argv1[arg] = argv[arg];

	assert(optarg == *optarg1);
	assert(optind == *optind1);
	assert(opterr == *opterr1);
	assert(optopt == *optopt1);

	int res = getopt(argc, argv, optstring);
	libgotcha_group_thread_set(group);

	int res1 = getopt(argc, argv1, optstring);
	libgotcha_group_thread_set(LIBGOTCHA_GROUP_SHARED);

	assert(res == res1);
	assert(optarg == *optarg1);
	assert(optind == *optind1);
	assert(opterr == *opterr1);
	assert(optopt == *optopt1);

	return res;
}

int main(int argc, char **argv) {
	if(argc == 1) {
		fprintf(stderr, "%s: missing optstring argument\n", argv[0]);
		return 1;
	}

	char optstring[strlen(argv[1]) + 2];
	sprintf(optstring, ":%s", argv[1]);
	argv[1] = argv[0];
	++argv;
	--argc;

	int option;
	while((option = getopts(argc, argv, optstring)) != -1)
		switch(option) {
		case '?':
			fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], optopt);
			break;
		case ':':
			fprintf(stderr, "%s: option requires an argument -- '%c'\n", argv[0], optopt);
			break;
		default:
			if(optarg)
				assert(optarg == argv[optind - 1]);
		}

	for(char **arg = argv + 1; arg < argv + optind; ++arg)
		printf(" %s", *arg);
	fputs(" --", stdout);
	for(char **arg = argv + optind; arg < argv + argc; ++arg)
		printf(" %s", *arg);
	putchar('\n');

	return 0;
}
