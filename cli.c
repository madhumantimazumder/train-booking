#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 8090
#define PASS_LENGTH 20

int irctc(int sock);
int menu2(int sock, int type);
int do_admin_action(int sock, int action);
int do_action(int sock, int opt,int t);
int view_booking(int sock);
void viewAllBooking(int sock);
void viewAllTrain(int sock);
void viewAllUser(int sock);

int main(int argc, char * argv[]){
	char *ip = "127.0.0.1";
	if(argc==2){
		ip = argv[1];
	}
	int cli_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(cli_fd == -1){
		printf("socket creation failed\n");
		exit(0);
	}
	struct sockaddr_in ca;
	ca.sin_family=AF_INET;
	ca.sin_port= htons(PORT);
	ca.sin_addr.s_addr = inet_addr(ip);
	if(connect(cli_fd, (struct sockaddr *)&ca, sizeof(ca))==-1){
		printf("connect failed\n");
		exit(0);
	}
	printf("connection established\n");
	
	while(irctc(cli_fd)!=3);
	close(cli_fd);

	return 0;
}

int irctc(int sock){
	int opt;
	system("clear");
	printf("+++++++TRAIN BOOKING++++++++\n");
	printf("1. Sign In\n");
	printf("2. Sign Up\n");
	printf("3. Exit\n");
	printf("+++++Enter Your Choice++++++\n");
	scanf("%d", &opt);
	write(sock, &opt, sizeof(opt));
	if(opt==1){
		int type, acc_no;
		char password[PASS_LENGTH];
		printf("Enter the type of account:\n");
		printf("1. Customer\n2. Agent\n3. Admin\n");
		printf("Your Response: ");
		scanf("%d", &type);
		printf("Enter Your Account Number: ");
		scanf("%d", &acc_no);
		strcpy(password,getpass("Enter the password: "));
		//getchar();
		//password[strlen(password)] = '\0';
		//password=htonl(password);
		int ff = strlen(password);
		write(sock, &type, sizeof(type));
		write(sock, &acc_no, sizeof(acc_no));
		write(sock, password, sizeof(password));
		//write(sock, &ff, sizeof(ff));
		int valid_login;
		read(sock, &valid_login, sizeof(valid_login));
		printf("%d", valid_login);
		if(valid_login == 1){
			while(menu2(sock, type)!=-1);
			system("clear");
			return 3;
		}
		else{
			printf("Login Failed\n");
			while(getchar()!='\n');
			getchar();
			return 1;
		}
	}
	else if(opt==2){
		int type, acc_no;
		char password[PASS_LENGTH]={'\0'}, secret_pin[5], name[10];
		printf("Enter the type of account:\n");
		printf("1. Customer\n2. Agent\n3. Admin\n");
		printf("Your Response: ");
		scanf("%d", &type);
		printf("Enter your name: ");scanf("%s", name);
		strcpy(password,getpass("Enter the password: "));
		if(type == 3){
			int attempt = 1;
			while(1){
				strcpy(secret_pin, getpass("Enter secret PIN to create ADMIN account: "));attempt++;
				if(strcmp(secret_pin, "root")!=0 && attempt<=3) printf("Invalid PIN. Please try again.\n");
				else break;
			}
			if(!strcmp(secret_pin, "root"));
			else exit(0);
		}
		//printf("stored %s\t%d\n",password,strlen(password) );
		write(sock, &type, sizeof(type));
		write(sock, &name, sizeof(name));
		write(sock, password, sizeof(password));
		read(sock, &acc_no, sizeof(acc_no));
		printf("Remember the account no of further login: %d\n", acc_no);
		while(getchar()!='\n');
		getchar();		
		return 3;
	}
	else
		return 3;
}

int menu2(int sock, int type){
	int opt = 0;
	if(type == 1 || type == 2){
		system("clear");
		printf("++++ OPTIONS ++++\n");
		printf("1. Book Ticket\n");
		printf("2. View Bookings\n");
		printf("3. Update Booking\n");
		printf("4. Cancel booking\n");
		printf("5. Logout\n");
		printf("Your Choice: ");
		scanf("%d", &opt);
		return do_action(sock, opt,type);
		//return -1;
	}
	else{
		system("clear");
		printf("++++ OPTIONS ++++\n");
		printf("1. Add Train\n");
		printf("2. Delete Train\n");
		printf("3. Modify Train\n");
		printf("4. Add User\n");
		printf("5. Delete User\n");
		printf("6. View All User\n");
		printf("7. View All Train\n");
		printf("8. View All Booking\n");
		printf("9. Logout\n");
		printf("Your Choice: ");
		scanf("%d", &opt);
		return do_admin_action(sock, opt);
	}
}

