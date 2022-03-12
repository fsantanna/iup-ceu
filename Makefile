export LD_LIBRARY_PATH=/usr/lib64/

all:
	ceu tst/$(SRC).ceu -cc "-g -liupimglib -liup -I include/"
	#./out.exe
	#./$(SRC)
