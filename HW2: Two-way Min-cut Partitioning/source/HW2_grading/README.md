# Homework 2 Grading Script
We will use this script to judge your program.
**Please make sure your program can be executed by this script.**

#
## Preparing
**Step1:**
Go into directory "student/" and generate a new directory with your student id.
```
$ cd student/
$ mkdir ${your_student_id}
$ cd ${your_student_id}/
```
e.g.:
```
$ cd student/
$ mkdir 110062501
$ cd 110062501/
```

**Step2:**
Put your compressed file in the directory which you just generate. The whole path is as follow: 
```
HW2_grading/student/${your_student_id}/CS6135_HW2_${your_student_id}.tar.gz
```
e.g.:
```
HW2_grading/student/110062501/CS6135_HW2_110062501.tar.gz
```
### Notice:
Do not put your original directory here because it will remove all directory before unzipping the compressed file.

#
## Working Flow
**Step1:**
Go into directory "HW2_grading/" and change the permission of "HW2_grading.sh".
```
$ chmod 744 HW2_grading.sh
```

**Step2:**
Run "HW2_grading.sh".
```
$ bash HW2_grading.sh
```

**Step3:**
Check your output.
* If status is "success", it means your program is finish in time and your output is correct. e.g.:
```
grading on 110062501:
  testcase |    cutsize |    runtime | status
      p2-1 |         20 |       0.01 | success
      p2-2 |        482 |       0.04 | success
      p2-3 |        605 |       1.62 | success
      p2-4 |      45220 |       2.76 | success
      p2-5 |     124677 |      21.19 | success
```
* If status is not "success", it means your program failed in this case. e.g.:
```
grading on 110062501:
  testcase |    cutsize |    runtime | status
      p2-1 |       fail |        TLE | p2-1 failed.
      p2-2 |       fail |        TLE | p2-2 failed.
      p2-3 |       fail |        TLE | p2-3 failed.
      p2-4 |       fail |        TLE | p2-4 failed.
      p2-5 |       fail |        TLE | p2-5 failed.
```
