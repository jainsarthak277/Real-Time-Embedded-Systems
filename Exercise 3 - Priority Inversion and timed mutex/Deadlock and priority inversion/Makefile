all:deadlock deadlock_timeout deadlock_unsafe pthread3 pthread3ok

deadlock:deadlock.c
	gcc deadlock.c -o deadlock -lpthread

deadlock_timeout:deadlock_timeout.c
	gcc deadlock_timeout.c -o deadlock_timeout -lpthread

deadlock_unsafe:deadlock_unsafe.c
	gcc deadlock_unsafe.c -o deadlock_unsafe -lpthread

pthread3:pthread3.c
	gcc pthread3.c -o pthread3 -lpthread

pthread3ok:pthread3ok.c
	gcc pthread3ok.c -o pthread3ok -lpthread

clean:
	-rm -f deadlock deadlock_timeout deadlock_unsafe pthread3 pthread3ok
