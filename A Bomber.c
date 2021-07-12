#include<stdio.h>
#include<string.h>
#include<windows.h>
#include<stdint.h>
#include<wchar.h>
#include <stdlib.h>

#define ACTIVE 1
#define INACTIVE 0

#define UP 0
#define LEFT 1
#define RIGHT 2
#define DOWN 3

#define WALL 0
#define ENEMY 1

#define UL 0
#define UC 1
#define UR 2
#define ML 3
#define MR 4
#define BL 5
#define BC 6
#define BR 7

int posX=1;
int posY=1;
int posXDoor=-1;
int posYDoor=-1;
int level=1;
char lastKey;
LARGE_INTEGER frequency,startGame;
float target = 1.0f;
LARGE_INTEGER bombs[100];
int bombLoc[100][2];
int bombIdx = 0;
int playerIdx=0;
int enemyCount = 2;
int doorCreated=0;
int point = 0;
int played = 0;
char map[18][35];
void printBomber(){
	printf("\n\n\t\t /\\   |~) _  _ _ |_  _  _\n");
	printf("\t\t/~~\\  |_)(_)| | ||_)(/_| \n");	
}
float getSecondsperFrame(LARGE_INTEGER start, LARGE_INTEGER end){
	return ((float)(end.QuadPart - start.QuadPart) / (float)frequency.QuadPart);
}
int is_white_space(char c) {
    return (c == ' ' || c == '\t' || c == '\n');
}
struct Enemy{
	int posX;
	int posY;
	int status;
}enemies[100];
struct Player{
	char name[50];
	int score;	
}players[1000];
void createPlayer(char* name, int score){
	strcpy(players[playerIdx].name,name);
	players[playerIdx].score=score;
	playerIdx+=1;
}
int validateMove(int desiredX,int desiredY){
	int yValid = (desiredY>=1&&desiredY<=15) ? 1 : 0;
	int xValid = (desiredX>=1&&desiredX<=32) ? 1 : 0;
	if(!yValid) return 1;
	if(!xValid) return 1;
	if(map[desiredY][desiredX]=='O') return 2;
	if(map[desiredY][desiredX]=='@') return 3;
	if(map[desiredY][desiredX]=='X') return 4;
	if(map[desiredY][desiredX]=='H') {
//		printf("\nDESX : %d, DESY : %d\n",desiredX,desiredY);
//		getchar();
		return 5;
	}
	if(map[desiredY][desiredX]==' ') return 0;
	else return 6;
	return 1;
}
void generateWall(){
//	resetWall();
	if(played==0){
		srand(time(0));
		int wallCount =	rand()%10;
		wallCount+=9;
		wallCount+=(level*10);
		int i,j;
		
		for(i = 0;i<wallCount;i++){
			int x = 0;
			int y = 0;
			int invalid = 1;
			while(1){
				x = (rand()%32)+1;
				y = (rand()%16)+1; 
				if(x<5&&y<4) continue;
				if(map[y][x]==' ') break;
			}
			map[y][x]='H';
		}
	}
}
int openFile(){
	FILE* fp = fopen("map.txt","r");
	if(fp==NULL) {
		printf("\n\t\tMAP NOT FOUND!");
		getchar();
		return 0;
	}
	char str[35],str2[35];
	int i=0;
	while(fgets(str,35,fp)!=NULL)
		strcpy(map[i++],str);
	fp = NULL;
	fclose(fp);
	fp = fopen("save.txt","r");
	if(fp==NULL) {
		printf("\n\t\tSAVE FILE NOT FOUND!\n\t\tCREATING A NEW ONE!");
		getchar();	
		fp = fopen("save.txt","w");
		fclose(fp);
		return 1;
	}
	char name[50];
	int score;
	while(fscanf(fp,"%[^#]#%d\n",name,&score)!=EOF) createPlayer(name,score);	
	fclose(fp);
	return 1;
}
void printMap(){
	printf("\n\tLEVEL : %d\n",level);
	int i;
	for(i=0;i<18;i++){
		printf("%s",map[i]);
	}
//	printf("\nDOOR X : %d Y : %d\n",posXDoor,posYDoor);
	printf("\nPOINT : %d",point);
}
void putBomb(LARGE_INTEGER time){
	int x,y;
	if(lastKey=='a'){
		x=posX-1;
		y=posY;
	} else if(lastKey=='s'){
		x=posX;
		y=posY+1;
	} else if(lastKey=='d'){
		x=posX+1;
		y=posY;
	} else {
		x=posX;
		y=posY-1;
	}
	bombLoc[bombIdx][0]=y;
	bombLoc[bombIdx][1]=x;
	if(validateMove(x,y)==0) {
		map[y][x]='*';
		bombs[bombIdx].QuadPart=time.QuadPart;
		if(bombIdx+1==99){
			bombIdx=0;
		} else bombIdx++;
	}
}
int validateDead(int x, int y){
	if(map[y][x]=='@') return 0;
	return 1;
}
void showHighscore(){
	system("cls");
	int i;
	printBomber();
	printf("\n\n\t+------------------------------------+---------+\n");
	printf("\t|                NAME                |  SCORE  |\n");
	printf(    "\t+------------------------------------+---------+\n");
	for(i=0;i<playerIdx;i++){
		printf("\t| %-34s |  %-6d |\n",players[i].name,players[i].score);
	}
	printf("\t+------------------------------------+---------+\n");
//	printf("");
	getchar();
}
void saveData(){
	char name[50];
	int i,invalid=1;
	while(invalid){
		int cnt = 0;
		printf("\n\t\tENTER NAME : ");
		scanf("%[^\n]",name); getchar();
		for(i=0;i<strlen(name);i++){		
			if(is_white_space(name[i])==0) cnt++;
		}
		if(cnt==strlen(name)&&strlen(name)>1) break;
		else {
//			printf("strlen : %d",strlen(name));
			printf("\t\tINVALID NAME!\n\t\t [Enter] to continue!"); getchar();
			strcpy(name,"");
		}
	}
	createPlayer(name,point);
	
	FILE* fp = fopen("save.txt","w");
//	int i;
	char data[100]="";
	char allData[10240]="";
	fputs(allData,fp);
	for(i = 0;i<playerIdx;i++){
		sprintf(data,"%s#%d\n",players[i].name,players[i].score);
		strcat(allData,data);
	}
	fputs(allData,fp);
	fclose(fp);
	printf("\t\t SUCCESS SAVE!\n\n\n\t\t PRESS [ENTER] TO CONTINUE!");
	getchar();
}
void lose(){
	system("cls");
	
	printf("\n\t\t\t\tYOU LOST!\n");
printf("\t\t________¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶_________\n");
printf("\t\t______¶¶¶¶¶¶_____________¶¶¶¶¶¶_______\n");
printf("\t\t_____¶¶¶¶¶_________________¶¶¶¶¶¶_____\n");
printf("\t\t____¶¶¶¶_____________________¶¶¶¶¶____\n");
printf("\t\t___¶¶¶¶_______________________¶¶¶¶¶___\n");
printf("\t\t__¶¶¶¶_____¶¶¶¶_______¶¶¶¶______¶¶¶___\n");
printf("\t\t__¶¶¶_____¶¶¶¶¶¶_____¶¶¶¶¶¶_____¶¶¶¶__\n");
printf("\t\t_¶¶¶¶_____¶¶¶¶¶¶_____¶¶¶¶¶¶______¶¶¶__\n");
printf("\t\t_¶¶¶_______¶¶¶¶_______¶¶¶¶_______¶¶¶¶_\n");
printf("\t\t_¶¶¶______________________________¶¶¶_\n");
printf("\t\t_¶¶¶______________________________¶¶¶_\n");
printf("\t\t_¶¶¶______________________________¶¶¶_\n");
printf("\t\t_¶¶¶____________¶¶¶¶¶____________¶¶¶¶_\n");
printf("\t\t_¶¶¶¶________¶¶¶¶¶¶¶¶¶¶¶_________¶¶¶__\n");
printf("\t\t__¶¶¶______¶¶¶¶¶_____¶¶¶¶¶______¶¶¶¶__\n");
printf("\t\t__¶¶¶¶____¶¶¶___________¶¶¶____¶¶¶¶___\n");
printf("\t\t___¶¶¶¶___¶¶_____________¶¶___¶¶¶¶____\n");
printf("\t\t____¶¶¶¶____________________¶¶¶¶¶_____\n");
printf("\t\t_____¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶______\n");
printf("\t\t_______¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶________\n");
	saveData();
	point = 0;
	level = 1;
	posX=1;
	posY=1;
	exit(0);
}
void createDoor(){
	srand(time(0));
	int x,y;
	while(1){
		x = (rand()%32)+1;
		y = (rand()%16)+1;
		if(map[y][x]=='H') break;
	}
	posXDoor = x;
	posYDoor = y;
	doorCreated=1;
}
void addPoint(int code){
	int p=0;
	if(code == WALL) p = level*50;
	else if (code == ENEMY) p = level*100;
	point+=p;
//	printf("P : %d POINT : %d",p,point);
//	getchar();
}
void detonateEnemy(int x,int y){
	if(map[y][x]!='X') return;
	else{
		int i;
		for(i = 0;i<enemyCount;i++){
			if (enemies[i].posX==x&&enemies[i].posY==y){
				enemies[i].posX=0;
				enemies[i].posY=0;
				enemies[i].status=INACTIVE;
				map[y][x]=' ';
			}
		}
		addPoint(ENEMY);
	}
}
void checkBomb(LARGE_INTEGER curr){
	int i,j,k;
	LARGE_INTEGER count,ms;
	for(i = 0;i<bombIdx;i++){
		if(bombs[i].QuadPart!=0){
			count.QuadPart = curr.QuadPart-bombs[i].QuadPart;
			ms.QuadPart = ((1000*count.QuadPart)/frequency.QuadPart);
			if(ms.QuadPart>=2000){
//				int ul = 0,uc=0,ur=0,ml=0,mr=0,bl=0,bc=0,br=0;
				bombs[i].QuadPart=0;
				int y = bombLoc[i][0];
				int x = bombLoc[i][1];
				int valid = 0;
				map[y][x]=' ';
				
				//detonate
				if(y-1>=1){
					if(x-1>=1 && map[y-1][x-1]!='#') {
						valid = validateMove(x-1,y-1);
						if(valid==3) lose();
						else if (valid==4) detonateEnemy(x-1,y-1);
						else if (valid==5) {
//							printf("UL\n");
							addPoint(WALL);
						}
						
						if(map[y-1][x-1]!='-'&&map[y-1][x-1]!='|') map[y-1][x-1]=' ';
						if(y-1==posYDoor&&x-1==posXDoor) map[y-1][x-1]='O';
					}
					if(x+1<=32 && map[y-1][x+1]!='#') {
						valid = validateMove(x+1,y-1);
						if(valid==3) lose();
						else if (valid==4) detonateEnemy(x+1,y-1);
						else if (valid==5) {
//							printf("UR\n");
							addPoint(WALL);
						}
						
						if(map[y-1][x+1]!='-'&&map[y-1][x+1]!='|') map[y-1][x+1]=' ';
						if(y-1==posYDoor&&x+1==posXDoor) map[y-1][x+1]='O';
					}
					if(map[y-1][x]!='#') {
						valid = validateMove(x,y-1);
						if(valid==3) lose();
						else if (valid==4) detonateEnemy(x,y-1);
						else if (valid==5) {
//							printf("UC\n");
							addPoint(WALL);
						}
						
						if(map[y-1][x]!='-'&&map[y-1][x]!='|') map[y-1][x]=' ';
						if(y-1==posYDoor&&x==posXDoor) map[y-1][x]='O';
					}
				}
				if(x-1>=1&&map[y][x-1]!='#') {
					valid = validateMove(x-1,y);
					if(valid==3) lose();
					else if (valid==4) detonateEnemy(x-1,y);
					else if (valid==5){
//							printf("ML\n");
							addPoint(WALL);
						}
					
					if(map[y][x-1]!='-'&&map[y][x-1]!='|') map[y][x-1]=' ';
					if(y==posYDoor&&x-1==posXDoor) map[y][x-1]='O';
				}
				if(x+1<=32&&map[y][x+1]!='#'){
					valid = validateMove(x+1,y);
					if(valid==3) lose();
					else if (valid==4) detonateEnemy(x+1,y);
					else if (valid==5) {
//							printf("MR\n");
							addPoint(WALL);
						}
					
					if(map[y][x+1]!='-'&&map[y][x+1]!='|') map[y][x+1]=' ';
					if(y==posYDoor&&x+1==posXDoor) map[y][x+1]='O';
				}
				if(y+1<=32){
					if(x-1>=1 && map[y+1][x-1]!='#') {
						valid = validateMove(x-1,y+1);
						if(valid==3) lose();
						else if (valid==4) detonateEnemy(x-1,y+1);
						else if (valid==5) {
							addPoint(WALL);
						}
						
						if(map[y+1][x-1]!='-'&&map[y+1][x-1]!='|') map[y+1][x-1]=' ';
						if(y+1==posYDoor&&x-1==posXDoor) map[y+1][x-1]='O';
					}
					if(x+1<=32 && map[y+1][x+1]!='#') {
						valid = validateMove(x+1,y+1);
						if(valid==3) lose();
						else if (valid==4) detonateEnemy(x+1,y+1);
						else if (valid==5) {
							addPoint(WALL);
						}
						
						if(map[y+1][x+1]!='-'&&map[y+1][x+1]!='|') map[y+1][x+1]=' ';
						if(y+1==posYDoor&&x+1==posXDoor) map[y+1][x+1]='O';
					}
					if(map[y+1][x]!='#'){
						valid = validateMove(x,y+1);
						if(valid==3) lose();
						else if (valid==4) detonateEnemy(x,y+1);
						else if (valid==5) {
							addPoint(WALL);
						}
						if(map[y+1][x]!='-'&&map[y+1][x]!='|') map[y+1][x]=' ';
						if(y+1==posYDoor&&x==posXDoor) map[y+1][x]='O';
					} 
				}
			}
		}
	}
}
void resetEnemy(){
	int i;
	for(i = 0;i<100;i++){
		enemies[i].posX=0;
		enemies[i].posY=0;
		enemies[i].status=INACTIVE;
	}
}
void generateEnemy(){
	if(played==0){
		resetEnemy();
		srand(time(0));
		int i;
		for(i=0;i<enemyCount;i++){
			int x,y;
			while(1){
				x = (rand()%32);
				y = (rand()%15);
				if(map[y][x]==' ') break;
			}
			enemies[i].posX=x;
			enemies[i].posY=y;
			enemies[i].status=ACTIVE;
			map[y][x]='X';
		}
	}
}
void enemyMove(LARGE_INTEGER curr){
	int i;
	srand(clock());
	for(i=0;i<enemyCount;i++){
		int move=0;
		int x = enemies[i].posX;
		int y = enemies[i].posY;	
		int noMove=0;
		if(enemies[i].status==ACTIVE){
			while(1){
				move = (rand()%4);
				int valid;
				if(move==UP){
					valid =validateMove(x,y-1);
					if(valid==3) lose();
					if(valid==6) noMove=1;
					if(valid==0) break;
				}else if(move==DOWN){
					valid =validateMove(x,y+1);
					if(valid==3) lose();
					if(valid==6) noMove=1;
					if(valid==0) break;
				}else if (move==LEFT){
					valid =validateMove(x-1,y);
					if(valid==3) lose();
					if(valid==6) noMove=1;
					if(valid==0) break;
				}else if (move==RIGHT){
					valid =validateMove(x+1,y);
					if(valid==3) lose();
					if(valid==6) noMove=1;
					if(valid==0) break;
				}
			}
			if(noMove==0){
				if(move==UP) y--;
				else if (move==DOWN) y++;
				else if (move==LEFT) x--;
				else if (move==RIGHT) x++;
				map[enemies[i].posY][enemies[i].posX]=' ';
				enemies[i].posY=y;
				enemies[i].posX=x;
				map[enemies[i].posY][enemies[i].posX]='X';
			}
		}
//		if(validateDead(x,y)==0){
//			lose();
//		}
//		printf("POS Y : %d POS X : %d\n",y,x);
	}
}
void nextLevel(){
	int i,j;
	played=0;
	level++;
	enemyCount++;
	openFile();
	doorCreated=0;
	generateWall();
	generateEnemy();
	posX=1;
	posY=1;
}

