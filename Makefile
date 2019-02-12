mkernel:
ifdef DEBUG
	${CC} ${DEBUG} -o mkernel mkernel.c
else
	${CC} -o mkernel mkernel.c
endif

.PHONY: mkernel
