- Clone the `ceu-iup` repository into the root directory of `ceu`:

```
cd ceu/
git clone https://github.com/fsantanna/iup-ceu/ iup
cd iup/
```

- Download and install the `iup` libraries:

```
mkdir tmp/
cd tmp/
wget https://sourceforge.net/projects/iup/files/3.31/Linux%20Libraries/iup-3.31_Linux54_64_lib.tar.gz
tar xvzf iup-3.31_Linux54_64_lib.tar.gz
sudo ./install
sudo ./install_dev
cd ..
rm -Rf tmp/
```

- Compile and execute a test program:

<!-- export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64-->

```
gcc -o hello -I /usr/include/iup hello.c -l iup
./hello
```
