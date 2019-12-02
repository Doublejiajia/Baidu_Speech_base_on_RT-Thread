/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <dfs_posix.h>
#include <string.h>
#include <fal.h>
#include <drv_lcd.h>
#include <cn_font.h>
#include <board.h>

extern void show_str(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *str, uint8_t size);	
extern int wavrecord_sample();
extern void bd();

#define THREAD_PRIORITY			25
#define THREAD_STACK_SIZE		1024
#define THREAD_TIMESLICE		10

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static rt_thread_t tid3 = RT_NULL;

static rt_sem_t dynamic_sem = RT_NULL;

static struct rt_mailbox mb;
static char mb_pool[128];

/* ¼���߳� tid1 ��ں��� */
static void thread1_entry(void *parameter)
{
		static rt_err_t result;
    while(1)
    {
        /* ���÷�ʽ�ȴ��ź�������ȡ���ź�������ִ�� number �ԼӵĲ��� */
        result = rt_sem_take(dynamic_sem, RT_WAITING_FOREVER);
        if (result != RT_EOK)
        {
            rt_kprintf("t2 take a dynamic semaphore, failed.\n");
            rt_sem_delete(dynamic_sem);
            return;
        }
        else
        {
					rt_kprintf("t2 take a dynamic semaphore, success.\n");
           wavrecord_sample();
					 rt_mb_send(&mb, NULL);
        }
				rt_thread_mdelay(100);
    }
		
}


/* ����ʶ���߳� tid2 ��ں��� */
static void thread2_entry(void *parameter)
{

    while (1)
    {
        rt_kprintf("thread1: try to recv a mail\n");

        /* ����������ȡ�ʼ� */
        if (rt_mb_recv(&mb, NULL, RT_WAITING_FOREVER) == RT_EOK)
        {
						show_str(20, 40, 200, 32, (rt_uint8_t *)"�ٶ�����ʶ��", 32);
						show_str(20, 100, 200, 32, (rt_uint8_t *)"ʶ������", 32);
            rt_kprintf("get a mail from mailbox!");
            bd();
            rt_thread_mdelay(100);
        }
    }
    /* ִ������������� */
    rt_mb_detach(&mb);
}

/* �����߳� tid3 ��ں��� */
static void thread3_entry(void *parameter)
{
		unsigned int count = 1;
		while(count > 0)
		{
			if(rt_pin_read(KEY0) == 0)
			{
					rt_kprintf("release a dynamic semaphore.\n");
          rt_sem_release(dynamic_sem);
			}
			rt_thread_mdelay(100);
		}
}

int main(void)
{
	
		fal_init();
		rt_pin_mode(KEY0, PIN_MODE_INPUT);
		rt_pin_mode(PIN_LED_R, PIN_MODE_OUTPUT);
		rt_pin_mode(PIN_LED_G, PIN_MODE_OUTPUT);
		rt_pin_mode(PIN_LED_B, PIN_MODE_OUTPUT);
		rt_pin_write(PIN_LED_R,1);
		rt_pin_write(PIN_LED_G,1);
		rt_pin_write(PIN_LED_B,1);
		
    /* ���� */
    lcd_clear(WHITE);

    /* ���ñ���ɫ��ǰ��ɫ */
    lcd_set_color(WHITE,BLACK);

    /* ��LCD ����ʾ�ַ� */
    lcd_show_string(55, 5, 24, "RT-Thread");
		
		show_str(120, 220, 200, 16, (rt_uint8_t *)"By ����������", 16);
	
		dynamic_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_FIFO);
    if (dynamic_sem == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create done. dynamic semaphore value = 0.\n");
    }
	
		rt_err_t result;

    /* ��ʼ��һ�� mailbox */
    result = rt_mb_init(&mb,
                        "mbt",                      /* ������ mbt */
                        &mb_pool[0],                /* �����õ����ڴ���� mb_pool */
                        sizeof(mb_pool) / 4,        /* �����е��ʼ���Ŀ����Ϊһ���ʼ�ռ 4 �ֽ� */
                        RT_IPC_FLAG_FIFO);          /* ���� FIFO ��ʽ�����̵߳ȴ� */
    if (result != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    }
		
		tid1 = rt_thread_create("thread1",
                            thread1_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);
		
		
		tid2 = rt_thread_create("thread2",
                            thread2_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);
		
		tid3 = rt_thread_create("thread3",
                            thread3_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid3 != RT_NULL)
        rt_thread_startup(tid3);
	
    return 0;
}

