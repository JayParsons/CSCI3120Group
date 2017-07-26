# CSCI3120Group
To my teammate:
--<update>--
Error is reduced to minimum, minor error still occur, I no longer have faith to debug: the big picture is still on path, possible being thread issue.
------------

The project code part is 99% done, what left is commenting and the part I did not mentioned in the project menu, please notice me if you find something missing.

The program will run on mac and only mac, I will not guarantee anything on Linux or Windows. And on mac, there is a low probability on file name reading and pthread disorder, yet they will not affect the general result.

Thusly, from now on, we can go seperate ways such that will keep on do some commenting as my finish up, and you guys can do the output sampling. The linux version is cancelled as you suggested to work on one single version of OS. And check the following useage. And in addition, I added a file call generate_file, which is used for generate files from 1Kb to 1Mb, I would recommand you to first generate that many files and then try the read file part.

------server side------<job, port part vary>
make sws 
./sws 38080 SJF 64
------client side------
./hydra.py < test#.in
------generate file----
./generate_files.sh



Repo for CSCI 3120 group project

Git tutorial:

To checkout a branch:

git checkout name_of_branch

To create a new branch:

git checkout -b name_of_new_branch

To push changes:

git add name_of_file_or_directory

git commit 'commit message'

git push name_of_remote_repo name_of_branch

To pull:

git pull name_of_remote_repo name_of_branch
