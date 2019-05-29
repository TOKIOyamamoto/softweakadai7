all:
	gcc -o wii_sample wii_sample.c -lm -lSDL2 -lSDL2_gfx -lcwiimote -D_ENABLE_TILT -D_ENABLE_FORCE -L/usr/lib -I/usr/local/include/libcwiimote
	gcc -o wii_kadai wii_kadai.c -lm -lSDL2 -lSDL2_gfx -lcwiimote -D_ENABLE_TILT -D_ENABLE_FORCE -L/usr/lib -I/usr/local/include/libcwiimote
