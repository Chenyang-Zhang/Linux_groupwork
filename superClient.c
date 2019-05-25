#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#define BUF_SIZE (8192)

unsigned char fileBuf[BUF_SIZE];

//file_receive
void *recv_file(int skfd, const char *path)
{
    FILE *fp = NULL;
    struct sockaddr_in sockAddr;
    unsigned int fileSize, fileSize2;
    int size, nodeSize;


    size = read(skfd, (unsigned char *)&fileSize, 4);
    if( size != 4 ) {
        printf("file size error!\n");
        exit(-1);
    }
    printf("file size:%d\n", fileSize);

    if( (size = write(skfd, "OK", 2) ) < 0 ) {
        perror("write");
        exit(1);
    }

    fp = fopen(path, "w+");
    if( fp == NULL ) {
        perror("fopen");
        return NULL;
    }

    fileSize2 = 0;
    while(memset(fileBuf, 0, sizeof(fileBuf)), (size = read(skfd, fileBuf, sizeof(fileBuf))) > 0) {
        unsigned int size2 = 0;
        while( size2 < size ) {
            if( (nodeSize = fwrite(fileBuf + size2, 1, size - size2, fp) ) < 0 ) {
                perror("write");
                exit(1);
            }
            size2 += nodeSize;
        }
        fileSize2 += size;
        if(fileSize2 >= fileSize) {
            break;
        }
    }
    fclose(fp);
    printf("file transfer success\n");
    return NULL;
}

void send_file(int cnfd, const char *path)
{

    FILE *fp;
    unsigned int fileSize;
    int size, netSize;
    char buf[10];
    char sendbuf[1024]={0};


    if( !path ) {
        printf("file server: file path error!\n");
        return;

   }
    fp=fopen(path,"r");
    if( fp == NULL ) {
        perror("fopen");
        return;
    }

    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if(write(cnfd, (unsigned char *)&fileSize, 4) != 4) {
        perror("write");
        exit(1);
    }

    if( read(cnfd, buf, 2) != 2) {
        perror("read");
        exit(1);
    }

    while( ( size = fread(fileBuf, 1, BUF_SIZE, fp) ) > 0 ) {
        unsigned int size2 = 0;
        while( size2 < size ) {
            if( (netSize = write(cnfd, fileBuf + size2, size - size2) ) < 0 ) {
                perror("write");
                exit(1);
            }
            size2 += netSize;
        }
    }

    fclose(fp);
}
                   

//接受服务器的信息
void *recvMessage(void *arg)
{
	int fd=*(int *)arg;
	int ret=0;
	char recvbuf[1024];
	char filename[1024]="file_receive";
	
	while(1)
	{
		memset(recvbuf,0,sizeof(recvbuf));
		if((ret=recv(fd,recvbuf,sizeof(recvbuf),0))==-1)
		{
			return NULL;
		}
		if(strcmp(recvbuf,"file_receive")==0)
		{
			recv_file(fd,filename);
		}
		if(strcmp(recvbuf,"file_send")==0)
		{
			recv(fd,recvbuf,sizeof(recvbuf),0);
			send_file(fd,recvbuf);
		}


		if(strcmp(recvbuf,"xiazai")==0)
		{
			system("rm -f mesg.txt");//确保本地没有此文件
			int fp;		//文件标识符
			fp=open("mesg.txt",O_WRONLY|O_CREAT|O_TRUNC,0666);
			memset(recvbuf,0,1024);
			//接收数据，如果“endend”表示接收结束
			while(1)		
			{
				recv(fd,&ret,sizeof(int),0);
				recv(fd,recvbuf,ret,0);
				if(strncmp(recvbuf,"endend",6)==0)
				{
					break;
				}
				write(fp,recvbuf,ret);
 
			}
			close(fp); 
			continue;
			
		}
		if(strcmp(recvbuf,"连接成功")==0)
		{
			system("clear");
		}
		puts(recvbuf);	
		if(ret==0)
		{
			exit(0);
		}
    }
	
}
void *sendMessage(void *arg)
{
	//发送
	int fd=*(int *)arg;
	char sendmsg[1024];
	while(1)
	{
			memset(sendmsg,0,sizeof(sendmsg));
			scanf("%s",sendmsg);
			if(send(fd,sendmsg,strlen(sendmsg),0)==-1)
			{
				return NULL; 	
			}
		
	}	
}

/*void send_file(int cnfd, const char*path)
{
	FILE *fp = NULL;
	unsigned int fileSize;
	int size, netSize;

	fp = fopen(path, "r");
	if( fp == NULL ) {
		perror("fopen");
		return;
	}

	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if(write(cnfd, (unsigned char *)&fileSize, 4) != 4) {
		perror("write");
		exit(1);
    }
	if( read(cnfd, buf, 2) != 2) {
		perror("read");
		exit(1);
	}
	
	while( ( size = fread(fileBuf, 1, BUF_SIZE, fp) ) > 0 ) {
		unsigned int size2 = 0;
	       	while( size2 < size ) {
			if( (netSize = write(cnfd, fileBuf + size2, size - size2) ) < 0 ) {
				perror("write");
				exit(1);
            }
            size2 += netSize;
        }
    }

    fclose(fp);
}*/


int main()
{
        int sockfd=0;
	int ret=0;
	int len=sizeof(struct sockaddr);
	
	struct sockaddr_in otheraddr;
	memset(&otheraddr,0,len);	
	//tcp套接字连接
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("sockfd");
		return -1;	
	}
	//初始化结构体，把服务器ip地址和端口号
	otheraddr.sin_family = AF_INET;
	otheraddr.sin_port = htons(8889);
	otheraddr.sin_addr.s_addr=inet_addr("0.0.0.0");
	//连接服务器
	if(connect(sockfd,(struct sockaddr*)&otheraddr,len)==-1)
	{
		perror("connect");
		return -1;		
	}
	printf("connect success...client fd=%d\n",sockfd);
	printf("client ip=%s,port=%d\n",inet_ntoa(otheraddr.sin_addr),ntohs(otheraddr.sin_port));		
         

	pthread_t id1,id2;

	char recvbuf[1024]={0};
        char sendbuf[1024]={0};
    
    //给服务器 发送信息
        strcpy(sendbuf,"hi,I am client");	

	if(send(sockfd,sendbuf,strlen(sendbuf),0)==-1)
	{
		perror("send");
		return -1;
	}
	if(recv(sockfd,recvbuf,sizeof(recvbuf),0)==-1)
	{
		perror("recv");
		return -1;
	}
	printf("sever say:%s\n",recvbuf);
	
	//创建收发线程
	pthread_create(&id1,NULL,sendMessage,&sockfd);
	pthread_create(&id2,NULL,recvMessage,&sockfd);

	
	//等待发送线程结束
	pthread_join(id1,NULL);
    return 0;
}




































