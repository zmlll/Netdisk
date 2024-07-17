#include <func.h>

int sendfd(int sockfd,int fdtosend,int exitflag)
{
    struct msghdr hdr;
    bzero(&hdr,sizeof(hdr));
    struct iovec iov[1];
    iov[0].iov_base = &exitflag;
    iov[0].iov_len = sizeof(int);
    hdr.msg_iov = iov;
    hdr.msg_iovlen = 1;
    struct cmsghdr* pcmsghdr;
    pcmsghdr = (struct cmsghdr*)calloc(1,CMSG_LEN(sizeof(int)));
    pcmsghdr->cmsg_len = CMSG_LEN(sizeof(int));
    pcmsghdr->cmsg_level = SOL_SOCKET;
    pcmsghdr->cmsg_type = SCM_RIGHTS;
    *(int*)CMSG_DATA(pcmsghdr) = fdtosend;
    hdr.msg_control = pcmsghdr;
    hdr.msg_controllen = CMSG_LEN(sizeof(int));
    int ret = sendmsg(sockfd,&hdr,0);
    ERROR_CHECK(ret,-1,"sendmsg");
    return 0;
}

int recvfd(int sockfd,int *pfdtorecv,int* pexitflag)
{
    struct msghdr hdr;
    bzero(&hdr,sizeof(hdr));
    struct iovec iov[1];
    iov[0].iov_base = pexitflag;
    iov[0].iov_len = sizeof(int);
    hdr.msg_iov = iov;
    hdr.msg_iovlen = 1;
    struct cmsghdr* pcmsghdr;
    pcmsghdr = (struct cmsghdr*)calloc(1,CMSG_LEN(sizeof(int)));
    pcmsghdr->cmsg_len = CMSG_LEN(sizeof(int));
    pcmsghdr->cmsg_level = SOL_SOCKET;
    pcmsghdr->cmsg_type = SCM_RIGHTS;
    hdr.msg_control = pcmsghdr;
    hdr.msg_controllen = CMSG_LEN(sizeof(int));
    int ret = recvmsg(sockfd,&hdr,0);
    *pfdtorecv = *(int*)CMSG_DATA(pcmsghdr);
    printf("*pexitflag = %d\n",*pexitflag);
    printf("*pfdtorecv = %d\n",*pfdtorecv);
    return 0;
}
