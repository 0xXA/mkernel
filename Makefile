mkernel:
ifdef DEBUG
	${CC} ${DEBUG} -std=c11 -o mkernel mkernel.c
else
	${CC} -std=c11 -o mkernel mkernel.c
endif

.PHONY: mkernel
