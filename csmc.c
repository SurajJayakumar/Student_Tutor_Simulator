//This code was written by Suraj Jayakumar- sxj240002 and Longyin Lin- lxl190019

#include<stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include<assert.h>
#include<unistd.h>

//inputs 

int chair_count;
int student_count;
int tutor_count;
int help_count;
#define MAX_NO_OF_STUDENTS 2000
#define MAX_NO_OF_TUTORS 100

//shared data structures

int request_number_list[MAX_NO_OF_STUDENTS];/*stores the global request number (1 for the first student who takes a seat, 
2 for the next student, and so on) of each student when they are seated*/
int student_helps_list[MAX_NO_OF_STUDENTS];//stores the number of times each student has been helped by the tutor 
int students_priority_queue[MAX_NO_OF_STUDENTS][2];/*stores the request number in the first index and number of 
helps in the second index for every seated student */
int tutor_table[MAX_NO_OF_STUDENTS];//stores the tutor ID of the tutor currently teaching each student


//other global variables

int total_requests=0;//stores the total number of tutor requests from students so far
int total_chairs=0;
int currently_tutoring=0;
int total_sessions=0;
int completed_students=0;//number of students who received all the help they need

//semaphores

sem_t student_waiting;
sem_t tutor_waiting;
sem_t tutored_student[MAX_NO_OF_STUDENTS];


//locks

//lock to atomically update number of chairs available
pthread_mutex_t update_chairs;
//lock so that one tutor helps one student at a time
pthread_mutex_t tutor_lock;
//lock when mapping student to tutor(filling tutor table)
pthread_mutex_t tutor_to_student_lock;
//lock to update the number of completed students
pthread_mutex_t completion_lock;





void *student(void *studentID){
    int sID=*(int*) studentID;
    
    while(1){
        
        if(student_helps_list[sID-1]>=help_count){
            pthread_mutex_lock(&completion_lock);
            completed_students++;//increment the number of completed students
            pthread_mutex_unlock(&completion_lock);
            sem_post(&student_waiting);//signal to the coordinator to exit in case all the students have exited
            pthread_exit(NULL);
        }
        
        pthread_mutex_lock(&update_chairs);
        if(chair_count>0){  
            chair_count--;
            total_requests++;
            request_number_list[sID-1]=total_requests;
            printf("S: Student %d takes a seat. Empty chairs = %d \n",sID,chair_count);
            pthread_mutex_unlock(&update_chairs);
            //signal to the coordinator that the student is waiting
            sem_post(&student_waiting);
            
            sem_wait(&tutored_student[sID-1]);
            int tID=tutor_table[sID-1];
            printf("S: Student %d received help from Tutor %d.\n",sID,tID);
            
            pthread_mutex_lock(&update_chairs);
            student_helps_list[sID-1]++;//increasing number of helps this particular student has taken
            pthread_mutex_unlock(&update_chairs);
            
            pthread_mutex_lock(&tutor_to_student_lock);
            tutor_table[sID-1]=-1;//reset the value of student's tutor to -1
            pthread_mutex_unlock(&tutor_to_student_lock);
            
            
        }
        else{
            printf("S: Student %d found no empty chair. Will try again later.\n",sID);
            pthread_mutex_unlock(&update_chairs);
            continue;
        }
        float student_sleep=(float)(rand()%2000)/1000;//student goes back to programming for some time up to 2ms
        usleep(student_sleep);
    }
    
}

void *coordinator(){
    while(1){
        if(completed_students==student_count){
            //if students are done and have exited, the tutors are closed one by one
            int tut;
            for(tut=0;tut<tutor_count;tut++){
                sem_post(&tutor_waiting);
            }
            pthread_exit(NULL);
        }
        sem_wait(&student_waiting);
        pthread_mutex_lock(&update_chairs);
        //time to enqueue all the seated students
        int student;
        for(student=0;student<student_count;student++){
            if(request_number_list[student]!=-1){
                
                students_priority_queue[student][0]=request_number_list[student];/*first element of priority queue stores the request
                number of the student so that we can determine if a student came first before another student */
                
                students_priority_queue[student][1]=student_helps_list[student];/* second element of priority queue stores the number of times 
                the students seated have already been helped by a tutor so that the tutor can prioritize the least helped student */
                
                printf("C: Student %d with priority %d in the queue. Waiting students now = %d. Total requests = %d\n",student+1,student_helps_list[student],total_chairs-chair_count,total_requests);
                
                request_number_list[student]=-1;//clear the list of current student requests since it has been already fed into the priority queue
            }
        }
        pthread_mutex_unlock(&update_chairs);
        sem_post(&tutor_waiting);
        
        
        
    }
}

