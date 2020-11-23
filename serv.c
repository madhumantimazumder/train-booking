#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>


#define PORT 8090
#define PASS_LENGTH 20
#define TRAIN "./db/train"
#define BOOKING "./db/booking"

struct account{
	int id;
	char name[10];
	char pass[PASS_LENGTH];
};

struct train{
	int tid;
	char train_name[20];
	int train_no;
	int total_seats;
	int booked_seats;
	//int av_seats;
	//int last_seatno_used;
};

struct bookings{
	int bid;
	int type;
	int acc_no;
	int tr_id;
	char trainname[20];
	int seat_booked;
	int cancelled;
};

char *ACC[3] = {"./db/accounts/customer", "./db/accounts/agent", "./db/accounts/admin"};

void service_cli(int sock);
int login(int sock);
int signup(int sock);
int menu2(int sock, int id);
int menu1(int sock, int id, int type);
int view_booking(int sock, int id, int type);
void viewAllBooking(int sock);
void viewAllTrain(int sock);
void viewAllUser(int sock);

int main(){
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd==-1) {
		printf("socket creation failed\n");
		exit(0);
	}
	int optval = 1;
	int optlen = sizeof(optval);
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, optlen)==-1){
		printf("set socket options failed\n");
		exit(0);
	}
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(PORT);

	if(bind(sockfd, (struct sockaddr *)&sa, sizeof(sa))==-1){
		printf("binding port failed\n");
		exit(0);
	}
	if(listen(sockfd, 100)==-1){
		printf("listen failed\n");
		exit(0);
	}
	while(1){ 
		int connectedfd;
		if((connectedfd = accept(sockfd, (struct sockaddr *)NULL, NULL))==-1){
			printf("connection error\n");
			exit(0);
		}
		pthread_t cli;
		if(fork()==0)
			service_cli(connectedfd);
	}

	close(sockfd);
	return 0;
}

void service_cli(int sock){
	int func_id;
	read(sock, &func_id, sizeof(int));
	printf("Client [%d] connected\n", sock);
	while(1){		
		if(func_id==1) {login(sock);exit(1);/* {printf("Client disconnected\n");exit(1);}*//*read(sock, &func_id, sizeof(int));*/}
		if(func_id==2) {signup(sock);exit(1);read(sock, &func_id, sizeof(int));}
		if(func_id==3) break;
	}
	close(sock);
	printf("Client [%d] disconnected\n", sock);
}

int login(int sock){
	int type, acc_no, fd, valid=1, invalid=0, login_success=0;
	char password[PASS_LENGTH];
	struct account temp;
	read(sock, &type, sizeof(type));
	read(sock, &acc_no, sizeof(acc_no));
	read(sock, &password, sizeof(password));

	if((fd = open(ACC[type-1], O_RDWR))==-1)printf("File Error\n");
	struct flock lock;
	
	lock.l_start = (acc_no-1)*sizeof(struct account);
	lock.l_len = sizeof(struct account);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	if(type == 1){
		lock.l_type = F_WRLCK;
		fcntl(fd,F_SETLKW, &lock);
		lseek(fd, (acc_no - 1)*sizeof(struct account), SEEK_SET);
		read(fd, &temp, sizeof(struct account));
		//lseek(fd, 0, SEEK_SET);
		//printf("%s\t%s\n",temp.pass,password);
		//printf("%d\t%d\n",temp.id,acc_no);
		//printf("%d\t%d\n", strlen(temp.pass), strlen(password));
		int x = temp.id;
		if( x== acc_no){
			printf("test");
			if(!strcmp(temp.pass, password)){

				write(sock, &valid, sizeof(valid));
				while(-1!=menu1(sock, temp.id, type));
				login_success = 1;
				
			}
		}
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);
		if(login_success)
		return 3;
	}
	else if(type == 2){
		printf("%d",type);
		read(fd, &temp, sizeof(struct account));
		if(temp.id == acc_no){
			if(!strcmp(temp.pass, password)){
				write(sock, &valid, sizeof(valid));
				while(-1!=menu1(sock, temp.id, type));
				return 3;
			}
		}	
	}
	else if(type == 3){
		lock.l_type = F_WRLCK;
		fcntl(fd,F_SETLKW, &lock);
		lseek(fd, (acc_no - 1)*sizeof(struct account), SEEK_SET);
		read(fd, &temp, sizeof(struct account));
		printf("%s\t%s\n",temp.pass,password);
		printf("%d\t%d\n",temp.id,acc_no);
		if(temp.id == acc_no){
			if(!strcmp(temp.pass, password)){
				printf("%s\t%s\n",temp.pass,password);
				write(sock, &valid, sizeof(valid));
				while(-1!=menu2(sock, temp.id));
				login_success = 1;
			}
		}
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);
		if(login_success)
		return 3;
	}
	write(sock, &invalid, sizeof(invalid));
	return -1;
}

