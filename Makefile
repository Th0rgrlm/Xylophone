midi2mxy.exe: ./src/pc/main.c
	gcc -g ./src/pc/**.c -o midi2mxy.exe -L ./src/pc

clean:
	rm -rf midi2mxy.exe