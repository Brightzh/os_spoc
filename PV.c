#include <stdio.h>
#include <proc.h>
#include <sem.h>
#include <monitor.h>
#include <assert.h>

#define N 2 /* number of process */
#define SLEEP 100

int x,y,z;
//---------- PV error problem using semaphore ----------------------

semaphore_t s1,s2,s3; /* semaphores control the order */

int semaproc_1(void* arg)
{
        int no = (int)arg;
        if(no == 1)
        {
                cprintf("s:Process1 first!\n");
                y = 1;
                y = y + 2;
                up(&s1);
                z = y + 1;
                down(&s2);
                y = z + y;
                up(&s3);
                return 0;
        }
        else
        {
                cprintf("s:Process2 second!\n");
                y = 1;
                y = y + 2;
                up(&s1);
                z = y + 1;
                down(&s2);
                down(&s3);
                y = z + y;
                return 0;
        }
}

int semaproc_2(void* arg)
{
        int no = (int)arg;
        if(no == 1)
        {
                cprintf("s:Process2 second!\n");
                x = 1;
                x = x + 1;
                down(&s1);
                x = x + y;
                up(&s2);
                down(&s3);
                z = x + z;
                return 0;
        }
        else
        {
                cprintf("s:Process2 first!\n");
                x = 1;
                x = x + 1;
                down(&s1);
                x = x + y;
                up(&s2);
                z = x + z;
                up(&s3);
                return 0;
        }
}

//---------- PV error problem using monitor ----------------------
monitor_t mt, *mtp=&mt;
int other_finished[3];

int moniproc_1(void* arg)
{
        int no = (int)arg;
        if(no == 1)
        {
                cprintf("m:Process1 first!\n");
                y = 1;
                y = y + 2;

                down(&mtp->mutex);
                other_finished[0] = 1;
                cond_signal(&mtp->cv[1]);
                if(mtp->next_count > 0)
                {
                        up(&mtp->next);
                }
                else
                {
                        up(&mtp->mutex);
                }

                z = y + 1;

                down(&mtp->mutex);
                while(!other_finished[1])
                        cond_wait(&mtp->cv[0]);
                if(mtp->next_count > 0)
                {
                        up(&mtp->next);
                }
                else
                {
                        up(&mtp->mutex);
                }

                y = z + y;

                down(&mtp->mutex);
                other_finished[2] = 1;
                cond_signal(&mtp->cv[1]);
                if(mtp->next_count > 0)
                {
                        up(&mtp->next);
                }
                else
                {
                        up(&mtp->mutex);
                }

                return 0;
        }
        else
        {
                cprintf("m:Process2 second!\n");
                y = 1;
                y = y + 2;

                down(&mtp->mutex);
                other_finished[0] = 1;
                cond_signal(&mtp->cv[1]);
                if(mtp->next_count > 0)
                {
                        up(&mtp->next);
                }
                else
                {
                        up(&mtp->mutex);
                }

                z = y + 1;

                down(&mtp->mutex);
                while(!other_finished[1])
                        cond_wait(&mtp->cv[0]);
                if(mtp->next_count > 0)
                {
                        up(&mtp->next);
                }
                else
                {
                        up(&mtp->mutex);
                }

                down(&mtp->mutex);
                while(!other_finished[2])
                        cond_wait(&mtp->cv[0]);
                if(mtp->next_count > 0)
                {
                        up(&mtp->next);
                }
                else
                {
                        up(&mtp->mutex);
                }

                y = z + y;
                return 0;
        }
}

int moniproc_2(void* arg)
{
        int no = (int)arg;
        if(no == 1)
        {
                cprintf("m:Process2 second!\n");
                x = 1;
                x = x + 1;

                down(&mtp->mutex);
                while(!other_finished[0])
                {
                        cond_wait(&mtp->cv[1]);
                }
                if(mtp->next_count > 0)
                {
                        up(&mtp->next);
                }
                else
                {
                        up(&mtp->mutex);
                }

                x = x + y;

                down(&mtp->mutex);
                other_finished[1] = 1;
                cond_signal(&mtp->cv[0]);
                if(mtp->next_count > 0)
                {
                        up(&mtp->next);
                }
                else
                {
                        up(&mtp->mutex);
                }

                down(&mtp->mutex);
                while(!other_finished[2])
                {
                        cond_wait(&mtp->cv[1]);
                }
                if(mtp->next_count > 0)
                {
                        up(&mtp->next);
                }
                else
                {
                        up(&mtp->mutex);
                }
                cprintf("there\n");
                z = x + z;

                return 0;
        }
        else
        {
                cprintf("m:Process2 first!\n");
                x = 1;
                x = x + 1;

                down(&mtp->mutex);
                while(!other_finished[0])
                {
                        cond_wait(&mtp->cv[1]);
                }
                if(mtp->next_count > 0)
                {
                        up(&mtp->next);
                }
                else
                {
                        up(&mtp->mutex);
                }

                x = x + y;

                down(&mtp->mutex);
                other_finished[1] = 1;
                cond_signal(&mtp->cv[0]);
                if(mtp->next_count > 0)
                {
                        up(&mtp->next);
                }
                else
                {
                        up(&mtp->mutex);
                }

                z = x + z;

                down(&mtp->mutex);
                other_finished[2] = 1;
                cond_signal(&mtp->cv[0]);
                if(mtp->next_count > 0)
                {
                        up(&mtp->next);
                }
                else
                {
                        up(&mtp->mutex);
                }

                return 0;
        }
}

void show_value(void)
{
        cprintf("x=%d, y=%d, z=%d.\n", x, y, z);
}

void check_sync(void)
{
        int pid;
        int i;
        //semephore: 1 first.
        x = y = z = 0;
        sem_init(&s1, 0);
        sem_init(&s2, 0);
        sem_init(&s3, 0);
        pid = kernel_thread(semaproc_1, (void*)1, 0);
        cprintf("Process1 in!\n");
        pid = kernel_thread(semaproc_2, (void*)1, 0);
        cprintf("Process2 in!\n");
        do_sleep(SLEEP);
        show_value();

        //semephore: 2 first.
        x = y = z = 0;
        sem_init(&s1, 0);
        sem_init(&s2, 0);
        sem_init(&s3, 0);
        pid = kernel_thread(semaproc_1, (void*)2, 0);
        cprintf("Process1 in!\n");
        pid = kernel_thread(semaproc_2, (void*)2, 0);
        cprintf("Process2 in!\n");
        do_sleep(SLEEP);
        show_value();

        //monitor: 1 first.
        x = y = z = 0;
        monitor_init(&mt, N);
        for(i = 0; i < 3 ; i ++)
                other_finished[i] = 0;
        cprintf("Process1 in!\n");
        pid = kernel_thread(moniproc_1, (void*)1, 0);
        cprintf("Process2 in!\n");
        pid = kernel_thread(moniproc_2, (void*)1, 0);
        do_sleep(SLEEP);
        show_value();

        //monitor: 2 first.
        x = y = z = 0;
        monitor_init(&mt, N);
        for(i = 0; i < 3 ; i ++)
                other_finished[i] = 0;
        cprintf("Process1 in!\n");
        pid = kernel_thread(moniproc_1, (void*)2, 0);
        cprintf("Process2 in!\n");
        pid = kernel_thread(moniproc_2, (void*)2, 0);
        do_sleep(SLEEP);
        show_value();
}
