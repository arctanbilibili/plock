

//用户空间的锁结构体
struct lock:
    state;//该锁是否已拿到，默认0
    fd;//进程内打开的/dev/plock文件描述符
    time;//超时时间
    clean_func;//业务自定义的清理函数指针
    poll_process_handler;//对应poll线程的句柄




acquire(lock):
    fd = open("/dev/plock",O_RDWR);

    ret = ioctl(要锁, time);//要锁命令
    if(ret < 0)
        close(fd);
        return "没拿到锁";

    //要锁成功则创建线程
    poll_process_handler = thread_create(poll_process, {clean_func, fd});//传入clean_func回调函数和fd给poll线程
    state = 1;


release(lock):
    if(!state)
        return;
    ret = ioctl(还锁, -1);//要锁命令
    pthread_cancel(poll_process_handler);//删除poll线程
    close(fd);//要最后再关fd，因为poll函数会用到fd



//业务AAA用锁的进程
main():
    acquire(lock);

    yewu();//业务函数

    release(lock);





//业务AAA专门处理poll的线程
poll_process({clean_func,fd}):
    fds = init(fd);//用主进程open的fd初始化fds

    for(;;)
        ret = poll(&fds[0],1,NULL);
        if(fds[0].revent | POLLMSG)
            fds[0].revent=0;
            clean_func();//让出锁，执行业务自定义的回调函数
            ioctl("还锁", para);
            state=0;//防止release里继续释放锁
            close(fd);
            pthread_cancel(poll_process_handler);//删除poll线程