int signup(int sock){
	int type, fd, acc_no=0,ff;
	char  name[10];
	char password[20]; //= {'\0'};
	struct account temp;

	read(sock, &type, sizeof(type));
	read(sock, &name, sizeof(name));
	read(sock, password, sizeof(password));
	if((fd = open(ACC[type-1], O_RDWR))==-1)printf("File Error\n");
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd,F_SETLKW, &lock);

	int fp = lseek(fd, 0, SEEK_END);
	struct account last;
	if(fp==0){
		temp.id = 1;
		strcpy(temp.name, name);
		int i=0;
		strcpy(temp.pass, password);
		write(fd, &temp, sizeof(temp));
		write(sock, &temp.id, sizeof(temp.id));
	}
	else{
		fp = lseek(fd, -1 * sizeof(struct account), SEEK_CUR);
		read(fd, &last, sizeof(last));
		temp.id = last.id+1;
		strcpy(temp.name, name);
		int i=0;
		strcpy(temp.pass, password);
		write(fd, &temp, sizeof(temp));
		write(sock, &temp.id, sizeof(temp.id));
	}

	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	close(fd);
	return 3;
}

int menu2(int sock, int id){
	int op_id;
	read(sock, &op_id, sizeof(op_id));
	if(op_id == 1){
		//add a train
		int tid = 0;
		int tno,seats; 
		char tname[20];
		//read(sock, &tid, sizeof(tid));
		read(sock, &tname, sizeof(tname));
		read(sock, &tno, sizeof(tno));
		read(sock, &seats, sizeof(seats));
		struct train temp, temp2;

		temp.tid = tid;
		temp.train_no = tno;
		strcpy(temp.train_name, tname);
		temp.total_seats = seats;
		//temp.av_seats = 15;
		temp.booked_seats = 0;
		//temp.last_seatno_used = 0;

		int fd = open(TRAIN, O_RDWR);
		struct flock lock;
		lock.l_type = F_WRLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd, F_SETLKW, &lock);

		int fp = lseek(fd, 0, SEEK_END);
		if(fp == 0){
			write(fd, &temp, sizeof(temp));
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			close(fd);
			write(sock, &op_id, sizeof(op_id));
		}
		else{
			lseek(fd, -1 * sizeof(struct train), SEEK_CUR);
			read(fd, &temp2, sizeof(temp2));
			temp.tid = temp2.tid + 1;
			write(fd, &temp, sizeof(temp));
			write(sock, &op_id, sizeof(op_id));	
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			close(fd);
		}
		return op_id;
	}
	if(op_id == 2){
		int fd = open(TRAIN, O_RDWR);

		struct flock lock;
		lock.l_type = F_WRLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd, F_SETLKW, &lock);

		int fp = lseek(fd, 0, SEEK_END);
		int no_of_trains = fp / sizeof(struct train);
		write(sock, &no_of_trains, sizeof(int));
		lseek(fd, 0, SEEK_SET);
		while(fp != lseek(fd, 0, SEEK_CUR)){
			struct train temp;
			read(fd, &temp, sizeof(struct train));
			write(sock, &temp.tid, sizeof(int));
			write(sock, &temp.train_name, sizeof(temp.train_name));
			write(sock, &temp.train_no, sizeof(int));				
		}
		read(sock, &no_of_trains, sizeof(int));
		if(no_of_trains == 0) write(sock, &no_of_trains, sizeof(int));
		else{
			struct train temp;
			lseek(fd, 0, SEEK_SET);
			lseek(fd, (no_of_trains-1)*sizeof(struct train), SEEK_CUR);
			read(fd, &temp, sizeof(struct train));			
			printf("%s is deleted\n", temp.train_name);
			strcpy(temp.train_name,"deleted");
			lseek(fd, -1*sizeof(struct train), SEEK_CUR);
			write(fd, &temp, sizeof(struct train));
			write(sock, &no_of_trains, sizeof(int));
		}

		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);
	}
	if(op_id == 3){
		int fd = open(TRAIN, O_RDWR);

		struct flock lock;
		lock.l_type = F_WRLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd, F_SETLKW, &lock);

		int fp = lseek(fd, 0, SEEK_END);
		int no_of_trains = fp / sizeof(struct train);
		write(sock, &no_of_trains, sizeof(int));
		lseek(fd, 0, SEEK_SET);
		while(fp != lseek(fd, 0, SEEK_CUR)){
			struct train temp;
			read(fd, &temp, sizeof(struct train));
			write(sock, &temp.tid, sizeof(int));
			write(sock, &temp.train_name, sizeof(temp.train_name));
			write(sock, &temp.train_no, sizeof(int));
			write(sock, &temp.total_seats, sizeof(int));			
		}
		read(sock, &no_of_trains, sizeof(int));
		if(no_of_trains == 0) write(sock, &no_of_trains, sizeof(int));
		else{
			struct train temp;
			lseek(fd, 0, SEEK_SET);
			lseek(fd, (no_of_trains-1)*sizeof(struct train), SEEK_CUR);
			read(fd, &temp, sizeof(struct train));			

			read(sock, &no_of_trains,sizeof(int));
			if(no_of_trains == 1){
				char name[20];
				write(sock, &temp.train_name, sizeof(temp.train_name));
				read(sock, &name, sizeof(name));
				strcpy(temp.train_name, name);
			}
			else if(no_of_trains == 2){
				write(sock, &temp.train_no, sizeof(temp.train_no));
				read(sock, &temp.train_no, sizeof(temp.train_no));
			}
			else{
				write(sock, &temp.total_seats, sizeof(temp.total_seats));
				read(sock, &temp.total_seats, sizeof(temp.total_seats));
			}

			no_of_trains = 3;
			printf("%s\t%d\t%d\n", temp.train_name, temp.train_no, temp.total_seats);
			lseek(fd, -1*sizeof(struct train), SEEK_CUR);
			write(fd, &temp, sizeof(struct train));
			write(sock, &no_of_trains, sizeof(int));
		}
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);
		return op_id;
	}
	if(op_id == 4){
		
		int type, fd, acc_no=0;
		char password[PASS_LENGTH], name[10];
		struct account temp;

		read(sock, &type, sizeof(type));
		read(sock, &name, sizeof(name));
		read(sock, &password, sizeof(password));

		printf("type and name %d\t %s\t",type,name);
		if((fd = open(ACC[type-1], O_RDWR))==-1)printf("File Error\n");
		struct flock lock;

		int fp = lseek(fd, 0, SEEK_END);

		int no_of_user = fp / sizeof(struct account);

		lock.l_type = F_WRLCK;
		lock.l_start = (no_of_user)*sizeof(struct account);
		lock.l_len = sizeof(struct account);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		fcntl(fd,F_SETLKW, &lock);
		//printf("%d fp\n",fp);
		if(fp==0){
			temp.id = 1;
			strcpy(temp.name, name);
			strcpy(temp.pass, password);
			write(fd, &temp, sizeof(temp));
			write(sock, &temp.id, sizeof(temp.id));
		}
		else{
			fp = lseek(fd, -1 * sizeof(struct account), SEEK_CUR);
			read(fd, &temp, sizeof(temp));
			temp.id++;
			strcpy(temp.name, name);
			strcpy(temp.pass, password);
			write(fd, &temp, sizeof(temp));
			write(sock, &temp.id, sizeof(temp.id));
		}
		op_id=4;
		write(sock, &op_id, sizeof(op_id));
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);
		return op_id;
	}
	if(op_id == 5){
		int type, id;
		struct account var;
		read(sock, &type, sizeof(type));

		int fd = open(ACC[type - 1], O_RDWR);
		struct flock lock;
		lock.l_type = F_WRLCK;
		lock.l_start = 0;
		lock.l_whence = SEEK_SET;
		lock.l_len = 0;
		lock.l_pid = getpid();

		fcntl(fd, F_SETLKW, &lock);

		int fp = lseek(fd, 0 , SEEK_END);
		int users = fp/ sizeof(struct account);
		write(sock, &users, sizeof(int));

		lseek(fd, 0, SEEK_SET);
		while(fp != lseek(fd, 0, SEEK_CUR)){
			read(fd, &var, sizeof(struct account));
			write(sock, &var.id, sizeof(var.id));
			write(sock, &var.name, sizeof(var.name));
		}

		read(sock, &id, sizeof(id));
		if(id == 0){write(sock, &op_id, sizeof(op_id));}
		else{
			lseek(fd, 0, SEEK_SET);
			lseek(fd, (id-1)*sizeof(struct account), SEEK_CUR);
			read(fd, &var, sizeof(struct account));
			lseek(fd, -1*sizeof(struct account), SEEK_CUR);
			strcpy(var.name,"deleted");
			strcpy(var.pass, "");
			write(fd, &var, sizeof(struct account));
			write(sock, &op_id, sizeof(op_id));
		}

		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);

		close(fd);

		return op_id;
	}
	if(op_id == 6)
	{
		int i=0,tp=0;
		int entries,fp,fd;
		tp = i+1;
		struct account var;
		fd = open(ACC[0],O_RDONLY);
		entries = 0;
		struct flock lock;
			lock.l_type = F_RDLCK;
			lock.l_start = 0;
			lock.l_len = 0;
			lock.l_whence = SEEK_SET;
			lock.l_pid = getpid();
			
			fcntl(fd, F_SETLKW, &lock);

			fp = lseek(fd, 0, SEEK_END);
			entries= fp/ sizeof(struct account);
			write(sock, &entries, sizeof(&entries));
			lseek(fd, 0, SEEK_SET);
			while(fp != lseek(fd, 0, SEEK_CUR)){
			read(fd, &var, sizeof(struct account));
			write(sock, &var.id, sizeof(var.id));
			write(sock, &var.name, sizeof(var.name));
			write(sock, &var.pass, sizeof(var.pass));
			write(sock, &tp, sizeof(tp));
			}
			
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			close(fd);
		fd = open(ACC[1],O_RDONLY);
		entries = 0;
		lock.l_type = F_RDLCK;
		fcntl(fd, F_SETLKW, &lock);

			fp = lseek(fd, 0, SEEK_END);
			entries= fp/ sizeof(struct account);
			write(sock, &entries, sizeof(&entries));
			lseek(fd, 0, SEEK_SET);
			while(fp != lseek(fd, 0, SEEK_CUR)){
			read(fd, &var, sizeof(struct account));
			write(sock, &var.id, sizeof(var.id));
			write(sock, &var.name, sizeof(var.name));
			write(sock, &var.pass, sizeof(var.pass));
			write(sock, &tp, sizeof(tp));
			}
			
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			close(fd);
		fd = open(ACC[2],O_RDONLY);
		entries = 0;
		lock.l_type = F_RDLCK;
		fcntl(fd, F_SETLKW, &lock);

			fp = lseek(fd, 0, SEEK_END);
			entries= fp/ sizeof(struct account);
			write(sock, &entries, sizeof(&entries));
			lseek(fd, 0, SEEK_SET);
			while(fp != lseek(fd, 0, SEEK_CUR)){
			read(fd, &var, sizeof(struct account));
			write(sock, &var.id, sizeof(var.id));
			write(sock, &var.name, sizeof(var.name));
			write(sock, &var.pass, sizeof(var.pass));
			write(sock, &tp, sizeof(tp));
			}
			
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			close(fd);
			write(sock, &op_id, sizeof(op_id));
		return op_id;
	}
	if(op_id == 7)
	{
		int fd = open(TRAIN, O_RDONLY);

		struct flock lock;
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd, F_SETLKW, &lock);

		int fp = lseek(fd, 0, SEEK_END);
		int no_of_trains = fp / sizeof(struct train);
		write(sock, &no_of_trains, sizeof(int));
		lseek(fd, 0, SEEK_SET);
		while(fp != lseek(fd, 0, SEEK_CUR)){
			struct train temp;
			read(fd, &temp, sizeof(struct train));
			write(sock, &temp.tid, sizeof(&temp.tid));
			write(sock, &temp.train_no, sizeof(&temp.train_no));
			write(sock, &temp.train_name, sizeof(&temp.train_name));
			write(sock, &temp.total_seats, sizeof(&temp.total_seats));
			write(sock, &temp.booked_seats, sizeof(&temp.booked_seats));
		}
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);
		write(sock, &op_id, sizeof(op_id));
		return op_id;
	}
	if(op_id == 8)
	{
		int fd = open(BOOKING, O_RDONLY);
		struct flock lock;
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd, F_SETLKW, &lock);

		int fp = lseek(fd, 0, SEEK_END);
		int entries = fp/ sizeof(struct bookings);
		write(sock, &entries, sizeof(int));
		lseek(fd, 0, SEEK_SET);
		while(fp != lseek(fd, 0, SEEK_CUR)){
			struct bookings temp;
			read(fd, &temp, sizeof(struct bookings));
			write(sock, &temp.bid, sizeof(&temp.bid));
			//write(sock, &temp.type, sizeof(&temp.type));
			write(sock, &temp.acc_no, sizeof(&temp.acc_no));
			write(sock, &temp.tr_id, sizeof(&temp.tr_id));
			write(sock, &temp.trainname, sizeof(&temp.trainname));
			write(sock, &temp.seat_booked, sizeof(&temp.seat_booked));
			write(sock, &temp.cancelled, sizeof(&temp.cancelled));
		}
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);
		
	write(sock, &op_id, sizeof(op_id));
	return op_id;
	}
	if(op_id == 9) {
		write(sock,&op_id, sizeof(op_id));
		return -1;
	}
}

