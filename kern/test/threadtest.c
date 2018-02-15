/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Thread test code.
 */
#include <types.h>
#include <lib.h>
#include <thread.h>
#include <synch.h>
#include <test.h>


static struct semaphore *tsem = NULL;
static struct semaphore *csem = NULL;
static struct lock *cloc = 0;
static struct spinlock sloc;
volatile int counter = 0;

static
void
inits(void)
{
	if (tsem==NULL) {
		tsem = sem_create("tsem", 0);
		if (tsem == NULL) {
			panic("threadtest: sem_create failed\n");
		}
	}
	if (csem==NULL) {
		csem = sem_create("csem", 1);
		if (csem == NULL) {
			panic("threadtest: sem_create failed\n");
		}
	}
	if (cloc == NULL){
			cloc = lock_create("cloc");
			if (cloc == NULL){
				panic("threadtest: lock_create failed\n");
			}
	}
	if (&sloc == NULL){
		spinlock_init(&sloc);
		if (&sloc == NULL){
				panic("threadtest: spinlock_init failed\n");
		}
	}
}

static
int
toInt(char *args){
	int r = 0;
	for (int i = 0; args[i] != '\0'; i++){
		r = r*10+(int)(args[i] - '0');
	}
	return r;
}

static
void
loudthread(void *junk, unsigned long num)
{
	(void)junk;
	P(csem);
	kprintf("This is thread num: %d\n", (int)num);
	V(csem);
	V(tsem);
}


static
void
runthreads(int nargs, char **args)
{
	int nthreads = args[1][0] - '0';
	char name[16];
	int i, result;
	(void)nargs;

	for (i=0; i<nthreads; i++) {
		snprintf(name, sizeof(name), "threadtest%d", i);
		result = thread_fork(name, NULL,
				     loudthread,
				     args, i);
		if (result) {
			panic("threadtest: thread_fork failed %s)\n", 
			      strerror(result));
		}
	}

	for (i=0; i<nthreads; i++) {
		P(tsem);
	}
}


int
threadtest(int nargs, char **args)
{
	inits();
	kprintf("Starting thread test...\n");
	if (nargs == 2){
		runthreads(nargs, args);
		kprintf("\nThread test done.\n");
	}else{ kprintf("Usage: tt1 [number of threads]\n"); }

	return 0;
}

int
threadtest2(int nargs, char **args)
{
	(void)nargs;
	(void)args;

	inits();
	kprintf("No longer implemented...\n");

	return 0;
}


static
void
utc(void *junk, unsigned long perthread){
	(void)junk;
	for(int i = (int)perthread; i > 0; i--){
		counter++;
	}
	V(tsem);
}


int 
unsafethreadcounter(int nargs, char **args)
{
	counter = 0;
	int tds, ptd;
	int i, result;
	char name[16];
	inits();
	kprintf("Starting unsafethreadcounter...\n");
	if (nargs == 3){
		tds = toInt(args[1]);
		ptd = toInt(args[2]);
	}else if (nargs == 2){
		tds = toInt(args[1]);
		ptd = 10;
	}else{ kprintf("Usage: utc [threads] [perthread] \n"); return 0;}
	kprintf("\nRunning w/\n	threads: %d\n	perthread: %d\n", tds, ptd);
	
		for (i=0; i<tds; i++) {
			snprintf(name, sizeof(name), "threadtest%d", i);
			result = thread_fork(name, NULL,
					     utc,
					     args, ptd);//args is garbage, counter is global
			if (result) {
				panic("threadtest: thread_fork failed %s)\n", 
					      strerror(result));
			}
		}
	
	kprintf("\nThread test done.\n");
	kprintf("\nCounter intended value: %d\n", tds*ptd);

	for (i=0; i<tds; i++) {
		P(tsem);
	}
	kprintf("\nCounter actual value: %d\n", (int)counter);
	return 0;
}

static
void
ltc(void *junk, unsigned long perthread){
	(void)junk;
	for(int i = (int)perthread; i > 0; i--){
		lock_acquire(cloc);
		counter++;
		lock_release(cloc);
	}
	V(tsem);
}


int lockthreadcounter(int nargs, char **args)
{
	counter = 0;
	int tds, ptd;
	int i, result;
	char name[16];
	inits();
	kprintf("Starting lockthreadcounter...\n");
	if (nargs == 3){
		tds = toInt(args[1]);
		ptd = toInt(args[2]);
	}else if (nargs == 2){
		tds = toInt(args[1]);
		ptd = 10;
	}else{ kprintf("Usage: utc [threads] [perthread] \n"); return 0;}

	kprintf("\nRunning w/\n	threads: %d\n	perthread: %d\n", tds, ptd);
	
		for (i=0; i<tds; i++) {
			snprintf(name, sizeof(name), "threadtest%d", i);
			result = thread_fork(name, NULL,
					     ltc,
					     args, ptd);//args is garbage, counter is global
			if (result) {
				panic("threadtest: thread_fork failed %s)\n", 
					      strerror(result));
			}
		}
	
	kprintf("\nThread test done.\n");
	kprintf("\nCounter intended value: %d\n", tds*ptd);


	for (i=0; i<tds; i++) {
		P(tsem);
	}
	kprintf("\nCounter actual value: %d\n", (int)counter);
	return 0;
}

static
void
stc(void *junk, unsigned long perthread){
	(void)junk;
	for(int i = (int)perthread; i > 0; i--){
		spinlock_acquire(&sloc);
		counter++;
		spinlock_release(&sloc);
	}
	V(tsem);
}

int spinlockthreadcounter(int nargs, char **args)
{
	counter = 0;
	int tds, ptd;
	int i, result;
	char name[16];
	inits();
	kprintf("Starting spinlockthreadcounter...\n");
	if (nargs == 3){
		tds = toInt(args[1]);
		ptd = toInt(args[2]);
	}else if (nargs == 2){
		tds = toInt(args[1]);
		ptd = 10;
	}else{ kprintf("Usage: utc [threads] [perthread] \n"); return 0;}

	kprintf("\nRunning w/\n	threads: %d\n	perthread: %d\n", tds, ptd);
	
		for (i=0; i<tds; i++) {
			snprintf(name, sizeof(name), "threadtest%d", i);
			result = thread_fork(name, NULL,
					     stc,
					     args, ptd);//args is garbage, counter is global
			if (result) {
				panic("threadtest: thread_fork failed %s)\n", 
					      strerror(result));
			}
		}
	
	kprintf("\nThread test done.\n");
	kprintf("\nCounter intended value: %d\n", tds*ptd);


	for (i=0; i<tds; i++) {
		P(tsem);
	}
	kprintf("\nCounter actual value: %d\n", (int)counter);
	return 0;
}
