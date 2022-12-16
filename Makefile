export LD_LIBRARY_PATH=/usr/lib64/

all:
	ceu $(SRC).ceu -cc "-g -liupimglib -liup -lm -I include/"
	#./out.exe
	#./$(SRC)

tst:
	make SRC=7guis/01_counter

.PHONY: tst
