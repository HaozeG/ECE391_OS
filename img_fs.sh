cp -RT ./syscalls/to_fsdir/ ./fsdir

./createfs -i ./fsdir/ -o ./student-distrib/filesys_img
