```
cd /tmp/
mkdir iup/
cd iup/
wget https://sourceforge.net/projects/iup/files/3.31/Linux%20Libraries/iup-3.31_Linux54_64_lib.tar.gz
tar xvzf iup-3.31_Linux54_64_lib.tar.gz
sudo ./install
sudo ./install_dev
```

```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64
gcc -o hello -I /usr/include/iup hello.c -l iup
./hello
```