void *tutor(void *tutorID){
    int tID=*(int*)tutorID;
    while(1){
        if(completed_students==student_count){
            pthread_exit(NULL);
        }
        
        sem_wait(&tutor_waiting);
        
        int least_helps_taken=help_count+1;/*the number of helps taken by students in the priority queue is compared with this to 
        find the students who took the least help from the queue.*/ 
        int earliest_arrival_time=student_count*help_count+1;/* The request times of the students are compared to this value to achieve
        the same result as above */
        int sid=-1;//the ID of the student tutor picks
        /*enqueueing the student with the highest priority from the priority queue by choosing the student with the least helps taken 
        and if there are multiple such students then the student who came first is chosen */
        pthread_mutex_lock(&update_chairs);
        int i;
        for(i=0;i<student_count;i++){
            if(students_priority_queue[i][0]>-1&&least_helps_taken>students_priority_queue[i][1]&&
            earliest_arrival_time>students_priority_queue[i][0]){
                
                least_helps_taken=students_priority_queue[i][1];
                earliest_arrival_time=students_priority_queue[i][0];
                sid=i+1;
                
            }
        }
        if(sid==-1) {
           
            pthread_mutex_unlock(&update_chairs);
            continue;
            
        }
        
        students_priority_queue[sid-1][0]=-1;
        students_priority_queue[sid-1][1]=-1;//remove the student chosen from the priority queue by resetting to -1
        chair_count++;
        currently_tutoring++;
        pthread_mutex_unlock(&update_chairs);
        
        float tutoring_time=(float)2/10;
        usleep(tutoring_time);//sleeping for 0.2 ms for tutoring
        pthread_mutex_lock(&tutor_lock);//ensuring mutex when updating the following shared variables
        currently_tutoring--;//done tutoring
        total_sessions++;
        printf("T: Student %d tutored by Tutor %d. Students tutored now = %d. Total sessions tutored = %d\n",sid,tID,currently_tutoring,total_sessions);
        pthread_mutex_unlock(&tutor_lock);
        pthread_mutex_lock(&tutor_to_student_lock);
        tutor_table[sid-1]=tID;
        pthread_mutex_unlock(&tutor_to_student_lock);
        sem_post(&tutored_student[sid-1]);
        
        
        
        
    }
}


int main(int argc, char *argv[])
{
    if(argc!=5){
        printf("Input {number of students} {number of tutors} {number of chairs} {number of helps}\n");
        exit(-1);
    }
    student_count=atoi(argv[1]);
    tutor_count=atoi(argv[2]);
    chair_count=atoi(argv[3]);
    help_count=atoi(argv[4]);
    if(tutor_count>MAX_NO_OF_TUTORS||student_count>MAX_NO_OF_STUDENTS||student_count<=0||tutor_count<=0){
        printf("Too many or you've entered zero or negative number of students or tutors. Maximum number of students is 2000 and maximum number of tutors is 100\n");
        exit(-1);
    }
    int i;
    //initializing semaphores
    sem_init(&student_waiting,0,0);
    sem_init(&tutor_waiting,0,0);
    for(i=0;i<student_count;i++){
        sem_init(&tutored_student[i],0,0);
    }
    
    //initializing locks
    pthread_mutex_init(&update_chairs,NULL);
    pthread_mutex_init(&tutor_lock,NULL);
    pthread_mutex_init(&tutor_to_student_lock,NULL);
    pthread_mutex_init(&completion_lock,NULL);
    
    //initializing threads
    pthread_t student_thread[student_count];
    pthread_t tutor_thread[tutor_count];
    pthread_t coordinator_thread;
    //initializing shared data structures
    for(i=0;i<student_count;i++){
        request_number_list[i]=-1;
        student_helps_list[i]=0;
        students_priority_queue[i][0]=-1;
        students_priority_queue[i][1]=-1;
        tutor_table[i]=-1;
    }
    total_chairs=chair_count;
    //creating threads and joining them
    assert(pthread_create(&coordinator_thread,NULL,coordinator,NULL)==0);
    int student_id[student_count];
    for(i = 0; i < student_count; i++){
        student_id[i] = i + 1;
        assert(pthread_create(&student_thread[i], NULL, student, (void*) &student_id[i])==0);
    }
    int tutor_id[tutor_count];
    for(i=0;i<tutor_count;i++){
        tutor_id[i]=i+1;
        
        assert(pthread_create(&tutor_thread[i],NULL,tutor,(void*) &tutor_id[i])==0);
    }
    pthread_join(coordinator_thread, NULL);
    for(i=0;i<student_count;i++){
        pthread_join(student_thread[i],NULL);
    }
    for(i=0;i<tutor_count;i++){
        pthread_join(tutor_thread[i],NULL);
    }
    
    return 0;
}