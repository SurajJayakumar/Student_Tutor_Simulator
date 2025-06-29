![Screenshot_20250618_203356_Samsung Notes](https://github.com/user-attachments/assets/5e62da3a-d612-43d8-9ab2-85e4c65be3d0)
# Problem Statement


Seeking Tutor Problem
The computer science department runs a mentoring center (csmc) to help undergraduate students with their programming assignments. The lab has a coordinator and several tutors to assist the students. The waiting area of the center has several chairs. Initially, all the chairs are empty. The coordinator is waiting for the students to arrive. The tutors are either waiting for the coordinator to notify them that there are students waiting or they are busy tutoring. The tutoring area is separate from the waiting area.



A student while programming for their project, decides to go to csmc to get help from a tutor. After arriving at the center, the student sits in an empty chair in the waiting area and waits to be called for tutoring. If no chairs are available, the student will go back to programming and come back to the center later. Once a student arrives, the coordinator queues the student based on the student’s priority (details on the priority discussed below), and then the coordinator notifies an idle tutor. A tutor, once woken up, finds the student with the highest priority and begins tutoring. A tutor after helping a student, waits for the next student. A student, after receiving help from a tutor goes back to programming.



The priority of a student is based on the number of times the student has taken help from a tutor. A student visiting the center for the first time gets the highest priority. In general, a student visiting to take help for the ith time has a priority higher than the priority of the
student visiting to take help for the kth time for any k > i. If two students have the same priority, then the student who came first has a higher priority. Using POSIX threads, mutex locks, and semaphores implement a solution that synchronizes the activities of the coordinator, tutors, and the students.



Once a student thread takes the required number of helps from the tutors, it should terminate. Once all the student threads are terminated, the tutor threads, the coordinator thread, and the main program should be terminated.
Your program should work for any number of students, tutors, chairs and help sought.



Output

Your program must output the following at appropriate times.
Output of a student thread (x and y are ids):
S: Student x takes a seat. Empty chairs = <# of empty chairs after student x took a seat>. S: Student x found no empty chair. Will try again later.
S: Student x received help from Tutor y.
Output of the coordinator thread (x is the id, and p is the priority):
C: Student x with priority p added to the queue. Waiting students now = <# students waiting>. Total requests = <total # requests (notifications sent) by students for tutoring so far>
Output of a tutor thread after tutoring a student (x and y are ids):
T: Student x tutored by Tutor y. Students tutored now = <# students receiving help now>. Total sessions tutored = <total no of tutoring sessions completed so far by all the tutors>



# How To Run?

1. Compile the code
2. run the executable with arguments (students, tutors, chairs, number of helps each student needs). For example: csmc 10 3 4 5
