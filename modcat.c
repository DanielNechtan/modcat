/*
 * Copyright (c) 2018 int16h <int16h@openbsd.space>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#include <sndio.h>

#define BUFLEN 2048

static int verbose;

typedef struct
{
	char *name;		/* 22 bytes */
	char length;		/* 2 bytes */
	int truelength;		/* truelength (in bytes) = length*2 */
	char finetune;		/* 1 byte - lower 4 bits = 0x00-0x0F / upper 4 bits: 0 */
	char volume;		/* 1 byte: 0x00-0x40 / 0-64 */
	int rptStart;		/* 2 bytes */
	int rptLength;		/* 2 bytes */

} modcat_sample;

typedef struct
{
	char *mdata;
	char *title;
	char *format;
	char tunelength;
	char maxpos;
	char n_samples;
	char n_patterns;
	char n_channels;
	char *poslist;
	char *patterns;
	int msize;
} modcat_mod;

int
load_samples(modcat_sample *sample, char *md)
{
	int i, n_samples;
	n_samples = 0;
	char *bp = md+20;
        for(i = 1; i < 32; i++) {
		sample = malloc(sizeof(*sample));

		sample[i].name = malloc(23);
		strlcpy(sample[i].name, bp, 22);

		sample[i].length = bp[22];

		if(sample[i].length > 0) {
			printf("Sample %d: %s | length:%d\n",
					i,
					sample[i].name,
					sample[i].length);
			n_samples++;
	         }
	         bp += 30;
        }
/*        mod->n_samples = n_samples; */
/*	printf("Samples: %d", mod->n_samples); */
	return n_samples;
}

int
init_mod(modcat_mod *mod, modcat_sample *smp, char *md, int mdsz)
{
	int i;

	mod->title = malloc(21);
	strlcpy(mod->title, md, 20);
 	if(verbose)
		printf("Title: %s\n", mod->title);
	mod->n_channels = 4;
	if(verbose)
		printf("Channels: %d\n", mod->n_channels);
	mod->tunelength = md[950];
	if(verbose)
		printf("Length: %d\n", mod->tunelength);
	mod->maxpos = md[951];
	if (verbose)
		printf("MaxPos: %d\n", mod->maxpos);
	mod->poslist = &md[952];
	mod->format = malloc(5);
	strlcpy(mod->format, md+1080, 5);
	if(verbose)
		printf("Format: %s\n", mod->format);
	mod->patterns = &md[1084];

	mod->n_samples = load_samples(smp, md);
	if(verbose)
		printf("Samples: %d\n", mod->n_samples);
	return 1;
}

int
main(int argc, char *argv [])
{
	modcat_mod *module;
	module = malloc(sizeof(*module));
	modcat_sample *sample[31];
/*	sample[31] = malloc(sizeof(*sample[31])); */
	FILE *modfile;
	char *moddata;
	int datasz;
	struct sio_hdl *hdl;
	struct sio_par par;

	verbose = 0;

	if (argc < 2 || argc > 3)
		errx(1, "Usage: %s <file.mod>", argv[0]);
	if ((modfile = fopen(argv[1], "rb")) == NULL)
		err(1, "Could not open file");
	if (argc == 3)
		if(!strncmp(argv[2], "-v", 3))
			verbose=1;
	fseek(modfile, 0, SEEK_END);
	datasz = ftell(modfile);
	rewind(modfile);
	if(verbose)
		printf("Module is %d bytes\n", datasz);
	if((moddata = malloc(datasz)) == NULL)
		err(1, "malloc error");
	if(!fread(moddata, datasz, 1, modfile)) {
		free(moddata);
		fclose(modfile);
		errx(1, "Couldn't read %d bytes into memory", datasz);
	}
	fclose(modfile);

	/* parse MOD data */
/*	memset(&module, '\0', sizeof(modcat_mod));*/
	if(!init_mod(module, sample, moddata, datasz))
		err(1, "init_mod failed!");
/*
	if(!load_samples(module, sample, moddata, datasz))
		err(1, "load_samples failed!");
*/
	if(verbose) {
//		printf("samples: %d", module->n_samples);
//		printf("Sample 0: %s", sample[1]->name);
	}
	/* tidy up for a clean exit() */
	free(moddata);
	free(module->title);
	free(module->format);
	return(1);
}
