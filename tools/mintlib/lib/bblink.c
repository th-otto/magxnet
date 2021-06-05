/*
 * block profile support for gcc.
 *	++jrb	bammi@cadence.com
 */
#include <compiler.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* block count struct produced by gcc -a */
struct bb
{
	long initialized;					/* has __bb_init_func been called */
	const char *filename;				/* filename for .d file       */
	long *counts;						/* address of block count table   */
	long ncounts;						/* sizeof block count table       */
	/* ie: # of basic blocks in file  */
	struct bb *next;					/* in memory link to next struct  */
	const unsigned long *addresses;		/* addr of basic block address table */
	/* size of address table == ncounts+1 */

	/* Older GCC's did not emit these fields.  */
	long nwords;
	const char **functions;
	const long *line_nums;
	const char **filenames;
};

void __bb_init_func __PROTO((struct bb *));
static void exit_func __PROTO((void));	/* installed to be called at exit */

#ifdef OLD
static void save_info __PROTO((struct bb *));
#endif

/* vars */
static struct bb *bb_head = NULL;		/* list of all bb_count for which 
										   __bb_init_func has been called */
static char first_call = 1;				/* flags first call to __bb_init_func */
static char at_exit_failed = 0;			/* flag to indicate that atexit() failed */

/*
 * called by gcc -a generated code on first entry into a function
 */
void __bb_init_func(bb_count)
struct bb *bb_count;
{
	if (at_exit_failed)
		return;

	if (first_call)
	{
		if (atexit(exit_func))
		{
			fprintf(stderr, "Failed to install exit function. No block \
profile information will be saved\n");
			at_exit_failed = 1;
			return;
		}
		first_call = 0;
	}

	if (bb_count->initialized == 0)
	{									/* link into list of bb_counts */
		bb_count->next = bb_head;
		bb_head = bb_count;
		bb_count->initialized = 1;
	}
}

#ifdef OLD
/*
 * called on normal exit
 *   write out block profile files for each bb_count struct in list.
 */
static void exit_func()
{
	struct bb *p;

	for (p = bb_head; p; p = p->next)
		save_info(p);
}

typedef struct
{
	long lineno;						/* start of block */
	long count;							/* # executions (cumulative over runs)  */
} DINFO;

/*
 * given a bb_count struct, save info into .d file
 */
static void save_info(p)
struct bb *p;
{
	FILE *fp;
	long i,
	*bcounts;
	DINFO *dinfo = malloc(p->ncounts * sizeof(DINFO));

	if (!dinfo)
	{
		fprintf(stderr, "No memory to process %s. Skipped\n", p->filename);
		return;
	}

	if ((fp = fopen(p->filename, "r")) == NULL)
	{
		fprintf(stderr, "Failed to open %s for read. Skipped\n", p->filename);
		free(dinfo);
		return;
	}
	/* read .d file & accumulate counts */
	for (i = 0, bcounts = p->counts; fscanf(fp, "%ld%ld", &dinfo[i].lineno, &dinfo[i].count) == 2; i++)
	{
		if (i >= p->ncounts)
		{
			fprintf(stderr, "Block counts in %s exceed expected %ld, rest skipped\n", p->filename, p->ncounts);
			break;
		}
		dinfo[i].count += bcounts[i];
	}
	fclose(fp);
	if (i < p->ncounts)
	{
		fprintf(stderr, "Warning Block counts in %s less than expected %ld\n", p->filename, p->ncounts);
	}
	if ((fp = fopen(p->filename, "w")) == NULL)
	{
		fprintf(stderr, "Failed to open %s for write. Skipped\n", p->filename);
		free(dinfo);
		return;
	}
	for (i = 0; i < p->ncounts; i++)
	{
		if (fprintf(fp, "\t%ld\t%ld\n", dinfo[i].lineno, dinfo[i].count) == EOF)
		{
			fprintf(stderr, "Write Failed to %s\n", p->filename);
			free(dinfo);
			fclose(fp);
			return;
		}
	}
	fclose(fp);
	free(dinfo);
}

#else /* !OLD */

/* from GCC's libgcc2.c */

/* Return the number of digits needed to print a value */
/* __inline__ */ static int num_digits(long value, int base)
{
	int minus = (value < 0 && base != 16);
	unsigned long v = (minus) ? -value : value;
	int ret = minus;

	do
	{
		v /= base;
		ret++;
	}
	while (v);

	return ret;
}

static void exit_func(void)
{
	FILE *file = fopen("bb.out", "a");
	time_t time_value;

	if (!file)
		perror("bb.out");

	else
	{
		struct bb *ptr;

		time(&time_value);
		fprintf(file, "Basic block profiling finished on %s\n", ctime(&time_value));

		/* We check the length field explicitly in order to allow compatibility
		   with older GCC's which did not provide it.  */

		for (ptr = bb_head; ptr != (struct bb *) NULL; ptr = ptr->next)
		{
			int i;
			int func_p = (ptr->nwords >= sizeof(struct bb) && ptr->nwords <= 1000);
			int line_p = (func_p && ptr->line_nums);
			int file_p = (func_p && ptr->filenames);
			long ncounts = ptr->ncounts;
			long cnt_max = 0;
			long line_max = 0;
			long addr_max = 0;
			int file_len = 0;
			int func_len = 0;
			int blk_len = num_digits(ncounts, 10);
			int cnt_len;
			int line_len;
			int addr_len;

			fprintf(file, "File %s, %ld basic blocks \n\n", ptr->filename, ncounts);

			/* Get max values for each field.  */
			for (i = 0; i < ncounts; i++)
			{
				const char *p;
				int len;

				if (cnt_max < ptr->counts[i])
					cnt_max = ptr->counts[i];

				if (addr_max < ptr->addresses[i])
					addr_max = ptr->addresses[i];

				if (line_p && line_max < ptr->line_nums[i])
					line_max = ptr->line_nums[i];

				if (func_p)
				{
					p = (ptr->functions[i]) ? (ptr->functions[i]) : "<none>";
					len = (int)strlen(p);
					if (func_len < len)
						func_len = len;
				}

				if (file_p)
				{
					p = (ptr->filenames[i]) ? (ptr->filenames[i]) : "<none>";
					len = (int)strlen(p);
					if (file_len < len)
						file_len = len;
				}
			}

			addr_len = num_digits(addr_max, 16);
			cnt_len = num_digits(cnt_max, 10);
			line_len = num_digits(line_max, 10);

			/* Now print out the basic block information.  */
			for (i = 0; i < ncounts; i++)
			{
				fprintf(file,
						"    Block #%*d: executed %*ld time(s) address= 0x%.*lx",
						blk_len, i + 1, cnt_len, ptr->counts[i], addr_len, ptr->addresses[i]);

				if (func_p)
					fprintf(file, " function= %-*s", func_len, (ptr->functions[i]) ? ptr->functions[i] : "<none>");

				if (line_p)
					fprintf(file, " line= %*ld", line_len, ptr->line_nums[i]);

				if (file_p)
					fprintf(file, " file= %s", (ptr->filenames[i]) ? ptr->filenames[i] : "<none>");

				fprintf(file, "\n");
			}

			fprintf(file, "\n");
			fflush(file);
		}

		fprintf(file, "\n\n");
		fclose(file);
	}
}
#endif /* OLD */