int main(){
	float f;
	if(startGame.QuadPart==0) {
		printBomber();
		printf("\n\t\tENTER DESIRED FPS : ");
		scanf("%f",&f); getchar();
		target/=f;
		QueryPerformanceCounter(&startGame);
	}
	if(openFile()==0) return 0;
	LARGE_INTEGER start_counter, end_counter, current_counter;
	QueryPerformanceCounter(&start_counter);
	QueryPerformanceFrequency(&frequency);
	char ch;
	int menu=-1;
	generateWall();
	generateEnemy();
	played=1;
	float frames = 0;
	while(1){
		while (1) {
			if(doorCreated==0){
				createDoor();
//				printf("DOOR X : %d Y : %d",posXDoor,posYDoor);
//				getchar();
			}
			QueryPerformanceCounter(&current_counter);
			checkBomb(current_counter);
			map[posY][posX]='@';
			system("cls");
			printMap();
	        if ( kbhit() ) {
	            ch = getch(); 
	            if ((int)(ch) == 27){
	                break;
				}
	            else if(ch=='w'){
	            	if(validateMove(posX,posY-1)==0){
		            	map[posY][posX]=' ';
		        		posY--;    	
		        		lastKey=ch;
					} else if (validateMove(posX,posY-1)==2){
						nextLevel();
					}
				} else if (ch=='s'){
					if(validateMove(posX,posY+1)==0){
		            	map[posY][posX]=' ';
						posY++;	
		        		lastKey=ch;
					} else if (validateMove(posX,posY+1)==2){
						nextLevel();
					}
				} else if (ch=='a'){
	            	if(validateMove(posX-1,posY)==0){
		            	map[posY][posX]=' ';
						posX--;	
		        		lastKey=ch;
					} else if (validateMove(posX-1,posY)==2){
						nextLevel();
					}
				} else if (ch=='d'){
	            	if(validateMove(posX+1,posY)==0){
		            	map[posY][posX]=' ';
						posX++;	
		        		lastKey=ch;
					} else if (validateMove(posX+1,posY)==2){
						nextLevel();
					}
				} else if((int)ch==32){
					putBomb(current_counter);
				}
	        }
	        QueryPerformanceCounter(&end_counter);
	        float spf = getSecondsperFrame(start_counter,end_counter);
	        if(spf<target){
				DWORD sleep_ms;
				sleep_ms = (DWORD)(1000*(target-spf)) ;
				Sleep(sleep_ms);
				while(spf<target){
					QueryPerformanceCounter(&end_counter);
					spf = getSecondsperFrame(start_counter,end_counter);
				}
			}
			frames++;
			if((int)frames%(int)f==0.0f) {
				enemyMove(current_counter);
				frames=0;
			}
			QueryPerformanceCounter(&end_counter);
			spf = getSecondsperFrame(start_counter,end_counter);
	        start_counter = end_counter;
		}
	    while(menu<1||menu>3){
			system("cls");
			printBomber();
		    printf("\n\t\t1. Back\n\t\t2. View Highscores\n\t\t3. Exit\n\t\t >> ");
		    scanf("%d",&menu); getchar();
		}
		if(menu==1)	{
			menu=-1;
			continue;
		}
		else if (menu==2) {
			menu=-1;
			showHighscore();
		}
		else if (menu==3) {
			saveData();
			return 0;
		}
	}
	
	return 0;
}