int do_admin_action(int sock, int opt){
	switch(opt){
		case 1:{
			int tno,seats;
			char tname[20];
			write(sock, &opt, sizeof(opt));
			printf("Enter Train Name: ");scanf("%s", tname);
			printf("Enter Train No. : ");scanf("%d", &tno);
			printf("Enter Total seats : ");scanf("%d", &seats);
			write(sock, &tname, sizeof(tname));
			write(sock, &tno, sizeof(tno));
			write(sock, &seats, sizeof(seats));
			read(sock, &opt, sizeof(opt));
			if(opt == 1 ) printf("Train Added Successfully.\n");
			while(getchar()!='\n');
			getchar();
			return opt;
			break;
		}
		case 2:{
			int no_of_trains;
			write(sock, &opt, sizeof(opt));
			read(sock, &no_of_trains, sizeof(int));
			while(no_of_trains>0){
				int tid, tno;
				char tname[20];
				read(sock, &tid, sizeof(tid));
				read(sock, &tname, sizeof(tname));
				read(sock, &tno, sizeof(tno));
				if(!strcmp(tname, "deleted"));else
				printf("%d.\t%d\t%s\n", tid+1, tno, tname);
				no_of_trains--;
			}
			printf("Enter 0 to cancel.\nEnter the train ID to delete: "); scanf("%d", &no_of_trains);
			write(sock, &no_of_trains, sizeof(int));
			read(sock, &opt, sizeof(opt));
			if(opt == no_of_trains && opt) printf("Train deleted successfully\n");
			while(getchar()!='\n');
			getchar();
			return opt;
			break;
		}
		case 3:{
			int no_of_trains;
			write(sock, &opt, sizeof(opt));
			read(sock, &no_of_trains, sizeof(int));
			while(no_of_trains>0){
				int tid, tno,seats;
				char tname[20];
				read(sock, &tid, sizeof(tid));
				read(sock, &tname, sizeof(tname));
				read(sock, &tno, sizeof(tno));
				read(sock, &seats, sizeof(seats));
				if(!strcmp(tname, "deleted"));else
				printf("%d.\t%d\t%s\t%d\n", tid+1, tno, tname,seats);
				no_of_trains--;
			}
			printf("Enter 0 to cancel.\nEnter the train ID to modify: "); scanf("%d", &no_of_trains);
			write(sock, &no_of_trains, sizeof(int));

			if(no_of_trains != 0){
				printf("What parameter do you want to modify?\n1. Train Name\n2. Train No.\n3. Total Seats\n");
				printf("Your Choice: ");scanf("%d", &no_of_trains);
				write(sock, &no_of_trains, sizeof(int));
				if(no_of_trains == 2 || no_of_trains == 3){
					read(sock, &no_of_trains, sizeof(int));
					printf("Current Value: %d\n", no_of_trains);				
					printf("Enter Value: ");scanf("%d", &no_of_trains);
					write(sock, &no_of_trains, sizeof(int));
				}
				else{
					char name[20];
					read(sock, &name, sizeof(name));
					printf("Current Value: %s\n", name);
					printf("Enter Value: ");scanf("%s", name);
					write(sock, &name, sizeof(name));
				}
				read(sock, &opt, sizeof(opt));
				if(opt == 3) printf("Train Data Modified Successfully\n");
			}
			else{
				read(sock, &no_of_trains, sizeof(int));
			}
			while(getchar()!='\n');
			getchar();
			return opt;
			break;
		}
		case 4:{

			write(sock, &opt, sizeof(opt));
			int type, acc_no;
			char password[PASS_LENGTH], secret_pin[5], name[10];
			printf("Enter the type of account:\n");
			printf("1. Customer\n2. Agent\n3. Admin\n");
			printf("Your Response: ");
			scanf("%d", &type);
			printf("Enter your name: ");scanf("%s", name);
			strcpy(password,getpass("Enter the password: "));
			int attempt = 1;
			write(sock, &type, sizeof(type));
			write(sock, &name, sizeof(name));
			write(sock, &password, strlen(password));

			read(sock, &acc_no, sizeof(acc_no));
			printf("Remember the account no of further login: %d\n", acc_no);			


			read(sock, &opt, sizeof(opt));
			if(opt == 4)printf("Successfully created ADMIN account\n");
			while(getchar()!='\n');
			getchar();
			return opt;
			break;
		}
		case 5: {
			int choice, users, id;
			write(sock, &opt, sizeof(opt));
			printf("What kind of account do you want to delete?\n");
			printf("1. Customer\n2. Agent\n3. Admin\n");
			printf("Your Choice: ");
			scanf("%d", &choice);
			write(sock, &choice, sizeof(choice));
			read(sock, &users, sizeof(users));
			while(users--){
				char name[10];
				read(sock, &id, sizeof(id));
				read(sock, &name, sizeof(name));
				if(strcmp(name, "deleted")!=0)
				printf("%d\t%s\n", id, name);
			}
			printf("Enter the ID to delete: ");scanf("%d", &id);
			write(sock, &id, sizeof(id));
			read(sock, &opt, sizeof(opt));
			if(opt == 5) printf("Successfully deleted user\n");
			while(getchar()!='\n');
			getchar();
			return opt;
		}
		case 6:
		{
			write(sock, &opt, sizeof(opt));
			int entries,i=0,k=0;
			int id,type;
			entries=0;
			read(sock, &entries, sizeof(&entries));
				if(entries == 0) 
				{
					if(i==0)
					printf("No customers found...\n");
					if(i==1)
					printf("No agents found...\n");
					if(i==2)
					printf("No admins found...\n");
				}
				else 
				{
					if(i==0)
					printf("Total Customers: %d \n", entries);
					if(i==1)
					printf("Total Agents: %d \n", entries);
					if(i==2)
					printf("Total Admins: %d \n", entries);
					while(!getchar());
					int j  =0;
					for(j=0;j<entries;j++){
					char name[10],pass[PASS_LENGTH];
					//type = i+1;
					read(sock, &id, sizeof(id));
					read(sock, &name, sizeof(name));
					read(sock, &pass, sizeof(pass));
					read(sock, &type, sizeof(type));
					printf("UserID: %d\tUser Name: %s\tPassword: %s\tUser Type: %d\n", id, name, pass, type);
					}
				}
			i++;
			read(sock, &entries, sizeof(&entries));
				if(entries == 0) 
				{
					if(i==0)
					printf("No customers found...\n");
					if(i==1)
					printf("No agents found...\n");
					if(i==2)
					printf("No admins found...\n");
				}
				else 
				{
					if(i==0)
					printf("Total Customers: %d \n", entries);
					if(i==1)
					printf("Total Agents: %d \n", entries);
					if(i==2)
					printf("Total Admins: %d \n", entries);
					while(!getchar());
					int j  =0;
					for(j=0;j<entries;j++){
					char name[10],pass[PASS_LENGTH];
					//type = i+1;
					read(sock, &id, sizeof(id));
					read(sock, &name, sizeof(name));
					read(sock, &pass, sizeof(pass));
					read(sock, &type, sizeof(type));
					printf("UserID: %d\tUser Name: %s\tPassword: %s\tUser Type: %d\n", id, name, pass, type);
					}
				}
			i++;
			read(sock, &entries, sizeof(&entries));
				if(entries == 0) 
				{
					if(i==0)
					printf("No customers found...\n");
					if(i==1)
					printf("No agents found...\n");
					if(i==2)
					printf("No admins found...\n");
				}
				else 
				{
					if(i==0)
					printf("Total Customers: %d \n", entries);
					if(i==1)
					printf("Total Agents: %d \n", entries);
					if(i==2)
					printf("Total Admins: %d \n", entries);
					while(!getchar());
					int j  =0;
					for(j=0;j<entries;j++){
					char name[10],pass[PASS_LENGTH];
					//type = i+1;
					read(sock, &id, sizeof(id));
					read(sock, &name, sizeof(name));
					read(sock, &pass, sizeof(pass));
					read(sock, &type, sizeof(type));
					printf("UserID: %d\tUser Name: %s\tPassword: %s\tUser Type: %d\n", id, name, pass, type);
					}
				}
			read(sock, &opt, sizeof(opt));
			if(opt == 6) printf("Press any key to continue\n");
			while(getchar()!='\n');
			getchar();
			return opt;
			break;
		}
		case 7:
		{
			write(sock, &opt, sizeof(opt));
			int entries;
			read(sock, &entries, sizeof(int));
			if(entries == 0) printf("No records found.\n");
			else printf("Total trains %d :\n", entries);
			while(!getchar());
			while(entries--){
				int tid,  train_no,total_seats,booked_seats;
				char trainname[20];

				read(sock, &tid, sizeof(&tid));
				read(sock, &train_no, sizeof(&train_no));
				read(sock, &trainname, sizeof(&trainname));
				read(sock, &total_seats, sizeof(&total_seats));
				read(sock, &booked_seats, sizeof(&booked_seats));
				
				printf("TrainID: %d\tTrain No: %d\tTrain Name: %s\tTotal Seats: %d\tBooked Seats: %d\n", tid, train_no, trainname, total_seats
					,booked_seats);
				}
			read(sock, &opt, sizeof(opt));
			if(opt == 7) printf("Press any key to continue\n");
			while(getchar()!='\n');
			getchar();
			return opt;
			break;
		}
		case 8:
		{
			write(sock, &opt, sizeof(opt));
			int entries;
			
			read(sock, &entries, sizeof(entries));
			if(entries == 0) printf("No records found.\n");
			else 
			{printf("Total bookings %d :\n", entries);
			while(!getchar());
			while(entries--){
				int bid, seat_booked,  cancelled,type,accno,tr_id;
				char trainname[20];
				read(sock, &bid, sizeof(&bid));
				//printf("test %d\n",bid);
				//read(sock, &type, sizeof(&type));
				read(sock, &accno, sizeof(&accno));
				read(sock, &tr_id, sizeof(&tr_id));
				read(sock, &trainname, sizeof(&trainname));
				read(sock, &seat_booked, sizeof(&seat_booked));
				read(sock, &cancelled, sizeof(&cancelled));	
				if(!cancelled)
					printf("Status: %d\tBookingID: %d\tAccount Number: %d\tTrain ID: %d\tTrain Name: %s\tSeat Booked: %d\n",
						cancelled, bid,accno,tr_id, trainname,seat_booked);
				else
					printf("Status: %d\tBookingID: %d\tAccount Number: %d\tTrain ID: %d\tTrain Name: %s\tSeat Booked: %d\n",
						cancelled, bid,accno,tr_id, trainname,seat_booked);
			}
			}
			if(opt == 8) printf("Press any key to continue\n");
			while(getchar()!='\n');
			getchar();
			read(sock, &opt, sizeof(opt));
			return opt;
			break;
		}
		case 9: {write(sock, &opt, sizeof(opt));
			read(sock, &opt, sizeof(opt));
			if(opt==9) printf("Logged out successfully.\n");
			while(getchar()!='\n');
			getchar();
			return -1;
			break;}
		default: return -1;
	}
}