int menu1(int sock, int id, int type){
	int op_id,avl_seats;
	read(sock, &op_id, sizeof(op_id));
	if(op_id == 1){
		//book a ticket
		int fd = open(TRAIN, O_RDWR);

		struct flock lock;
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd, F_SETLKW, &lock);

		struct train temp;
		int fp = lseek(fd, 0, SEEK_END);
		int no_of_trains = fp / sizeof(struct train);
		write(sock, &no_of_trains, sizeof(int));
		if(no_of_trains !=0){
			lseek(fd, 0, SEEK_SET);
			while(fp != lseek(fd, 0, SEEK_CUR)){
				read(fd, &temp, sizeof(struct train));
				write(sock, &temp.tid, sizeof(int));
				write(sock, &temp.train_no, sizeof(int));	
				avl_seats = temp.total_seats-temp.booked_seats;
				write(sock, &avl_seats, sizeof(int));	
				write(sock, &temp.train_name, sizeof(temp.train_name));		
			}

			int trainid, seats,tp1;
			read(sock, &trainid, sizeof(trainid));
			read(sock, &tp1, sizeof(tp1));
			lseek(fd, 0, SEEK_SET);
			lseek(fd, (trainid )*sizeof(struct train), SEEK_CUR);
			read(fd, &temp, sizeof(struct train));
			write(sock, &avl_seats, sizeof(int));
			read(sock, &seats, sizeof(seats));
			if(seats>0){
				temp.booked_seats += seats;
				int fd2 = open(BOOKING, O_RDWR);
				fcntl(fd2, F_SETLKW, &lock);
				struct bookings bk;
				int fp2 = lseek(fd2, 0, SEEK_END);
				if(fp2 > 0)
				{
					lseek(fd2, -1*sizeof(struct bookings), SEEK_CUR);
					read(fd2, &bk, sizeof(struct bookings));
					bk.bid++;
				}
				else 
					bk.bid = 0;
				bk.type = tp1;
				bk.acc_no = id;
				bk.tr_id = trainid;
				bk.cancelled = 0;
				strcpy(bk.trainname, temp.train_name);
				bk.seat_booked = seats;
				//bk.seat_start = temp.last_seatno_used + 1;
				//bk.seat_end = temp.last_seatno_used + seats;
				//temp.last_seatno_used = bk.seat_end;
				write(fd2, &bk, sizeof(bk));
				lock.l_type = F_UNLCK;
				fcntl(fd2, F_SETLK, &lock);
			 	close(fd2);
			}
			lseek(fd, -1*sizeof(struct train), SEEK_CUR);
			write(fd, &temp, sizeof(temp));
			if(seats<=0)
				op_id = -1;
			write(sock, &op_id, sizeof(op_id));
		}
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
	 	close(fd);
		return 1;
	}
	if(op_id == 2){
		view_booking(sock, id, type);
		write(sock, &op_id, sizeof(op_id));
		return 2;
	}
	if(op_id == 3){
		//update booking
		if(view_booking(sock, id, type)!=0){
		
			int fd1 = open(TRAIN, O_RDWR);
			int fd2 = open(BOOKING, O_RDWR);
			struct flock lock;
			lock.l_type = F_WRLCK;
			lock.l_start = 0;
			lock.l_len = 0;
			lock.l_whence = SEEK_SET;
			lock.l_pid = getpid();

			fcntl(fd1, F_SETLKW, &lock);
			fcntl(fd2, F_SETLKW, &lock);

			int val;
			struct train temp1;
			struct bookings temp2;
			read(sock, &val, sizeof(int));	//Read the Booking ID to updated
			// read the booking to be updated
			lseek(fd2, 0, SEEK_SET);
			lseek(fd2, val*sizeof(struct bookings), SEEK_CUR);
			read(fd2, &temp2, sizeof(temp2));
			lseek(fd2, -1*sizeof(struct bookings), SEEK_CUR);
			printf("%d %s %d\n", temp2.tr_id, temp2.trainname, temp2.seat_booked);
			// read the train details of the booking
			lseek(fd1, 0, SEEK_SET);
			lseek(fd1, (temp2.tr_id-1)*sizeof(struct train), SEEK_CUR);
			read(fd1, &temp1, sizeof(temp1));
			lseek(fd1, -1*sizeof(struct train), SEEK_CUR);
			printf("%d %s %d\n", temp1.tid, temp1.train_name, temp1.booked_seats);


			read(sock, &val, sizeof(int));	//Increase or Decrease


			if(val==1){//increase
				read(sock, &val, sizeof(int)); //No of Seats
				if(temp1.total_seats - temp1.booked_seats>= val){
					temp2.cancelled = 1;
					temp1.booked_seats -= temp2.seat_booked;
					write(fd2, &temp2, sizeof(temp2));

					int tot_seats = temp2.seat_booked + val;
					struct bookings bk;

					int fp2 = lseek(fd2, 0, SEEK_END);
					lseek(fd2, -1*sizeof(struct bookings), SEEK_CUR);
					read(fd2, &bk, sizeof(struct bookings));

					bk.bid++;
					bk.type = temp2.type;
					bk.acc_no = temp2.acc_no;
					bk.tr_id = temp2.tr_id;
					bk.cancelled = 0;
					strcpy(bk.trainname, temp2.trainname);
					//bk.seat_start = temp1.last_seatno_used + 1;
					//bk.seat_end = temp1.last_seatno_used + tot_seats;
					bk.seat_booked = tot_seats;
					temp1.booked_seats += tot_seats;
					//temp1.last_seatno_used = bk.seat_end;

					write(fd2, &bk, sizeof(bk));
					write(fd1, &temp1, sizeof(temp1));
				}
				else{
					op_id = -2;
					write(sock, &op_id, sizeof(op_id));
				}
			}
			else{//decrease			
				read(sock, &val, sizeof(int)); //No of Seats
				if(temp2.seat_booked - val <= 0){
					temp2.cancelled = 1;
					temp1.booked_seats -= temp2.seat_booked;
				}
				else{
					temp2.seat_booked -= val;
					temp1.booked_seats -= val;
				}
				write(fd2, &temp2, sizeof(temp2));
				write(fd1, &temp1, sizeof(temp1));
			}
			lock.l_type = F_UNLCK;
			fcntl(fd1, F_SETLK, &lock);
			fcntl(fd2, F_SETLK, &lock);
			close(fd1);
			close(fd2);
		}
		if(op_id>0)

		write(sock, &op_id, sizeof(op_id));
		return 3;

	}
	if(op_id == 4) {
		//cancel booking
		//write(sock, &op_id, sizeof(op_id));
		if(view_booking(sock, id, type)!=0){
			view_booking(sock, id, type);

			int fd1 = open(TRAIN, O_RDWR);
			int fd2 = open(BOOKING, O_RDWR);
			struct flock lock;
			lock.l_type = F_WRLCK;
			lock.l_start = 0;
			lock.l_len = 0;
			lock.l_whence = SEEK_SET;
			lock.l_pid = getpid();

			fcntl(fd1, F_SETLKW, &lock);
			fcntl(fd2, F_SETLKW, &lock);

			int val;
			struct train temp1;
			struct bookings temp2;
			read(sock, &val, sizeof(int));	//Read the Booking ID to updated
			// read the booking to be updated
			lseek(fd2, 0, SEEK_SET);
			lseek(fd2, val*sizeof(struct bookings), SEEK_CUR);
			read(fd2, &temp2, sizeof(temp2));
			lseek(fd2, -1*sizeof(struct bookings), SEEK_CUR);
			printf("%d %s %d \n", temp2.tr_id, temp2.trainname, temp2.seat_booked);
			int seat_decrease=temp2.seat_booked;
			// read the train details of the booking
			lseek(fd1, 0, SEEK_SET);
			lseek(fd1, (temp2.tr_id)*sizeof(struct train), SEEK_CUR);
			read(fd1, &temp1, sizeof(temp1));
			lseek(fd1, -1*sizeof(struct train), SEEK_CUR);
			printf("%d %s %d\n", temp1.tid, temp1.train_name, temp1.booked_seats);
			temp1.booked_seats -= seat_decrease;
			temp2.cancelled = 1;
			write(fd1, &temp1, sizeof(temp1));
			write(fd2, &temp2, sizeof(temp2));
			lock.l_type = F_UNLCK;
			fcntl(fd1, F_SETLK, &lock);
			fcntl(fd2, F_SETLK, &lock);
			close(fd1);
			close(fd2);
		}
		if(op_id>0)
		write(sock, &op_id, sizeof(op_id));
		return 4;
	}
	if(op_id == 5) {
		write(sock, &op_id, sizeof(op_id));
		return -1;
	}
	return 0;
}

int view_booking(int sock, int id, int type){
	int fd = open(BOOKING, O_RDONLY);
	struct flock lock;
	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();
	
	fcntl(fd, F_SETLKW, &lock);

	int fp = lseek(fd, 0, SEEK_END);
	int entries = 0;
	if(fp == 0)
		write(sock, &entries, sizeof(entries));
	else{
		struct bookings bk[10];
		while(fp>0 && entries<10){
			struct bookings temp;
			fp = lseek(fd, -1*sizeof(struct bookings), SEEK_CUR);
			read(fd, &temp, sizeof(struct bookings));
			if(temp.acc_no == id && temp.type == type)
				bk[entries++] = temp;
			fp = lseek(fd, -1*sizeof(struct bookings), SEEK_CUR);
		}
		write(sock, &entries, sizeof(entries));
		for(fp=0;fp<entries;fp++){
			write(sock, &bk[fp].bid, sizeof(bk[fp].bid));
			write(sock, &bk[fp].trainname, sizeof(bk[fp].trainname));
			write(sock, &bk[fp].seat_booked, sizeof(int));
			//write(sock, &bk[fp].seat_end, sizeof(int));
			write(sock, &bk[fp].cancelled, sizeof(int));
		}
	}
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
	return entries;
}
