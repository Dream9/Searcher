/*************************************************************************
	> File Name: rio.cpp
	> Author: Dream9
	> Brief:健壮的read/write
	> Created Time: 2019年05月07日 星期五 16时15分43秒
 ************************************************************************/
#include"rio.h"

static ssize_t _rio_read(rio_t *, void *, size_t);
static const int kRIO_DEFAULT_TIMEOUT=30;


//初始化rio_t结构
void Rio_init(rio_t*rp, int fd) {
	rp->rio_fd = fd;
	rp->rio_cnt = 0;
	rp->rio_bufptr = rp->rio_buf;
}

//brief:带缓冲的读
ssize_t RioReadnb(rio_t*rp, void *usrbuf, size_t n) {
	size_t nleft = n;
	ssize_t nread;
	char *bufp = (char*)usrbuf;

	while (nleft > 0) {
		//if((nread=rio_read(rp,bufp,nleft)<0))
		//这里的优先级考虑错了，
		//改正如下
		nread = _rio_read(rp, (void*)bufp, nleft);
		if (nread < 0)
			return -1;   //errno set by read()
		else if (nread == 0)
			break;  /*EOF*/
		nleft -= nread;
		bufp += nread;
	}
	return (n - nleft);
}

//brief:带缓冲的按行读
ssize_t RioReadlineb(rio_t*rp, void *usrbuf, size_t maxlen) {
	ssize_t rc;
	size_t n;
	char c, *bufp = (char*)usrbuf;
	for (n = 1; n < maxlen; n++) {
		if ((rc = _rio_read(rp, &c, 1)) == 1)
		{
			*bufp++ = c;
			if (c == '\n') {
				++n;
				break;
			}
		}
		else if (rc == 0) {
			if (n == 1)
				return 0;
			else
				break;
		}
		else
			return -1;
	}
	*bufp = '\0';
	return n - 1;
}

//brief:无缓冲的写
//return:写入个数
//允许中断
ssize_t RioWrite(int fd, void *usrbuf, size_t n) {
	size_t nleft = n;
	ssize_t nwritten;
	char *bufp = (char*)usrbuf;

	while (nleft > 0) {
		if ((nwritten = write(fd, bufp, nleft)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;
			else {
				return -1;
			}
		}
		nleft -= nwritten;
		bufp += nwritten;
	}
	return n - nleft;
}

//brief:无缓冲的读
//return：读出个数
//允许中断
ssize_t RioRead(int fd, void *usrbuf, size_t n) {
	size_t nleft = n;
	ssize_t nread;
	char *bufp = (char*)usrbuf;

	while (nleft > 0) {
		if ((nread = read(fd, bufp, nleft)) < 0) {
			if (errno == EINTR) {
				nread = 0;
			}
			else {
				return -1;
			}
		}
		else if (nread == 0) {
			//读到EOF
			break;
		}
		nleft -= nread;
		bufp += nread;
	}
	return n - nleft;
}


//brief:非阻塞版本的I/O读,有默认超时时间
//return:读取个数或者错误信息，-3超时，-2描述符属性修改失败，-1其他错误
ssize_t RioRead_nonb(int fd, void *usrbuf, size_t n, int sec=kRIO_DEFAULT_TIMEOUT){
    ssize_t nleft=n;
	timeval tv;
	fd_set fdset_read;
	int flags;
	int flags_old;
	int nread;
	char *pos_buf = (char*)usrbuf;

	if((flags_old=fcntl(fd,F_GETFL,0))<0){
		return -2;
	}

	flags= flags_old | O_NONBLOCK;//保证非阻塞模式

	if(fcntl(fd,F_SETFL,flags)<0){
		return -2;
	}

	while(nleft){
		//每次重置
		//因为只有一个描述符轮询，因此返回值如果成功只能为1
		FD_ZERO(&fdset_read);
		FD_SET(fd,&fdset_read);
		tv.tv_sec=sec;
		tv.tv_usec=0;
		nread=select(fd+1,&fdset_read,nullptr,nullptr,&tv);

		if(nread!=1){
			if(errno==EINTR){
				continue;
			}
			//区分一下超时,
			return nread==0?-3:-1;
		}
		nread=read(fd,pos_buf,nleft);
		pos_buf+=nread;
		nleft-=nread;
	}

	if(fcntl(fd,F_SETFL,flags_old)<0){
		return -2;
	}

	return n-nleft;

}



//brief:带有buf的读操作最终调用函数，健壮的写入
//允许中断
static ssize_t _rio_read(rio_t *rp, void *usrbuf, size_t n) {
	int cnt;
	while (rp->rio_cnt <= 0) {
		//装填缓冲区
		rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
		if (rp->rio_cnt < 0) {
			if (errno != EINTR) /*Interrupted by sig handler return*/
				return -1;
			//其他情况下，重现陷入内核read
		}
		else if (rp->rio_cnt == 0)
			return 0;  /*EOF*/
		else
			rp->rio_bufptr = rp->rio_buf;
	}
	cnt = rp->rio_cnt < n ? rp->rio_cnt : n;
	memcpy(usrbuf, rp->rio_bufptr, cnt);
	rp->rio_cnt -= cnt;
	rp->rio_bufptr += cnt;
	return cnt;
}
