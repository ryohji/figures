test: figures.c
	$(CC) -o $@ $^ -lm -DUNIT_TEST
	./$@

.PHONY: clean
clean:
	$(RM) *~ *.o
