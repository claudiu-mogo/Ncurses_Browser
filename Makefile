FLAG = -g -Wall -lm -std=c99
 
browser: browser.c
	gcc -o browser browser.c $(FLAG) -lncurses -lmenu

# Regulile de clean 
.PHONY : clean
clean :
	rm -f task1 task2 task3 task4
	rm -f *.out