int do_action(int sock, int opt,int t){
	switch(opt){
		case 1:{
			//book a ticket
			int trains, trainid, trainavseats, trainno, required_seats;
			char trainname[20];
			write(sock, &opt, sizeof(opt));
			read(sock, &trains, sizeof(trains));
				if(trains != 0){
				printf("ID\tT_NO\tAV_SEAT\tTRAIN NAME\n");
				while(trains--){
					read(sock, &trainid, sizeof(trainid));
					read(sock, &trainno, sizeof(trainno));
					read(sock, &trainavseats, sizeof(trainavseats));
					read(sock, &trainname, sizeof(trainname));
					if(strcmp(trainname, "deleted")!=0)
					printf("%d\t%d\t%d\t%s\n", trainid, trainno, trainavseats, trainname);
				}
				printf("Enter the train ID: "); scanf("%d", &trainid);
				write(sock, &trainid, sizeof(trainid));
				int x = t;
				write(sock, &x, sizeof(x));
				read(sock, &trainavseats, sizeof(trainavseats));
				printf("Enter the number of seats: "); scanf("%d", &required_seats);
				if(trainavseats>=required_seats && required_seats>0)
					write(sock, &required_seats, sizeof(required_seats));
				else{
					required_seats = -1;
					write(sock, &required_seats, sizeof(required_seats));
				}
				read(sock, &opt, sizeof(opt));
				
				if(opt == 1) printf("Tickets booked successfully\n");
				else printf("Tickets were not booked. Please try again.\n");
			}
			printf("Press any key to continue...\n");
			while(getchar()!='\n');
			getchar();
			while(!getchar());
			return 1;
		}
		case 2:{
			//View your bookings
			write(sock, &opt, sizeof(opt));
			view_booking(sock);
			read(sock, &opt, sizeof(opt));
			return 2;
		}
		case 3:{
			//update bookings
			int val;
			write(sock, &opt, sizeof(opt));
			if(view_booking(sock)!=0){
				printf("Enter the booking id to be updated: "); scanf("%d", &val);
				write(sock, &val, sizeof(int));	//Booking ID
				printf("What information do you want to update:\n1.Increase No of Seats\n2. Decrease No of Seats\nYour Choice: ");
				scanf("%d", &val);
				write(sock, &val, sizeof(int));	//Increase or Decrease
				if(val == 1){
					printf("How many tickets do you want to increase: ");scanf("%d",&val);
					write(sock, &val, sizeof(int));	//No of Seats
				}else if(val == 2){
					printf("How many tickets do you want to decrease: ");scanf("%d",&val);
					write(sock, &val, sizeof(int));	//No of Seats		
				}
				
			}
			read(sock, &opt, sizeof(opt));
			if(opt == -2)
				printf("Operation failed. No more available seats\n");
			else printf("Operation succeded.\n");
			while(getchar()!='\n');
			getchar();
			return 3;
		}
		case 4: {
			//cancel bookings
			int val;
			write(sock, &opt, sizeof(opt));
			if(view_booking(sock)!=0){
				//view_booking(sock);
				printf("Enter the booking id to be cancelled: "); scanf("%d", &val);
				write(sock, &val, sizeof(int));	//Booking ID
			}
			read(sock, &opt, sizeof(opt));
			printf("Operation succeded.\n");
			while(getchar()!='\n');
			getchar();
			break;
		}
		case 5: {
			write(sock, &opt, sizeof(opt));
			read(sock, &opt, sizeof(opt));
			if(opt == 5) printf("Logged out successfully.\n");
			while(getchar()!='\n');
			getchar();
			return -1;
			break;
		}
		default: return -1;
	}
}

