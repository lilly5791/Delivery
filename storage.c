#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage.h"

/* 
  definition of storage cell structure ----
  members :
  int building : building number of the destination
  int room : room number of the destination
  int cnt : number of packages in the cell
  char passwd[] : password setting (4 characters)
  char *contents : package context (message string)
*/
typedef struct {
	int building;
	int room;
	int cnt;
	char passwd[PASSWD_LEN+1];
	
	char *context;
} storage_t;


static storage_t** deliverySystem; 			//deliverySystem
static int storedCnt = 0;					//number of cells occupied
static int systemSize[2] = {0, 0};  		//row/column of the delivery system
static char masterPassword[PASSWD_LEN+1];	//master password




// ------- inner functions ---------------

//print the inside context of a specific cell
//int x, int y : cell to print the context
static void printStorageInside(int x, int y) {
	printf("\n------------------------------------------------------------------------\n");
	printf("------------------------------------------------------------------------\n");
	if (deliverySystem[x][y].cnt > 0)
		printf("<<<<<<<<<<<<<<<<<<<<<<<< : %s >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", deliverySystem[x][y].context);
	else
		printf("<<<<<<<<<<<<<<<<<<<<<<<< empty >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		
	printf("------------------------------------------------------------------------\n");
	printf("------------------------------------------------------------------------\n\n");
}

//initialize the storage
//set all the member variable as an initial value
//and allocate memory to the context pointer
//int x, int y : cell coordinate to be initialized
static void initStorage(int x, int y) {

    	deliverySystem[x][y].building = 0;
    	deliverySystem[x][y].room = 0;
    	strcpy(deliverySystem[x][y].passwd, "pass"); // initialize passwd to "pass"
   		deliverySystem[x][y].cnt = 0;
   		deliverySystem[x][y].context = (char*)malloc(sizeof(char)*MAX_MSG_SIZE); // create context malloc
   		strcpy(deliverySystem[x][y].context, "context"); // initialize context to "context"
	
}

//get password input and check if it is correct for the cell (x,y)
//int x, int y : cell for password check
//return : 0 - password is matching, -1 - password is not matching
static int inputPasswd(int x, int y) {
	
	int index; // index is for comparing the password. if index = 0 then match, if index = -1 then wrong
	int masterindex; // masterindex is for comparing your input password and masterpassword
	char pass[PASSWD_LEN+1];
	
	printf("input password for (%d, %d) storage :", x, y);
	scanf("%s", &pass);
	
	index = strcmp(pass, deliverySystem[x][y].passwd);
	masterindex = strcmp(pass, masterPassword);
	
	
	if(index != 0) // if input password and real password is different
	{
		return masterindex; // compare with master password
	}
	else
	{
		return index; // when input password is right
	}
	
	return 0;
	
}





// ------- API function for main.c file ---------------

//backup the delivery system context to the file system
//char* filepath : filepath and name to write
//return : 0 - backup was successfully done, -1 - failed to backup
int str_backupSystem(char* filepath) {
	FILE *fp;
   int i,j;

   fp = fopen(filepath, "w");
   
   if(fp == NULL) // if failed to backup
   {
      return -1;
   }
   
   fprintf(fp, "%d %d\n", systemSize[0], systemSize[1]); // put row, column
   fprintf(fp, "%s\n", masterPassword); // put masterpassword
   
   for(i=0;i<systemSize[0]; i++)
   {
      for(j=0;j<systemSize[1]; j++)
      {
         if(deliverySystem[i][j].cnt > 0) // if deliverySystem[i][j] has a package
         {
            fprintf(fp, "%d %d %d %d %s ",i,j,deliverySystem[i][j].building, deliverySystem[i][j].room, deliverySystem[i][j].passwd);

            fprintf(fp, "%s\n",deliverySystem[i][j].context);
         }
         
      }
      
   }
   
   fclose(fp);
   
   return 0;
}



//create delivery system on the double pointer deliverySystem
//char* filepath : filepath and name to read config parameters (row, column, master password, past contexts of the delivery system
//return : 0 - successfully created, -1 - failed to create the system
int str_createSystem(char* filepath) {
	int i;
    int j;
    int x,y; // each row & column
    FILE *fp;
    
    //this makes deliverySystem dimension array (like delivery box)
    fp   = fopen(filepath, "r");
    fscanf( fp, "%d %d", &systemSize[0], &systemSize[1]); // accept table row, column
    
    
	deliverySystem = (storage_t**)malloc(sizeof(storage_t*)*systemSize[0]); // create row
    for(i=0;i<systemSize[0];i++)
    {
        deliverySystem[i] = (storage_t*)malloc(sizeof(storage_t)*systemSize[1]); // create column
    }
    
    for(i=0;i<systemSize[0];i++)
    {
    	for(j=0;j<systemSize[1];j++)
    	{
    		 initStorage(i, j); // initialize the delivery box before creating struct srorage_t
		}
	}
   
    
    //this sets masterPassword
    fscanf(fp, "%s", masterPassword);
    
    while(!feof(fp)) // if file is available
    {
        //this is about delivery information
        //ex: 0 0 3 103 1234 noPassword
        //from building number to passwd
        
        fscanf( fp, "%d %d", &x, &y);//row, column

        fscanf( fp, "%d %d %s", &deliverySystem[x][y].building, &deliverySystem[x][y].room, deliverySystem[x][y].passwd);//building, room, passwd

		deliverySystem[x][y].cnt = 1; // it means there is a package
        
        fscanf(fp,"%s",deliverySystem[x][y].context);

		storedCnt++;
        
    }
    
    if(fp == NULL) // failed to create due to failed to open file 
    {
    	return -1;
	}
    
    fclose(fp);


	return 0;
}

//free the memory of the deliverySystem 
void str_freeSystem(void) {
	int i, j;
	
	// free context in each storage
	for(i=0;i<systemSize[0];i++)
	{
		for(j=0;j<systemSize[1];j++)
		{
			free(deliverySystem[i][j].context); 
		}
	}
	
	
	// free all deliverySystem
	for(i=0;i<systemSize[0];i++) 
    {
        free(deliverySystem[i]); 
    }
    free(deliverySystem); 
	
}



//print the current state of the whole delivery system (which cells are occupied and the destination of the each occupied cells)
void str_printStorageStatus(void) {
	int i, j;
	printf("----------------------------- Delivery Storage System Status (%i occupied out of %i )-----------------------------\n\n", storedCnt, systemSize[0]*systemSize[1]);
	
	
	
	printf("\t");
	for (j=0;j<systemSize[1];j++)
	{
		printf(" %i\t\t",j); // show row
	}
	printf("\n-----------------------------------------------------------------------------------------------------------------\n");
	
	for (i=0;i<systemSize[0];i++)
	{
		printf("%i|\t",i); // show column
		for (j=0;j<systemSize[1];j++)
		{
			if (deliverySystem[i][j].cnt != 0) // if resident has some packagem then show 
			{
				printf("%i,%i\t|\t", deliverySystem[i][j].building, deliverySystem[i][j].room);
				
			}
			else // cnt == 0 means no package
			{
				printf(" -  \t|\t");
			}
		}
		printf("\n");
	}
	printf("--------------------------------------- Delivery Storage System Status --------------------------------------------\n\n");
}



//check if the input cell (x,y) is valid and whether it is occupied or not
int str_checkStorage(int x, int y) {
	if (x < 0 || x >= systemSize[0])
	{
		return -1;
	}
	
	if (y < 0 || y >= systemSize[1])
	{
		return -1;
	}
	
	return deliverySystem[x][y].cnt;	
}


//put a package (msg) to the cell
//input parameters
//int x, int y : coordinate of the cell to put the package
//int nBuilding, int nRoom : building and room numbers of the destination
//char msg[] : package context (message string)
//char passwd[] : password string (4 characters)
//return : 0 - successfully put the package, -1 - failed to put
int str_pushToStorage(int x, int y, int nBuilding, int nRoom, char msg[MAX_MSG_SIZE+1], char passwd[PASSWD_LEN+1]) {
	
	if(deliverySystem[x][y].cnt == 0) // when the storage is empty input user's informations in the storage
	{
		deliverySystem[x][y].building = nBuilding;
		deliverySystem[x][y].room = nRoom;
		deliverySystem[x][y].context = msg;
		for (i=0;i<sizeof(passwd[PASSWD_LEN+1]);i++)
		{
			deliverySystem[x][y].passwd[i] = passwd[i]; // put password
		}
		
		deliverySystem[x][y].cnt=1;
		storedCnt++;
	}
	else
	{
		return -1; //when the storage is not empty then failed to put
	}
	return 0;
}



//extract the package context with password checking
//after password checking, then put the msg string on the screen and re-initialize the storage
//int x, int y : coordinate of the cell to extract
//return : 0 - successfully extracted, -1 = failed to extract
int str_extractStorage(int x, int y) {
	
	char inputPasswd[PASSWD_LEN+1];
	
	printf("input password for (%d, %d) storage :", x, y);
	scanf("%s", inputPasswd);

	
//	inputPasswd(x, y)
	if(strcmp(deliverySystem[x][y].passwd, inputPasswd))
	{
		printf("extracting the storage (%d, %d)...\n", x, y);
		printStorageInside(x, y);
		storedCnt--;
		initStorage(x, y);
		return 0;
	}
	else
		return -1;

}

//find my package from the storage
//print all the cells (x,y) which has my package
//int nBuilding, int nRoom : my building/room numbers
//return : number of packages that the storage system has
int str_findStorage(int nBuilding, int nRoom) {
	
	int i, j;
	int check = 0;
	
	for (i=0;i<systemSize[0];i++)
	{
		for (j=0;j<systemSize[1];j++)
		{
			if(nBuilding == deliverySystem[i][j].building && nRoom == deliverySystem[i][j].room)
			{
				printf("-----------> Found a package in (%d, %d)\n", i, j);
				check++;
			}
		}
	}
	
	
	
	if(check == 0)
	{
		return 0;
	}
	
	return 1;
}
