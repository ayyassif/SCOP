run:
	g++ *.cpp glad/src/glad.c -Iglad/include -o scop -lglfw -lGL -ldl
	./scop
	