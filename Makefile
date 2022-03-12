export LD_LIBRARY_PATH=/usr/lib64/

all:
	ceu $(SRC).ceu -cc "-g -liupimglib -liup -I include/"
	#./out.exe
	#./$(SRC)
