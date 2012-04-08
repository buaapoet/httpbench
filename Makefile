all: src 
src:
	sh -c 'cd ./src && make -w'
deb: src
	dpkg-deb --build debian
   
