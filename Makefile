run:
	g++ main.cpp glad/src/glad.c -Iglad/include -o app -lglfw -lGL -ldl
	./app