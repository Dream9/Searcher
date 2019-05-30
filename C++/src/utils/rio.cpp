/*************************************************************************
	> File Name: rio.cpp
	> Author: Dream9
	> Brief:��׳��read/write
	> Created Time: 2019��05��07�� ������ 16ʱ15��43��
 ************************************************************************/
#include"rio.h"

static ssize_t _rio_read(rio_t *, void *, size_t);
static const int kRIO_DEFAULT_TIMEOUT=30;


//��ʼ��rio_t�ṹ
void Rio_init(rio_t*rp, int fd) {
	rp->rio_fd = fd;
	rp->rio_cnt = 0;
	rp->rio_bufptr = rp->rio_buf;
}

//brief:������Ķ�
ssize_t RioReadnb(rio_t*rp, void *usrbuf, size_t n) {
	size_t nleft = n;
	ssize_t nread;
	char *bufp = (char*)usrbuf;

	while (nleft > 0) {
		//if((nread=rio_read(rp,bufp,nleft)<0))
		//��������ȼ����Ǵ��ˣ�
		//��������
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

//brief:������İ��ж�
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

//brief:�޻����д
//return:д�����
//�����ж�
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

//brief:�޻���Ķ�
//return����������
//�����ж�
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
			//����EOF
			break;
		}
		nleft -= nread;
		bufp += nread;
	}
	return n - nleft;
}


//brief:�������汾��I/O��,��Ĭ�ϳ�ʱʱ��
//return:��ȡ�������ߴ�����Ϣ��-3��ʱ��-2�����������޸�ʧ�ܣ�-1��������
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

	flags= flags_old | O_NONBLOCK;//��֤������ģʽ

	if(fcntl(fd,F_SETFL,flags)<0){
		return -2;
	}

	while(nleft){
		//ÿ������
		//��Ϊֻ��һ����������ѯ����˷���ֵ����ɹ�ֻ��Ϊ1
		FD_ZERO(&fdset_read);
		FD_SET(fd,&fdset_read);
		tv.tv_sec=sec;
		tv.tv_usec=0;
		nread=select(fd+1,&fdset_read,nullptr,nullptr,&tv);

		if(nread!=1){
			if(errno==EINTR){
				continue;
			}
			//����һ�³�ʱ,
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



//brief:����buf�Ķ��������յ��ú�������׳��д��
//�����ж�
static ssize_t _rio_read(rio_t *rp, void *usrbuf, size_t n) {
	int cnt;
	while (rp->rio_cnt <= 0) {
		//װ�����
		rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
		if (rp->rio_cnt < 0) {
			if (errno != EINTR) /*Interrupted by sig handler return*/
				return -1;
			//��������£����������ں�read
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
