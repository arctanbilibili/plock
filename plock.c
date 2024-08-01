plock.ko
//暂时只讨论一个锁的情况

mutex_lock;//
wait_queue_head[N];//该锁相关的N条队列，对应N个优先级，注意这里装的都是poll线程
plock_count=0;//默认计数0, 当前占着锁的进程数
plock_count_max=2;//最大计数2
queue_prio_max=0;
queue_prio_min=999;


service_wait_queue_head[...];//该锁的业务线程等待队列，专门存放等待slot空槽的业务线程



drv_poll:
    mask = 0;
    current_task_prio = get_curr_prio();//获取当前业务所需的优先级

    poll_wait(&wait_queue_head[current_task_prio]);//加到进程对应优先限级的那一条队列

    //如果该poll线程接收到need_free信号，poll返回mask
    if(need_free)
        need_free = 0;
        mask |= POLLMSG;
    
    return mask;



drv_ioctl(cmd, para):
    current_task_prio = get_curr_prio();//获取当前业务所需的优先级

    if(cmd=="要锁")
        time_out = para;//参数是time_out

        if(plock_count < plock_count_max)//有空位，直接占
            plock_count++;
            update(queue_prio_max, queue_prio_min);
        else
            //暂时简化优先级相等的处理情况
            if( current_task_prio <= queue_prio_min )//优先级小于队列内所有业务，阻塞
                wait_event_timeout(service_wait_queue_head, \
                    plock_count < plock_count_max, time_out);//空槽标志位slot_cond动态调整，会在每次plock_count变更时更新（特别是有业务释放时）
                //注：空槽条件plock_count 会在ioctl("还锁", para)时候更新

            //poll机制告知低优先级的进程
            else
                need_free = 1;
                wake_up(wait_queue_head[queue_prio_min]);//传送信号给任意一个最低优先级的任务
                wait_event_timeout(service_wait_queue_head, \
                    plock_count < plock_count_max, time_out);//空槽标志位slot_cond动态调整，会在每次plock_count变更时更新（特别是有业务释放时）

    else if(cmd=="还锁")
        plock_count--;
        wake_up(service_wait_queue_head);//一个业务已释放，唤醒任意一个业务


        





