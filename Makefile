all: scl2trd

scl2trd: scl2trd.c
	zcc +zx -vn -startup=30 -clib=new scl2trd.c -SO3 -o scl2trd -subtype=dot -create-app

clean:
	rm *.bin *.def SCL2TRD