int view_booking(int sock){
	int entries;
	read(sock, &entries, sizeof(int));
	int init_entry=0;
	if(entries == 0) printf("No records found.\n");
	//else printf("Your recent %d bookings are :\n", entries);
	while(!getchar());
	while(entries--){
		int bid, bks_seat, bke_seat, cancelled;
		char trainname[20];
		read(sock,&bid, sizeof(bid));
		read(sock,&trainname, sizeof(trainname));
		read(sock,&bks_seat, sizeof(int));
		//read(sock,&bke_seat, sizeof(int));
		read(sock,&cancelled, sizeof(int));
		if(!cancelled){
			init_entry++;
			printf("Status: Confirmed\tBookingID: %d\tSeats booked: %d\tTRAIN :%s\n", bid, bks_seat,  trainname);
		}
		//else
		//	printf("Status: Cancelled\tBookingID: %d\tSeats booked: %d\tTRAIN :%s\n", bid, bks_seat,  trainname);
	}
	printf("Press any key to continue...\n");
	while(getchar()!='\n');
	getchar();
	return init_entry;
}

void viewAllBooking(int sock){
	int entries;
	read(sock, &entries, sizeof(int));
	if(entries == 0) printf("No records found.\n");
	else printf("Total bookings %d :\n", entries);
	while(!getchar());
	while(entries--){
		int bid, seat_booked,  cancelled,type,accno,tr_id;
		char trainname[20];

		read(sock, &bid, sizeof(bid));

		read(sock, &type, sizeof(type));
		read(sock, &accno, sizeof(accno));
		read(sock, &tr_id, sizeof(tr_id));
		read(sock, &trainname, sizeof(trainname));
		read(sock, &seat_booked, sizeof(seat_booked));
		read(sock, &cancelled, sizeof(int));	
		if(!cancelled)
			printf("Status: %d\tBookingID: %d\tAccount Number: %d\tUser Type: %d\tTrain ID: %d\tTrain Name: %s\tSeat Booked: %d\n",
				cancelled, bid,accno,type,tr_id, trainname,seat_booked);
		else
			printf("Status: %d\tBookingID: %d\tAccount Number: %d\tUser Type: %d\tTrain ID: %d\tTrain Name: %s\tSeat Booked: %d\n",
				cancelled, bid,accno,type,tr_id, trainname,seat_booked);
	}
	printf("Press any key to continue...\n");
	while(getchar()!='\n');
	getchar();
}
