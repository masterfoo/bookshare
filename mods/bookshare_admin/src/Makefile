all: test_op

test_op: op.c
	gcc -s -o test_op op.c

push: all
	cp test_op ../operate
